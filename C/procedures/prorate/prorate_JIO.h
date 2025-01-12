#ifndef PRORATE_JIO_H
#define PRORATE_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "PRORATE"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_NO_BY_STATS,
    UPO_VERIFY_EDITS,
    UPO_LOWER_BOUND,
    UPO_UPPER_BOUND,
    UPO_DECIMAL,
    UPO_EDITS,
    UPO_METHOD,
    UPO_MODIFIER,
    UPO_UNIT_ID,
    UPO_BY,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO_INDATA,
    DSIO_INSTATUS,
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO_OUTDATA,
    DSOO_OUTREJECT,
    DSOO_OUTSTATUS,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;
} DSR_indata;

typedef struct __SP_prorate {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata dsr_indata;      // indata dataset reader
    DSR_instatus dsr_instatus;  // instatus dataset reader

    /* OUTPUT DATA*/
    DSW_generic dsw_outdata;
    DSW_generic dsw_outreject;
    DSW_generic dsw_outstatus;

    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_FLAG no_by_stats;
    UP_FLAG verify_edits;
    UP_INTNBR lower_bound;
    UP_INTNBR upper_bound;
    UP_INT decimal;
    UP_QS edits;
    UP_NAME method;
    UP_NAME modifier;
    UP_QS unit_id;
    UP_QS by;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_prorate;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_prorate* sp, T_in_ds in_ds_indata);
IO_RETURN_CODE DSR_instatus_init(SP_prorate* sp, T_in_ds in_ds_instatus);

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
);
PROC_RETURN_CODE SP_wrap(SP_prorate* sp);

#endif