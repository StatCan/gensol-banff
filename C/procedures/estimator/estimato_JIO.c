#include "estimato_JIO.h"

IO_RETURN_CODE DSR_inalgorithm_init(SP_estimato* sp, T_in_ds in_ds_inalgorithm) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    DSR_inalgorithm* dsr = &sp->dsr_inalgorithm;
    rc_io = DSR_init_simple(&sp->spg, DSIO_INALGORITHM,
        &dsr->dsr, DSN_INALGORITHM, in_ds_inalgorithm,
        5,
            &dsr->VL_algorithm_name,
            &dsr->VL_description,
            &dsr->VL_formula,
            &dsr->VL_status,
            &dsr->VL_type
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists */
    rc_io = VL_init(&dsr->VL_algorithm_name, &dsr->dsr, VL_NAME_inalgorithm, DS_VARNAME_ALG_NAME, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    // OPTIONAL: the "algorithm description" column is optional, hence using `*_silent`, no error check
    VL_init_silent(&dsr->VL_description, &dsr->dsr, VL_NAME_inalgorithm, DS_VARNAME_ALG_DESCRIPTION, IOVT_CHAR);

    rc_io = VL_init(&dsr->VL_formula, &dsr->dsr, VL_NAME_inalgorithm, DS_VARNAME_ALG_FORMULA, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_status, &dsr->dsr, VL_NAME_inalgorithm, DS_VARNAME_ALG_STATUS, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_type, &dsr->dsr, VL_NAME_inalgorithm, DS_VARNAME_ALG_TYPE, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    return IORC_SUCCESS;
}

/* DSR_indata_init
    helper functions calls `DSR_GENERIC_indata_init` for "indata" dataset */
IO_RETURN_CODE DSR_indata_init(SP_estimato* sp, T_in_ds in_ds_indata) {
    return DSR_GENERIC_indata_init(sp, &sp->dsr_indata, DSIO_INDATA, in_ds_indata, DSN_INDATA, BPN_DATA_EXCL_VAR, sp->data_excl_var.value);
}

/* DSR_indata_hist_init
    helper functions calls `DSR_GENERIC_indata_init` for "indata_hist" dataset */
IO_RETURN_CODE DSR_indata_hist_init(SP_estimato* sp, T_in_ds in_ds_indata) {
    return DSR_GENERIC_indata_init(sp, &sp->dsr_indata_hist, DSIO_INDATA_HIST, in_ds_indata, DSN_INDATA_HIST, BPN_DATA_EXCL_VAR_HIST, sp->hist_excl_var.value);
}

/* DSR_GENERIC_indata_init
    GENERIC: because it can be used on indata or indata_hist */
IO_RETURN_CODE DSR_GENERIC_indata_init(SP_estimato* sp, DSR_indata* dsr, int index_to_add, T_in_ds in_ds_indata, char* ds_name, char* excl_varlist_name, const char* excl_var) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    rc_io = DSR_init(&sp->spg, index_to_add,
        &dsr->dsr, ds_name, in_ds_indata, sp->unit_id.value, sp->by.value, 
        IOB_FALSE,
        1,
            &dsr->VL_data_excl
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists */
    rc_io = VL_init_single(&dsr->VL_data_excl, &dsr->dsr, excl_varlist_name, excl_var, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    return IORC_SUCCESS;
}

IO_RETURN_CODE DSR_inestimator_init(SP_estimato* sp, T_in_ds in_ds_inestimator) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    DSR_inestimator* dsr = &sp->dsr_inestimator;
    rc_io = DSR_init_simple(&sp->spg, DSIO_INESTIMATOR,
        &dsr->dsr, DSN_INESTIMATOR, in_ds_inestimator,
        12,
            &dsr->VL_algorithm_name,
            &dsr->VL_aux_vars,
            &dsr->VL_count_criteria,
            &dsr->VL_exclude_imputed,
            &dsr->VL_exclude_outliers,
            &dsr->VL_fieldid,
            &dsr->VL_percent_criteria,
            &dsr->VL_random_error,
            &dsr->VL_variance_exponent,
            &dsr->VL_variance_period,
            &dsr->VL_variance_var,
            &dsr->VL_weight_var
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists */
    rc_io = VL_init(&dsr->VL_algorithm_name, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_ALGORITHMNAME, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_aux_vars, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_AUX_VARS, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_count_criteria, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_COUNT_CRITERIA, IOVT_NUM);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_exclude_imputed, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_EXCLUDE_IMPUTED, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_exclude_outliers, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_EXCLUDE_OUTLIERS, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_fieldid, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_FIELD_ID, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_percent_criteria, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_PERCENT_CRITERIA, IOVT_NUM);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_random_error, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_RAND_ERR, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_variance_exponent, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_VARIANCE_EXPONENT, IOVT_NUM);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_variance_period, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_VARIANCE_PERIOD, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_variance_var, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_VARIANCE_VAR, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_weight_var, &dsr->dsr, VL_NAME_inestimator, DS_VARNAME_EST_WEIGHT_VAR, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    return IORC_SUCCESS;
}

/* DSR_instatus_init
    calls common setup function for instatus dataset */
IO_RETURN_CODE DSR_instatus_init(SP_estimato* sp, T_in_ds in_ds_instatus) {
    return banff_init_dsr_instatus(
        &sp->spg,
        DSIO_INSTATUS,
        &sp->dsr_instatus,
        in_ds_instatus,
        sp->unit_id.value,
        sp->by.value
    );
}

/* DSR_instatus_hist_init
    This function based on `banff_init_dsr_instatus` */
IO_RETURN_CODE DSR_instatus_hist_init(SP_estimato* sp, T_in_ds in_ds_instatus_hist) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    DSR_instatus* dsr = &sp->dsr_instatus_hist;

    /* initialize DSR */
    rc_io = DSR_init(&sp->spg, DSIO_INSTATUS_HIST,
        &dsr->dsr, DSN_INSTATUS_HIST, in_ds_instatus_hist, sp->unit_id.value, sp->by.value,
        IOB_TRUE,
        2,
        &dsr->VL_field_id,
        &dsr->VL_status
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists */
    rc_io = VL_init(&dsr->VL_field_id, &dsr->dsr, VL_NAME_instatus_hist, DSC_FIELD_ID, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = VL_init(&dsr->VL_status, &dsr->dsr, VL_NAME_instatus_hist, DSC_STATUS, IOVT_CHAR);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    return IORC_SUCCESS;
}

IO_RETURN_CODE SP_init(
    SP_estimato* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_inalgorithm,
    T_in_ds in_ds_indata,
    T_in_ds in_ds_indata_hist,
    T_in_ds in_ds_inestimator,
    T_in_ds in_ds_instatus,
    T_in_ds in_ds_instatus_hist,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outacceptable,
    T_out_ds out_arr_outacceptable,
    T_out_ds out_sch_outest_ef,
    T_out_ds out_arr_outest_ef,
    T_out_ds out_sch_outest_lr,
    T_out_ds out_arr_outest_lr,
    T_out_ds out_sch_outest_parm,
    T_out_ds out_arr_outest_parm,
    T_out_ds out_sch_outrand_err,
    T_out_ds out_arr_outrand_err
){
    TIME_WALL_START(SP_init);
    TIME_CPU_START(SP_init);

    /* INITIALIZE SP_generic struct */
    IO_RETURN_CODE rc_io = SPG_init(&sp->spg, in_parms, UPO__COUNT, DSIO__COUNT, DSOO__COUNT);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* GET PARAMETERS */
    TIME_CPU_START(read_parms);
    /****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] ***************************/
    PARM_init(&sp->spg, UPO_ACCEPT_NEGATIVE,     (PARM_generic*) &sp->accept_negative,    BPN_ACCEPT_NEGATIVE,        IOPT_FLAG);
    PARM_init(&sp->spg, UPO_NO_BY_STATS,         (PARM_generic*) &sp->no_by_stats,        BPN_NO_BY_STATS,            IOPT_FLAG);
    PARM_init(&sp->spg, UPO_VERIFY_SPECS,        (PARM_generic*) &sp->verify_specs,       BPN_VERIFY_SPECS,           IOPT_FLAG);
    PARM_init(&sp->spg, UPO_SEED,                (PARM_generic*) &sp->seed,               BPN_SEED,                   IOPT_INT);
    PARM_init(&sp->spg, UPO_DATA_EXCL_VAR,       (PARM_generic*) &sp->data_excl_var,      BPN_DATA_EXCL_VAR,          IOPT_STRING);
    PARM_init(&sp->spg, UPO_HIST_EXCL_VAR,       (PARM_generic*) &sp->hist_excl_var,      BPN_DATA_EXCL_VAR_HIST,     IOPT_STRING);
    PARM_init(&sp->spg, UPO_UNIT_ID,             (PARM_generic*) &sp->unit_id,            BPN_UNIT_ID,                IOPT_STRING);
    PARM_init(&sp->spg, UPO_BY,                  (PARM_generic*) &sp->by,                 BPN_BY,                     IOPT_STRING);
    /****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
    TIME_CPU_STOPDIFF(read_parms);

    /* INITIALIZE OUTPUT DATA */
    rc_io = DSW_init(&sp->spg, DSOO_OUTACCEPTABLE,      &sp->dsw_outacceptable,      DSN_OUTACCEPTABLE,      out_sch_outacceptable, out_arr_outacceptable);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTDATA,            &sp->dsw_outdata,            DSN_OUTDATA,            out_sch_outdata, out_arr_outdata);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTEST_EF,          &sp->dsw_outest_ef,          DSN_OUTEST_EF,          out_sch_outest_ef, out_arr_outest_ef);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTEST_LR,          &sp->dsw_outest_lr,          DSN_OUTEST_LR,          out_sch_outest_lr, out_arr_outest_lr);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTEST_PARM,        &sp->dsw_outest_parm,        DSN_OUTEST_PARM,        out_sch_outest_parm, out_arr_outest_parm);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTRAND_ERR,        &sp->dsw_outrand_err,        DSN_OUTRAND_ERROR,      out_sch_outrand_err, out_arr_outrand_err);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTSTATUS,          &sp->dsw_outstatus,          DSN_OUTSTATUS,          out_sch_outstatus, out_arr_outstatus);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    // include BY variables on some output datasets
    DSW_set_by_var_reference(&sp->dsw_outacceptable,   (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outest_ef,       (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outest_lr,       (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outest_parm,     (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outrand_err,     (DSR_generic*)&sp->dsr_indata);

    /* INITIALIZE INPUT DATA */
    TIME_WALL_START(decode_datasets);

    TIME_CPU_THREAD_START(dsr_inalgorithm);
    rc_io = DSR_inalgorithm_init(sp, in_ds_inalgorithm);
    TIME_CPU_THREAD_STOPDIFF(dsr_inalgorithm);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_inalgorithm.dsr, rc_io, IOB_FALSE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_indata_init);
    rc_io = DSR_indata_init(sp, in_ds_indata);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_indata_hist_init);
    rc_io = DSR_indata_hist_init(sp, in_ds_indata_hist);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_hist_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata_hist.dsr, rc_io, IOB_FALSE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_inestimator_init);
    rc_io = DSR_inestimator_init(sp, in_ds_inestimator);
    TIME_CPU_THREAD_STOPDIFF(dsr_inestimator_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_inestimator.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_instatus_init);
    rc_io = DSR_instatus_init(sp, in_ds_instatus);
    TIME_CPU_THREAD_STOPDIFF(dsr_instatus_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_instatus.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_instatus_hist_init);
    rc_io = DSR_instatus_hist_init(sp, in_ds_instatus_hist);
    TIME_CPU_THREAD_STOPDIFF(dsr_instatus_hist_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_instatus_hist.dsr, rc_io, IOB_FALSE)) {
        return rc_io;
    }

    TIME_WALL_STOPDIFF(decode_datasets);

    /* stop timers: all expensive operations now complete */
    TIME_CPU_STOPDIFF(SP_init);
    TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}

PROC_RETURN_CODE SP_wrap(SP_estimato* sp) {
    IO_RETURN_CODE rc = IORC_ERROR;
    TIME_WALL_START(SP_wrap);
    TIME_CPU_START(SP_wrap);

    TIME_CPU_THREAD_START(encode_outdata);
    rc = DSW_wrap(&sp->dsw_outdata);
    TIME_CPU_THREAD_STOPDIFF(encode_outdata);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outstatus);
    rc = DSW_wrap(&sp->dsw_outstatus);
    TIME_CPU_THREAD_STOPDIFF(encode_outstatus);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outacceptable);
    rc = DSW_wrap(&sp->dsw_outacceptable);
    TIME_CPU_THREAD_STOPDIFF(encode_outacceptable);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outest_ef);
    rc = DSW_wrap(&sp->dsw_outest_ef);
    TIME_CPU_THREAD_STOPDIFF(encode_outest_ef);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outest_lr);
    rc = DSW_wrap(&sp->dsw_outest_lr);
    TIME_CPU_THREAD_STOPDIFF(encode_outest_lr);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outest_parm);
    rc = DSW_wrap(&sp->dsw_outest_parm);
    TIME_CPU_THREAD_STOPDIFF(encode_outest_parm);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outrand_err);
    rc = DSW_wrap(&sp->dsw_outrand_err);
    TIME_CPU_THREAD_STOPDIFF(encode_outrand_err);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_STOPDIFF(SP_wrap);
    TIME_WALL_STOPDIFF(SP_wrap);

    return PRC_SUCCESS;
}
