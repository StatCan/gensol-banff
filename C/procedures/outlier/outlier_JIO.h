#ifndef OUTLIER_JIO_H
#define OUTLIER_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME             "OUTLIER"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_ACCEPT_ZERO,
    UPO_NO_BY_STATS,
    UPO_OUTLIER_STATS,
    UPO_BETA_E,
    UPO_BETA_I,
    UPO_EXPONENT,
    UPO_MDM,
    UPO_MEI,
    UPO_MII,
    UPO_START_CENTILE,
    UPO_MIN_OBS,
    UPO_METHOD,
    UPO_SIDE,
    UPO_SIGMA,
    UPO_UNIT_ID,
    UPO_WEIGHT,
    UPO_BY,
    UPO_VAR,
    UPO_WITH_VAR,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO_INDATA,
    DSIO_INDATA_HIST,
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO_OUTSTATUS,
    DSOO_OUTSTATUS_DETAILED,
    DSOO_OUTSUMMARY,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_var;          // VAR
    DS_varlist VL_weight;       // WEIGHT: optional single variable (applies only to `indata`, not `indata_hist`)
} DSR_indata;

typedef struct __SP_outlier {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata         dsr_indata;         // indata dataset reader
    DSR_indata         dsr_indata_hist;    // historical indata dataset reader

    /* OUTPUT DATA*/
    DSW_generic dsw_outstatus;
    DSW_generic dsw_outstatus_detailed;
    DSW_generic dsw_outsummary;

    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_FLAG accept_zero;
    UP_FLAG no_by_stats;
    UP_FLAG outlier_stats;
    UP_INTNBR beta_e;
    UP_INTNBR beta_i;
    UP_INTNBR exponent;
    UP_INTNBR mdm;
    UP_INTNBR mei;
    UP_INTNBR mii;
    UP_INTNBR start_centile;
    UP_INT min_obs;
    UP_NAME method;
    UP_NAME side;
    UP_NAME sigma;
    UP_QS unit_id;
    UP_QS weight;
    UP_QS by;
    UP_QS var;
    UP_QS with_var;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_outlier;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_outlier* sp, const char* dataset_name, const char* varlist_name_varwith, DSR_indata* dsr, T_in_ds in_ds_dataset, int index_to_add, const char* instr_weight);

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
);
PROC_RETURN_CODE SP_wrap(SP_outlier* sp);

void JPF_copy_indata(SP_outlier* sp);

#endif