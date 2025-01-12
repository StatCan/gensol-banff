#include "IOUtil.h"

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <math.h>  // NAN value

#include "STC_Memory.h"

#include "PlatformSupport.h"  // platform specific string functions (memccpy)
#include "RuntimeDiagnostics.h"

/* variable type descriptions
    These correspond to the `IO_VAR_TYPE` enumeration */
const char* var_type_desc[IOVT__COUNT] = {
    "unknown",
    "numeric",
    "character",
    "raw",
    "disabled"
};

/* VARLIST_DEFAULT_FLAGS:
    used by `DSR_default_var_list()` to categorize a DSR_generic's variables
    while determining a valid default variable list 
    NOTE: this enum not placed in the header file as it is for internal use only */
typedef enum __VARLIST_DEFAULT_FLAGS {
    VDF_EXCLUDE,
    VDF_NUMERIC,
    VDF_CHAR,
    VDF_EXCLUDE_USED,
    VDF_INCLUDE,
} VARLIST_DEFAULT_FLAGS;

/* ~~~~~ DATASET READER FUNCTIONS ~~~~~ */
/* DSR_compare_rec_by_group
    Compares BY variables in records "x" and "y" in from their respective datasets.  
        Assumes record X comes before record Y, when 
        sorted in ascending order over BY variable values. 
            - See points about DSRC_BY_NOT_SORTED below
        Requires dsr_x and dsr_y to have an identical BY variable setup
            - returns `DSRC_ERROR` if this isn't true

        Missing Data: logic in this function assumes "missing" (null) data should be 
        sorted to the last position
            - i.e. nothing comes after "missing", everything comes before "missing"

    PARAMETERS:
      dsr_x     - pointer to associated DSR_generic struct containing record "x"
      index_x   - index of record x in dsr_x
      dsr_y     - pointer to associated DSR_generic struct containing record "y"
      index_y   - index of record y in dsr_y

    RETURNS:
      DSRC_BY_MATCH 
        - if 0 BY variables in dsr
        - OR if each of the BY variables are identical in value and datatype
      DSRC_DIFFERENT_GROUPS
        - the value of at least one of y's BY variables is greater than
          x's value for the same variable

    ERRORS:
      DSRC_INVALID_BY_TYPES
        - the datatype of any of the BY variables is not compatible with this function
      DSRC_BY_NOT_SORTED
        - one of Y's BY variable values, when sorted in ascending order, comes before X's value
        - this indicates that X and Y are in different groups AND aren't sorted in ascending order
      DSRC_ERROR
        - any unhandled error

    OPTIMIZATION:
        This function uses some Apache Arrow functions directly (i.e. not via some IO_Arrow function).  
        This is done to ensure maximum optimization of the function, such as by re

    BEWARE:
        This function should not be called on a dataset which has 0 rows of data.  
        Callers must validate that the datasets both have data prior to calling this function
*/
IO_DATASET_RC DSR_compare_rec_by_group(DSR_generic* dsr_x, int index_x, DSR_generic* dsr_y, int index_y) {
    int by_var_count_x = dsr_x->VL_by_var.count;
    int by_var_count_y = dsr_y->VL_by_var.count;
    if (by_var_count_x != by_var_count_y) {
        return DSRC_ERROR;
    }
    else if (by_var_count_x == 0) {
        return DSRC_BY_MATCH;
    }

    int* by_positions_x = dsr_x->VL_by_var.positions;
    int* by_positions_y = dsr_y->VL_by_var.positions;

    // get array chunk for each dsr
    array_chunk* ac_x = ADI_get_chunk(&dsr_x->ad, index_x);
    T_ds_size i_x = index_x - ac_x->index_offset;
    array_chunk* ac_y = ADI_get_chunk(&dsr_y->ad, index_y);
    T_ds_size i_y = index_y - ac_y->index_offset;
    if (ac_x == NULL || ac_y == NULL) {
        return DSRC_ERROR;
    }

    // compare BY groups
    for (int i = 0; i < by_var_count_x; i++) {
        // get array views and type for this BY variable
        struct ArrowArrayView* av_x = ac_x->view.children[by_positions_x[i]];
        struct ArrowArrayView* av_y = ac_y->view.children[by_positions_y[i]];
        enum ArrowType type_x = av_x->storage_type;
        enum ArrowType type_y = av_y->storage_type;
        int8_t miss_x = ArrowArrayViewIsNull(av_x, i_x);
        int8_t miss_y = ArrowArrayViewIsNull(av_y, i_y);

        if (miss_x && miss_y) {
            // both are missing: they match, continue to next
            continue;
        }
        else if (!miss_x && miss_y) {
            // "missing" (null) data should be sorted last
            // only y group "missing": different groups, correct order
            return DSRC_DIFFERENT_GROUPS;
        }
        else if (miss_x && !miss_y) {
            // "missing" data should come last
            // only x group "missing": incorrect sort order
            return DSRC_BY_NOT_SORTED;
        }
        else if (arrow_is_string(type_x) && arrow_is_string(type_y)) {
            /* BOTH STRING */
            // get each string
            struct ArrowStringView asv_x = ArrowArrayViewGetStringUnsafe(av_x, i_x);
            struct ArrowStringView asv_y = ArrowArrayViewGetStringUnsafe(av_y, i_y);

            /* incase one string is longer, only compare to shortest string's length */
            T_ds_size min_size = asv_x.size_bytes > asv_y.size_bytes ? asv_y.size_bytes : asv_x.size_bytes;
            int rv_strcmp = strncmp(asv_x.data, asv_y.data, min_size);
            if (rv_strcmp > 0) {
                return DSRC_BY_NOT_SORTED;
            }
            else if (rv_strcmp < 0) {
                return DSRC_DIFFERENT_GROUPS;
            }
            // shortest string comparison passed


            if (asv_x.size_bytes == asv_y.size_bytes) {
                // if they're equal content and size, they match
                continue;
            }
            else if (asv_x.size_bytes > asv_y.size_bytes) {
                // *nothing* comes before *something*, so x should be shorter if sorted properly
                return DSRC_BY_NOT_SORTED;
            }
            else {
                // x is shorter but matches y otherwise
                return DSRC_DIFFERENT_GROUPS;
            }
        }
        else if(arrow_is_numeric(type_x) && arrow_is_numeric(type_y)){
            /* both are numeric, so get double value for each and compare
                We treat all numeric types as equivalent, so all are casted to double
                WARNING: it may be possible that `int` and `unsigned int` are not subsets of `double` */
            double v_x, v_y;

            ARROW_RETURN_CODE arc = av_get_double_safe(av_x, i_x, &v_x);
            if (arc == ARC_INVALID_TYPE) {
                return DSRC_INVALID_BY_TYPES;
            }
            else if (arc != ARC_SUCCESS) {
                return DSRC_ERROR;
            }

            arc = av_get_double_safe(av_y, i_y, &v_y);
            if (arc == ARC_INVALID_TYPE) {
                return DSRC_INVALID_BY_TYPES;
            }
            else if (arc != ARC_SUCCESS) {
                return DSRC_ERROR;
            }

            double rv_by_compare = v_x - v_y;
            if (rv_by_compare > 0) {
                return DSRC_BY_NOT_SORTED;
            }
            else if (rv_by_compare < 0) {
                return DSRC_DIFFERENT_GROUPS;
            }
        }
        else {
            return DSRC_INVALID_BY_TYPES;
        }
    }

    /* if we arrived here, BY groups must be matching */
    return DSRC_BY_MATCH;
}

/* DSR_copy_reallocate
    Purpose: Call this on any `DSR_generic` which has been copied. 
      When a single dataset contains, for example, current and historical data, it 
      may be copied to emulate the provision of 2 distinct datasets.  
        i.e. `DSR_generic a, b; DSR_init(a,...); b=a;`
      After copying a `DSR_generic`, run this function on the copy (`b`)
      to rename the dataset and reallocate some dynamically allocated members.  

      When a DSR is copied, pointers to dynamically allocated memory are copied too.  
      In most cases this is acceptable (example: column names), 
      however sharing pointers to values which are modified during 
      processing (example: `DS_varlist` pointers) will cause errors.  
      This operation prevents such errors by "reallocating" new pointers.  

      It calls `VL_copy_reallocate()` to handle the same situation 
      regarding some `DS_varlist` pointer members.  
    
    `DSR_generic.is_a_copy` set to TRUE
      informs `DSR_free()` to only free "reallocated" members.  */
void DSR_copy_reallocate(DSR_generic* dsr_dest, char* name_dst) {
    dsr_dest->is_a_copy = 1 == 1;
    dsr_dest->dataset_name = name_dst;

    VL_copy_reallocate(&dsr_dest->VL_by_var,   dsr_dest);
    VL_copy_reallocate(&dsr_dest->VL_in_var,   dsr_dest);
    VL_copy_reallocate(&dsr_dest->VL_unit_id,  dsr_dest);
}

/*DSR_cursor_next_by
    Initialize or advance by group of a `DSR_generic`.  

    PARAMETERS:
      dsr       - pointer to DSR_generic struct

    RETURNS:
      DSRC_NEXT_BY_SUCCESS
        - if initializing call
            - set to first record in dataset
            - is valid to call repeatedly if cur_record_index is reset repeatedly
              such as by `DSR_cursor_rewind()`
        - if BY group successfully advanced
            - sets dsr's record cursor to that BY group
      DSRC_NO_MORE_REC_IN_DS
        - current record is in last BY group in dataset

    ERRORS:
      DSRC_BY_NOT_SORTED
        - next BY group not in expected (ascending) order
        - NOTE: this isn't always considered an error, it's up to the caller
      <error codes from>
        - DSR_compare_rec_by_group()

    NOTES:
      If no by groups
        - all data treated as single BY group

    PERFORMANCE CONSIDERATIONS: could be improved
        - When advancing BY groups, this function may perform a linear search of the data
          comparing all BY variables of each record in the range 
            (current record, first record in next BY group]
          In some cases where significant amounts of excess data is present, this may be expensive
          and should probably optimize for cases with no BY variables, by returning `DSRC_NO_MORE_REC_IN_DS`
          instead of searching for the next BY group.  
*/
IO_DATASET_RC DSR_cursor_next_by(DSR_generic* dsr) {
    // optimize: if no more records in the dataset
    if ((T_ds_size)dsr->rec_cursor.cur_record_index + 1 == dsr->num_records
        || dsr->num_records == 0) {
        return DSRC_NO_MORE_REC_IN_DS;
    }

    if ((T_ds_size)dsr->rec_cursor.cur_record_index + 1 > dsr->num_records){
        // arriving here likely indicates a bug in the code
        return DSRC_ERROR;
    }

    // is this the initializing call?
    if (dsr->rec_cursor.cur_record_index == IOSV_UNSET) {
        IO_DATASET_RC rc_dsr = DSR_cursor_set_by(dsr, 0);
        if (rc_dsr != DSRC_SUCCESS) {
            return rc_dsr;
        }
    } 
    else { //FIND NEXT BY GROUP
        int cur_rec_i = dsr->rec_cursor.cur_record_index;

        // if current record comes before current BY group, use first record in current BY group
        if (cur_rec_i < dsr->rec_cursor.first_cur_by_index) {
            cur_rec_i = dsr->rec_cursor.first_cur_by_index;
        }

        IO_DATASET_RC rc_compare_by = DSRC_ERROR;

        // search for next BY group
        while ((T_ds_size)cur_rec_i + 1 < dsr->num_records // are there still more records?
            && (rc_compare_by = DSR_compare_rec_by_group(dsr, cur_rec_i, dsr, cur_rec_i + 1)) == DSRC_BY_MATCH  // next rec in same BY group?
        ) {
            cur_rec_i++;
        }

        // assess results
        if ((T_ds_size)cur_rec_i + 1 == dsr->num_records) {// NO MORE BY GROUPS
            return DSRC_NO_MORE_REC_IN_DS;
        }
        else if (rc_compare_by == DSRC_DIFFERENT_GROUPS) {// ADVANCE TO NEXT BY GROUP 
            IO_DATASET_RC rc_dsr = DSR_cursor_set_by(dsr, cur_rec_i + 1);
            if (rc_dsr != DSRC_SUCCESS) {
                return rc_dsr;
            }
        }
        else if (rc_compare_by == DSRC_BY_NOT_SORTED) {// NOT SORTED
            IOM_print_data_not_sorted(dsr, cur_rec_i);
            return rc_compare_by;
        }
        else {// some other error code from DSR_compare_rec_by_group()
            return rc_compare_by;
        }
    }

    return DSRC_NEXT_BY_SUCCESS;
}

/*DSR_cursor_next_rec
    Advance to the next record in the current BY group, if exists.

    PARAMETERS:
      dsr       - pointer to DSR_generic struct

    RETURNS:
      DSRC_NEXT_REC_SUCCESS
        - advanced record cursor to next record in current BY group
            - if initial call for current BY group, set to first record in group
      DSRC_NO_MORE_REC_IN_BY
        - current record is the last record in current BY group
        - OR current record is last record in dataset (implying the above point)

    ERRORS:
      <error codes from>
        - DSR_compare_rec_by_group()
            - EXCEPT: DSRC_BY_NOT_SORTED
                - return DSRC_NO_MORE_REC_IN_BY
                - in this context, we don't care about sort order, just group membership
*/
IO_DATASET_RC DSR_cursor_next_rec(DSR_generic* dsr) {
    // if no more records in the dataset => no more records in current BY group
    if (dsr->rec_cursor.cur_record_index + 1 == dsr->num_records
        || dsr->num_records == 0) {
        return DSRC_NO_MORE_REC_IN_BY;
    }

    // is this the initializing call for the current BY group?
    if (dsr->rec_cursor.cur_record_index < dsr->rec_cursor.first_cur_by_index){
        dsr->rec_cursor.cur_record_index = dsr->rec_cursor.first_cur_by_index;
    } 
    else {// current record valid AND more records exist => compare BY group of next record
        IO_DATASET_RC rc_compare_by = DSR_compare_rec_by_group(
            dsr,
            dsr->rec_cursor.cur_record_index,
            dsr,
            dsr->rec_cursor.cur_record_index + 1
        );

        if (rc_compare_by == DSRC_BY_MATCH) {  //next rec in same BY group: advance
            dsr->rec_cursor.cur_record_index++;
        }
        else if (rc_compare_by == DSRC_DIFFERENT_GROUPS){
            return DSRC_NO_MORE_REC_IN_BY;
        }
        else if (rc_compare_by == DSRC_BY_NOT_SORTED) {  // next rec in different BY group, wrong sort order
            IOM_print_data_not_sorted(dsr, dsr->rec_cursor.cur_record_index);
            return rc_compare_by;
        }
        else {// if not handled yet, the RC must be an error code
            return rc_compare_by;
        }
    }

    /* OPTIMIZATION: could cache the corresponding dsr.ad.ac_list member */
    return DSRC_NEXT_REC_SUCCESS;
}

/* DSR_cursor_print_by_message
    Prints a message followed by the name and value of each BY variable.
    Intended to identify (in the log) the start or end of each group.  

    Returns immediately if dsr has no BY groups */
void DSR_cursor_print_by_message(DSR_generic* dsr, const char* msg_prefix, int num_newlines) {
    if (dsr->VL_by_var.count > 0) {
        // print `msg_prefix` followed by BY variable name-value pairs, one per line
        IOM_print_rec_by_group(dsr, dsr->rec_cursor.cur_record_index, msg_prefix , "", "\n      ", num_newlines);
    }
}

/* DSR_cursor_rewind
    set BY group cursor to beginning of
      dataset    - when `action_info` is `DCA_rewind_dataset`
      BY group   - when `action_info` is `DCA_rewind_by_group`*/
IO_DATASET_RC DSR_cursor_rewind(DSR_generic* dsr, IO_CURSOR_ACTION cursor_action) {
    if(cursor_action == IOCA_REWIND_DATASET){
        return DSR_cursor_set_by(dsr, 0);
    }
    else if (cursor_action == IOCA_REWIND_BY_GROUP) {
        if (dsr->VL_by_var.count > 0) {
            return DSR_cursor_set_by(dsr, dsr->rec_cursor.first_cur_by_index);
        }
        else {
            return DSR_cursor_set_by(dsr, 0);
        }
    }
    else {
        return DSRC_ERROR;
    }
}

/* DSR_cursor_set_by
sets value start for the current by group to by_start_index
sets values if by_var_cur_val using record[by_start_index]
    This must be called even if no "BY groups" are provided
    - consider this case a single by group*/
IO_DATASET_RC DSR_cursor_set_by(DSR_generic* dsr, int by_start_index) {
    if (by_start_index >= dsr->num_records) {
        return DSRC_INVALID_INDEX;
    }

    /* SET RECORD CURSOR */
    dsr->rec_cursor.first_cur_by_index = by_start_index;
    dsr->rec_cursor.cur_record_index = by_start_index - 1; //indicates no record from the new BY group has been read yet

    return DSRC_SUCCESS;
}

/* DSR_cursor_sync_by
    Set the cursor of dsr_secondary to the first record in the BY group which 
    matches the current BY group in dsr_primary.  

    HOW dsr_secondary IS SEARCHED
      - current BY groups are compared, if match, secondary is `DSR_cursor_rewind()` to the beginning
      - if secondary ahead of primary there is no matching group, return `DSRC_DIFFERENT_GROUPS`
      - if secondary behind primary
        - advance secondary until its group matches or is ahead of primary
      - searching stops when a 
            - match is found 
            - orfound a BY group in dsr_secondary that is > dsr_primary's current BY group
                i.e. when there are BY groups in primary that don't exist in secondary
            - or no more records

    Returns `DSRC_SUCCESS` for "match found", `DSRC_DIFFERENT_GROUPS` for "no match"
        Otherwise, error codes
    */
IO_DATASET_RC DSR_cursor_sync_by(DSR_generic* dsr_primary, DSR_generic* dsr_secondary) {
    // return immediately if secondary is empty
    if (dsr_secondary->num_records == 0) {
        return DSRC_DIFFERENT_GROUPS;
    }

    // index of first record in current BY group for each dataset
    int index_primary = dsr_primary->rec_cursor.first_cur_by_index;
    int index_secondary = dsr_secondary->rec_cursor.first_cur_by_index;

    if (index_primary < 0 || index_secondary < 0) {
        // values should always be >= 0
        return DSRC_ERROR;
    }

    /* compare current groups */
    IO_DATASET_RC rc_compare_by = DSR_compare_rec_by_group(
        dsr_secondary,
        index_secondary,
        dsr_primary,
        index_primary
    );

    if (rc_compare_by == DSRC_BY_MATCH) {
        // group matches already, go back to start of group
        DSR_cursor_rewind(dsr_secondary, IOCA_REWIND_BY_GROUP);
        return DSRC_SUCCESS;
    }
    else if (rc_compare_by == DSRC_BY_NOT_SORTED) {
        // primary's group < secondary's group
        // this isn't an error, secondary just doesn't have primary's current BY group
        return DSRC_DIFFERENT_GROUPS;
    }

    /* need to search secondary for matching group */
    if (dsr_secondary->rec_cursor.cur_record_index == IOSV_UNSET){
        // otherwise, call to `DSR_cursor_next_by()` will just return the current BY group
        dsr_secondary->rec_cursor.cur_record_index = 0;
    }

    while (rc_compare_by == DSRC_DIFFERENT_GROUPS) {
        // primary's group > secondary's group, advance secondary to next group and check
        IO_DATASET_RC rc_next_by = DSR_cursor_next_by(dsr_secondary);
        if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) {  // i.e. secondary has no matching group
            return DSRC_DIFFERENT_GROUPS;
        }
        else if (rc_next_by != DSRC_NEXT_BY_SUCCESS) {  // some unhandled error
            return rc_next_by;
        }

        // with secondary now advanced to next group, compare against primary's group
        index_secondary = dsr_secondary->rec_cursor.first_cur_by_index;  // compare first record in secondary's current BY group
        rc_compare_by = DSR_compare_rec_by_group(
            dsr_secondary,
            index_secondary,
            dsr_primary,
            index_primary
        );
        if (rc_compare_by == DSRC_BY_MATCH) {
            // secondary has been advanced to a matching group, hooray!!
            return DSRC_SUCCESS;
        }
        else if (rc_compare_by == DSRC_BY_NOT_SORTED) {
            // this means that secondary's group is ahead of primary's group
            // perhaps because secondary doesn't have primary's current BY group, but does have more groups
            return DSRC_DIFFERENT_GROUPS;
        }
    }

    // error
    return rc_compare_by;
}

/* DSR_default_var_list
    Purpose: Builds a default variable list from all numeric vars that are not
             the unit_id or one of the BY variables
        - replacement for Outlier's `SetDefaultVarList()`

    Given a `dsr` and associated `vl`, initialize `vl` to a list of 
    variables from the `dsr` which match the following requirements
    - not listed in unit_id varlist
    - not listed in BY variable varlist
    - is a numeric variable */
IO_RETURN_CODE DSR_default_var_list(DSR_generic* dsr, DS_varlist* vl, const char* varlist_name) {
    if (dsr->num_columns <= 0) {
        // no columns? initialize to empty list
        return VL_init_silent(vl, dsr, varlist_name, NULL, IOVT_NUM);
    }

    // allocate and initialize "include_flags`, one for each variable in `dsr`
    VARLIST_DEFAULT_FLAGS* include_flags = STC_AllocateMemory(sizeof(*include_flags) * dsr->num_columns);
    for (int i = 0; i < dsr->num_columns; i++) {
        include_flags[i] = VDF_EXCLUDE;
    }

    // find and flag numeric variables
    for (int i = 0; i < dsr->num_columns; i++) {
        IO_VAR_TYPE var_type = IOVT_UNKNOWN;
        if ((var_type = DSR_get_col_type(dsr, i)) == IOVT_NUM) {
            include_flags[i] = VDF_NUMERIC;
        }
    }

    /* find and flag variables already being used in other variable lists
        Currently hardcoded to process only generic (standard) variable lists 
        In the future, this function may accept "variable arguments", listing additional 
        varlists to exclude */
    // allocate and build temporary list of varlists to search
    int num_vl_to_exclude = DSR_FIXED_VARLIST_COUNT;
    DS_varlist** vl_to_exclude = { 0 };
    vl_to_exclude = (DS_varlist**)STC_AllocateMemory(sizeof(*vl_to_exclude) * num_vl_to_exclude);
    vl_to_exclude[0] = &dsr->VL_unit_id;
    vl_to_exclude[1] = &dsr->VL_by_var;
    vl_to_exclude[2] = &dsr->VL_in_var;

    // process variable lists
    for (int vl_i = 0; vl_i < num_vl_to_exclude; vl_i++) {
        for (int i = 0; i < vl_to_exclude[vl_i]->count; i++) {
            include_flags[vl_to_exclude[vl_i]->positions[i]] = VDF_EXCLUDE_USED;
        }
    }

    // free temporary list of varlists
    STC_FreeMemory(vl_to_exclude);

    // determine count and names of variables to include
    int var_list_size = 0;
    size_t var_list_buffer_size = 1;   // start at 1, room for final trailing double '\0' ...
    for (int i = 0; i < dsr->num_columns; i++) {
        if (include_flags[i] == VDF_NUMERIC) {
            var_list_size++;
            var_list_buffer_size += strlen(dsr->col_names[i]) + 1; // `+ 1` for trailing ' ' (space)
            include_flags[i] = VDF_INCLUDE;
        }
    }

    if (var_list_size == 0) {
        // empty list: initialize to empty
        STC_FreeMemory(include_flags);
        return VL_init_silent(vl, dsr, varlist_name, NULL, IOVT_NUM);
    }

    // create temporary space delmited variable list initializer string from include flags
    char* temp_vl_string;
    size_t temp_vl_string_size = sizeof(*temp_vl_string) * var_list_buffer_size;
    temp_vl_string = STC_AllocateMemory(temp_vl_string_size);
    temp_vl_string[0] = '\0';

    for (int i = 0; i < dsr->num_columns; i++) {
        if (include_flags[i] == VDF_INCLUDE) {
            strcat_s(temp_vl_string, temp_vl_string_size, dsr->col_names[i]);  // copy var name
            strcat_s(temp_vl_string, temp_vl_string_size, " ");  // add trailing space
        }
    }
    STC_FreeMemory(include_flags);

    temp_vl_string[var_list_buffer_size - 2] = '\0';  // replace final trailing space with null terminator

    // initialize the default variable list 
    IO_RETURN_CODE rc = VL_init(vl, dsr, varlist_name, temp_vl_string, IOVT_NUM);

    STC_FreeMemory(temp_vl_string);

    return rc;
}

/* DSR_free
    Free all allocations referenced by the `DSR_generic` structure, or its members.  
    For copied structures, only free members which are "reallocated" via `DSR_copy_reallocate()`*/
void DSR_free(DSR_generic* dsr) {
    if (dsr == NULL) {
        return;
    }
    /* Free ORIGINAL members
        Note that this is skipped when the dsr is a copy because some members 
        are "shallow copied" (eg. `col_names` simply points to the original's `col_names`) */
    if (!dsr->is_a_copy  // not a copy
        && dsr->num_columns > 0  // has at least 1 column
    ) {
        if (dsr->col_lengths != NULL) {
            STC_FreeMemory((void*)dsr->col_lengths);
            dsr->col_lengths = NULL;
        }
        if (dsr->col_names != NULL) {
            STC_FreeMemory((void*)dsr->col_names);
            dsr->col_names = NULL;
        }
        ADI_free(&dsr->ad);
    }

    /* Free Varlists */
    if (dsr->list_of_varlist != NULL) {
        for (int i = 0; i < dsr->varlist_count; i++) {
            VL_free(dsr->list_of_varlist[i]);
        }
        STC_FreeMemory(dsr->list_of_varlist);
        dsr->list_of_varlist = NULL;
    }
}

/* DSR_get_col_type 
    replaces SUtil_GetVariableType(...)
    Returns the column's `IO_VAR_TYPE` (numeric or character) 
    
    Performance
        If this function is causing performance issues, store the `IOVT_*` column
        type in the `DSR_generic` structure after calculation and refer to that in 
        future calls */
IO_VAR_TYPE DSR_get_col_type(DSR_generic* dsr, int position) {
    struct ArrowSchemaView sv;
    struct ArrowError arrow_error;

    ArrowErrorCode rc_arrow = ArrowSchemaViewInit(&sv, dsr->ad.schema.children[position], &arrow_error);
    if (arrow_had_error(rc_arrow)) {
        arrow_print_error(&arrow_error, NULL);
        return IOVT_UNKNOWN;
    }

    if (arrow_is_string(sv.storage_type)) {
        return IOVT_CHAR;
    }
    else if (arrow_is_floating_point(sv.storage_type)
             || arrow_is_int_like(sv.storage_type))
    {
        return IOVT_NUM;
    }
    else {
        return IOVT_UNKNOWN;
    }
}

/* DSR_get_column_length
    Determine the length of the longest value for `dsr`'s column at `col_index`.  

    RETURNS
        - maximum string length (NOT including null terminator)
        - 8 for numeric columns (SAS did this, pre-redesign code expects this)
        For errors
            A negative integer from `IO_SENTINAL_VALUE`
    Performance
        For string columns, every value in the column is inspected, the maximum length
        is determined and stored in the DSR_generic structure for future reference.  */
int DSR_get_column_length(DSR_generic* dsr, int col_index) {
    TIME_WALL_START(get_column_length);
    TIME_CPU_START(get_column_length);

    if (col_index >= dsr->num_columns) {
        return IOSV_REFERENCE_ERROR;
    }

    IO_VAR_TYPE var_type = DSR_get_col_type(dsr, col_index);

    if (var_type == IOVT_NUM) {
        /* LEGACY EMULATION
            Pre redesign, the "numeric" datatype has a "length" of 8 */
        return 8;
    } else if (var_type != IOVT_CHAR) {
        return IOSV_UNKNOWN_TYPE;
    }

    // validation 
    if (dsr->num_records == 0) {
        return IOSV_NO_OBS;
    }

    // scan all observations, determine longest value
    if (dsr->col_lengths[col_index] == IOSV_UNSET) {
        T_ds_size max_length = 0;
        array_chunk* ac = dsr->ad.ac_list;
        while (ac != NULL) {
            for (int i = 0; i < ac->view.length; i++) {
                struct ArrowStringView asv = ArrowArrayViewGetStringUnsafe(ac->view.children[col_index], i);
                if (asv.size_bytes > max_length) {
                    max_length = asv.size_bytes;
                }
            }
            ac = ac->next_chunk;
        }
        dsr->col_lengths[col_index] = max_length;
    }

    TIME_CPU_STOPDIFF(get_column_length);
    TIME_WALL_STOPDIFF(get_column_length);
    return (int) dsr->col_lengths[col_index]; // we COULD return a `size_t`, but shouldn't need numbers that large
}

/* DSR_get_pos_from_names
    Determine the column indices of variables listed in var_names 
    and store them in var_positions list.  

    WARNING: `positions` must be allocated prior to calling and isn't freed in this function.  

    PARAMETERS: i-input, o-output
        [i] dsr - the dsr containing the associated dataset
        [i] num_vars - number of variables in var_names list
        [i] var_names - names of each variable
        [o] positions - the index of each var_names in the dsr
            - MUST BE ALLOCATED PRIOR TO CALLING */
IO_RETURN_CODE DSR_get_pos_from_names(DSR_generic* dsr, int num_vars, char** var_names, int* positions) {
   int i, j;
    /* find and store location (column index) of each var */
    IO_BOOLEAN var_missing = IOB_FALSE;  // overall flag set to IOB_TRUE if ANY variable not found
    for (i = 0; i < num_vars; i++) {  // for each var in list
        positions[i] = IOSV_VAR_MISSING;  // sentinal value: variable missing
        // search dataset for variable
        for (j = 0; j < dsr->num_columns && positions[i] == IOSV_VAR_MISSING; j++) {  // until found or searched all columns
            if (IOUtil_stricmp_is_equal(var_names[i], dsr->col_names[j])) {
                positions[i] = j;   //FOUND VAR
            }
        }

        // still missing? set global flag but continue getting positions
        if (j == dsr->num_columns && positions[i] == IOSV_VAR_MISSING) {
            var_missing = IOB_TRUE;
        }
    }

    if (var_missing) {
        return IORC_VARLIST_NOT_FOUND;
    }

    return IORC_SUCCESS;
}

/* DSR_get_varname_str
    ALLOCATES and returns a string of space-separated variable names corresponding
    to each variable in `dsr` identified in `position_list`.  

    `position_list` determines the order of var-names output. 

    WARNING: Any non-NULL value returned by this function MUST BE FREED BY THE CALLER!

    Parameters
        dsr           - the `DSR_generic` structure the variables are from
        num_variables - number of variables in ...
        position_list - list of variable positions (column index) in dsr 

    Why isn't this named `DSR_get_names_from_pos()`? 
        It's not a true opposite of `DSR_get_pos_from_names()`, which accepts
        a string array.  This function accepts a single string with space-delimited
        variable names.  */
char* DSR_get_varname_str(DSR_generic* dsr, int num_variables, int* position_list) {
    // calculate buffer size
    size_t buff_size = 1; // extra double '\0'
    for (int i = 0; i < num_variables; i++) {
        buff_size += strlen(dsr->col_names[position_list[i]]) + 1; // +1 for trailing space
    }

    // allocate buffer
    char* varlist_str = STC_AllocateMemory(sizeof(*varlist_str) * buff_size);
    varlist_str[0] = '\0';

    if (num_variables == 0) {
        return varlist_str;
    }

    // fill buffer with space separated variable list
    for (int i = 0; i < num_variables; i++) {
        strcat_s(varlist_str, buff_size, dsr->col_names[position_list[i]]);
        strcat_s(varlist_str, buff_size, " ");
    }
    varlist_str[buff_size - 2] = '\0';

    return varlist_str;
}

/* DSR_init
    initialize the DSR_generic structure
        - set dataset name
        - add `dsr` to list of dataset readers maintained by `spg`
        - allocate and populate `list_of_varlist` with procedure-specific and standard varlists
        - load input dataset
        - extract input dataset metadata
        - initialize record cursor
        - set `is_specified` flag

    PARAMETERS
        spg             - pointer to `SP_generic` struct, tracking associated `dsr`
        index_to_add    - index in `spg.list_dsr` to store `dsr` 
                          NOTE: caller should use associated `DSIO_<dataset-name>` enum value
        dsr             - pointer to `DSR_generic` struct to be initialized
        dataset_name    - used in print statements, like "INSTATUS"
        in_ds_dataset   - dataset from calling application
        unit_id         - name of unit ID variable
                            - pass `NULL` to skip unit_id initalization
        by_var          - space-delimited list of 0 or more BY variables
                            - pass `NULL` to skip BY varlist initialization
        by_var_optional - `IOB_TRUE` or `IOB_FALSE`
                          when `by_var` is not `NULL`, passing 
                          true suppresses error messages and return codes 
                          related to BY variable list initialization
        child_vl_count  - number of `...` that follows (see ...)
        ...             - exactly `child_vl_count` instances of `DS_varlist*`
                          these are added to `DSR_generic.list_of_varlist`
                          but not initialized here (caller must initialize)

    HELPER/BACKEND FUNCTIONS
        The `DSR_init*()` function set has 1 backend and 2 exposed functions
          - exposed `DSR_init()` used in almost all cases
          - exposed `DSR_init_simple()` used for non-data datasets (eg. estimato.inalgorithm)
          - backend `DSR_init__backend()` contains the actual logic
        The exposed functions are "variadic" (take `...` as the final parameter).  
          These exposed functions retrieve the variadic arguments (`va_list`) and pass
          it to the backend function.  

    RETURNS:
        - `IORC_SUCCESS`
        - `IORC_DATASET_MISSING`: caller to determine if this is an error

    ERRORS:
        - `IORC_ERROR`: general error occurred
        - `IORC_BY_VARLIST_NOT_FOUND`: by vars not optional but initialization error occurred
    */
IO_RETURN_CODE DSR_init(
    SP_generic* spg,
    int index_to_add,
    DSR_generic* dsr,
    const char* dataset_name,
    T_in_ds in_ds_dataset,
    const char* unit_id,
    const char* by_var,
    IO_BOOLEAN by_var_optional,
    int child_vl_count,
    ...
) {
    va_list args;
    va_start(args, child_vl_count);
    IO_RETURN_CODE rc = DSR_init__backend(spg, index_to_add, dsr, dataset_name, in_ds_dataset, unit_id, by_var, by_var_optional, child_vl_count, args);
    va_end(args);
    return rc;
}

/* DSR_init__backend - see `DSR_init()` documentation */
IO_RETURN_CODE DSR_init__backend(
    SP_generic* spg,
    int index_to_add,
    DSR_generic* dsr,
    const char* dataset_name,
    T_in_ds in_ds_dataset,
    const char* unit_id,
    const char* by_var,
    IO_BOOLEAN by_var_optional,
    int child_vl_count,
    va_list args
) {
    dsr->dataset_name = dataset_name;
    spg->dsr_list[index_to_add] = dsr;

    /* add variable lists to DSR_generic.list_of_varlist
        this should ALWAYS be done, even if the dataset is known to be empty */
    dsr->varlist_count = child_vl_count + DSR_FIXED_VARLIST_COUNT;
    dsr->list_of_varlist = (DS_varlist**)STC_AllocateMemory(sizeof(*dsr->list_of_varlist) * ((size_t)dsr->varlist_count));
    // add procedure-specific variable lists
    for (int i = 0; i < child_vl_count; i++) {
        dsr->list_of_varlist[i] = va_arg(args, DS_varlist*);
    }
    // add standard variable lists
    dsr->list_of_varlist[child_vl_count + 0] = &dsr->VL_unit_id;
    dsr->list_of_varlist[child_vl_count + 1] = &dsr->VL_by_var;
    dsr->list_of_varlist[child_vl_count + 2] = &dsr->VL_in_var;


    /* load dataset */
    ARROW_RETURN_CODE rc_init_data = ADI_init(&dsr->ad, in_ds_dataset);
    if (rc_init_data == ARC_DATASET_MISSING) {
        dsr->is_specified = IOSV_NOT_SPECIFIED;
        /* the setting of values likely unnecessary, as struct should be zero-initialized already */
        dsr->num_records = 0;
        dsr->num_columns = 0;
        dsr->col_names = NULL;
        return IORC_DATASET_MISSING;
    }
    else if (rc_init_data != ARC_SUCCESS) {
        return IORC_ERROR;
    }

    /* extract metadata */
    dsr->num_records = dsr->ad.num_records;
    dsr->num_columns = dsr->ad.schema.n_children;
    dsr->col_names = STC_AllocateMemory(sizeof(*dsr->col_names) * dsr->num_columns);
    dsr->col_lengths = STC_AllocateMemory(sizeof(*dsr->col_lengths) * dsr->num_columns);
    if (dsr->col_names == NULL || dsr->col_lengths == NULL) {
        return IORC_ERROR;
    }
    for (int i = 0; i < dsr->num_columns; i++) {
        dsr->col_names[i] = dsr->ad.schema.children[i]->name;
        if (dsr->col_names[i] == NULL) {
            return IORC_ERROR;
        }
        dsr->col_lengths[i] = IOSV_UNSET;
    }

    /* initialize CURRENT RECORD POINTERS */
    dsr->rec_cursor.cur_record_index = IOSV_UNSET;   // no records have been read yet
    dsr->rec_cursor.first_cur_by_index = 0; // the truth, this is always the first BY group start location

    /* Initialize Standard Variable Lists 
        procedure-specific varlists handled by procedure code */
    IO_RETURN_CODE rc = IORC_ERROR;
    // `unit_id`: mandatory character variable
    rc = VL_init_single(&dsr->VL_unit_id, dsr, VL_NAME_unit_id, unit_id, IOVT_CHAR);
    if (rc != IORC_SUCCESS){
        return rc;
    }

    // `by`: uses special type `IOVT_RAW` which allows mixed datatypes, sometimes optional
    if (by_var_optional == IOB_TRUE) {
        rc = VL_init_silent(&dsr->VL_by_var, dsr, VL_NAME_by_var, by_var, IOVT_RAW);
    }
    else if (by_var_optional == IOB_FALSE) {
        rc = VL_init(&dsr->VL_by_var, dsr, VL_NAME_by_var, by_var, IOVT_RAW);
    }
    else {
        return IORC_ERROR;// invalid value
    }

    if (rc == IORC_VARLIST_NOT_FOUND) {
        // missing `by` vars only an error if not optional
        if (by_var_optional == IOB_FALSE) {
            return IORC_BY_VARLIST_NOT_FOUND;
        }
    }
    else if (rc != IORC_SUCCESS) {
        return rc;
    }

    // input variables: initialize with type `IOVT_DISABLED` so procedure specific code can finish the job as-needed
    VL_init(&dsr->VL_in_var, dsr, VL_NAME_in_var, NULL, IOVT_DISABLED);

    dsr->is_specified = IOSV_SPECIFIED;

    return IORC_SUCCESS;
}

/* DSR_init_simple
    call DSR_init without `unit_id` or `by_var` initialization 
    see `DSR_init()` documentation for full details */
IO_RETURN_CODE DSR_init_simple(SP_generic* spg, int index_to_add, DSR_generic* dsr, const char* dataset_name, T_in_ds in_ds_dataset, int child_vl_count, ...) {
    va_list args;
    va_start(args, child_vl_count);
    return DSR_init__backend(spg, index_to_add, dsr, dataset_name, in_ds_dataset, NULL, NULL, IOB_TRUE, child_vl_count, args);
    va_end(args);
}

/* DSR_read_num
    Get the value of a numeric variable by column and row index.  
    As long as the variable is numeric, it is retrieved , casting to double
    if necessary.  

    RETURNS: `IORC_SUCCESS`
    ERROR: `IORC_FAIL_TYPE_INCORRECT` - not numeric, for example a string column
    */
IO_RETURN_CODE DSR_read_num(DSR_generic* dsr, T_ds_size row_index, T_ds_size col_index, double* dest) {
    // locate row in ac_list
    array_chunk* ac = ADI_get_chunk(&dsr->ad, row_index);
    row_index -= ac->index_offset;
    ARROW_RETURN_CODE rc_arrow = AC_read_num(ac, row_index, col_index, dest);
    if (rc_arrow == ARC_INVALID_TYPE) {
        return IORC_FAIL_TYPE_INCORRECT;
    }
    else if (rc_arrow != ARC_SUCCESS) {
        return IORC_ERROR;
    }

    return IORC_SUCCESS;
}

/* DSR_rec_get
    Process all variable lists on the current record.  

    Read all variables listed in variable lists in the `DSR_generic` structure 
    from the record pointed to by the record cursor, writing values according to 
    each varlist.  */
IO_RETURN_CODE DSR_rec_get(DSR_generic* dsr) {
    IO_RETURN_CODE rc = IORC_SUCCESS;
    for (int i = 0; i < dsr->varlist_count; i++) {  // for each varlist in `dsr`
        rc = VL_read(dsr->list_of_varlist[i]);
        if (rc != IORC_SUCCESS)
            return rc;
    }
    return rc;
}

/* DSR_validate_init
    Checks the returncode from `DSR_init*()` function, prints message if invalid
    Returns IORC_SUCCESS if no issue
    Otherwise, prints error messages (if applicable) and returns 
    original init return code*/
IO_RETURN_CODE DSR_validate_init(DSR_generic* dsr, IO_RETURN_CODE rc_init, IO_BOOLEAN is_mandatory) {
    if (is_mandatory == IOB_TRUE
        && rc_init == IORC_DATASET_MISSING) {
        printf(MSG_PREFIX_ERROR MsgDataSetMandatory "\n", dsr->dataset_name);
        return rc_init;
    }
    else if (rc_init == IORC_FAIL_INIT_IN_DATASET) {
        printf(MSG_PREFIX_ERROR JUM_JSON_PARSE_ERROR_INDATA);
        return rc_init;
    }
    else if (rc_init != IORC_SUCCESS
        && rc_init != IORC_DATASET_MISSING /* handled above, only when mandatory */) {
        return rc_init;
    }

    return IORC_SUCCESS;
}

/* DSR_var_exists
    Determine if variable (column) `var_name` exists in dataset `dsr` 

    NOTE: this function may be unused but should be retained and maintained for its usefulness */
IO_BOOLEAN DSR_var_exists(DSR_generic* dsr, char* var_name) {
    IO_RETURN_CODE rc = IORC_SUCCESS;
    int not_used;
    rc = DSR_get_pos_from_names(dsr, 1, &var_name, &not_used);
    if (rc == IORC_SUCCESS) {
        return IOB_TRUE;
    }
    else {
        return IOB_FALSE;
    }
}


/* ~~~~~ DATASET WRITER FUNCTIONS ~~~~~ */
/* DSW_add_record
    add record to output dataset managed by `DSW_generic` struct. */
IO_RETURN_CODE DSW_add_record(DSW_generic* dsw) {
    if (dsw->is_requested == IOB_FALSE){
        return IORC_SUCCESS;
    }

    if (IORC_SUCCESS != DSW_add_record_by_vars(dsw)) {
        return IORC_ERROR;
    }

    /* add data to new record */
    for (int i = 0; i < dsw->dynamic_var_count; i++) {  // for each non-by variable
        if (dsw->dynamic_var_types[i] == IOVT_CHAR) {  // string variable

            ArrowErrorCode rc_arrow = ArrowArrayAppendString(
                dsw->ado.data->children[i],
                ArrowCharView(((const char**)dsw->dynamic_var_ptrs)[i])
            );

            if (arrow_had_error(rc_arrow)) {
                return IORC_ERROR;
            }
        } else if (dsw->dynamic_var_types[i] == IOVT_NUM) {  // numeric data
            if (isnan(*((double**)dsw->dynamic_var_ptrs)[i])) {
                if (ARC_SUCCESS != ADO_append_missing(&dsw->ado, i)) {
                    return IORC_ERROR;
                }
            }
            else {
                ArrowErrorCode rc_arrow = ArrowArrayAppendDouble(
                    dsw->ado.data->children[i],
                    *((double**)dsw->dynamic_var_ptrs)[i]
                );

                if (arrow_had_error(rc_arrow)) {
                    return IORC_ERROR;
                }
            }
        }
    }

    // appending above increases each individual column's length
    // manually increase overall dataset length
    dsw->ado.data->length++;

    return IORC_SUCCESS;
}

/* DSW_add_record_by_vars
    add "BY" variables to current output record managed by `DSW_generic` struct.

    BY VARIABLES
        if reference dataset `ref_by_vars` is specified and includes by variables
        add them as well.

        !!! WARNING !!!
            this function assumes the record cursor from `ref_by_vars` is pointing to
            the correct record.  Some procs read a whole BY group before processing,
            which would cause an issue.
    */
IO_RETURN_CODE DSW_add_record_by_vars(DSW_generic* dsw) {
    /* add (optional) BY variables */
    if (dsw->ref_by_vars == NULL) {
        return IORC_SUCCESS;
    }

    int by_var_count = dsw->ref_by_vars->VL_by_var.count;
    if (by_var_count <= 0) {
        return IORC_SUCCESS;
    }

    // locate row in ac_list
    T_ds_size row_index = dsw->ref_by_vars->rec_cursor.cur_record_index;
    array_chunk* ac = ADI_get_chunk(&dsw->ref_by_vars->ad, row_index);
    row_index -= ac->index_offset;

    for (int i = 0; i < by_var_count; i++) {
        struct ArrowArrayView* av = ac->view.children[dsw->ref_by_vars->VL_by_var.positions[i]];
        enum ArrowType type_ = av->storage_type;

        // data type doesn't matter if value is missing, all types handled the same
        if (ArrowArrayViewIsNull(av, row_index)) {
            if (ARC_SUCCESS != ADO_append_missing(&dsw->ado, i + dsw->dynamic_var_count)) {
                return IORC_ERROR;
            }
            continue;
        }

        // handle all other types
        ArrowErrorCode rc_arrow = ARC_FAILURE;

        if (arrow_is_string(type_)) {
            // make temp copy of string
            struct ArrowStringView asv = ArrowArrayViewGetStringUnsafe(av, row_index);
            char* temp_str = (char*)malloc(sizeof(*temp_str) * (asv.size_bytes + 1));
            if (temp_str == NULL) {
                return IORC_ERROR;
            }
            memccpy(temp_str, asv.data, '\0', asv.size_bytes);
            temp_str[asv.size_bytes] = '\0';

            // append string to output and free temp string
            rc_arrow = ArrowArrayAppendString(
                dsw->ado.data->children[i + dsw->dynamic_var_count],
                ArrowCharView(temp_str)
            );

            // free temp copy
            free(temp_str);
        }
        else if (arrow_is_signed_int(type_)) {
            int value = { 0 };
            if (ARC_SUCCESS != av_get_int_safe(av, row_index, &value)) {
                return IORC_ERROR;
            }

            rc_arrow = ArrowArrayAppendInt(
                dsw->ado.data->children[i + dsw->dynamic_var_count],
                value
            );
        }
        else if (arrow_is_unsigned_int(type_)) {
            int value = { 0 };
            if (ARC_SUCCESS != av_get_int_safe(av, row_index, &value)) {
                return IORC_ERROR;
            }

            rc_arrow = ArrowArrayAppendUInt(
                dsw->ado.data->children[i + dsw->dynamic_var_count],
                value
            );
        }
        else if (arrow_is_floating_point(type_)) {
            double value = { 0 };
            if (ARC_SUCCESS != av_get_double_safe(av, row_index, &value)) {
                return IORC_ERROR;
            }

            rc_arrow = ArrowArrayAppendDouble(
                dsw->ado.data->children[i + dsw->dynamic_var_count],
                value
            );
        }

        if (arrow_had_error(rc_arrow)) {
            return IORC_ERROR;
        }
    }

    return IORC_SUCCESS;
}

/* DSW_allocate
    allocate space for dynamic variables and, when applicable, BY variables.  

    For dynamic variables, set `dynamic_var_count` and allocate space for
    - names
    - pointers
    - types */
IO_RETURN_CODE DSW_allocate(DSW_generic* dsw, int vars_to_allocate) {
    if (dsw->is_requested == IOB_FALSE){
        return IORC_SUCCESS;
    }

    // check for inclusion of BY variables
    int by_var_count = 0;
    if (dsw->ref_by_vars != NULL){
        by_var_count = dsw->ref_by_vars->VL_by_var.count;
    }

    ARROW_RETURN_CODE rc_arrow = ADO_allocate(&dsw->ado, vars_to_allocate + by_var_count);
    if (rc_arrow != ARC_SUCCESS) {
        return IORC_ERROR;
    }

    // only allocate for non-BY variables
    dsw->dynamic_var_count = vars_to_allocate;
    dsw->dynamic_var_names = (char**)STC_AllocateMemory(sizeof(*dsw->dynamic_var_names) * dsw->dynamic_var_count);
    dsw->dynamic_var_ptrs = (void**)STC_AllocateMemory(sizeof(*dsw->dynamic_var_ptrs) * dsw->dynamic_var_count);
    dsw->dynamic_var_types = (IO_VAR_TYPE*)STC_AllocateMemory(sizeof(*dsw->dynamic_var_types) * dsw->dynamic_var_count);

    return IORC_SUCCESS;
}

/* DSW_start_appending
    Initialize BY variable columns if applicable, and 
    enable appending data to output dataset.  */
IO_RETURN_CODE DSW_start_appending(DSW_generic* dsw) {
    if (dsw->is_requested == IOB_FALSE) {
        return IORC_SUCCESS;
    }

    /* if including BY variables, copy their metadata from the reference dataset */
    if (dsw->ref_by_vars != NULL ) {
        for (int i = 0; i < dsw->ref_by_vars->VL_by_var.count; i++) {
            ARROW_RETURN_CODE rc_arrow = ADO_define_column_copy(
                &dsw->ado,
                i + dsw->dynamic_var_count,  // add AFTER dynamic variables
                &dsw->ref_by_vars->ad,
                dsw->ref_by_vars->VL_by_var.positions[i]
            );
            if (rc_arrow != ARC_SUCCESS) {
                return IORC_ERROR;
            }
        }
    }

    // add error handling
    if (ARC_SUCCESS != ADO_begin_building(&dsw->ado)) {
        return IORC_ERROR;
    }

    return IORC_SUCCESS;
}

/* DSW_define_column
    - define the name, type, and location to retrieve value for an output column*/
IO_RETURN_CODE DSW_define_column(DSW_generic* dsw, int destination_index, char* var_name, void* var_ptr, IO_VAR_TYPE var_type) {
    if (dsw->is_requested == IOB_FALSE) {
        return IORC_SUCCESS;
    }

    if (destination_index < 0 || destination_index >= dsw->dynamic_var_count) {
        // arriving here likely indicates a bug in the code
        return IORC_ERROR;
    }

    // define column in output dataset metadata
    ARROW_RETURN_CODE rc_arrow = ARC_FAILURE;
    if (var_type == IOVT_NUM) {
        rc_arrow = ADO_define_column(&dsw->ado, destination_index, (const char*)var_name, NANOARROW_TYPE_DOUBLE);
    }
    else if (var_type == IOVT_CHAR) {
        rc_arrow = ADO_define_column(&dsw->ado, destination_index, (const char*)var_name, NANOARROW_TYPE_STRING);
    }
    else {
        printf(MSG_PREFIX_ERROR MSG_IO_OUT_COL_TYPE_INVALID);
        return IORC_ERROR;
    }

    if (rc_arrow != ARC_SUCCESS) {
        return IORC_ERROR;
    }

    // store column name, type, and pointer for retrieving value
    dsw->dynamic_var_names[destination_index] = IOUtil_strdup(var_name);
    dsw->dynamic_var_ptrs[destination_index] = var_ptr;
    dsw->dynamic_var_types[destination_index] = var_type;

    return IORC_SUCCESS;
}

/* DSW_free
    Free a DSW_generic structure members allocated in `DSW_allocate()`
    NOTE: does not free the calling-application-provided output dataset, 
          the calling application must handle that*/
void DSW_free(DSW_generic* dsw) {
    if (dsw == NULL) {
        return;
    }

    if (dsw->dynamic_var_count > 0) {
        if (dsw->dynamic_var_names != NULL) {
            for (int i = 0; i < dsw->dynamic_var_count; i++) {
                if (dsw->dynamic_var_names[i] != NULL) {
                    STC_FreeMemory(dsw->dynamic_var_names[i]);
                    dsw->dynamic_var_names[i] = NULL;
                }
            }
            STC_FreeMemory(dsw->dynamic_var_names);
            dsw->dynamic_var_names = NULL;
        }

        if (dsw->dynamic_var_ptrs != NULL) {
            STC_FreeMemory(dsw->dynamic_var_ptrs);
            dsw->dynamic_var_ptrs = NULL;
        }

        if (dsw->dynamic_var_types != NULL) {
            STC_FreeMemory(dsw->dynamic_var_types);
            dsw->dynamic_var_types = NULL;
        }

        dsw->dynamic_var_count = IOSV_UNSET;
    }
}

/* DSW_init
    initialize the generic dataset writer struct 

    The dataset is considered NOT requested if `DSW_generic.schema` is `NULL`, as when
    the calling application doesn't provide a location to send output.  */
IO_DATASET_RC DSW_init(SP_generic* spg, int index_to_add, DSW_generic* dsw, const char* dataset_name, void* out_sch, void* out_arr) {
    dsw->dataset_name = dataset_name;
    spg->dsw_list[index_to_add] = dsw;
    dsw->ref_by_vars = NULL;

    if(ADO_init(&dsw->ado, out_sch, out_arr) != ARC_SUCCESS){
        return IORC_FAIL_INIT_OUT_DATASET;
    }

    if (dsw->ado.schema == NULL || dsw->ado.data == NULL) {
        dsw->is_requested = IOB_FALSE;
        dsw->dynamic_var_count = 0;
        return DSRC_SUCCESS;
    }

    dsw->is_requested = IOB_TRUE;
    dsw->dynamic_var_count = IOSV_UNSET;

    return DSRC_SUCCESS;
}

/* DSW_set_by_var_reference
    Enables inclusion of BY variables on output dataset.  
    - call this function AFTER calling `DSW_init()`

    Optionally specify a reference `DSR_generic` from which to copy BY variables.  
    If input dataset `ref_ds` includes BY variables, they will be copied 
    onto each record in the output dataset.  

    Variables will be copied from the reference datasets `DF_cursor` during `DSW_add_record()`.  
    Getting correct values depends on reference data's cursor remaining on the group being
    processed until all output records from that group have been added.  */
void DSW_set_by_var_reference(DSW_generic* dsw, DSR_generic* dsr_reference) {
    dsw->ref_by_vars = dsr_reference;
}

/* DSW_wrap
    "wrap up" data added to generic dataset writer */
IO_RETURN_CODE DSW_wrap(DSW_generic* dsw) {
    if (dsw->is_requested == IOB_FALSE) {
        return IORC_SUCCESS;
    }

    // finish building dataset
    if(ARC_SUCCESS != ADO_finish_building(&dsw->ado)){
        return IORC_ERROR;
    }

    return IORC_SUCCESS;
}

/* ~~~~~ STATCAN PROCEDURE GENERIC FUNCTIONS ~~~~~ */

/* SPG_free
    Free dynamic fields in the SP_generic structure */
void SPG_free(SP_generic* spg){
    JOP_free_json(spg->json_parameters);

    /* Free parameter list */
    if (spg->parameters != NULL) {
        STC_FreeMemory(spg->parameters);
        spg->parameters = NULL;
    }

    /* Free procedure specific DSR structures */
    if (spg->dsr_list != NULL) {
        for (int i = 0; i < spg->count_dsr; i++) {
            DSR_free(spg->dsr_list[i]);
        }
        STC_FreeMemory(spg->dsr_list);
        spg->dsr_list = NULL;
    }

    /* Free outputs */
    if (spg->dsw_list != NULL) {
        for (int i = 0; i < spg->count_dsw; i++) {
            DSW_free(spg->dsw_list[i]);
        }
        STC_FreeMemory(spg->dsw_list);
        spg->dsw_list = NULL;
    }
}

/* SPG_allocate
    Allocates dynamic fields in the `SP_generic` structure */
void SPG_allocate(SP_generic* spg, int count_parameters, int count_dsr, int count_dsw) {
    spg->count_parameters = count_parameters;
    spg->parameters = (PARM_generic**)STC_AllocateMemory(sizeof(*spg->parameters) * spg->count_parameters);

    spg->count_dsr = count_dsr;
    spg->dsr_list = (DSR_generic**)STC_AllocateMemory(sizeof(*spg->dsr_list) * spg->count_dsr);

    spg->count_dsw = count_dsw;
    spg->dsw_list = (DSW_generic**)STC_AllocateMemory(sizeof(*spg->dsw_list) * spg->count_dsw);
}

/* SPG_init 
    - load parameters from calling application
    - allocate dynamic members */
IO_RETURN_CODE SPG_init(SP_generic* spg, const T_in_parm in_parms, int count_parameters, int count_dsr, int count_dsw) {
    TIME_CPU_START(decode_parms);
    _JS_T_ json_parameters = JOP_load_json_string(in_parms);
    TIME_CPU_STOPDIFF(decode_parms);
    spg->json_parameters = json_parameters;
    if (json_parameters == NULL) {
        return IORC_FAIL_PARSE_JSON;
    }

    SPG_allocate(spg, count_parameters, count_dsr, count_dsw);
    return IORC_SUCCESS;
}

/* SPG_validate
    Run all validations on a `SP_generic` structure
        - parameter types
      for each dataset reader
        - validate each varlist's type*/
IO_RETURN_CODE SPG_validate(SP_generic* spg) {
    IO_RETURN_CODE rc = IORC_SUCCESS;

    /* validate parameter type
        This refers to the received JSON value's type
            i.e. number, string, object, boolean, etc. */
    for (int i = 0; i < spg->count_parameters; i++) {
        if (rc = PARM_validate_type(spg->json_parameters, spg->parameters[i]) != IORC_SUCCESS) {
            return rc;
        }
    }

    /* Variable List: type of variables
        This refers to the variables on the dataset
        associated with the varlist 
        - skip lists which aren't specified */
    DSR_generic* dsr;
    for (int ds = 0; ds < spg->count_dsr; ds++) {   // for each dataset reader
        dsr = spg->dsr_list[ds];
        if (dsr->is_specified != IOSV_SPECIFIED) {
            continue;
        }
        for (int i = 0; i < dsr->varlist_count; i++) {  // for each varlist
            rc = VL_validate_type(dsr->list_of_varlist[i]);
            if (rc != IORC_SUCCESS) {
                return rc;
            }
        }
    }

    return rc;
}

/* ~~~~~ PARAMETER FUNCTIONS ~~~~~ 
    For managing user parameters (eg. unit_id, seed, BY varlist, etc.) */

/* PARM_init
    Initialize the `PARM_generic` structure
        - add parameter to `SP_generic` structure's list of parameters
        - set parameter name and type
        - set `is_specified` flag
        - extract parameter value

        PARAMETERS (to this function)
            spg             - pointer to `SP_generic` struct, tracking associated `pg`
            index_to_add    - index in `spg.parameters` to store `pg`
                              NOTE: caller should use associated `UPO_<parameter-name>` enum value
            pg              - pointer to `PARM_generic` structure to be initialized
            parm_id         - name (key) of parameter
            parm_type       - the type of the parameter */
IO_RETURN_CODE PARM_init(SP_generic* spg, int index_to_add, PARM_generic* pg, const char* parm_id, IO_PARAM_TYPE parm_type) {
    if (index_to_add >= spg->count_parameters) {
        // arriving here likely indicates a bug in the code
        return IORC_ERROR;
    }

    spg->parameters[index_to_add] = pg;
    pg->parm_id = parm_id;
    pg->parm_type = parm_type;
    pg->is_specified = PARM_is_specified(spg->json_parameters, pg->parm_id);

    const char* string_value;
    /* retrieve value if specified, otherwise initialize to arbitrary value */
    switch (pg->parm_type) {
    case IOPT_FLAG:
        if (pg->is_specified == IOSV_SPECIFIED)
            ((UP_FLAG*) pg)->value = JOP_get_bool(JLM_object_get(spg->json_parameters, pg->parm_id));
        else
            ((UP_FLAG*) pg)->value = IOB_UNSET;
        break;
    case IOPT_INT:
        if (pg->is_specified == IOSV_SPECIFIED)
            ((UP_INT*)pg)->value = JOP_get_int(JLM_object_get(spg->json_parameters, pg->parm_id));
        else
            ((UP_INT*)pg)->value = 0;
        break;
    case IOPT_NUMERIC: // fall through
    case IOPT_FLOAT:
        if (pg->is_specified == IOSV_SPECIFIED)
            ((UP_INTNBR*)pg)->value = JOP_get_double(JLM_object_get(spg->json_parameters, pg->parm_id));
        else
            ((UP_INTNBR*)pg)->value = 0.0;
        break;
    case IOPT_NAME_STRING:
        if (pg->is_specified == IOSV_SPECIFIED && NULL != (string_value = JOP_get_string(JLM_object_get(spg->json_parameters, pg->parm_id)))) {
            strncpy_s(((UP_NAME*)pg)->value, UP_NAME_LENGTH + 1, string_value, UP_NAME_LENGTH);
            /* convert to uppercase as SAS used to do automatically */
            IOUtil_str_upcase(((UP_NAME*)pg)->value);
        }
        else
            memset(((UP_NAME*)pg)->value, '\0', sizeof(((UP_NAME*)pg)->value));
        break;
    case IOPT_STRING:
        if (pg->is_specified == IOSV_SPECIFIED)
            ((UP_QS*)pg)->value = JOP_get_string(JLM_object_get(spg->json_parameters, pg->parm_id));
        else
            ((UP_QS*)pg)->value = NULL;
        break;
    default:
        printf(MSG_PREFIX_ERROR JUM_PARM_TYPE_UNKNOWN, pg->parm_type);
    }

    return IORC_SUCCESS;
}

/* PARM_is_specified
    given the calling-application-provided  parameters, and a key,
    indicate whether or not the parameter `key` was specified by the user
    RETURNS
      IOSV_NOT_SPECIFIED
        - key does not exist
        - key exists AND value is JSON-null
      IOSV_SPECIFIED
        - key exists AND value is not JSON-null */
IO_SENTINAL_VALUE PARM_is_specified(_JS_T_ json_root, const char* key) {
    _JS_T_ js_obj = JLM_object_get(json_root, key);
    if (js_obj == NULL) {  // key doesn't exist
        return IOSV_NOT_SPECIFIED;
    }

    if (JLM_typeof(js_obj) == JLT_NULL) {  // key exists and is NULL
        return IOSV_NOT_SPECIFIED;
    }

    return IOSV_SPECIFIED;
}

/* PARM_validate_type 
    Validate that parameter with key `jsk` conforms to the 
    data type requirements of `parm_type`.

    Returns `JRC_SUCCESS` when
      - type matches
      - OR parameter wasn't specified by user

    INVALID PARAMETER
      - print error message
      - returns IORC_FAIL_TYPE_INCORRECT */
IO_RETURN_CODE PARM_validate_type(_JS_T_ json_root, PARM_generic* pg) {
    // cannot validate unspecified parameter
    if (PARM_is_specified(json_root, pg->parm_id) != IOB_TRUE) {
        return IORC_SUCCESS;
    }

    // get parameter from JSON
    _JS_T_ parm_obj = JLM_object_get(json_root, pg->parm_id);

    /* INTEGER */
    if (pg->parm_type == IOPT_INT) {
        if ((JLM_typeof(parm_obj) != JLT_INTEGER)
            && (JLM_typeof(parm_obj) != JLT_REAL)) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_INT);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }

        // is it an integer by value?
        double parm_value = JOP_get_double(parm_obj);
        if (ceil(parm_value) != parm_value) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_INT);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }
    }/* ANY NUMBER */
    else if (pg->parm_type == IOPT_FLOAT
        || pg->parm_type == IOPT_NUMERIC) {
        if ((JLM_typeof(parm_obj) != JLT_INTEGER)
            && (JLM_typeof(parm_obj) != JLT_REAL)) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_REAL);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }
    } /* ANY STRING */
    else if (pg->parm_type == IOPT_STRING) {
        if (JLM_typeof(parm_obj) != JLT_STRING) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_QS);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }
    } /* STRING with length <= UP_NAME_LENGTH */
    else if (pg->parm_type == IOPT_NAME_STRING) {
        if (JLM_typeof(parm_obj) != JLT_STRING) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_NAME);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }

        const char* parm_value = JOP_get_string(parm_obj);
        const int max_allowed_length = UP_NAME_LENGTH;
        if (strlen(parm_value) > max_allowed_length) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID_NAME_MSG, pg->parm_id, strlen(parm_value), max_allowed_length, parm_value);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }
    } /* FLAG: boolean value */
    else if (pg->parm_type == IOPT_FLAG) {
        if (JLM_typeof(parm_obj) != JLT_TRUE
            && JLM_typeof(parm_obj) != JLT_FALSE) {
            printf(MSG_PREFIX_ERROR JUM_PARM_INVALID, pg->parm_id, JUM_PARM_INVALID_FLAG);
            JOP_print_invalid_parm(parm_obj);
            return IORC_FAIL_TYPE_INCORRECT;
        }
    }

    return IORC_SUCCESS;
}

/* ~~~~~ VARIABLE LIST FUNCTIONS ~~~~~ */
/* VL_allocate
    Given a number of `vars_to_allocate`, allocate `DS_varlist` members
      - names
      - positions
      - ptrs

    NOTE: member `.names_buffer` must be allocated by caller */
void VL_allocate(DS_varlist* vl, int vars_to_allocate) {
    if (vars_to_allocate <= 0) {
        vl->count = 0;
        return;
    }

    vl->count =     vars_to_allocate;
    vl->names =     (char**)    STC_AllocateMemory((vl->count) * sizeof(*vl->names));
    vl->positions = (int*)      STC_AllocateMemory((vl->count) * sizeof(*vl->positions));
    vl->ptrs =      (void**)  STC_AllocateMemory((vl->count) * sizeof(*vl->ptrs));
    //`vl->names_buffer` must be allocated by caller
}

/* VL_copy_reallocate 
    Brief: must be ran on any copy of a `VL_init`ialized `DS_varlist`
      , but not on the original.  

    Purpose: decouple copied varlist from original
      When a `DS_varlist` has been "shallow copied" via assignment
        i.e. `DS_varlist a, b; VL_init(a,...); b=a;`
      pointers in the copy (`b`) point to the same memory as the original (`a`).
      In most cases (`.name`, `.positions`, etc.) this is desirable, however 
      for member `.ptrs`, the copy will need to modify the values.  
      Without "reallocating" `.ptrs`, the original (`a`) would be affected.  
    `.is_a_copy` - set to TRUE, informs `VL_free()` 
    
    WARNING(memory leak): Read this documentation before using
      This function overwrites pointers to allocated memory.  
      Misuse could lead to memory leaks.  
    Related functions
      This function performs a portion of copying a `DSR_generic` struct.
      Read `DSR_copy_reallocate()` documentation for more details */
void VL_copy_reallocate(DS_varlist* vl, DSR_generic* dsr) {
    vl->is_a_copy = 1 == 1;
    if (vl->count > 0) {
        vl->ptrs = (void**)STC_AllocateMemory(vl->count * sizeof(vl->ptrs));
    }
    else {
        vl->ptrs = NULL;
    }
    vl->dsr = dsr;
}

/* VL_free
    Given a `DS_varlist` free `VL_allocate` allocated members
            names - `VL_allocate()`
        positions - `VL_allocate()`
             ptrs - `VL_allocate()`
      names_buffe - caller allocated */
void VL_free(DS_varlist* vl) {
    if (vl->count <= 0) {
        return;
    }

    if (vl->ptrs != NULL) {
        STC_FreeMemory(vl->ptrs);
        vl->ptrs = NULL;
    }

    // if a copy, these fields are shared, will be freed when original VL is freed
    if (!vl->is_a_copy) { 
        if (vl->names_buffer != NULL) {
            STC_FreeMemory(vl->names_buffer);
            vl->names_buffer = NULL;
        }

        if (vl->names != NULL) {
            STC_FreeMemory(vl->names);
            vl->names = NULL;
        }

        if (vl->positions != NULL) {
            STC_FreeMemory(vl->positions);
            vl->positions = NULL;
        }
    }

    vl->count = 0;

    return;
}

/* VL_init
    see `VL_init__backend()` documentation*/
IO_RETURN_CODE VL_init(DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type) {
    IO_BOOLEAN should_i_print_error_messages = IOB_TRUE;
    return VL_init__backend(vl, dsr, varlist_name, varlist_str, var_type, should_i_print_error_messages);
}

/* VL_init__backend
    Initialize DS_varlist struct for each space-separated 
    variable in string `varlist_str`, according to its position
    in the dataset associated with DSR_generic struct `dsr`
    Pass `varlist_str=NULL` to initialize empty list.  

    - calculates number of variables in `varlist_str`
    - allocates remaining struct members
    - finds position of each variable in `dsr`
    - frees allocations upon error

    Note
      `varlist_str` is copied (ALLOCATION), but freed
        upon error and freed by `VL_free()`
    
    RETURNS
        `IORC_SUCCESS` 
          - successful initialization of 0 or more variables
        `IORC_VARLIST_NOT_FOUND`
            - 1 or more variables not found 
            - varlist initialized to empty 

    ...suffix `__backend` because pure C doesn't support overloading or default arguments -_- */
IO_RETURN_CODE VL_init__backend(DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type, IO_BOOLEAN print_errors) {
    vl->var_type = var_type;
    vl->dsr = dsr;
    vl->varlist_name = varlist_name;

    if (varlist_str == NULL) {
        vl->count = 0;
        vl->names = NULL;
        vl->positions = NULL;
        return IORC_SUCCESS; /* this is fine, some varlists can be empty */
    }

    /* TOKENIZE VARIABLE LIST
        - count variables
        - convert buffer into string array ( \0 terminate all var names) */
    char* str_tok = { 0 };
    char* str_context = { 0 };
    int count = 0;
    char* names_buffer = IOUtil_strdup(varlist_str);
    str_tok = strtok_r(names_buffer, " ", &str_context);
    while (str_tok != NULL) {
        (count)++;
        str_tok = strtok_r(NULL, " ", &str_context);
    }

    /* ALLOCATE & SET LISTS */
    VL_allocate(vl, count);
    vl->names_buffer = names_buffer;

    // set var name pointers
    for (int offset = 0, i = 0; i < vl->count; i++) {
        while (vl->names_buffer[offset] == ' ') {  // skip any leading spaces
            offset++;
        }
        vl->names[i] = &(vl->names_buffer[offset]);
        size_t name_len = strlen(&(vl->names_buffer[offset]));
        if (name_len > IO_COL_NAME_LEN) {
            printf(MSG_PREFIX_ERROR MSG_IO_VAR_NAME_TOO_LONG, 
                IO_COL_NAME_LEN,
                varlist_name,
                name_len,
                vl->names[i]
            );
            VL_free(vl);
            return IORC_VAR_NAME_TOO_LONG;
        }
        offset += ((int)name_len) + 1;
    }

    // set var positions list
    IO_RETURN_CODE rc = DSR_get_pos_from_names(vl->dsr, vl->count, vl->names, vl->positions);

    // if variables not found, free stuff, print errors, return error code
    if (rc != IORC_SUCCESS) {    // most likely `JRC_VARLIST_NOT_FOUND`
        /* print error messages, only if requested */
        if (print_errors == IOB_TRUE) {
            for (int i = 0; i < vl->count; i++) {
                if ((vl->positions)[i] == IOSV_VAR_MISSING) {
                    printf(MSG_PREFIX_ERROR JUM_VARLIST_MEMBER_MISSING, (vl->names)[i], vl->varlist_name, vl->dsr->dataset_name);
                }
            }
        }

        VL_free(vl);
        return rc;
    }

    return IORC_SUCCESS;
}

/* VL_init_from_positon_list
    Like other `VL_init*()` functions, but takes an array of variable 
    positions instead of a string of space-separated variable names */
IO_RETURN_CODE VL_init_from_position_list(DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, IO_VAR_TYPE var_type, int num_variables, int* position_list) {
    if (num_variables == 0) {
        return VL_init(vl, dsr, varlist_name, NULL, var_type);
    }

    // get space separated list of variable names
    char* temp_vl_string = DSR_get_varname_str(dsr, num_variables, position_list);

    // call normal VL_init using generated varlist
    IO_RETURN_CODE rc = VL_init(vl, dsr, varlist_name, temp_vl_string, var_type);

    // free buffer
    STC_FreeMemory(temp_vl_string);

    return rc;
}

/* VL_init_silent
    Convenience function; like `VL_init()`,
    but it doesn't print error messages */
IO_RETURN_CODE VL_init_silent(DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type) {
    IO_BOOLEAN should_i_print_errors = IOB_FALSE;
    return VL_init__backend(vl, dsr, varlist_name, varlist_str, var_type, should_i_print_errors);
}

/* VL_init_single
    Convenience function; like `VL_init()`,
    but it also validates that at most 1 variable was specified */
IO_RETURN_CODE VL_init_single(DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type) {
    IO_RETURN_CODE rc = VL_init(vl, dsr, varlist_name, varlist_str, var_type);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    if(vl->count > 1){
        printf(MSG_PREFIX_ERROR JUM_SINGLE_VARLIST_COUNT_INVALID, vl->varlist_name, vl->var_type == IOVT_NUM ? JUM__NUMERIC : JUM__CHARACTER);
        return IORC_VARLIST_INVALID_COUNT;
    }

    return rc;
}

/* VL_is_specified
    Check whether or not a valid **NON-EMPTY** variable list was specified.  
    Use this check prior to referencing a variable list's dynamically allocated members.  
    If the check is not related to referencing members, consider whether referencing the 
    associated parameter's `PARM_generic.is_specified` flag is a better choice. */
IO_BOOLEAN VL_is_specified(DS_varlist* vl) {
    if (vl->count > 0) {
        return IOB_TRUE;
    }

    return IOB_FALSE;
}

/* VL_Read
    call the appropriate read function based on varlist's type */
IO_RETURN_CODE VL_read(DS_varlist* vl) {
    if (vl->var_type == IOVT_NUM) {
        return VL_read_num(vl);
    } else if (vl->var_type == IOVT_CHAR) {
        return VL_read_char(vl);
    }
    else if (
        vl->var_type == IOVT_RAW  // example: for BY variables we just track their positions
        || vl->var_type == IOVT_DISABLED  // don't read disabled lists
    ) {
        return IORC_SUCCESS;
    }
    else {
        return IORC_ERROR;
    }
}

/* VL_read_char
    Read a character variable list, writing values to preset pointers */
IO_RETURN_CODE VL_read_char(DS_varlist* vl) {
    // locate row in ac_list
    T_ds_size row_index = vl->dsr->rec_cursor.cur_record_index;
    array_chunk* ac = ADI_get_chunk(&vl->dsr->ad, row_index);
    row_index -= ac->index_offset;

    for (int i = 0; i < vl->count; i++) {
        ARROW_RETURN_CODE rc_arrow = AC_read_char(ac, row_index, vl->positions[i], (char*)vl->ptrs[i]);
        if (rc_arrow == ARC_INVALID_TYPE) {
            printf(MSG_PREFIX_ERROR JUM_TYPE_MISMATCH_CHAR, vl->names[i], vl->dsr->rec_cursor.cur_record_index);
            return IORC_FAIL_TYPE_INCORRECT;
        }
        else if (rc_arrow != ARC_SUCCESS) {
            return IORC_ERROR;
        }
    }
    return IORC_SUCCESS;
}

/* VL_read_num
    Read a numeric variable list, writing values to preset pointers */
IO_RETURN_CODE VL_read_num(DS_varlist* vl) {
    // locate row in ac_list
    T_ds_size row_index = vl->dsr->rec_cursor.cur_record_index;
    array_chunk* ac = ADI_get_chunk(&vl->dsr->ad, row_index);
    row_index -= ac->index_offset;

    for (int i = 0; i < vl->count; i++) {
        ARROW_RETURN_CODE rc_arrow = AC_read_num(ac, row_index, vl->positions[i], (double*)vl->ptrs[i]);
        if (rc_arrow == ARC_INVALID_TYPE) {
            printf(MSG_PREFIX_ERROR JUM_TYPE_MISMATCH_NUM, vl->names[i], vl->dsr->rec_cursor.cur_record_index);
            return IORC_FAIL_TYPE_INCORRECT;
        }
        else if (rc_arrow != ARC_SUCCESS) {
            return IORC_ERROR;
        }

    }
    return IORC_SUCCESS;
}

/* validate_varlist_type:
    Given a `DS_varlist`
    Validate that all (0 or more) variables match the varlist's 
    prescribed type, `.var_type`.  

    Return `IORC_SUCCESS`
        - if no mismatch found
        - if type is "disabled" or "raw"
        - if count is 0

    MISMATCH FOUND:
        - print error message to log
        - return `IORC_ERROR` */
IO_RETURN_CODE VL_validate_type(DS_varlist* vl) {
    IO_VAR_TYPE var_datatype;
    int var_count = vl->count;

    if (vl->var_type == IOVT_RAW
        || vl->var_type == IOVT_DISABLED) { // for BY variables, skip this validation
        return IORC_SUCCESS;
    }
    else if (vl->var_type != IOVT_CHAR
        && vl->var_type != IOVT_NUM) {
        return IORC_ERROR;
    }

    for (int i = 0; i < var_count; i++) {
        var_datatype = DSR_get_col_type(vl->dsr, vl->positions[i]);
        if (var_datatype != vl->var_type) {
            printf(MSG_PREFIX_ERROR JUM_VARLIST_WRONG_TYPE, vl->names[i], vl->varlist_name, vl->dsr->dataset_name, var_type_desc[vl->var_type], var_type_desc[var_datatype]);
            return IORC_ERROR;
        }
    }
    return IORC_SUCCESS;
}