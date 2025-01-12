#include "editstat_JIO.h"

IO_RETURN_CODE DSR_indata_init(SP_editstat* sp, T_in_ds in_ds_indata) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    DSR_indata* dsr = &sp->dsr_indata;
    char* editstat_has_no_unit_id = NULL;
    rc_io = DSR_init(&sp->spg, DSIO_INDATA,
        &dsr->dsr, DSN_INDATA, in_ds_indata, editstat_has_no_unit_id, sp->by.value,
        IOB_FALSE,
        0
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* no procedure-specific varlists to initialize */
    return IORC_SUCCESS;
}

IO_RETURN_CODE SP_init(
    SP_editstat* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_indata,

    T_out_ds out_sch_outedit_applic,
    T_out_ds out_arr_outedit_applic,
    T_out_ds out_sch_outedit_status,
    T_out_ds out_arr_outedit_status,
    T_out_ds out_sch_outglobal_status,
    T_out_ds out_arr_outglobal_status,
    T_out_ds out_sch_outk_edits_status,
    T_out_ds out_arr_outk_edits_status,
    T_out_ds out_sch_outedits_reduced,
    T_out_ds out_arr_outedits_reduced,
    T_out_ds out_sch_outvars_role,
    T_out_ds out_arr_outvars_role
) {
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
    PARM_init(&sp->spg, UPO_EDITS,               (PARM_generic*) &sp->edits,              BPN_EDITS,                  IOPT_STRING);
    PARM_init(&sp->spg, UPO_BY,                  (PARM_generic*) &sp->by,                 BPN_BY,                     IOPT_STRING);
    /****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
    TIME_CPU_STOPDIFF(read_parms);

    /* INITIALIZE OUTPUT DATA */
    rc_io = DSW_init(&sp->spg, DSOO_OUTEDIT_APPLIC,     &sp->dsw_outedit_applic,     DSN_OUTEDIT_APPLIC,     out_sch_outedit_applic, out_arr_outedit_applic);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTEDIT_STATUS,     &sp->dsw_outedit_status,     DSN_OUTEDITS_STATUS,    out_sch_outedit_status, out_arr_outedit_status);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTGLOBAL_STATUS,   &sp->dsw_outglobal_status,   DSN_OUTGLOBAL_STATUS,   out_sch_outglobal_status, out_arr_outglobal_status);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTK_EDITS_STATUS,  &sp->dsw_outk_edits_status,  DSN_OUTK_EDITS_STATUS,  out_sch_outk_edits_status, out_arr_outk_edits_status);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTEDITS_REDUCED,   &sp->dsw_outedits_reduced,   DSN_OUTEDITS_REDUCED,   out_sch_outedits_reduced, out_arr_outedits_reduced);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTVARS_ROLE,       &sp->dsw_outvars_role,       DSN_OUTVARS_ROLE,       out_sch_outvars_role, out_arr_outvars_role);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    // include BY variables on some output datasets
    DSW_set_by_var_reference(&sp->dsw_outedit_status, (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outk_edits_status, (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outglobal_status, (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outedit_applic, (DSR_generic*)&sp->dsr_indata);
    DSW_set_by_var_reference(&sp->dsw_outvars_role, (DSR_generic*)&sp->dsr_indata);

    /* INITIALIZE INPUT DATA */
    TIME_WALL_START(decode_datasets);

    TIME_CPU_THREAD_START(dsr_indata_init);
    rc_io = DSR_indata_init(sp, in_ds_indata);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_WALL_STOPDIFF(decode_datasets);

    /* stop timers: all expensive operations now complete */
    TIME_CPU_STOPDIFF(SP_init);
    TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}

PROC_RETURN_CODE SP_wrap(SP_editstat* sp) {
    IO_RETURN_CODE rc = IORC_ERROR;
    TIME_WALL_START(SP_wrap);
    TIME_CPU_START(SP_wrap);

    TIME_CPU_THREAD_START(encode_outedit_applic);
    rc = DSW_wrap(&sp->dsw_outedit_applic);
    TIME_CPU_THREAD_STOPDIFF(encode_outedit_applic);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outedit_status);
    rc = DSW_wrap(&sp->dsw_outedit_status);
    TIME_CPU_THREAD_STOPDIFF(encode_outedit_status);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outglobal_status);
    rc = DSW_wrap(&sp->dsw_outglobal_status);
    TIME_CPU_THREAD_STOPDIFF(encode_outglobal_status);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outk_edits_status);
    rc = DSW_wrap(&sp->dsw_outk_edits_status);
    TIME_CPU_THREAD_STOPDIFF(encode_outk_edits_status);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outedits_reduced);
    rc = DSW_wrap(&sp->dsw_outedits_reduced);
    TIME_CPU_THREAD_STOPDIFF(encode_outedits_reduced);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_THREAD_START(encode_outvars_role);
    rc = DSW_wrap(&sp->dsw_outvars_role);
    TIME_CPU_THREAD_STOPDIFF(encode_outvars_role);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_STOPDIFF(SP_wrap);
    TIME_WALL_STOPDIFF(SP_wrap);

    return PRC_SUCCESS;
}
