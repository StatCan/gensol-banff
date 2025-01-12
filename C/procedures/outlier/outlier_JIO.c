#include "outlier_JIO.h"

/* NOTE: this function differs from similarly named functions in other procedures
    - the `VL_var` variable list is not initialized as usual and must be handled later on 
        depending on the "input mode"
    - it is used for initializing both "indata" and "indata_hist"
        - `.VL_weight` only applies to "indata", so it always has 0 variables for "indata_hist" */
IO_RETURN_CODE DSR_indata_init(SP_outlier* sp, const char* dataset_name, const char* varlist_name_varwith, DSR_indata* dsr, T_in_ds in_ds_dataset, int index_to_add, const char* instr_weight) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    rc_io = DSR_init(&sp->spg, index_to_add,
        &dsr->dsr, dataset_name, in_ds_dataset, sp->unit_id.value, sp->by.value, 
        IOB_FALSE,
        2,
            &dsr->VL_weight,    // when initializing "indata_hist" this is always empty, but must be added regardless
            &dsr->VL_var
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* initialize procedure-specific varlists 
        NOTE: unusual behaviour for outlier
              VAR variable list initialized to NULL here, reinitialized later 
              in `SP_init(), after "input mode" is determined */
    rc_io = VL_init_single(&dsr->VL_weight, &dsr->dsr, BPN_WEIGHT, instr_weight, IOVT_NUM);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    // NULL initialize VAR list, will reinitialize later 
    VL_init(&dsr->VL_var, &dsr->dsr, varlist_name_varwith, NULL, IOVT_NUM);

    return IORC_SUCCESS;
}

IO_RETURN_CODE SP_init(
    SP_outlier* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_indata_hist,

    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outstatus_detailed,
    T_out_ds out_arr_outstatus_detailed,
    T_out_ds out_sch_outsummary,
    T_out_ds out_arr_outsummary
){
    TIME_WALL_START(SP_init);
    TIME_CPU_START(SP_init);

    /* INITIALIZE SP_generic struct */
    IO_RETURN_CODE rc_io = SPG_init(&sp->spg, in_parms, UPO__COUNT, DSIO__COUNT, DSOO__COUNT);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* GET PARAMETERS */
	/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] ***************************/
    TIME_CPU_START(read_parms);
    PARM_init(&sp->spg, UPO_ACCEPT_NEGATIVE,     (PARM_generic*) &sp->accept_negative,    BPN_ACCEPT_NEGATIVE,        IOPT_FLAG);
    PARM_init(&sp->spg, UPO_ACCEPT_ZERO,         (PARM_generic*) &sp->accept_zero,        BPN_ACCEPT_ZERO,            IOPT_FLAG);
    PARM_init(&sp->spg, UPO_NO_BY_STATS,         (PARM_generic*) &sp->no_by_stats,        BPN_NO_BY_STATS,            IOPT_FLAG);
    PARM_init(&sp->spg, UPO_OUTLIER_STATS,       (PARM_generic*) &sp->outlier_stats,      BPN_OUTLIER_STATS,          IOPT_FLAG);
    PARM_init(&sp->spg, UPO_BETA_E,              (PARM_generic*) &sp->beta_e,             BPN_BETA_E,                 IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_BETA_I,              (PARM_generic*) &sp->beta_i,             BPN_BETA_I,                 IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_EXPONENT,            (PARM_generic*) &sp->exponent,           BPN_EXPONENT,               IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_MDM,                 (PARM_generic*) &sp->mdm,                BPN_MDM,                    IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_MEI,                 (PARM_generic*) &sp->mei,                BPN_MEI,                    IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_MII,                 (PARM_generic*) &sp->mii,                BPN_MII,                    IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_START_CENTILE,       (PARM_generic*) &sp->start_centile,      BPN_START_CENTILE,          IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_MIN_OBS,             (PARM_generic*) &sp->min_obs,            BPN_MIN_OBS,                IOPT_INT);
    PARM_init(&sp->spg, UPO_METHOD,              (PARM_generic*) &sp->method,             BPN_METHOD,                 IOPT_NAME_STRING);
    PARM_init(&sp->spg, UPO_SIDE,                (PARM_generic*) &sp->side,               BPN_SIDE,                   IOPT_NAME_STRING);
    PARM_init(&sp->spg, UPO_SIGMA,               (PARM_generic*) &sp->sigma,              BPN_SIGMA,                  IOPT_NAME_STRING);
    PARM_init(&sp->spg, UPO_UNIT_ID,             (PARM_generic*) &sp->unit_id,            BPN_UNIT_ID,                IOPT_STRING);
    PARM_init(&sp->spg, UPO_WEIGHT,              (PARM_generic*) &sp->weight,             BPN_WEIGHT,                 IOPT_STRING);
    PARM_init(&sp->spg, UPO_BY,                  (PARM_generic*) &sp->by,                 BPN_BY,                     IOPT_STRING);
    PARM_init(&sp->spg, UPO_VAR,                 (PARM_generic*) &sp->var,                BPN_VAR,                    IOPT_STRING);
    PARM_init(&sp->spg, UPO_WITH_VAR,            (PARM_generic*) &sp->with_var,           BPN_WITH,                   IOPT_STRING);
    /****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
    TIME_CPU_STOPDIFF(read_parms);
    /* INITIALIZE INPUT DATA */
    TIME_WALL_START(decode_datasets);

    TIME_CPU_THREAD_START(dsr_indata_init);
    rc_io = DSR_indata_init(sp, DSN_INDATA, BPN_VAR, &sp->dsr_indata, in_ds_indata, DSIO_INDATA, sp->weight.value);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_indata_hist_init);
    rc_io = DSR_indata_init(sp, DSN_INDATA_HIST, BPN_WITH, &sp->dsr_indata_hist, in_ds_indata_hist, DSIO_INDATA_HIST, NULL);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_hist_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata_hist.dsr, rc_io, IOB_FALSE)) {
        return rc_io;
    }

    TIME_WALL_STOPDIFF(decode_datasets);

    /* continue DSR initialization 
        the "input mode" determines how to finish setting up the DSR structures */
    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        if (sp->with_var.meta.is_specified == IOSV_NOT_SPECIFIED) {
            // INPUT_MODE: 1ds nowith
            if (sp->var.meta.is_specified == IOSV_NOT_SPECIFIED) {
                rc_io = DSR_default_var_list((DSR_generic*) &sp->dsr_indata, &sp->dsr_indata.VL_var, BPN_VAR);
            }
            else {
                // setup VAR on indata dataset
                rc_io = VL_init(&sp->dsr_indata.VL_var, &sp->dsr_indata.dsr, BPN_VAR, sp->var.value, IOVT_NUM);
            }
        }
        else {// 'with' specified
            // INPUT_MODE: 1ds with
            // make indata_hist a copy of indata
            JPF_copy_indata(sp);
            // setup VAR varlist on indata dataset
            rc_io = VL_init(&sp->dsr_indata.VL_var, &sp->dsr_indata.dsr, BPN_VAR, sp->var.value, IOVT_NUM);
            if (rc_io != IORC_SUCCESS) {
                return rc_io;
            }

            // setup WITH varlist on indata_hist
            rc_io = VL_init(&sp->dsr_indata_hist.VL_var, &sp->dsr_indata_hist.dsr, BPN_WITH, sp->with_var.value, IOVT_NUM);
        }
    }
    else {
        if (sp->with_var.meta.is_specified == IOSV_NOT_SPECIFIED) {
            // INPUT_MODE: 2ds nowith
            // setup VAR on indata
            rc_io = VL_init(&sp->dsr_indata.VL_var, &sp->dsr_indata.dsr, BPN_VAR, sp->var.value, IOVT_NUM);
            if (rc_io != IORC_SUCCESS) {
                return rc_io;
            }
            // setup WITH (copy of VAR) on indata_hist
            rc_io = VL_init(&sp->dsr_indata_hist.VL_var, &sp->dsr_indata_hist.dsr, BPN_WITH, sp->var.value, IOVT_NUM);
        }
        else {
            // INPUT_MODE: 2ds with
            // setup VAR for indata, WITH on indata_hist
            rc_io = VL_init(&sp->dsr_indata.VL_var, &sp->dsr_indata.dsr, BPN_VAR, sp->var.value, IOVT_NUM);
            if (rc_io != IORC_SUCCESS) {
                return rc_io;
            }

            rc_io = VL_init(&sp->dsr_indata_hist.VL_var, &sp->dsr_indata_hist.dsr, BPN_WITH, sp->with_var.value, IOVT_NUM);
        }
    }

    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* INITIALIZE OUTPUT DATA */
    rc_io = DSW_init(&sp->spg, DSOO_OUTSTATUS,          &sp->dsw_outstatus,          DSN_OUTSTATUS,          out_sch_outstatus, out_arr_outstatus);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTSTATUS_DETAILED, &sp->dsw_outstatus_detailed, DSN_OUTSTATUS_DETAILED, out_sch_outstatus_detailed, out_arr_outstatus_detailed);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTSUMMARY,         &sp->dsw_outsummary,         DSN_OUTSUMMARY,         out_sch_outsummary, out_arr_outsummary);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    // include BY variables on some output datasets
    DSW_set_by_var_reference(&sp->dsw_outsummary, (DSR_generic*)&sp->dsr_indata);

    /* stop timers: all expensive operations now complete 
        NOTE: usually (see other procs) we do this earlier on, but in outlier
              there is the "DSR init continued" code, which we capture 
              using the SP_init timer */
    TIME_CPU_STOPDIFF(SP_init);
    TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}


PROC_RETURN_CODE SP_wrap(SP_outlier* sp) {
    IO_RETURN_CODE rc = IORC_ERROR;
    TIME_WALL_START(SP_wrap);
    TIME_CPU_START(SP_wrap);

    TIME_CPU_THREAD_START(encode_outstatus);
    rc = DSW_wrap(&sp->dsw_outstatus);
    TIME_CPU_THREAD_STOPDIFF(encode_outstatus);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outstatus_detailed);
    rc = DSW_wrap(&sp->dsw_outstatus_detailed);
    TIME_CPU_THREAD_STOPDIFF(encode_outstatus_detailed);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outsummary);
    rc = DSW_wrap(&sp->dsw_outsummary);
    TIME_CPU_THREAD_STOPDIFF(encode_outsummary);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_STOPDIFF(SP_wrap);
    TIME_WALL_STOPDIFF(SP_wrap);

    return PRC_SUCCESS;
}

/* JPF_copy_indata - Procedure-specific function
    Outlier may treat the `indata` dataset as both the `indata` and `indata_hist` datasets.  
    This is implemented by using a C-language feature to "shallow copy" indata's 
    `DSR_indata` structure into indata_hist's structure.  
    This function contains additional custom code and a library function call which 
    performs custom copying in addition to the shallow copy
    Note: this function ignores procedure-specific varlists, so call this before initializing any such varlists */
void JPF_copy_indata(SP_outlier* sp) {
    /* this performs a "shallow copy" of `.dsr_indata` into `.dsr_indata_hist` */
    // retain destination's `.list_of_varlist`
    DS_varlist** list_of_varlist = sp->dsr_indata_hist.dsr.list_of_varlist;
    // shallow copy
    sp->dsr_indata_hist = sp->dsr_indata;
    // restore `.list_of_varlist`
    sp->dsr_indata_hist.dsr.list_of_varlist = list_of_varlist;

    /* to complete the copy, use `DSR_copy_reallocate`, it will
        reallocate some DSR_generic members which cannot be shared */
    DSR_copy_reallocate(&sp->dsr_indata_hist.dsr, DSN_INDATA_HIST);

    /* `VL_weight` only applies to `indata`, so set to `NULL` in `indata_hist` */
    VL_init(&sp->dsr_indata_hist.VL_weight, &sp->dsr_indata_hist.dsr, BPN_WEIGHT, NULL, IOVT_NUM);
}
