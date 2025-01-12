#ifndef VERIFYED_JIO_H
#define VERIFYED_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME         "VERIFYED"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_EXTREMAL,
    UPO_IMPLY,
    UPO_EDITS,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __SP_verifyed {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT PARAMETERS */
    /****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_INT extremal;
    UP_INT imply;
    UP_QS edits;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_verifyed;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE SP_init(
    SP_verifyed* sp,
    T_in_parm in_parms
);
PROC_RETURN_CODE SP_wrap(SP_verifyed* sp);

#endif