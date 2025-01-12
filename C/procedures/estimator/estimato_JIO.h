#ifndef ESTIMATO_JIO_H
#define ESTIMATO_JIO_H


#include "proc_common.h"
#include "BanffIdentifiers.h"
#include "IOUtil.h"
#include "SUtil.h"
#include "RuntimeDiagnostics.h"   // header defines macros for measuring execution time (CPU and WALL)

/******************************** MACRO DEFINITIONS **************************/
#define BANFF_PROC_NAME             "ESTIMATOR"

// the following extracted from `estimato.c`
// why? It relates to input datasets, _JIO.c uses the input dataset's column names
// when configuring the dataset for input
// for output datasets, it's the main procedure .c file that needs these references

#define DS_VARNAME_ALG_NAME             "ALGORITHMNAME"
#define DS_VARNAME_ALG_TYPE             "TYPE"
#define DS_VARNAME_ALG_STATUS           SUTIL_FIELDSTAT_STATUS
#define DS_VARNAME_ALG_FORMULA          "FORMULA"
#define DS_VARNAME_ALG_DESCRIPTION      "DESCRIPTION"

#define DS_VARNAME_EST_ALGORITHMNAME        "ALGORITHMNAME"
#define DS_VARNAME_EST_FIELD_ID             SUTIL_FIELDSTAT_FIELDID
#define DS_VARNAME_EST_AUX_VARS             "AUXVARIABLES"
#define DS_VARNAME_EST_RAND_ERR             "RANDOMERROR"
#define DS_VARNAME_EST_WEIGHT_VAR           "WEIGHTVARIABLE"
#define DS_VARNAME_EST_VARIANCE_VAR         "VARIANCEVARIABLE"
#define DS_VARNAME_EST_VARIANCE_EXPONENT    "VARIANCEEXPONENT"
#define DS_VARNAME_EST_VARIANCE_PERIOD      "VARIANCEPERIOD"
#define DS_VARNAME_EST_EXCLUDE_IMPUTED      "EXCLUDEIMPUTED"
#define DS_VARNAME_EST_EXCLUDE_OUTLIERS     "EXCLUDEOUTLIERS"
#define DS_VARNAME_EST_COUNT_CRITERIA       "COUNTCRITERIA"
#define DS_VARNAME_EST_PERCENT_CRITERIA     "PERCENTCRITERIA"

/******************************** ENUMERATIONS *******************************/
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] **************************
    These enumeration MUST NOT assign values explicitly.
    `***__COUNT` MUST be the final element listed, ensuring it
    indicates the number of other elements in the enumeration.  */

typedef enum __user_parameter_order {
    UPO_ACCEPT_NEGATIVE,
    UPO_NO_BY_STATS,
    UPO_VERIFY_SPECS,
    UPO_SEED,
    UPO_DATA_EXCL_VAR,
    UPO_HIST_EXCL_VAR,
    UPO_UNIT_ID,
    UPO_BY,
    UPO__COUNT,
} user_parameter_order;

typedef enum __input_dataset_order {
    DSIO_INALGORITHM,
    DSIO_INDATA,
    DSIO_INDATA_HIST,
    DSIO_INESTIMATOR,
    DSIO_INSTATUS,
    DSIO_INSTATUS_HIST,
    DSIO__COUNT,
} input_dataset_order;

typedef enum __output_dataset_order {
    DSOO_OUTACCEPTABLE,
    DSOO_OUTDATA,
    DSOO_OUTEST_EF,
    DSOO_OUTEST_LR,
    DSOO_OUTEST_PARM,
    DSOO_OUTRAND_ERR,
    DSOO_OUTSTATUS,
    DSOO__COUNT,
} output_dataset_order;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[enum code] *****************************/

/******************************** STRUCTURES *********************************/
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_data_excl;    // DATA_EXCL_VAR: optional single variable
} DSR_indata;

typedef struct __DSR_inalgorithm {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_algorithm_name;
    DS_varlist VL_type;
    DS_varlist VL_status;
    DS_varlist VL_formula;
    DS_varlist VL_description;
} DSR_inalgorithm;

typedef struct __DSR_inestimator {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_algorithm_name;
    DS_varlist VL_fieldid;
    DS_varlist VL_aux_vars;
    DS_varlist VL_weight_var;
    DS_varlist VL_variance_var;
    DS_varlist VL_variance_exponent;
    DS_varlist VL_variance_period;
    DS_varlist VL_exclude_imputed;
    DS_varlist VL_exclude_outliers;
    DS_varlist VL_count_criteria;
    DS_varlist VL_percent_criteria;
    DS_varlist VL_random_error;
} DSR_inestimator;

typedef struct __SP_estimato {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_inalgorithm    dsr_inalgorithm;    // inalgorithm dataset reader
    DSR_indata         dsr_indata;         // indata dataset reader
    DSR_indata         dsr_indata_hist;    // historical indata dataset reader
    DSR_inestimator    dsr_inestimator;    // inestimator dataset reader
    DSR_instatus       dsr_instatus;       // instatus dataset reader
    DSR_instatus       dsr_instatus_hist;  // historical instatus dataset reader

    /* OUTPUT DATA*/
    DSW_generic dsw_outacceptable;
    DSW_generic dsw_outdata;
    DSW_generic dsw_outest_ef;
    DSW_generic dsw_outest_lr;
    DSW_generic dsw_outest_parm;
    DSW_generic dsw_outrand_err;
    DSW_generic dsw_outstatus;

    /* INPUT PARAMETERS */
/****** GENERATED CODE START: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] ***************************/
    UP_FLAG accept_negative;
    UP_FLAG no_by_stats;
    UP_FLAG verify_specs;
    UP_INT seed;
    UP_QS data_excl_var;
    UP_QS hist_excl_var;
    UP_QS unit_id;
    UP_QS by;
/****** GENERATED CODE END: Procedure_IO_analysis.xlsx:Parameter Procedure ->[struct code] *****************************/
} SP_estimato;

/*************************** FUNCTION PROTOTYPES ******************************/
IO_RETURN_CODE DSR_inalgorithm_init(SP_estimato* sp, T_in_ds in_ds_inalgorithm);
IO_RETURN_CODE DSR_indata_init(SP_estimato* sp, T_in_ds in_ds_indata);
IO_RETURN_CODE DSR_indata_hist_init(SP_estimato* sp, T_in_ds in_ds_indata);
IO_RETURN_CODE DSR_GENERIC_indata_init(SP_estimato* sp, DSR_indata* dsr, int index_to_add, T_in_ds in_ds_indata, char* ds_name, char* excl_varlist_name, const char* excl_var);
IO_RETURN_CODE DSR_inestimator_init(SP_estimato* sp, T_in_ds in_ds_inestimator);
IO_RETURN_CODE DSR_instatus_init(SP_estimato* sp, T_in_ds in_ds_instatus);
IO_RETURN_CODE DSR_instatus_hist_init(SP_estimato* sp, T_in_ds in_ds_instatus_hist);

IO_RETURN_CODE SP_init(
    SP_estimato* sp,

    T_in_parm in_parms,

    T_in_ds in_ds_inalgorithm,
    T_in_ds in_ds_indata,
    T_in_ds in_ds_indata_hist,
    T_in_ds in_ds_inestimator,
    T_in_ds in_ds_instatus,
    T_in_ds in_ds_instatus_hist,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outacceptable,
    T_out_ds out_arr_outacceptable,
    T_out_ds out_sch_outest_ef,
    T_out_ds out_arr_outest_ef,
    T_out_ds out_sch_outest_lr,
    T_out_ds out_arr_outest_lr,
    T_out_ds out_sch_outest_parm,
    T_out_ds out_arr_outest_parm,
    T_out_ds out_sch_outrand_err,
    T_out_ds out_arr_outrand_err
);
PROC_RETURN_CODE SP_wrap(SP_estimato* sp);

#endif