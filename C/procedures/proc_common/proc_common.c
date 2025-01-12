#include "proc_common.h"

#include "PlatformSupport.h"


/* Statcan Procedurefunctions
    `SP_*` functions are procedure-specific */
    /* SP_validate_init
        generic function validates return code from procedure-specific `SP_init()` function */
BANFF_RETURN_CODE SP_validate_init(IO_RETURN_CODE init_rc) {
    if (init_rc == IORC_SUCCESS) {
        return BRC_SUCCESS;
    }

    if (init_rc == IORC_VARLIST_NOT_FOUND
        || init_rc == IORC_BY_VARLIST_NOT_FOUND) {
        return BRC_FAIL_VARLIST_NOT_FOUND;
    }
    else if (init_rc == IORC_VAR_NAME_TOO_LONG) {
        return BRC_FAIL_NAME_TOO_LONG;
    }
    else if (init_rc == IORC_DATASET_MISSING) {
        return BRC_FAIL_MISSING_DATASET;
    }
    else if (init_rc == IORC_FAIL_INIT_IN_DATASET) {
        return BRC_FAIL_INIT_IN_DATASET;
    }
    else if (init_rc == IORC_FAIL_INIT_OUT_DATASET) {
        return BRC_FAIL_WRITE_GENERIC;
    }
    else if (init_rc == IORC_VARLIST_INVALID_COUNT) {
        return BRC_FAIL_VARLIST_INVALID_COUNT;
    }

    return BRC_FAIL_UNHANDLED;
}

/* convert_processing_rc
    This will probably be renamed and moved elsewhere, but here's the initial implementation */
BANFF_RETURN_CODE convert_processing_rc(PROC_RETURN_CODE rc) {
    switch (rc) {
        // success
    case PRC_SUCCESS: // fall through
    case PRC_SUCCESS_NO_MORE_DATA:
        return BRC_SUCCESS;

        // sort order
    case PRC_FAIL_REC_NOT_SORTED: // fall through
    case PRC_FAIL_BY_NOT_SORTED:
        return BRC_FAIL_WRONG_SORT_ORDER;

        // dataset syncronization 
    case PRC_FAIL_SYNC_GENERIC: // fall through
    case PRC_FAIL_SYNC_NOT_SORTED:
        return BRC_FAIL_READ_SYNC;

        // dataset reading
    case PRC_FAIL_GET_REC: // fall through
    case PRC_FAIL_ADVANCE_REC: // fall through
    case PRC_FAIL_ADVANCE_BY:
        return BRC_FAIL_READ_GENERIC;

        // duplicate data
    case PRC_FAIL_DUPLICATE_REC:
        return BRC_FAIL_READ_DUPLICATE_DATA;

        // processing error (all fall through)
    case PRC_FAIL_PROCESSING_GENERIC:
    case PRC_FAIL_DETERMINISTIC:
    case PRC_FAIL_DONORIMP_RANDNUMVAR:
    case PRC_FAIL_ERRORLOC_DO_LOCALIZATION:
    case PRC_FAIL_ERRORLOC_BOUNDS:
    case PRC_FAIL_ERRORLOC_RANDOM_NUM:
    case PRC_FAIL_ESTIMATO_INVALID_WEIGHT_VARIANCE:
    case PRC_FAIL_ESTIMATO_COMPUTE_BETAS:
    case PRC_FAIL_ESTIMATO_DO_ESTIMATION:
    case PRC_FAIL_PRORATE_RANKING:
        return BRC_FAIL_PROCESSING_GENERIC;
    
        // writing data
    case PRC_FAIL_WRITE_DATA:
        return BRC_FAIL_WRITE_GENERIC;

        // unexpected error
    case PRC_FAIL_UNHANDLED: // fall through
    default:
        return BRC_FAIL_UNHANDLED;
    }
}

/* banff_init_dsr_instatus
    Initialize the standard `DSR_instatus` structure */
IO_RETURN_CODE banff_init_dsr_instatus(
    SP_generic* spg,
    int index_spg,
    DSR_instatus* dsr,
    T_in_ds in_ds_instatus,
    const char* unit_id_value,
    const char* by_value
    )
{
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    rc_io = DSR_init(spg, index_spg,
        &dsr->dsr, DSN_INSTATUS, in_ds_instatus, unit_id_value, by_value,
        IOB_TRUE,
        2,
        &dsr->VL_field_id,
        &dsr->VL_status
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists */
    rc_io = VL_init(&dsr->VL_field_id, &dsr->dsr, VL_NAME_instatus, DSC_FIELD_ID, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_status, &dsr->dsr, VL_NAME_instatus, DSC_STATUS, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    return IORC_SUCCESS;
}

/* banff_setup_dsr_instatus
    Set pointers for standard instatus dataset */
void banff_setup_dsr_instatus(
    DSR_instatus* dsr,
    void* unit_id_ptr,
    void* field_id_ptr,
    void* status_ptr)
{
    dsr->dsr.VL_unit_id.ptrs[0] = unit_id_ptr;
    dsr->VL_field_id.ptrs[0] = field_id_ptr;
    dsr->VL_status.ptrs[0] = status_ptr;
}

/* banff_setup_dsw_outstatus
    Setup standard outstatus dataset.  
    Allocates DSW_generic, defines columns.  */
PROC_RETURN_CODE banff_setup_dsw_outstatus(
    DSW_generic* dsw,
    char* unit_id_name,
    void* unit_id_ptr,
    void* field_id_ptr,
    void* status_ptr,
    void* field_value_ptr
) {
    int var_added = 0;

    if (IORC_SUCCESS != DSW_allocate(dsw, NB_VARS_OUTSTATUS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, unit_id_name, unit_id_ptr, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DSC_FIELD_ID, field_id_ptr, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DSC_STATUS, status_ptr, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DSC_FIELD_VALUE, field_value_ptr, IOVT_NUM)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

static UINT saved_codepage;

void deinit_runtime_env() {
    // restore original console encoding
    set_console_output_encoding(saved_codepage);

    /* Flush standard output: otherwise, it may become mixed with
        output from other calls/programs */
    flush_std_buffers();
}

void init_runtime_env() {
    // set console encoding to UTF-8
    saved_codepage = set_console_output_encoding(UTF8_ENCODING_ID);
	init_debug_env("BANFF_DEBUG_STATS");

#ifdef _OPENMP
    omp_set_dynamic(1 == 1); /* TRUE: enable dynamic threads */
#endif
}

EXPORTED_FUNCTION const char* get_rc_description(BANFF_RETURN_CODE brc) {
    switch (brc) {
    case BRC_SUCCESS:
        return RC_DESC_SUCCESS;
        break;
    case BRC_FAIL_UNHANDLED:
        return RC_DESC_FAIL_UNHANDLED;
        break;

    // setup related errors
    case BRC_FAIL_INIT_IN_DATASET:
        return RC_DESC_FAIL_INIT_IN_DATASET;
        break;
    case BRC_FAIL_READ_PARMS_LEGACY:
        return RC_DESC_FAIL_READ_PARMS_LEGACY;
        break;
    case BRC_FAIL_MISSING_DATASET:
        return RC_DESC_FAIL_MISSING_DATASET;
        break;
    case BRC_FAIL_VARLIST_NOT_FOUND:
        return RC_DESC_FAIL_VARLIST_NOT_FOUND;
        break;
    case BRC_FAIL_VARLIST_INVALID_COUNT:
        return RC_DESC_FAIL_VARLIST_INVALID_COUNT;
        break;
    case BRC_FAIL_VARLIST_SYSTEM_GENERATED:
        return RC_DESC_FAIL_VARLIST_SYSTEM_GENERATED;
        break;
    case BRC_FAIL_SETUP_DATASET_IN:
        return RC_DESC_FAIL_SETUP_DATASET_IN;
        break;
    case BRC_FAIL_SETUP_OTHER:
        return RC_DESC_FAIL_SETUP_OTHER;
        break;
    case BRC_FAIL_ALLOCATE_MEMORY:
        return RC_DESC_FAIL_ALLOCATE_MEMORY;
        break;
    case BRC_FAIL_LPI_INIT:
        return RC_DESC_FAIL_LPI_INIT;
        break;
    case BRC_FAIL_EDITS_PARSE:
        return RC_DESC_FAIL_EDITS_PARSE;
        break;
    // validation related errors
    case BRC_FAIL_VALIDATION_LEGACY:
        return RC_DESC_FAIL_VALIDATION_LEGACY;
        break;
    case BRC_FAIL_VALIDATION_NEW:
        return RC_DESC_FAIL_VALIDATION_NEW;
        break;
    case BRC_FAIL_EDITS_OTHER:
        return RC_DESC_FAIL_EDITS_OTHER;
        break;
    case BRC_FAIL_EDITS_CONSISTENCY:
        return RC_DESC_FAIL_EDITS_CONSISTENCY;
        break;
    case BRC_FAIL_EDITS_REDUNDANCY:
        return RC_DESC_FAIL_EDITS_REDUNDANCY;
        break;
    case BRC_FAIL_NAME_TOO_LONG:
        return RC_DESC_FAIL_NAME_TOO_LONG;
        break;
    // dataset related errors
    case BRC_FAIL_READ_GENERIC:
        return RC_DESC_FAIL_READ_GENERIC;
        break;
    case BRC_FAIL_WRONG_SORT_ORDER:
        return RC_DESC_FAIL_WRONG_SORT_ORDER;
        break;
    case BRC_FAIL_READ_SYNC:
        return RC_DESC_FAIL_READ_SYNC;
        break;
    case BRC_FAIL_READ_DUPLICATE_DATA:
        return RC_DESC_FAIL_READ_DUPLICATE_DATA;
        break;
    // output related errors
    case BRC_FAIL_WRITE_GENERIC:
        return RC_DESC_FAIL_WRITE_GENERIC;
        break;
    // processing related errors
    case BRC_FAIL_PROCESSING_GENERIC:
        return RC_DESC_FAIL_PROCESSING_GENERIC;
        break;
    case BRC_EIE_TRANSFORM_FAIL:
        return RC_DESC_EIE_TRANSFORM_FAIL;
        break;
    case BRC_EIE_KDTREE_FAIL:
        return RC_DESC_EIE_KDTREE_FAIL;
        break;
    case BRC_EIE_MATCHFIELDS_FAIL:
        return RC_DESC_EIE_MATCHFIELDS_FAIL;
        break;
    default:
        return "";
    }
}