#ifndef EDITSTAT_JIO_H
#define EDITSTAT_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "EDITSTATS"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_EDITS,
    UPO_BY,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO_INDATA,
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO_OUTEDIT_APPLIC,
    DSOO_OUTEDIT_STATUS,
    DSOO_OUTGLOBAL_STATUS,
    DSOO_OUTK_EDITS_STATUS,
    DSOO_OUTEDITS_REDUCED,
    DSOO_OUTVARS_ROLE,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;
} DSR_indata;

typedef struct __SP_editstat {
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata dsr_indata;

    /* OUTPUT DATA*/
    DSW_generic dsw_outedit_applic;
    DSW_generic dsw_outedit_status;
    DSW_generic dsw_outglobal_status;
    DSW_generic dsw_outk_edits_status;
    DSW_generic dsw_outedits_reduced;
    DSW_generic dsw_outvars_role;


    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_QS edits;
    UP_QS by;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_editstat;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_editstat* sp, T_in_ds in_ds_indata);

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
);
PROC_RETURN_CODE SP_wrap(SP_editstat* sp);

#endif