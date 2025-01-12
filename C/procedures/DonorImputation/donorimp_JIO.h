#ifndef DONORIMP_JIO_H
#define DONORIMP_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "DONORIMPUTATION"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
	These enumeration MUST NOT assign values explicitly.
	`***__COUNT` MUST be the final element listed, ensuring it
	indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
	UPO_ACCEPT_NEGATIVE,
	UPO_NO_BY_STATS,
	UPO_RANDOM,
	UPO_MRL,
	UPO_PERCENT_DONORS,
	UPO_DISPLAY_LEVEL,
	UPO_MIN_DONORS,
	UPO_N,
	UPO_N_LIMIT,
	UPO_SEED,
	UPO_EDITS,
	UPO_ELIGDON,
	UPO_POST_EDITS,
	UPO_DATA_EXCL_VAR,
	UPO_UNIT_ID,
	UPO_RAND_NUM_VAR,
	UPO_BY,
	UPO_MUST_MATCH,
	UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
	DSIO_INDATA,
	DSIO_INSTATUS,
	DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
	DSOO_OUTDATA,
	DSOO_OUTDONORMAP,
	DSOO_OUTSTATUS,
	DSOO_OUTMATCHING_FIELDS,
	DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
	/* GENERIC INFO */
	DSR_generic dsr;

	/* VARIABLE LISTS */
	DS_varlist VL_data_excl;	// DATA_EXCL_VAR: optional single variable
	DS_varlist VL_must_match;	// MUST_MATCH: list with 0 or more elements
	DS_varlist VL_rand_num;		// RAND_NUM_VAR: optional single variable
} DSR_indata;

typedef struct __SP_donorimp {
	SP_generic spg;

	/* INPUT DATA */
	DSR_indata dsr_indata;
	DSR_instatus dsr_instatus;

	/* OUTPUT DATA*/
	DSW_generic dsw_outdata;
	DSW_generic dsw_outdonormap;
	DSW_generic dsw_outstatus;
	DSW_generic dsw_outmatching_fields;

	/* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
	UP_FLAG accept_negative;
	UP_FLAG no_by_stats;
	UP_FLAG random;
	UP_INTNBR mrl;
	UP_INTNBR percent_donors;
	UP_INT display_level;
	UP_INT min_donors;
	UP_INT n;
	UP_INT n_limit;
	UP_INT seed;
	UP_QS edits;
	UP_NAME eligdon;
	UP_QS post_edits;
	UP_QS data_excl_var;
	UP_QS unit_id;
	UP_QS rand_num_var;
	UP_QS by;
	UP_QS must_match;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_donorimp;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_donorimp* sp, T_in_ds in_ds_indata);
IO_RETURN_CODE DSR_instatus_init(SP_donorimp* sp, T_in_ds in_ds_instatus);

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
);
PROC_RETURN_CODE SP_wrap(SP_donorimp* sp);

#endif