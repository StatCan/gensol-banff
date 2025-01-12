#include "donorimp_JIO.h"

IO_RETURN_CODE DSR_indata_init(SP_donorimp* sp, T_in_ds in_ds_indata) {
	IO_RETURN_CODE rc_io = IORC_SUCCESS;

	/* initialize DSR */
	DSR_indata* dsr = &sp->dsr_indata;
	rc_io = DSR_init(&sp->spg, DSIO_INDATA, 
		&dsr->dsr, DSN_INDATA, in_ds_indata, sp->unit_id.value, sp->by.value, 
		IOB_FALSE, 
		3, 
			&dsr->VL_rand_num, 
			&dsr->VL_data_excl, 
			&dsr->VL_must_match
	);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	/* initialize procedure-specific varlists */
	rc_io = VL_init_single(&dsr->VL_rand_num, &dsr->dsr, BPN_RAND_NUM_VAR, sp->rand_num_var.value, IOVT_NUM);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	rc_io = VL_init_single(&dsr->VL_data_excl, &dsr->dsr, BPN_DATA_EXCL_VAR, sp->data_excl_var.value, IOVT_CHAR);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	rc_io = VL_init(&dsr->VL_must_match, &dsr->dsr, BPN_MUST_MATCH, sp->must_match.value, IOVT_NUM);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	return IORC_SUCCESS;
}

IO_RETURN_CODE DSR_instatus_init(SP_donorimp* sp, T_in_ds in_ds_instatus) {
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
	SP_donorimp* sp,

	T_in_parm in_parms,

	T_in_ds in_ds_indata,
	T_in_ds in_ds_instatus,

	T_out_ds out_sch_outdata,
	T_out_ds out_arr_outdata,
	T_out_ds out_sch_outstatus,
	T_out_ds out_arr_outstatus,
	T_out_ds out_sch_outdonormap,
	T_out_ds out_arr_outdonormap,
	T_out_ds out_sch_outmatching_fields,
	T_out_ds out_arr_outmatching_fields
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
	PARM_init(&sp->spg, UPO_RANDOM,              (PARM_generic*) &sp->random,             BPN_RANDOM,                 IOPT_FLAG);
	PARM_init(&sp->spg, UPO_MRL,                 (PARM_generic*) &sp->mrl,                BPN_MRL,                    IOPT_NUMERIC);
	PARM_init(&sp->spg, UPO_PERCENT_DONORS,      (PARM_generic*) &sp->percent_donors,     BPN_PERCENT_DONORS,         IOPT_NUMERIC);
	PARM_init(&sp->spg, UPO_DISPLAY_LEVEL,       (PARM_generic*) &sp->display_level,      BPN_DISPLAY_LEVEL,          IOPT_INT);
	PARM_init(&sp->spg, UPO_MIN_DONORS,          (PARM_generic*) &sp->min_donors,         BPN_MIN_DONORS,             IOPT_INT);
	PARM_init(&sp->spg, UPO_N,                   (PARM_generic*) &sp->n,                  BPN_N,                      IOPT_INT);
	PARM_init(&sp->spg, UPO_N_LIMIT,             (PARM_generic*) &sp->n_limit,            BPN_N_LIMIT,                IOPT_INT);
	PARM_init(&sp->spg, UPO_SEED,                (PARM_generic*) &sp->seed,               BPN_SEED,                   IOPT_INT);
	PARM_init(&sp->spg, UPO_EDITS,               (PARM_generic*) &sp->edits,              BPN_EDITS,                  IOPT_STRING);
	PARM_init(&sp->spg, UPO_ELIGDON,             (PARM_generic*) &sp->eligdon,            BPN_ELIGDON,                IOPT_NAME_STRING);
	PARM_init(&sp->spg, UPO_POST_EDITS,          (PARM_generic*) &sp->post_edits,         BPN_POST_EDITS,             IOPT_STRING);
	PARM_init(&sp->spg, UPO_DATA_EXCL_VAR,       (PARM_generic*) &sp->data_excl_var,      BPN_DATA_EXCL_VAR,          IOPT_STRING);
	PARM_init(&sp->spg, UPO_UNIT_ID,             (PARM_generic*) &sp->unit_id,            BPN_UNIT_ID,                IOPT_STRING);
	PARM_init(&sp->spg, UPO_RAND_NUM_VAR,        (PARM_generic*) &sp->rand_num_var,       BPN_RAND_NUM_VAR,           IOPT_STRING);
	PARM_init(&sp->spg, UPO_BY,                  (PARM_generic*) &sp->by,                 BPN_BY,                     IOPT_STRING);
	PARM_init(&sp->spg, UPO_MUST_MATCH,          (PARM_generic*) &sp->must_match,         BPN_MUST_MATCH,             IOPT_STRING);
	/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[init code] *****************************/
	TIME_CPU_STOPDIFF(read_parms);

	/* INITIALIZE OUTPUT DATA */
	rc_io = DSW_init(&sp->spg, DSOO_OUTDATA,            &sp->dsw_outdata,            DSN_OUTDATA,            out_sch_outdata, out_arr_outdata);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	rc_io = DSW_init(&sp->spg, DSOO_OUTDONORMAP,        &sp->dsw_outdonormap,        DSN_OUTDONORMAP,        out_sch_outdonormap, out_arr_outdonormap);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	rc_io = DSW_init(&sp->spg, DSOO_OUTSTATUS,          &sp->dsw_outstatus,          DSN_OUTSTATUS,          out_sch_outstatus, out_arr_outstatus);
	if (rc_io != IORC_SUCCESS) {
		return rc_io;
	}

	rc_io = DSW_init(&sp->spg, DSOO_OUTMATCHING_FIELDS, &sp->dsw_outmatching_fields, DSN_OUTMATCHING_FIELDS, out_sch_outmatching_fields, out_arr_outmatching_fields);
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
	if (IORC_SUCCESS != DSR_validate_init(&sp->dsr_instatus.dsr, rc_io, IOB_TRUE)) {
		return rc_io;
	}
	
	TIME_WALL_STOPDIFF(decode_datasets);

	/* stop timers: all expensive operations now complete */
	TIME_CPU_STOPDIFF(SP_init);
	TIME_WALL_STOPDIFF(SP_init);

    return IORC_SUCCESS;
}

PROC_RETURN_CODE SP_wrap(SP_donorimp* sp) {
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

	TIME_CPU_THREAD_START(encode_donormap);
	rc = DSW_wrap(&sp->dsw_outdonormap);
	TIME_CPU_THREAD_STOPDIFF(encode_donormap);
	if (rc != IORC_SUCCESS) {
		return PRC_FAIL_WRITE_DATA;
	}

	TIME_CPU_THREAD_START(encode_outmatching_fields);
	rc = DSW_wrap(&sp->dsw_outmatching_fields);
	TIME_CPU_THREAD_STOPDIFF(encode_outmatching_fields);
	if (rc != IORC_SUCCESS) {
		return PRC_FAIL_WRITE_DATA;
	}
	
	TIME_CPU_STOPDIFF(SP_wrap);
	TIME_WALL_STOPDIFF(SP_wrap);

	return PRC_SUCCESS;
}
