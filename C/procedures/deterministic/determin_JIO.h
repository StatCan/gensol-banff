#ifndef DETERMIN_JIO_H
#define DETERMIN_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "DETERMINISTIC"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.  
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_NO_BY_STATS,
    UPO_EDITS,
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
    DSOO_OUTSTATUS,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;
} DSR_indata;

typedef struct __SP_determin {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata dsr_indata;
    DSR_instatus dsr_instatus;

    /* OUTPUT DATA*/
    DSW_generic dsw_outdata;
    DSW_generic dsw_outstatus;

    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_FLAG no_by_stats;
    UP_QS edits;
    UP_QS unit_id;
    UP_QS by;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_determin;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_indata_init(SP_determin* sp, T_in_ds in_ds_indata);
IO_RETURN_CODE DSR_instatus_init(SP_determin* sp, T_in_ds in_ds_instatus);

IO_RETURN_CODE SP_init(
    SP_determin* sp, 

    T_in_parm in_parms, 

    T_in_ds in_ds_indata, 
    T_in_ds in_ds_instatus,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus
);
PROC_RETURN_CODE SP_wrap(SP_determin* sp);

#endif