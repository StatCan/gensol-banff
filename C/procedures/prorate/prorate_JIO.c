#include "prorate_JIO.h"

IO_RETURN_CODE DSR_indata_init(SP_prorate* sp, T_in_ds in_ds_indata) {
    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* initialize DSR */
    DSR_indata* dsr = &sp->dsr_indata;
    rc_io = DSR_init(&sp->spg, DSIO_INDATA, 
        &dsr->dsr, DSN_INDATA, in_ds_indata, sp->unit_id.value, sp->by.value, 
        IOB_FALSE, 
        0
    );
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* no procedure-specific varlists to initialize */
    return IORC_SUCCESS;
}

IO_RETURN_CODE DSR_instatus_init(SP_prorate* sp, T_in_ds in_ds_instatus) {
    return banff_init_dsr_instatus(
        &sp->spg,
        DSIO_INSTATUS,
        &sp->dsr_instatus,
        in_ds_instatus,
        sp->unit_id.value,
        sp->by.value
    );
}
IO_RETURN_CODE SP_init(
    SP_prorate* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_instatus,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outreject,
    T_out_ds out_arr_outreject
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
    PARM_init(&sp->spg, UPO_NO_BY_STATS,         (PARM_generic*) &sp->no_by_stats,        BPN_NO_BY_STATS,            IOPT_FLAG);
    PARM_init(&sp->spg, UPO_VERIFY_EDITS,        (PARM_generic*) &sp->verify_edits,       BPN_VERIFY_EDITS,           IOPT_FLAG);
    PARM_init(&sp->spg, UPO_LOWER_BOUND,         (PARM_generic*) &sp->lower_bound,        BPN_LOWER_BOUND,            IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_UPPER_BOUND,         (PARM_generic*) &sp->upper_bound,        BPN_UPPER_BOUND,            IOPT_NUMERIC);
    PARM_init(&sp->spg, UPO_DECIMAL,             (PARM_generic*) &sp->decimal,            BPN_DECIMAL,                IOPT_INT);
    PARM_init(&sp->spg, UPO_EDITS,               (PARM_generic*) &sp->edits,              BPN_EDITS,                  IOPT_STRING);
    PARM_init(&sp->spg, UPO_METHOD,              (PARM_generic*) &sp->method,             BPN_METHOD,                 IOPT_NAME_STRING);
    PARM_init(&sp->spg, UPO_MODIFIER,            (PARM_generic*) &sp->modifier,           BPN_MODIFIER,               IOPT_NAME_STRING);
    PARM_init(&sp->spg, UPO_UNIT_ID,             (PARM_generic*) &sp->unit_id,            BPN_UNIT_ID,                IOPT_STRING);
    PARM_init(&sp->spg, UPO_BY,                  (PARM_generic*) &sp->by,                 BPN_BY,                     IOPT_STRING);
    /****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
    TIME_CPU_STOPDIFF(read_parms);

    /* INITIALIZE OUTPUT DATA */
    rc_io = DSW_init(&sp->spg, DSOO_OUTDATA,            &sp->dsw_outdata,            DSN_OUTDATA,            out_sch_outdata, out_arr_outdata);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTREJECT,          &sp->dsw_outreject,          DSN_OUTREJECT,          out_sch_outreject, out_arr_outreject);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    rc_io = DSW_init(&sp->spg, DSOO_OUTSTATUS,          &sp->dsw_outstatus,          DSN_OUTSTATUS,          out_sch_outstatus, out_arr_outstatus);
    if (rc_io != IORC_SUCCESS) {
        return rc_io;
    }

    /* INITIALIZE INPUT DATA */
    TIME_WALL_START(decode_datasets);

    TIME_CPU_THREAD_START(dsr_indata_init);
    rc_io = DSR_indata_init(sp, in_ds_indata);
    TIME_CPU_THREAD_STOPDIFF(dsr_indata_init);
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_indata.dsr, rc_io, IOB_TRUE)) {
        return rc_io;
    }

    TIME_CPU_THREAD_START(dsr_instatus_init);
    rc_io = DSR_instatus_init(sp, in_ds_instatus);
    TIME_CPU_THREAD_STOPDIFF(dsr_instatus_init);
    // instatus not always mandatory, so treat as optional here, legacy code validates later
    if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_instatus.dsr, rc_io, IOB_FALSE)) {
        return rc_io;
    }

    TIME_WALL_STOPDIFF(decode_datasets);

    /* stop timers: all expensive operations now complete */
    TIME_CPU_STOPDIFF(SP_init);
    TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}

PROC_RETURN_CODE SP_wrap(SP_prorate* sp) {
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

    TIME_CPU_THREAD_START(encode_outreject);
    rc = DSW_wrap(&sp->dsw_outreject);
    TIME_CPU_THREAD_STOPDIFF(encode_outreject);
    if (rc != IORC_SUCCESS) {
        return PRC_FAIL_WRITE_DATA;
    }

    TIME_CPU_STOPDIFF(SP_wrap);
    TIME_WALL_STOPDIFF(SP_wrap);

    return PRC_SUCCESS;
}
