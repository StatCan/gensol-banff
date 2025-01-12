#include "IO_Arrow.h"

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "STC_Memory.h"
#include "IO_Messages.h"

#include <stdio.h>  // for `printf()`
#include "PlatformSupport.h"  // platform specific string functions (memccpy)

/* ADI_init
    Casts and stores Arrow array stream, initializes arrow input dataset.

    From stream, gets dataset's schema and data, saving in `ad`
        - one schema contains metadata about all data
        - data comes in 1 or more "chunks"
            - loads all chunks into linked list of `array_chunk`
    See https://arrow.apache.org/docs/format/CStreamInterface.html */
ARROW_RETURN_CODE ADI_init(arrow_dataset_in* ad, arrow_in_type arrow_dataset) {
    /* cast calling-application-provided pointer to ArrowArrayStream */
    if (arrow_dataset == NULL) {
        return ARC_DATASET_MISSING;
    }
    ad->stream = (struct ArrowArrayStream*)arrow_dataset;
    if (ad->stream == NULL) {
        return ARC_FAILURE;
    }

    struct ArrowError arrow_error;
    ArrowErrorCode rc_arrow = NANOARROW_OK;

    /* get schema */
    rc_arrow = ArrowArrayStreamGetSchema(ad->stream, &ad->schema, &arrow_error);
    if (arrow_had_error(rc_arrow)) {
        arrow_print_error(&arrow_error, NULL);
        return ARC_FAILURE;
    }

    /* get all array chunks 
        builds a link list of array chunks */
    array_chunk* ac_last = NULL;  // for setting each list member's `.previous_chunk`
    ad->num_records = 0;  // track total number of records (sum of records in each chunk)
    ad->ac_list = NULL;  // initialize list to NULL
    array_chunk chunk_in;  // temporary, used for loading each chunk
    while (
        (rc_arrow = ArrowArrayStreamGetNext(ad->stream, &chunk_in.chunk, &arrow_error)) == NANOARROW_OK
        && chunk_in.chunk.release != NULL  /* `chunk_in.release == NULL` indicates that no more chunks available  */
    ) {
        // set chunk's index_offset: tracks which overall index this chunk's index 0 corresponds to
        chunk_in.index_offset = ad->num_records;
        ad->num_records += chunk_in.chunk.length;

        /* initialize and set array view
            `..InitFromSchema` also initializes `.children` with a call to
            `ArrowArrayViewAllocateChildren()`, so don't call that here */
        rc_arrow = ArrowArrayViewInitFromSchema(&chunk_in.view, &ad->schema, &arrow_error);
        if (arrow_had_error(rc_arrow)) {
            arrow_print_error(&arrow_error, NULL);
            return ARC_FAILURE;
        }
        rc_arrow = ArrowArrayViewSetArray(&chunk_in.view, &chunk_in.chunk, &arrow_error);
        if (arrow_had_error(rc_arrow)) {
            arrow_print_error(&arrow_error, NULL);
            ArrowArrayViewReset(&chunk_in.view);
            return ARC_FAILURE;
        }

        // set linked-list pointers
        chunk_in.next_chunk = NULL;
        chunk_in.previous_chunk = ac_last;

        // make copy of structure
        array_chunk* chunk_save = (array_chunk*)STC_AllocateMemory(sizeof(*chunk_save));
        if (chunk_save == NULL) {
            return ARC_FAILURE;
        }
        *chunk_save = chunk_in;  // this method of copying is safe in this case

        // add copy to list
        if (ac_last == NULL) {  // first chunk: becomes list's head
            ad->ac_list = chunk_save;
        }
        else {  // subsequent chunks: add to list's tail
            ac_last->next_chunk = chunk_save;
        }

        // retain last added chunk for next iteration
        ac_last = chunk_save;
    }
    if (arrow_had_error(rc_arrow)) {  // check for errors from `ArrowArrayStreamGetNext()`
        arrow_print_error(&arrow_error, NULL);
        return ARC_FAILURE;
    }

    if (ad->ac_list == NULL) {
        // dataset is empty, has 0 chunks, that's okay
        return ARC_SUCCESS;
    }

    return ARC_SUCCESS;
}

/* ADI_free
    Free Arrow C Stream
        Using streams, the schema and each chunk must each be released "independently".
            https://arrow.apache.org/docs/format/CStreamInterface.html#result-lifetimes
        The stream must also be freed.
            https://arrow.apache.org/docs/format/CStreamInterface.html#stream-lifetime
            https://arrow.apache.org/docs/format/CDataInterface.html#release-callback-semantics-for-consumers */
void ADI_free(arrow_dataset_in* ad) {
    AC_free_recursive(ad->ac_list);
    ad->ac_list = NULL;

    ArrowSchemaRelease(&ad->schema); // alternatively `ad->schema.release(&ad->schema);`

    ad->stream->release(ad->stream);
    ad->stream = NULL;
}

/* ADI_get_chunk
    returns the array chunk which contains record at `index_ds`
        `index_ds`      - index of record in dataset (`dsr`)

    `arrow_dataset.ac_list` contains a linked list of 1 or more
    "array chunks", each containing a subset of the dataset.  */
array_chunk* ADI_get_chunk(arrow_dataset_in* ad, int64_t index_ds) {
    if (index_ds >= ad->num_records) {
        // invalid index
        return NULL;
    }

    array_chunk* ac = ad->ac_list;
    if (ac == NULL) {
        return NULL;
    }

    /* walk array_chunk list until chunk contains `rec_index` */
    while (ac != NULL
        && index_ds >= (ac->index_offset + ac->view.length)) {
        ac = ac->next_chunk;
    }

    return ac;
}

/* ADO_allocate
    allocate columns on output dataset
    Must have previously called `ADO_init()` on `ado`.  */
ARROW_RETURN_CODE ADO_allocate(arrow_dataset_out* ado, int num_columns) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // allocate
    if (arrow_had_error(ArrowSchemaAllocateChildren(ado->schema, num_columns))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    return ARC_SUCCESS;
}

/* ADO_append_missing
    append MISSING value to output dataset column
    Works with any type of column (i.e. numeric or character data).  */
ARROW_RETURN_CODE ADO_append_missing(arrow_dataset_out* ado, int dest_index) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_column(dest_index, ado->schema->n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // append missing value
    if (arrow_had_error(ArrowArrayAppendNull(ado->data->children[dest_index], 1))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_WRITE);
        return ARC_FAIL_DS_WRITE;
    }

    return ARC_SUCCESS;
}

/* ADO_begin_building
    Finish initialization of dataset, allow appending data.  

    Only valid following initialization and allocation of the schema
    and definition of columns.  
    Must call before appending any data.  */
ARROW_RETURN_CODE ADO_begin_building(arrow_dataset_out* ado) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // begin building
    struct ArrowError arrow_error;
    if (arrow_had_error(ArrowArrayInitFromSchema(ado->data, ado->schema, &arrow_error))) {
        arrow_print_error(&arrow_error, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    if (arrow_had_error(ArrowArrayStartAppending(ado->data))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    return ARC_SUCCESS;
}

/* ADO_define_column
    set a column's type and name
    `dest_index` must be in allocated range.  */
ARROW_RETURN_CODE ADO_define_column(arrow_dataset_out* ado, int dest_index, const char* column_name, enum ArrowType type_) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_column(dest_index, ado->schema->n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // define column
    if (arrow_had_error(ArrowSchemaInitFromType(ado->schema->children[dest_index], type_))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    if (arrow_had_error(ArrowSchemaSetName(ado->schema->children[dest_index], column_name))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    return ARC_SUCCESS;
}

/* ADO_define_column_copy
    copy a column's type and name from an input dataset */
ARROW_RETURN_CODE ADO_define_column_copy(arrow_dataset_out* ado, int dest_index, arrow_dataset_in* adi, int src_index) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_column(dest_index, ado->schema->n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_adi(adi);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_row(src_index, adi->schema.n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // get schema view from source dataset column
    struct ArrowSchemaView sv = { 0 };
    struct ArrowError arrow_error = { 0 };
    if (arrow_had_error(ArrowSchemaViewInit(&sv, adi->schema.children[src_index], &arrow_error))) {
        arrow_print_error(&arrow_error, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    // set destination column type
    ARROW_RETURN_CODE rc_arrow = ADO_define_column(
        ado,
        dest_index,
        adi->schema.children[src_index]->name,
        sv.storage_type
    );


    if (rc_arrow != ARC_SUCCESS) {
        return rc_arrow;
    }

    return ARC_SUCCESS;
}

/* ADO_finish_building
    to be called after final row added to output dataset, before returning to calling application */
ARROW_RETURN_CODE ADO_finish_building(arrow_dataset_out* ado) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ado(ado);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // finish building
    struct ArrowError arrow_error = { 0 };
    if (arrow_had_error(ArrowArrayFinishBuildingDefault(ado->data, &arrow_error))) {
        arrow_print_error(&arrow_error, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    return ARC_SUCCESS;
}

/* ADO_init
    initialize output dataset
    `NANOARROW_TYPE_STRUCT` format allows multiple columns, each with its own type.  
    We initialize the schema with that type but the number of columns not yet known.  
    Once known, use `AOD_allocate()` for allocation of columns.  */
ARROW_RETURN_CODE ADO_init(arrow_dataset_out* ado, struct ArrowSchema* schema, struct ArrowArray* data) {
    // validate parameters
    if (ado == NULL) {
        return ARC_FAIL_DS_INIT;
    }

    // NULL for schema or data indicates dataset not requested
    if (schema == NULL || data == NULL) {
        ado->schema = NULL;
        ado->data = NULL;
        return ARC_SUCCESS;
    }

    ado->schema = schema;
    ado->data = data;

    // set type to `_STRUCT` but don't allocate children (i.e. don't use `ArrowSchemaSetTypeStruct()`)
    if (arrow_had_error(ArrowSchemaInitFromType(ado->schema, NANOARROW_TYPE_STRUCT))) {
        arrow_print_error(NULL, MSG_ARROW_FAIL_DS_INIT);
        return ARC_FAIL_DS_INIT;
    }

    return ARC_SUCCESS;
}

/* AC_free_recursive
    recursive function for freeing `arrow_dataset.ac_list` */
void AC_free_recursive(array_chunk* ac) {
    if (ac == NULL) {
        return;
    }

    // free following chunk first
    if (ac->next_chunk != NULL) {
        AC_free_recursive(ac->next_chunk);
        ac->next_chunk = NULL;
    }

    // safe release
    if (ac->chunk.release != NULL) {
        ac->chunk.release(&ac->chunk);
    }

    // free array view
    ArrowArrayViewReset(&ac->view);

    STC_FreeMemory(ac);
}

/* AC_read_char
    Read a string ("character") value from an array chunk (AC)

    Given an array chunk, row and column indices, and a destination,
    populates destination with the associated string value.
    Sets to '\0' if value is null (missing).  

    Returns ARC_FAILURE if indices correspond to non-string value.

    WARNING: `row_index` refers to the row in the array chunk, `ac`, which
             may differ from the index in the overall dataset 
*/
ARROW_RETURN_CODE AC_read_char(array_chunk* ac, arrow_dataset_size_type row_index, arrow_dataset_size_type col_index, char* dest) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ac(ac);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_column(col_index, ac->view.n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    struct ArrowArrayView* av = ac->view.children[col_index];


    rc_validate = arrow_check_ref_row(row_index, av->length);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // validate column type
    if (!arrow_is_string(av->storage_type)) {
        return ARC_INVALID_TYPE;
    }

    // read value
    if (ArrowArrayViewIsNull(av, row_index)) {
        dest[0] = '\0';
    }
    else {
        struct ArrowStringView asv = ArrowArrayViewGetStringUnsafe(av, row_index);
        size_t src_len = asv.size_bytes;
        memccpy(dest, asv.data, '\0', asv.size_bytes);
        dest[src_len] = '\0';
    }

    return ARC_SUCCESS;
}


/* AC_read_num
    Read a numeric value from an array chunk (AC)

    Given an array chunk, row and column indices, and a destination,
    populates destination with the associated numeric value as a `double`, 
    casting from any non-double numeric format.  
    Sets to 'NAN' if value is null (missing).

    Returns ARC_FAILURE if indices correspond to non-numeric value.

    WARNING: `row_index` refers to the row in the array chunk, `ac`, which
             may differ from the index in the overall dataset
*/
ARROW_RETURN_CODE AC_read_num(array_chunk* ac, arrow_dataset_size_type row_index, arrow_dataset_size_type col_index, double* dest) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_ac(ac);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }
    rc_validate = arrow_check_ref_column(col_index, ac->view.n_children);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    struct ArrowArrayView* av = ac->view.children[col_index];
    
    rc_validate = arrow_check_ref_row(row_index, av->length);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // read value
    if (ArrowArrayViewIsNull(av, row_index)) {
        *dest = NAN;
    }
    else {
        ARROW_RETURN_CODE rc = av_get_double_safe(av, row_index, dest);
        if (rc != ARC_SUCCESS) {
            return rc;
        }
    }

    return ARC_SUCCESS;
}

/* arrow_check_ref_adi
    Check that arrow_dataset_in pointer is not NULL*/
ARROW_RETURN_CODE arrow_check_ref_adi(arrow_dataset_in* adi) {
    if (adi == NULL) {
        arrow_print_error(NULL, MSG_ARROW_INVALID_REF_DS);
        return ARC_INVALID_DS_REF;
    }

    return ARC_SUCCESS;
}

/* arrow_check_ref_ado
    Check that arrow_dataset_out pointer is not NULL*/
ARROW_RETURN_CODE arrow_check_ref_ado(arrow_dataset_out* ado) {
    if (ado == NULL || ado->data == NULL || ado->schema == NULL) {
        arrow_print_error(NULL, MSG_ARROW_INVALID_REF_DS);
        return ARC_INVALID_DS_REF;
    }

    return ARC_SUCCESS;
}

/* arrow_check_ref_ac
    Check that array_chunk pointer is not NULL*/
ARROW_RETURN_CODE arrow_check_ref_ac(array_chunk* ac) {
    if (ac == NULL) {
        arrow_print_error(NULL, MSG_ARROW_INVALID_REF_DS);
        return ARC_INVALID_DS_REF;
    }

    return ARC_SUCCESS;
}

/* arrow_check_ref_column
    Check that ref_index is not out of bounds*/
ARROW_RETURN_CODE arrow_check_ref_column(int64_t ref_index, int64_t num_columns) {
    if (ref_index < 0 || ref_index >= num_columns) {
        arrow_print_error(NULL, MSG_ARROW_OOB_REF_COL);
        return ARC_OUT_OF_BOUNDS;
    }

    return ARC_SUCCESS;
}


/* arrow_check_ref_row
    Check that ref_index is not out of bounds*/
ARROW_RETURN_CODE arrow_check_ref_row(int64_t ref_index, int64_t num_rows) {
    if (ref_index < 0 || ref_index >= num_rows) {
        arrow_print_error(NULL, MSG_ARROW_OOB_REF_ROW);
        return ARC_OUT_OF_BOUNDS;
    }

    return ARC_SUCCESS;
}

/* arrow_had_error
    Check if an error occurred.
    If error occurred, returns `ARC_SUCCESS.
    Otherwise, returns `ARC_FAILURE`.

    Usage: following a call to a relevant Arrow function
    ```
        if (arrow_had_error(...)){
            //handle error
        }
    ```
    */
ARROW_RETURN_CODE arrow_had_error(ArrowErrorCode rc_arrow) {
    if (rc_arrow == NANOARROW_OK) {
        return ARC_SUCCESS;
    }
    else {
        return ARC_FAILURE;
    }
}

/* arrow_print_error
    print an error message related to the nanoarrow library

    If `arrow_error` is non-null, print any message it contains.  
    Otherwise, prints a generic error message.  

    Usage: following a call to a relevant Arrow function
    ```
        if (arrow_had_error(...)){
            arrow_print_error(...);
        }
    ```
    */
void arrow_print_error(struct ArrowError* arrow_error, const char* custom_message) {
    printf(MSG_PREFIX_ERROR);

    if (custom_message != NULL) {
        printf(custom_message);
    } else {
        printf(MSG_ARROW_ERROR_MESSAGE);
    }

    const char* arrow_message = ArrowErrorMessage(arrow_error);
    if (strlen(arrow_message) > 0) {
        printf(" (%s)", arrow_message);
    }

    printf("\n");
}

/* arrow_is_floating_point
    returns "true" if type is any size (precision) of floating point number */
int arrow_is_floating_point(enum ArrowType var_type) {
    switch (var_type) {
    case NANOARROW_TYPE_DOUBLE:
    case NANOARROW_TYPE_FLOAT:
    case NANOARROW_TYPE_HALF_FLOAT:
        return 1 == 1; // "true"
    default:
        return 1 == 0; // "false"
    }
}

/* arrow_is_int_like
    returns "true" if type is any size of signed or unsigned integer */
int arrow_is_int_like(enum ArrowType var_type) {
    if (arrow_is_signed_int(var_type) || arrow_is_unsigned_int(var_type)) {
        return 1 == 1; // "true"
    }
    else {
        return 1 == 0; // "false"
    }
}

/* arrow_is_numeric
    returns "true" if type is any size of floating point or signed/unsigned integer */
int arrow_is_numeric(enum ArrowType var_type) {
    if (
        arrow_is_floating_point(var_type)
        || arrow_is_signed_int(var_type)
        || arrow_is_unsigned_int(var_type)
    ) {
        return 1 == 1; // true
    }
    else {
        return 1 == 0; // false
    }
}

/* arrow_is_signed_int
    returns "true" if type is any size of signed integer */
int arrow_is_signed_int(enum ArrowType var_type) {
    switch (var_type) {
    case NANOARROW_TYPE_INT8:
    case NANOARROW_TYPE_INT16:
    case NANOARROW_TYPE_INT32:
    case NANOARROW_TYPE_INT64:
        return 1 == 1; // "true"
    default:
        return 1 == 0; // "false"
    }
}

/* arrow_is_string
    returns "true" if type is NANOARROW_TYPE_STRING */
int arrow_is_string(enum ArrowType var_type) {
    if (var_type == NANOARROW_TYPE_STRING
        || var_type == NANOARROW_TYPE_LARGE_STRING) {
        return 1 == 1; // "true"
    }
    else {
        return 1 == 0; // "false"
    }
}

/* arrow_is_unsigned_int
    returns "true" if type is any size of unsigned integer */
int arrow_is_unsigned_int(enum ArrowType var_type) {
    switch (var_type) {
    case NANOARROW_TYPE_UINT8:
    case NANOARROW_TYPE_UINT16:
    case NANOARROW_TYPE_UINT32:
    case NANOARROW_TYPE_UINT64:
        return 1 == 1; // "true"
    default:
        return 1 == 0; // "false"
    }
}

/* av_get_double_safe
    Get numeric value casted to double.
    Considered "safe" because it returns error code `ARC_FAILURE` if value is not numeric.

    WARNING: `index_av` is index of record in `av`, which may 
             differ from the index in the overall dataset) */
ARROW_RETURN_CODE av_get_double_safe(struct ArrowArrayView* av, int64_t index_av, double* out) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_row(index_av, av->length);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // read value
    if (arrow_is_unsigned_int(av->storage_type)) {
        *out = (double)ArrowArrayViewGetUIntUnsafe(av, index_av);
    }
    else if (arrow_is_signed_int(av->storage_type)) {
        *out = (double)ArrowArrayViewGetIntUnsafe(av, index_av);
    }
    else if (arrow_is_floating_point(av->storage_type)) {
        *out = (double)ArrowArrayViewGetDoubleUnsafe(av, index_av);
    }
    else {
        return ARC_INVALID_TYPE;
    }

    return ARC_SUCCESS;
}

/* av_get_double_unsafe
    Returns numeric value casted to double.
    "_unsafe": may an arbitrary default value
        `index_av`  - index of record in `av` (not in overall dataset) */
double av_get_double_unsafe(struct ArrowArrayView* av, int64_t index_av) {
    double out = NAN;
    if (av_get_double_safe(av, index_av, &out) == ARC_SUCCESS) {
        return out;
    }
    else {
        // ignore error, return this value
        return NAN;
    }
}

/* av_get_int_safe
    Get numeric value casted to integer.
    Considered "safe" because it returns error code `ARC_FAILURE` if value is not numeric.

    WARNING: `index_av` is index of record in `av`, which may 
             differ from the index in the overall dataset) */
ARROW_RETURN_CODE av_get_int_safe(struct ArrowArrayView* av, int64_t index_av, int* out) {
    // validate parameters
    ARROW_RETURN_CODE rc_validate = arrow_check_ref_row(index_av, av->length);
    if (rc_validate != ARC_SUCCESS) {
        return rc_validate;
    }

    // read value
    if (arrow_is_unsigned_int(av->storage_type)) {
        *out = (int)ArrowArrayViewGetUIntUnsafe(av, index_av);
    }
    else if (arrow_is_signed_int(av->storage_type)) {
        *out = (int)ArrowArrayViewGetIntUnsafe(av, index_av);
    }
    else if (arrow_is_floating_point(av->storage_type)) {
        *out = (int)ArrowArrayViewGetDoubleUnsafe(av, index_av);
    }
    else {
        return ARC_INVALID_TYPE;
    }

    return ARC_SUCCESS;
}

/* av_get_int_unsafe
    Returns numeric value casted to int.
    "_unsafe: may an arbitrary default value
        `index_av`  - index of record in `av` (not in overall dataset) */
int av_get_int_unsafe(struct ArrowArrayView* av, int64_t index_av) {
    int out = -1;
    if (av_get_int_safe(av, index_av, &out) == ARC_SUCCESS) {
        return out;
    }
    else {
        // ignore error, return this value
        return -1;
    }
}