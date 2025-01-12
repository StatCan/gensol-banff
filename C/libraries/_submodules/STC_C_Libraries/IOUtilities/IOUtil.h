#ifndef IOUTIL_H
#define IOUTIL_H

// dataset column ("variable") name length excluding null terminator
#define IO_COL_NAME_LEN 64

/* Aliases for use in replacing legacy log printing calls */
#define IO_PRINT printf
#define IO_PRINT_LINE(format, ...) printf(format "\n", ##__VA_ARGS__)

// local library headers
#include "IO_Jansson.h"  // need in `.h` for JSON types
#include "IO_Arrow.h"


/* Types */
typedef arrow_in_type T_in_ds;  // use arrow library for input datasets
typedef arrow_out_type T_out_ds;  // use arrow library for output datasets
typedef arrow_dataset_size_type T_ds_size;  // Type used to represent size (rows, columns) of dataset
typedef jansson_in_type T_in_parm;  // JSON string for input parameters


/* ENUMERATIONS */

/* IO_RETURN_CODE
    Generic set of return codes used extensively in this library and by its callers */
typedef enum __IORC_IO_RETURN_CODE {
    IORC_SUCCESS,  // generic success code
    IORC_ERROR,  // generic error code
    IORC_VAR_NAME_TOO_LONG,
    IORC_VARLIST_NOT_FOUND,
    IORC_VARLIST_INVALID_COUNT,
    IORC_BY_VARLIST_NOT_FOUND,
    IORC_DATASET_MISSING,
    IORC_FAIL_PARSE_JSON,
    IORC_FAIL_TYPE_INCORRECT,
    IORC_FAIL_INIT_IN_DATASET,
    IORC_FAIL_INIT_OUT_DATASET,
    IORC_FAIL_LIST_EXCLUSIVITY,
} IO_RETURN_CODE;

/* set of "sentinal values" for flagging various things, such as 
    - parameter specification: specified or not
    - dataset specification: specified or not
    - varlist specification: specified or not
    - varlist variable missing: special value 

    This enum has hardcoded values.  When modifying, be sure to
    document the reason for duplicate values.  */
typedef enum __IOSV_IO_SENTINAL_VALUE {
    IOSV_UNKNOWN_TYPE       = -3,
    IOSV_REFERENCE_ERROR    = -2,
    IOSV_VAR_MISSING        = -1, // duplicate intentional
    IOSV_UNSET              = -1, // duplicate intentional
    IOSV_NO_OBS             = 0,  // duplicate intentional, denotes max string size for empty string column
    IOSV_NOT_SPECIFIED      = 0,  // duplicate intentional
    IOSV_SPECIFIED          = 1,
} IO_SENTINAL_VALUE;

/* To support legacy behaviour the values of IOVT_NUM=1 and IOVT_CHAR=2 !!MUST NOT BE MODIFIED!!
    NO DUPLICATES ALLOWED

    This enumeration is used for a few purposes, including identifying 
    the "type" a particular dataset variable (column) or a `DS_varlist`*/
typedef enum __IOVT_IO_VAR_TYPE {
    IOVT_UNKNOWN = 0,        // 0- the initial value of a zero-initialized enum variable
    IOVT_NUM = 1,//MUST==1   // numeric variables (integer or decimal)
    IOVT_CHAR = 2,//MUST==2  // character variables (strings of text)
    IOVT_RAW = 3,            // for BY variables, whose JSON values compared as raw JSON
    IOVT_DISABLED = 4,       // associated `DS_varlist` disabled
    IOVT__COUNT
} IO_VAR_TYPE;
extern const char* var_type_desc[IOVT__COUNT]; /* description of each enum value, to be populated in IOUtil.c */

/* in-house boolean implementation */
typedef enum __IOB_IO_BOOLEAN {
    IOB_FALSE = 0,  // IOB_FALSE **MUST = 0** so expressions evaluate correctly
    IOB_TRUE = 1,  // IOB_TRUE **MUST = 1** so expressions evaluate correctly
    IOB_UNSET = 2, // initial value for not-yet-set boolean variable
} IO_BOOLEAN;

// Used by DSR_*() functions
typedef enum __DSRC_IO_DATASET_RETURN_CODE {
    // generic success
    DSRC_SUCCESS,
    // BY group advancement
    DSRC_NEXT_BY_SUCCESS,
    DSRC_NO_MORE_REC_IN_BY,
    // Record advancement
    DSRC_NEXT_REC_SUCCESS,
    DSRC_NO_MORE_REC_IN_DS,
    // BY group comparison
    DSRC_BY_MATCH,
    DSRC_DIFFERENT_GROUPS,
    // errors
    DSRC_ERROR,
    DSRC_INVALID_INDEX,
    DSRC_INVALID_BY_TYPES,
    DSRC_BY_NOT_SORTED,
} IO_DATASET_RC;

// datatype of parameter, example 'SEED' is an integer, not a float
typedef enum __IOPT_IO_PARAM_TYPE {
    IOPT_FLAG,          // flags (like SAS' ACCEPTNEGATIVE)
    IOPT_INT,           // integer values only
    IOPT_FLOAT,         // note: we treat INT as a subset of FLOAT
    IOPT_NUMERIC,       // any numeric value
    IOPT_NAME_STRING,   // string of length [0,8]
    IOPT_STRING         // string of length [0,?]
} IO_PARAM_TYPE;

typedef enum __IOCA_IO_CURSOR_ACTION {   /* for `DSR_cursor_rewind()` */
    IOCA_REWIND_DATASET,  //corresponds to 'F' in `SAS_XBYRWND`
    IOCA_REWIND_BY_GROUP  //corresponds to 'B' in `SAS_XBYRWND`
} IO_CURSOR_ACTION;


/* STRUCTURES */
/* DS_cursor
    Enable iteration through a dataset through BY groups 
    and the records within those groups.  */
typedef struct __IO_DATASET_cursor {
    int         cur_record_index;  /* current row */
    int         first_cur_by_index;  /* first (lowest #) row in current BY group */
} DS_cursor;

typedef struct __DSR_dataset_reader_generic DSR_generic; // forward declaration, as necessary
/* DS_varlist
    Tracks set of variables in a dataset for reading/writing.  

    Variable Lists are often provided as a parameter by users, 
    specifying which variables from a particular dataset to use in 
    calculations.  This struct and associated functions hold the
    necessary metadata to validate the existance, and read the values, 
    of these variables at a given row in the dataset */
typedef struct __DS_varlist {
    DSR_generic*    dsr;                /* pointer to associated "dataset reader" struct */

    IO_VAR_TYPE     var_type;           /* 1 type applies to ALL vars in varlist */
    const char*     varlist_name;       /* name of the variable list (eg. 'must_match') 
                                           used in console log messages */

    int             count;              /* number of variables in list*/
    char*           names_buffer;       /* single buffer for holding all null-terminated variable names */
    char**          names;              /* string array points to each name in `names_buffer` */
    int*            positions;          /* positions (i.e. column index) of variables */
    void**          ptrs;               /* location in memory to write each variable value 
                                           when `VL_read()` is called */

    int             is_a_copy;          /* flag must be set on copies (not originals) when this
                                           struct is copied (see `VL_copy_reallocate()`) */
} DS_varlist;

/* DSR_generic 
    Generic Dataset Reader structure

    Super structure for procedure-specific `DSR_<dataset-name>` structures
    Holds data and metadata required to read an input dataset.  */
struct __DSR_dataset_reader_generic { // NOTE: typedef is forward declared (above)
    /* RAW DATA */
    arrow_dataset_in ad;                /* the actual data */

    /* FLAGS */
    int             is_specified;       /* whether or not user provided a given dataset */
    int             is_a_copy;          /* flag must be set on copies (not originals) when this
                                           struct is copied (see `DSR_copy_reallocate()`) */

    /* METADATA */
    const char*     dataset_name;       /* example: "INDATA", "INSTATUS" 
                                           used in console log messages */
    T_ds_size       num_records;        /* number of rows in dataset */
    T_ds_size       num_columns;        /* number of columns in dataset... obviously */
    T_ds_size       *col_lengths;       /* for string columns, length of largest string,
                                           initially `IOSV_UNSET`, calculated only if needed */
    const char**    col_names;          /* array of column names, used
                                           in processing and console log messages */

    /* STATE */
    DS_cursor       rec_cursor;         /* track current row and BY group */

    /* VARIABLE LISTS 
        THIS structure defines a number of `DS_varlist` members which are "generic".  
        Procedures will implement a child structure which has 0 or more additional 
        variable lists.  
        Members `varlist_count` and `list_of_varlist` must account for the child
        structure's varlists as well.  */
    int             varlist_count;      /* include this struct and child structure */
    DS_varlist**    list_of_varlist;    /* include this struct and child structure.  
                                           This is used for operations which iterate over all variable 
                                           lists in a `DSR_generic` structure.  */
    // standard variable lists
    DS_varlist      VL_unit_id;         // UNIT_ID: typically single mandatory variable
    DS_varlist      VL_by_var;          // BY_VAR:  typically 0 or more variables
    DS_varlist      VL_in_var;          /* IN_VAR:  input variable determined at runtime(from EDITS for example) 
                                               this varlist is a bit odd, some procedures don't use it */
}; /* NOTE: typedef forward declared above */
#define DSR_FIXED_VARLIST_COUNT 3  // number of standard varlists in DSR_generic struct

/* DSW_generic
    Generic Dataset Writer structure

    This struct facilitates building of a dataset with 
    any number of variables of mixed types through use of
    associated functions.  */
typedef struct __DSW_dataset_writer_generic {
    /* RAW DATA */
    arrow_dataset_out ado;              /* caller-provided container for writing raw data to */

    /* FLAGS */
    IO_BOOLEAN      is_requested;       /* whether or not user requested output dataset */

    /* METADATA */
    const char*     dataset_name;       /* example: "OUTDATA", "OUTSTATUS" */

    /* VARIABLE LIST 
        Similar to `DS_varlist` structure but implemented without a substructure
        Use the related-arrays below to define each output column */
    int             dynamic_var_count;  /* number of output variables */
    char**          dynamic_var_names;  /* names of variables */
    void**          dynamic_var_ptrs;   /* location in memory to retrieve variable values */
    IO_VAR_TYPE*    dynamic_var_types;  /* type of variable (character/numeric) */

    /* BY VARIABLE REFERENCE
        Records on output datasets sometimes relate to a specific "BY" group 
        in an input dataset.  This reference to an input dataset allows copying of
        its BY variable values each time a new row is added to this output dataset */
    DSR_generic* ref_by_vars;
} DSW_generic;

/* PARAMETER structures 
    User-provided parameters come packaged in a loosely-typed dictionary.  
    The following structures allow each parameter to be extracted, 
    validated, and its value accessed using generic functions.  */
typedef struct __PARM_generic {
    const char*         parm_id;        /* the "key" of the parameter in dictionary */
    IO_PARAM_TYPE       parm_type;      /* allowed data type */
    IO_SENTINAL_VALUE   is_specified;   /* flag: whether specified or not*/
} PARM_generic;

/* `UP_*` ("user parameter") structures for each datatype of parameter
    Combines the above `PARM_generic` structure
    with a type-specific `.value` member */
typedef struct __USER_PARM_FLAG {
    PARM_generic meta;
    int value;
} UP_FLAG;

typedef struct __USER_PARM_INT {
    PARM_generic meta;
    int value;
} UP_INT;

typedef struct __USER_PARM_NBR {
    PARM_generic meta;
    double value;
} UP_NBR;

typedef struct __USER_PARM_INTNBR {
    PARM_generic meta;
    double value;
} UP_INTNBR;

#define UP_NAME_LENGTH 8
typedef struct __USER_PARM_NAME {
    PARM_generic meta;
    char value[UP_NAME_LENGTH + 1];
} UP_NAME;

typedef struct __USER_PARM_QS {
    PARM_generic meta;
    const char* value;
} UP_QS;

/* SP_generic - "Statcan Procedure generic"
    Super structure for procedure specific `SP_<proc-name>` structures 
    This structure tracks all parameters, input datasets, and output datasets
    during the execution of a Banff procedure.  
    By maintaining pointers to each parameter and dataset's generic structure, 
    operations on the parameters or datasets can be implemented generically using 
    `SPG_*()` library functions */
typedef struct __SPG_STATCAN_PROCEDURE_GENERIC {
    _JS_T_          json_parameters;    /* JSON dictionary with parameter key-value pairs */
    int             count_parameters;   /* number of parameters in `.parameters` list*/
    PARM_generic**  parameters;         /* list of generic parameter structures */

    int count_dsr;                      /* number of input dataset readers */
    DSR_generic** dsr_list;             /* list of input dataset readers */

    int count_dsw;                      /* number of output dataset writers */
    DSW_generic** dsw_list;             /* list of output dataset writers */
} SP_generic ;


/* FUNCTIONS */

/* Dataset Reader functions */
IO_DATASET_RC   DSR_compare_rec_by_group(   DSR_generic* dsr_x, int index_x, DSR_generic* dsr_y, int index_y);
void            DSR_copy_reallocate(        DSR_generic* g_dst, char* name_dst);
IO_DATASET_RC   DSR_cursor_next_by(         DSR_generic* dsr);
IO_DATASET_RC   DSR_cursor_next_rec(        DSR_generic* dsr);
void            DSR_cursor_print_by_message(DSR_generic* dsr, const char* msg_prefix, int num_newlines);
IO_DATASET_RC   DSR_cursor_rewind(          DSR_generic* dsr, IO_CURSOR_ACTION action_info);
IO_DATASET_RC   DSR_cursor_set_by(          DSR_generic* dsr, int by_start);
IO_DATASET_RC   DSR_cursor_sync_by(         DSR_generic* dsr_primary, DSR_generic* dsr_secondary);
IO_RETURN_CODE  DSR_default_var_list(       DSR_generic* dsr, DS_varlist* vl, const char* varlist_name);
void            DSR_free(                   DSR_generic* dsr);
IO_VAR_TYPE     DSR_get_col_type(           DSR_generic* dsr, int position);
int             DSR_get_column_length(DSR_generic* dsr, int col_index);
IO_RETURN_CODE  DSR_get_pos_from_names(     DSR_generic* dsr, int num_vars, char** var_names, int* positions);
char*           DSR_get_varname_str(        DSR_generic* dsr, int num_variables, int* position_list);
IO_RETURN_CODE  DSR_init(                   SP_generic* spg, int index_to_add, DSR_generic* dsr, const char* dataset_name, T_in_ds in_ds_dataset, const char* unit_id, const char* by_var, IO_BOOLEAN by_var_optional, int child_vl_count, ...);
IO_RETURN_CODE  DSR_init__backend(          SP_generic* spg, int index_to_add, DSR_generic* dsr, const char* dataset_name, T_in_ds in_ds_dataset, const char* unit_id, const char* by_var, IO_BOOLEAN by_var_optional, int child_vl_count, va_list args);
IO_RETURN_CODE  DSR_init_simple(            SP_generic* spg, int index_to_add, DSR_generic* dsr, const char* dataset_name, T_in_ds in_ds_dataset, int child_vl_count, ...);
IO_RETURN_CODE  DSR_read_num(               DSR_generic* dsr, T_ds_size row_index, T_ds_size col_index, double* out_val);
IO_RETURN_CODE  DSR_rec_get(                DSR_generic* dsr);
IO_RETURN_CODE  DSR_validate_init(          DSR_generic* dsr, IO_RETURN_CODE rc_init, IO_BOOLEAN is_mandatory);
IO_BOOLEAN      DSR_var_exists(             DSR_generic* dsr, char* var_name);

/* Dataset Writer functions */
IO_RETURN_CODE  DSW_add_record(             DSW_generic* dsw);
IO_RETURN_CODE  DSW_add_record_by_vars(     DSW_generic* dsw);
IO_RETURN_CODE  DSW_allocate(               DSW_generic* dsw, int vars_to_allocate);
IO_RETURN_CODE  DSW_define_column(          DSW_generic* dsw, int destination_index, char* var_name, void* var_ptr, IO_VAR_TYPE var_type);
void            DSW_free(                   DSW_generic* dsw);
IO_DATASET_RC   DSW_init(                   SP_generic* spg, int index_to_add, DSW_generic* dsw, const char* dataset_name, void* out_sch, void* out_arr);
void            DSW_set_by_var_reference(   DSW_generic* dsw, DSR_generic* ref_ds);
IO_RETURN_CODE  DSW_start_appending(        DSW_generic* dsw);
IO_RETURN_CODE  DSW_wrap(                   DSW_generic* dsw);

/* Statcan Procedure Generic functions */
void            SPG_free(                   SP_generic* spg);
void            SPG_allocate(               SP_generic* spg, int count_parameters, int count_dsr, int count_dsw);
IO_RETURN_CODE  SPG_init(                   SP_generic* spg, const T_in_parm in_parms, int count_parameters, int count_dsr, int count_dsw);
IO_RETURN_CODE  SPG_validate(               SP_generic* spg);

/* User-Parameter functions */
IO_RETURN_CODE  PARM_init(                  SP_generic* spg, int index_to_add, PARM_generic* pg, const char* parm_id, IO_PARAM_TYPE parm_type);
IO_SENTINAL_VALUE PARM_is_specified(        _JS_T_ json_root, const char* jsk);
IO_RETURN_CODE  PARM_validate_type(         _JS_T_ json_root, PARM_generic* pg);

/* Variable List Functions */
void            VL_allocate(                DS_varlist* vl, int vars_to_allocate);
void            VL_copy_reallocate(         DS_varlist* vl, DSR_generic* dsr);
void            VL_free(                    DS_varlist* vl);
IO_RETURN_CODE  VL_init(                    DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type);
IO_RETURN_CODE  VL_init__backend(           DS_varlist* vl, DSR_generic* dsr, const char* varlist_name, const char* varlist_str, IO_VAR_TYPE var_type, IO_BOOLEAN print_errors);
IO_RETURN_CODE  VL_init_from_position_list( DS_varlist* vl, DSR_generic* dsr, const char* varlist_name,   IO_VAR_TYPE var_type, int num_variables, int* position_list);
IO_RETURN_CODE  VL_init_silent(             DS_varlist* vl, DSR_generic* dsr, const char* varlist_name,   const char* varlist_str, IO_VAR_TYPE var_type);
IO_RETURN_CODE  VL_init_single(             DS_varlist* vl, DSR_generic* dsr, const char* varlist_name,   const char* varlist_str, IO_VAR_TYPE var_type);
IO_BOOLEAN      VL_is_specified(            DS_varlist* vl);
IO_RETURN_CODE  VL_read(                    DS_varlist* vl);
IO_RETURN_CODE  VL_read_char(               DS_varlist* vl);
IO_RETURN_CODE  VL_read_num(                DS_varlist* vl);
IO_RETURN_CODE  VL_validate_type(           DS_varlist* vl);

/* the following includes placed here because they reference things defined earlier in the file */
#include "IO_Messages.h"
#include "ValueHelpers.h"

#endif