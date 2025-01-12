#ifndef MASSIMPU_JIO_H
#define MASSIMPU_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "MASSIMPUTATION"

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
    UPO_MIN_DONORS,
    UPO_N_LIMIT,
    UPO_SEED,
    UPO_UNIT_ID,
    UPO_BY,
    UPO_MUST_IMPUTE,
    UPO_MUST_MATCH,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO_INDATA,
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO_OUTDATA,
    DSOO_OUTDONORMAP,
    DSOO_OUTSTATUS,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_must_match;
    DS_varlist VL_must_impute;
} DSR_indata;

typedef struct __SP_massimpu {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata dsr_indata;

    /* OUTPUT DATA*/
    DSW_generic dsw_outdata;
    DSW_generic dsw_outdonormap;
    DSW_generic dsw_outstatus;

    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_FLAG no_by_stats;
    UP_FLAG random;
    UP_INTNBR mrl;
    UP_INTNBR percent_donors;
    UP_INT min_donors;
    UP_INT n_limit;
    UP_INT seed;
    UP_QS unit_id;
    UP_QS by;
    UP_QS must_impute;
    UP_QS must_match;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_massimpu;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_massimpu* sp, T_in_ds in_ds_indata);

IO_RETURN_CODE SP_init(
    SP_massimpu* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_indata,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outdonormap,
    T_out_ds out_arr_outdonormap
);
PROC_RETURN_CODE SP_wrap(SP_massimpu* sp);

#endif