#ifndef _IO_ARROW_H_
#define _IO_ARROW_H_

#include "nanoarrow/nanoarrow.h"  // need in `.h` for arrow types

typedef enum __ARC_ARROW_RETURN_CODE {
    ARC_SUCCESS,  // MUST == 0
    ARC_DATASET_MISSING,
    ARC_FAILURE,
    ARC_INVALID_TYPE,
    ARC_OUT_OF_BOUNDS,
    ARC_INVALID_DS_REF,
    ARC_FAIL_DS_INIT,
    ARC_FAIL_DS_WRITE,
} ARROW_RETURN_CODE;

/* array_chunk
    linked list wrapper for `struct ArrowArray` returned by `struct ArrowArrayStream.getNext()`

    Conventions:
        list head has `.previous_chunk == NULL`
        list tail has `.next_chunk == NULL` */
typedef struct __array_chunk array_chunk;
struct __array_chunk {
    struct ArrowArray chunk;
    struct ArrowArrayView view;         /* associated with `.chunk` */
    int64_t index_offset;               /* index 0 in `chunk` matches `index_offset` in parent dataset */
    array_chunk* next_chunk;
    array_chunk* previous_chunk;
};

typedef struct __arrow_dataset_in {
    struct ArrowArrayStream* stream;
    struct ArrowSchema schema;
    array_chunk* ac_list;               /* list of wrapped Arrow Arrays, received from `.stream` */
    int64_t num_records;
} arrow_dataset_in;

typedef struct __arrow_dataset_out {
    struct ArrowSchema* schema;
    struct ArrowArray* data;
} arrow_dataset_out;

/* in-house typedef for arrow input datatype */
typedef void* arrow_in_type;
typedef void* arrow_out_type;
typedef int64_t arrow_dataset_size_type;

/* 'arrow_dataset_in' (`ADI_`) functions */
ARROW_RETURN_CODE   ADI_init(               arrow_dataset_in* ad, arrow_in_type arrow_dataset);
void                ADI_free(               arrow_dataset_in* ad);
array_chunk*        ADI_get_chunk(          arrow_dataset_in* ad, int64_t index_ds);

/* 'arrow_dataset_out' (`ADO_`) functions */
ARROW_RETURN_CODE   ADO_allocate(           arrow_dataset_out* ado, int num_columns);
ARROW_RETURN_CODE   ADO_append_missing(     arrow_dataset_out* ado, int dest_index);
ARROW_RETURN_CODE   ADO_begin_building(     arrow_dataset_out* ado);
ARROW_RETURN_CODE   ADO_define_column(      arrow_dataset_out* ado, int dest_index, const char* column_name, enum ArrowType type_);
ARROW_RETURN_CODE   ADO_define_column_copy( arrow_dataset_out* ado, int dest_index, arrow_dataset_in* adi, int src_index);
ARROW_RETURN_CODE   ADO_finish_building(    arrow_dataset_out* ado);
ARROW_RETURN_CODE   ADO_init(               arrow_dataset_out* ado, struct ArrowSchema* schema, struct ArrowArray* data);

/* 'array_chunk' (`AC_`) functions */
void                AC_free_recursive(      array_chunk* ac);
ARROW_RETURN_CODE   AC_read_char(           array_chunk* ac, arrow_dataset_size_type row_index, arrow_dataset_size_type col_index, char* dest);
ARROW_RETURN_CODE   AC_read_num(            array_chunk* ac, arrow_dataset_size_type row_index, arrow_dataset_size_type col_index, double* dest);

/* error handling */
ARROW_RETURN_CODE   arrow_check_ref_adi(arrow_dataset_in* adi);
ARROW_RETURN_CODE   arrow_check_ref_ado(arrow_dataset_out* ado);
ARROW_RETURN_CODE   arrow_check_ref_ac(array_chunk* ac);
ARROW_RETURN_CODE   arrow_check_ref_column(int64_t ref_index, int64_t num_columns);
ARROW_RETURN_CODE   arrow_check_ref_row(int64_t ref_index, int64_t num_rows);
ARROW_RETURN_CODE   arrow_had_error(ArrowErrorCode rc_arrow);
void                arrow_print_error(struct ArrowError* arrow_error, const char* custom_message);

/* type checking*/
int                 arrow_is_floating_point(enum ArrowType var_type);
int                 arrow_is_int_like(      enum ArrowType var_type);
int                 arrow_is_numeric(       enum ArrowType var_type);
int                 arrow_is_signed_int(    enum ArrowType var_type);
int                 arrow_is_string(        enum ArrowType var_type);
int                 arrow_is_unsigned_int(  enum ArrowType var_type);

/* Array View (`av_`) functions
    These operating on Arrow structures, not in-house structures.  */
ARROW_RETURN_CODE   av_get_double_safe(     struct ArrowArrayView* av, int64_t index_av, double* out);
double              av_get_double_unsafe(   struct ArrowArrayView* av, int64_t index_av);
ARROW_RETURN_CODE   av_get_int_safe(        struct ArrowArrayView* av, int64_t index_av, int* out);
int                 av_get_int_unsafe(      struct ArrowArrayView* av, int64_t index_av);

#endif