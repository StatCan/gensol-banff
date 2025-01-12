#include "IO_Messages.h"

#include <stdio.h>

/* IOM_print_rec_by_group
    For printing messages which identify a specific BY group.  

    Given a dataset (`dsr`) and index, prints a `pre_message` 
    followed by the name and value of each BY variable.  
    
    Example: JMC_print_rec_by_group(some_dsr, 0, "NOTE: By values are: ", "", ", ", 1); prints
        "NOTE: By values are: <key1>=<value1>, <key2>=<value2>
        "

    `dsr`               - the dataset the record is contained in
    `index_ds`          - index of the record in the dataset
    `kv_spaces`         - determines what to print between the <key>, equal sign (=), and <value>
                          Example: " " for "<key> = <value>", "" for "<key>=<value>"
    `pair_separator`    - printed between each BY variable
                          Example ", " for "<key1>=<value1>, <key2>=<value2>"
    `num_newlines`      - number of newlines to print following the last key-value pair */
void IOM_print_rec_by_group(DSR_generic* dsr, int index_ds, const char* pre_message, const char* kv_spaces, const char* pair_separator, int num_newlines) {
    array_chunk* ac = ADI_get_chunk(&dsr->ad, index_ds);
    if (ac == NULL){
        return;
    }
    int64_t index_av = index_ds - ac->index_offset;

    if (pre_message != NULL) {
        printf("%s", pre_message);
    }

    for (int i = 0; i < dsr->VL_by_var.count; i++) {
        int64_t col_index = dsr->VL_by_var.positions[i];
        struct ArrowArrayView* av = ac->view.children[col_index];

        if (i > 0) {
            printf("%s", pair_separator);
        }

        printf("%s%s=%s", dsr->VL_by_var.names[i], kv_spaces, kv_spaces);

        if (ArrowArrayViewIsNull(av, index_av)) {
            printf("%s", JUM_MISSING);
        }
        else {
            if (arrow_is_string(av->storage_type)) {
                struct ArrowStringView asv = ArrowArrayViewGetStringUnsafe(av, index_av);
                printf("%.*s", (int)asv.size_bytes, asv.data);
            }
            else if (arrow_is_floating_point(av->storage_type)) {
                printf("%.6f", av_get_double_unsafe(av, index_av));
            }
            else if (arrow_is_int_like(av->storage_type)) {
                printf("%d", av_get_int_unsafe(av, index_av));
            }
        }
    }

    for (; num_newlines > 0; num_newlines--) {
        printf("\n");
    }
}

/* IOM_print_data_not_sorted
    print error message demonstrating the BY groups are not sorted in ascending order 
    for records at `index_i` and i+1 */
void IOM_print_data_not_sorted(DSR_generic* dsr, int index_ds) {
    // print initial error message
    printf(MSG_PREFIX_ERROR JUM_DATA_NOT_SORTED_P1, dsr->dataset_name);

    // if index safe, print BY value details
    if (0 <= index_ds
        && index_ds + 1 < dsr->num_records) 
    {
        IOM_print_rec_by_group(dsr, index_ds, JUM_DATA_NOT_SORTED_P2, " ", " ", 0);
        IOM_print_rec_by_group(dsr, index_ds + 1, JUM_DATA_NOT_SORTED_P3, " ", " ", 0);
        printf(".");
    }
    printf("\n");
}