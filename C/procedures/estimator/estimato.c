/*********************************************************************
NAME:         ESTIMATOR
DESCRIPTION:  implements the estimator imputation.
*********************************************************************/
#define MAINPROC 1
#define SASPROC  1

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "estimato_JIO.h"

#include "EI_Algorithm.h"
#include "EI_Average.h"
#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Estimator.h"
#include "EI_EstimatorSpec.h"
#include "EI_Message.h"
#include "EI_Regression.h"
#include "EI_Residual.h"
#include "EI_Stack.h"
#include "EI_Symbol.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "slist.h"
#include "util.h"

#include "MessageBanffAPI.h"

/* number of digit to  in report before IO_PRINT_LINE will start to use
scientific notation */
#define PRINT_PRECISION    7
#define PRINT_STATISTICS_WIDTH PRINT_STATISTICS_WIDTH_DEFAULT + 7


/* end constants for data set, parameters and options */

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

/* Return codes for the ReadByGroup() function*/
typedef enum {
    E_READBYGROUP_FAIL,
    E_READBYGROUP_NOTFOUND,
    E_READBYGROUP_FOUND
} T_READBYGROUP_RETURNCODE;


/* related to DATA data set */

/* related to DATASTATUS data set */
#define STATUS_NBVARS              3

/* related to HIST data set */

/* related to HISTSTATUS data set */
/* use STATUS_NBVARS */

/* related to ALGORITHM data set */
#define ALGORITHM_NBVARS           5
#define ALGORITHM_NAME             DS_VARNAME_ALG_NAME
#define ALGORITHM_TYPE             DS_VARNAME_ALG_TYPE
#define ALGORITHM_STATUS           DS_VARNAME_ALG_STATUS
#define ALGORITHM_FORMULA          DS_VARNAME_ALG_FORMULA
#define ALGORITHM_DESCRIPTION      DS_VARNAME_ALG_DESCRIPTION

/* related to ESTIMATOR data set */
#define ESTIMATOR_NBVARS           12
#define ESTIMATOR_ALGORITHMNAME    DS_VARNAME_EST_ALGORITHMNAME
#define ESTIMATOR_FIELDID          DS_VARNAME_EST_FIELD_ID
#define ESTIMATOR_AUXVARIABLES     DS_VARNAME_EST_AUX_VARS
#define ESTIMATOR_RANDOMERROR      DS_VARNAME_EST_RAND_ERR
#define ESTIMATOR_WEIGHTVARIABLE   DS_VARNAME_EST_WEIGHT_VAR
#define ESTIMATOR_VARIANCEVARIABLE DS_VARNAME_EST_VARIANCE_VAR
#define ESTIMATOR_VARIANCEEXPONENT DS_VARNAME_EST_VARIANCE_EXPONENT
#define ESTIMATOR_VARIANCEPERIOD   DS_VARNAME_EST_VARIANCE_PERIOD
#define ESTIMATOR_EXCLUDEIMPUTED   DS_VARNAME_EST_EXCLUDE_IMPUTED
#define ESTIMATOR_EXCLUDEOUTLIERS  DS_VARNAME_EST_EXCLUDE_OUTLIERS
#define ESTIMATOR_COUNTCRITERIA    DS_VARNAME_EST_COUNT_CRITERIA
#define ESTIMATOR_PERCENTCRITERIA  DS_VARNAME_EST_PERCENT_CRITERIA

/* related to OUTDATA data set */

/* related to OUTSTATUS data set */
/* use STATUS_NBVARS */

/* related to OUTRANDOMERROR data set */
#define OUTRANDOMERROR_NBVARS        9
#define OUTRANDOMERROR_ESTIMID       "ESTIMID"
#define OUTRANDOMERROR_ALGORITHMNAME "ALGORITHMNAME"
#define OUTRANDOMERROR_DONOR         "DONOR"
#define OUTRANDOMERROR_RECIPIENT     "RECIPIENT"
#define OUTRANDOMERROR_FIELDID       SUTIL_FIELDSTAT_FIELDID
#define OUTRANDOMERROR_RESIDUAL      "RESIDUAL"
#define OUTRANDOMERROR_RANDOMERROR   "RANDOMERROR"
#define OUTRANDOMERROR_ORIGINALVALUE "ORIGINALVALUE"
#define OUTRANDOMERROR_IMPUTEDVALUE  "IMPUTEDVALUE"

/* related to OUTACCEPTABLE data set */
#define OUTACCEPTABLE_NBVARS         3
#define OUTACCEPTABLE_ESTIMID        "ESTIMID"
#define OUTACCEPTABLE_ALGORITHMNAME  "ALGORITHMNAME"
/* "key" is the second variable */

/* related to OUTESTPARMS data set */
#define OUTESTPARMS_NBVARS           7
#define OUTESTPARMS_ESTIMID          "ESTIMID"
#define OUTESTPARMS_ALGORITHMNAME    "ALGORITHMNAME"
#define OUTESTPARMS_FIELDID          SUTIL_FIELDSTAT_FIELDID
#define OUTESTPARMS_FTI              "FTI"
#define OUTESTPARMS_IMP              "IMP"
#define OUTESTPARMS_DIVISIONBYZERO   "DIVISIONBYZERO"
#define OUTESTPARMS_NEGATIVE         "NEGATIVE"

/* related to OUTESTEF data set */
#define OUTESTEF_NBVARS              6
#define OUTESTEF_ESTIMID             "ESTIMID"
#define OUTESTEF_ALGORITHMNAME       "ALGORITHMNAME"
#define OUTESTEF_FIELDID             SUTIL_FIELDSTAT_FIELDID
#define OUTESTEF_PERIOD              "PERIOD"
#define OUTESTEF_AVERAGEVALUE        "AVERAGE_VALUE"
#define OUTESTEF_COUNT               "COUNT"

/* related to OUTESTLR data set */
#define OUTESTLR_NBVARS              7
#define OUTESTLR_ESTIMID             "ESTIMID"
#define OUTESTLR_ALGORITHMNAME       "ALGORITHMNAME"
#define OUTESTLR_FIELDID             SUTIL_FIELDSTAT_FIELDID
#define OUTESTLR_EXPONENT            "EXPONENT"
#define OUTESTLR_PERIOD              "PERIOD"
#define OUTESTLR_BETAVALUE           "BETA_VALUE"
#define OUTESTLR_COUNT               "COUNT"

/* related to OUTESTEF and OUTESTLR data sets */
#define OUTEST_PERIODSIZE 1

#define NOTFOUND -1

/* used to print underline to a message */
#define PRINT_UNDERLINE_HEADER(MsgLength) \
IO_PRINT_LINE ("%.*s", (int)MsgLength, \
"-----------------------------------------------------------");

struct tAllOriginalVarNames {
    char ** CurrentVarName;
    int NumberOfCurrentVars;
    char ** HistVarName;
    int NumberOfHistVars;
};
typedef struct tAllOriginalVarNames tAllOriginalVarNames;

struct tVariableChar {
    char Name[LEGACY_MAXNAME+1];
    int Size;
    int Position;
    char * Value;
};
typedef struct tVariableChar tVariableChar;

struct tVariableNumeric {
    char Name[LEGACY_MAXNAME+1];
    int Size;
    int Position;
    double Value;
};
typedef struct tVariableNumeric tVariableNumeric;

struct tInAlgorithmDataSet {
    tVariableChar Name;
    tVariableChar Status;
    tVariableChar Type;
    tVariableChar Formula;
    tVariableChar Description;
};
typedef struct tInAlgorithmDataSet tInAlgorithmDataSet;

struct tInEstimatorDataSet {
    tVariableChar AlgorithmName;
    tVariableChar FieldId;
    tVariableChar AuxVariables;
    tVariableChar WeightVariable;
    tVariableChar VarianceVariable;
    tVariableChar VariancePeriod;
    tVariableChar ExcludeImputed;
    tVariableChar ExcludeOutliers;
    tVariableChar RandomError;

    tVariableNumeric VarianceExponent;
    tVariableNumeric Error;
    tVariableNumeric CountCriteria;
    tVariableNumeric PercentCriteria;
};
typedef struct tInEstimatorDataSet tInEstimatorDataSet;

struct tDataDataSet {
    /* XVPtr will point to a
       XVGET for input data set and to a
       XVPUT for output data set */

    tVariableChar Key;

    /* for input data set only */
    /* Exclusion is optional */
    EIT_BOOLEAN ExclusionInUse;
    tVariableChar ExclusionIndicator;

    char ** VariableName;
    int NumberOfVariables;
    tVariableNumeric * Variable;
};
typedef struct tDataDataSet tDataDataSet;

struct tStatusDataSet {
    /* XVPtr will point to a
       XVGET for input data set and to a
       XVPUT for output data set */
    tVariableChar Key;
    tVariableChar FieldId;
    tVariableChar Status;
    double FieldValue;

    EIT_BOOLEAN HasAllByVariables;
};
typedef struct tStatusDataSet tStatusDataSet;

struct tOutAcceptableDataSet {
    EIT_BOOLEAN Requested;

    tVariableNumeric EstimatorId;
    tVariableChar AlgorithmName;
    tVariableChar Key;
};
typedef struct tOutAcceptableDataSet tOutAcceptableDataSet;

struct tOutEstimEFDataSet {
    EIT_BOOLEAN Requested;

    tVariableNumeric EstimatorId;
    tVariableChar AlgorithmName;
    tVariableChar FieldId;
    tVariableChar Period;
    tVariableNumeric AverageValue;
    tVariableNumeric Count;
};
typedef struct tOutEstimEFDataSet tOutEstimEFDataSet;

struct tOutEstimLRDataSet {
    EIT_BOOLEAN Requested;

    tVariableNumeric EstimatorId;
    tVariableChar AlgorithmName;
    tVariableChar FieldId;
    tVariableNumeric Exponent;
    tVariableChar Period;
    tVariableNumeric BetaValue;
    tVariableNumeric Count;
};
typedef struct tOutEstimLRDataSet tOutEstimLRDataSet;

struct tOutEstimParmsDataSet {
    EIT_BOOLEAN Requested;

    tVariableNumeric EstimatorId;
    tVariableChar AlgorithmName;
    tVariableChar FieldId;
    tVariableNumeric Fti;
    tVariableNumeric Imputed;
    tVariableNumeric DivisionByZero;
    tVariableNumeric NegativeImputation;
};
typedef struct tOutEstimParmsDataSet tOutEstimParmsDataSet;

struct tOutRandomErrorDataSet {
    EIT_BOOLEAN Requested;

    tVariableNumeric EstimatorId;
    tVariableChar AlgorithmName;
    tVariableChar Recipient;
    tVariableChar Donor;
    tVariableChar FieldId;

    tVariableNumeric Residual;
    tVariableNumeric RandomError;
    tVariableNumeric OriginalValue;
    tVariableNumeric ImputedValue;
};
typedef struct tOutRandomErrorDataSet tOutRandomErrorDataSet;

/* info needed for execution statistics */
struct tCounter {
/* # of HIST not paired on ID with DATA     */
/*    int UnpairedHistObs;*/

    /* 2 of each counters. 2 periods each.                                    */
    int Obs[2];           /* # of obs in data set                             */
    int ValidObs[2];      /* # of valid obs in data set                       */
    int MissingKeys[2];   /* # of missing key in data data set                */
    int MissingStatus[2]; /* # of missing key/fieldid in STATUS data set      */
    int MissingValues[2]; /* # of missing non FTI variable in data data sets  */
    int NegativeValues[2];/* # of negative non FTI variable in data data sets */
};
typedef struct tCounter tCounter;


static void AllocateDataDataSetVariables (tSList *, tDataDataSet *);
static EIT_BOOLEAN AreRandomErrorRequested (EIT_ESTIMATORSPEC_LIST *);
static void GetAllOriginalVarNames (SP_estimato* sp, tDataDataSet *, tDataDataSet *,
    tAllOriginalVarNames *);
static void CreateMappings (tDataDataSet *, tDataDataSet *, int **, int **);
static PROC_RETURN_CODE DefineGatherWriteAcceptable (DSW_generic* dsw, tOutAcceptableDataSet *);
static PROC_RETURN_CODE DefineGatherWriteData (DSW_generic* dsw, tDataDataSet *);
static PROC_RETURN_CODE DefineGatherWriteEstimEF (DSW_generic* dsw, tOutEstimEFDataSet *);
static PROC_RETURN_CODE DefineGatherWriteEstimLR (DSW_generic* dsw, tOutEstimLRDataSet *);
static PROC_RETURN_CODE DefineGatherWriteEstimParms (DSW_generic* dsw, tOutEstimParmsDataSet *,
    EIPT_POSITIVITY_OPTION);
static PROC_RETURN_CODE DefineGatherWriteRandomError (DSW_generic* dsw, tOutRandomErrorDataSet *);
static PROC_RETURN_CODE DefineGatherWriteStatus (DSW_generic* dsw, tStatusDataSet *);
static void DefineScatterReadAlgorithm (DSR_inalgorithm* dsr, tInAlgorithmDataSet *);
static IO_RETURN_CODE DefineScatterReadData (DSR_indata* dsr, tDataDataSet *);
static void DefineScatterReadEstimator (DSR_inestimator* dsr, tInEstimatorDataSet *);
static void DefineScatterReadStatus (DSR_instatus* dsr, tStatusDataSet *);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_estimato* sp, tDataDataSet *, tDataDataSet *);
static void FreeAllOriginalVarNames (tAllOriginalVarNames *);
static void FreeInAlgorithmDataSet (tInAlgorithmDataSet *);
static void FreeInDataDataSet (tDataDataSet *);
static void FreeInEstimatorDataSet (tInEstimatorDataSet *);
static void FreeInStatusDataSet (tStatusDataSet *);
static void FreeOutAcceptableDataSet (tOutAcceptableDataSet *);
static void FreeOutDataDataSet (tDataDataSet *);
static void FreeOutEstimEFDataSet (tOutEstimEFDataSet *);
static void FreeOutEstimLRDataSet (tOutEstimLRDataSet *);
static void FreeOutEstimParmsDataSet (tOutEstimParmsDataSet *);
static void FreeOutRandomErrorDataSet (tOutRandomErrorDataSet *);
static void FreeOutStatusDataSet (tStatusDataSet *);
static int * GetHistPositionsOfByVars (DSR_indata* dsr, int);
static IO_RETURN_CODE GetParms (SP_estimato* sp, tDataDataSet *, tDataDataSet *,
    tInAlgorithmDataSet *, tInEstimatorDataSet *,
    EIT_ALGORITHM_LIST *, EIT_ESTIMATORSPEC_LIST *,
    tDataDataSet *, tOutRandomErrorDataSet *,
    tOutAcceptableDataSet *, tOutEstimParmsDataSet *, tOutEstimEFDataSet *,
    tOutEstimLRDataSet *, EIT_BOOLEAN *, EIPT_POSITIVITY_OPTION *, int *);
static void InitDataSets (EIT_ESTIMATORSPEC_LIST *, tDataDataSet *,
    tDataDataSet *, tDataDataSet *);
static int Intersect (int *, int, int *, int);
static IO_RETURN_CODE LoadAlgorithm (DSR_inalgorithm* dsr, tInAlgorithmDataSet *, EIT_ALGORITHM_LIST *);
static IO_RETURN_CODE LoadEstimator (DSR_inestimator* dsr, tInEstimatorDataSet *, EIT_ESTIMATORSPEC_LIST *);
static void PrintEstimatorSpecifications (EIT_ESTIMATORSPEC_LIST *,
    EIT_ALGORITHM_LIST *);
static void PrintIntArray (char *, int *, int);
static void PrintFormulas (EIT_ESTIMATORSPEC_LIST *);
static void PrintImputationStatistics (EIT_ESTIMATORSPEC_LIST *);
static void PrintImputationStatisticsRejectNegative (EIT_ESTIMATORSPEC_LIST *);
static void PrintMissingStatusCount (char *, tCounter *);
static void PrintParameters (EIT_ESTIMATORSPEC_LIST *, tCounter *);
static void PrintParms (SP_estimato* sp, tDataDataSet *, tDataDataSet *,
    EIT_ALGORITHM_LIST *, EIT_ESTIMATORSPEC_LIST *, 
    EIT_BOOLEAN, EIPT_POSITIVITY_OPTION, int ByGroupLoggingLevel);
static void PrintStatistics (SP_estimato* sp, tCounter *, char *);
static PROC_RETURN_CODE ProcessDataGroups (SP_estimato* sp, tDataDataSet *, tStatusDataSet *,
    tDataDataSet *, tStatusDataSet *, tDataDataSet *, tStatusDataSet *,
    tOutRandomErrorDataSet *, tOutAcceptableDataSet *, tOutEstimParmsDataSet *,
    tOutEstimEFDataSet *, tOutEstimLRDataSet *, EIT_ESTIMATORSPEC_LIST *,
    EIT_SYMBOL **, EIPT_POSITIVITY_OPTION, EIT_BOOLEAN, int);
static PROC_RETURN_CODE ReadAlgorithm (DSR_inalgorithm* dsr, tInAlgorithmDataSet *, EIT_ALGORITHM_LIST *);
static PROC_RETURN_CODE ReadByGroup (SP_estimato* sp, tDataDataSet *, tStatusDataSet *,
    tDataDataSet *, tStatusDataSet *, EIT_DATATABLE *, tCounter *, int);
static PROC_RETURN_CODE ReadData (DSR_indata* dsr, tDataDataSet *, EIT_EXCLUSION, EIT_DATATABLE *,
    tCounter *);
static PROC_RETURN_CODE ReadEstimator (DSR_inestimator* dsr, tInEstimatorDataSet *, EIT_ESTIMATORSPEC_LIST *);
static PROC_RETURN_CODE ReadHist (DSR_indata* dsr, tDataDataSet *, EIT_DATATABLE *, tCounter *);
static PROC_RETURN_CODE ReadStatus (DSR_indata* dsr_indata, DSR_instatus* dsr_instatus, tDataDataSet *, tStatusDataSet *, int,
    EIT_DATATABLE *, tCounter *);
static void RemoveUnflagged (EIT_DATATABLE *, EIPT_POSITIVITY_OPTION,
    tCounter *);
static void SetAllRandomErrorSpecifications (EIT_ESTIMATORSPEC_LIST *, char *);
static IO_RETURN_CODE SetUpDataSets (SP_estimato* sp, tDataDataSet *, tStatusDataSet *, tDataDataSet *,
    tStatusDataSet *, tDataDataSet *, tStatusDataSet *,tOutRandomErrorDataSet *,
    tOutAcceptableDataSet *, tOutEstimParmsDataSet *, tOutEstimEFDataSet *,
    tOutEstimLRDataSet *, EIPT_POSITIVITY_OPTION, EIT_BOOLEAN);
static IO_RETURN_CODE SetUpInAlgorithmDataSet (DSR_inalgorithm* dsr, tInAlgorithmDataSet *);
static IO_RETURN_CODE SetUpInDataDataSet (DSR_indata* dsr, tDataDataSet *);
static IO_RETURN_CODE SetUpInEstimatorDataSet (DSR_inestimator* dsr, tInEstimatorDataSet *);
static IO_RETURN_CODE SetUpInStatusDataSet (DSR_instatus* dsr, tStatusDataSet *);
static void ShowTime (char *);
static void UnloadAlgorithm (EIT_ALGORITHM_LIST *);
static void UnloadEstimator (EIT_ESTIMATORSPEC_LIST *);
static void UnsetUpDataSets (SP_estimato* sp, tDataDataSet *, tStatusDataSet *, tDataDataSet *,
    tStatusDataSet *, tDataDataSet *, tStatusDataSet *,
    tOutRandomErrorDataSet *, tOutAcceptableDataSet *, tOutEstimParmsDataSet *,
    tOutEstimEFDataSet *, tOutEstimLRDataSet *,EIT_BOOLEAN);
static void ValidateExclVarStatement (SP_estimato* sp, EIT_ESTIMATORSPEC_LIST *);
static EIT_RETURNCODE ValidateParms (SP_estimato* sp, 
    tOutRandomErrorDataSet *, tOutAcceptableDataSet *, tOutEstimParmsDataSet *,
    tOutEstimEFDataSet *, tOutEstimLRDataSet *, EIT_ESTIMATORSPEC_LIST *,
    EIT_SYMBOL **, EIPT_POSITIVITY_OPTION);
static void VariableCharAllocate(tVariableChar* Variable);
static IO_RETURN_CODE VariableCharDefineForInput (DSR_generic* dsr, char *, tVariableChar *);
static PROC_RETURN_CODE VariableCharDefineForOutput(DSW_generic* dsw, int destination_index, tVariableChar *);
static void VariableCharFree (tVariableChar *);
static void VariableCharPrint (tVariableChar *);
static void VariableCharSet (tVariableChar *, char *);
static IO_RETURN_CODE VariableNumericDefineForInput (DSR_generic* dsr, char *,
    tVariableNumeric *);
static PROC_RETURN_CODE VariableNumericDefineForOutput (DSW_generic* dsw, int destination_index, tVariableNumeric *);
static void VariableNumericPrint (tVariableNumeric *);
static void VariableNumericSet (tVariableNumeric *, double);
static PROC_RETURN_CODE WriteResults (SP_estimato* sp, EIT_ESTIMATORSPEC_LIST *, EIT_DATATABLE *,
    tDataDataSet *, tStatusDataSet *, tOutRandomErrorDataSet *,
    tOutAcceptableDataSet *, tOutEstimParmsDataSet *, tOutEstimEFDataSet *,
    tOutEstimLRDataSet *, int *, int *, tAllOriginalVarNames,
    EIPT_POSITIVITY_OPTION);



/*********************************************************************
*********************************************************************/
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int estimato(
    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_instatus,
    T_in_ds in_ds_indata_hist,
    T_in_ds in_ds_instatus_hist,
    T_in_ds in_ds_inalgorithm,
    T_in_ds in_ds_inestimator,

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
)
{
    EIT_ALGORITHM_LIST * AlgorithmList;
    EIT_ESTIMATORSPEC_LIST * EstimatorList;
    tInAlgorithmDataSet InAlgorithm;
    tDataDataSet InCurr;
    tStatusDataSet InCurrStatus;
    tInEstimatorDataSet InEstimator;
    tDataDataSet InHist;
    tStatusDataSet InHistStatus;
    tOutAcceptableDataSet OutAcceptable;
    tDataDataSet OutData;
    tOutEstimEFDataSet OutEstimEF;
    tOutEstimLRDataSet OutEstimLR;
    tOutEstimParmsDataSet OutEstimParms;
    tOutRandomErrorDataSet OutRandomError;
    tStatusDataSet OutStatus;
    EIPT_POSITIVITY_OPTION PositivityOption;
    EIT_RETURNCODE rc;
    EIT_SYMBOL ** SymbolTable;
    EIT_BOOLEAN VerifySpecsOption;
	int ByGroupLoggingLevel;

	BANFF_RETURN_CODE proc_exit_code = BRC_SUCCESS;

    IO_RETURN_CODE rc_io = IORC_SUCCESS;

    /* Initialize runtime environment */
    init_runtime_env();

    /* TIME MEASUREMENT */
    TIME_WALL_DECLARE(cleanup);
    TIME_CPU_DECLARE(cleanup);
    TIME_WALL_START(main);
    TIME_CPU_START(main);
/*
to do?
Valider estimator auxvariable: doit etre \"nom1,nom2,etc\"
ajouter compteur indiquant le nombre de HIST pas matché avec DATA
*/
#ifdef _CRTDBG_MAP_ALLOC
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	STC_AllocateMemorySetCB(SUtil_AllocateMemory);
	STC_ReallocationMemorySetCB(SUtil_ReallocateMemory);
	STC_FreeMemorySetCB(SUtil_FreeMemory);

	EI_PrintMessagesSetCB (SUtil_PrintMessages);

    EI_AllocateMessageList ();

	UTIL_ResetTime ();

    /* SI : System Info */
    SUtil_PrintSystemInfo (SI_SYSTEMNAME, SI_SYSTEMVERSION, BANFF_PROC_NAME,
        SI_PROCVERSION, SI_EMAIL, NULL);

    /* Initialize JSON input/output related variables */
    // master: Statcan Procedurestructure
    SP_estimato        sp                 = { 0 };

    TIME_WALL_START(load_init);
    TIME_CPU_START(load_init);
    mem_usage("before SP_init");
    IO_RETURN_CODE rc_sp_init = SP_init(
        &sp, 
        in_parms,
        in_ds_inalgorithm,
        in_ds_indata,
        in_ds_indata_hist,
        in_ds_inestimator,
        in_ds_instatus,
        in_ds_instatus_hist,
        out_sch_outdata,
        out_arr_outdata,
        out_sch_outstatus,
        out_arr_outstatus,
        out_sch_outacceptable,
        out_arr_outacceptable,
        out_sch_outest_ef,
        out_arr_outest_ef,
        out_sch_outest_lr,
        out_arr_outest_lr,
        out_sch_outest_parm,
        out_arr_outest_parm,
        out_sch_outrand_err,
        out_arr_outrand_err
    );
    mem_usage("after SP_init");

    proc_exit_code = SP_validate_init(rc_sp_init);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }

    IO_RETURN_CODE rc_validation = SPG_validate((SP_generic*) &sp);
    if (rc_validation != IORC_SUCCESS) {
        proc_exit_code = BRC_FAIL_VALIDATION_NEW;
        goto error_cleanup;
    }

    AlgorithmList = EI_AlgorithmAllocate ();
    EstimatorList = EI_EstimatorAllocate ();
    SymbolTable = EI_SymbolTableAllocate ();

    rc_io = GetParms (&sp, &InCurr, &InHist,
        &InAlgorithm, &InEstimator, AlgorithmList,
        EstimatorList, &OutData, &OutRandomError, &OutAcceptable,
        &OutEstimParms, &OutEstimEF, &OutEstimLR, &VerifySpecsOption,
        &PositivityOption, &ByGroupLoggingLevel);
    if (rc_io != IORC_SUCCESS) {
        proc_exit_code = BRC_FAIL_READ_PARMS_LEGACY;
        goto error_cleanup;
    }

    PrintParms (&sp, &InCurr, &InHist,
        AlgorithmList, EstimatorList,
       VerifySpecsOption,
        PositivityOption, ByGroupLoggingLevel);

    ShowTime ("Printing parameters completed");

    /* Validate Parameters: except exclusivity between "lists" variables */
    rc = ValidateParms (&sp, &OutRandomError, &OutAcceptable, &OutEstimParms,
        &OutEstimEF, &OutEstimLR, EstimatorList, SymbolTable, PositivityOption);
	if (rc != EIE_SUCCEED) {
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
	}

    PrintFormulas (EstimatorList);

    ShowTime ("Parameter validation completed");

    InitDataSets (EstimatorList, &InCurr, &InHist, &OutData);

    rc_io = SetUpDataSets (&sp, &InCurr, &InCurrStatus, &InHist, &InHistStatus,
        &OutData, &OutStatus, &OutRandomError, &OutAcceptable,
        &OutEstimParms, &OutEstimEF, &OutEstimLR,
        PositivityOption, VerifySpecsOption);
    if (rc_io != IORC_SUCCESS) {
        if (rc_io == IORC_VARLIST_NOT_FOUND) {
            proc_exit_code = BRC_FAIL_VARLIST_NOT_FOUND;
        }
        else if (rc_io == IORC_FAIL_INIT_OUT_DATASET) {
            proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        }
        else {
            proc_exit_code = BRC_FAIL_SETUP_OTHER;
        }
        goto error_cleanup;
    }

    EI_SetUpEstimators (EstimatorList,
        InCurr.VariableName, InCurr.NumberOfVariables,
        InHist.VariableName, InHist.NumberOfVariables,
        OutRandomError.Requested, OutAcceptable.Requested);

    ShowTime ("Set up completed");

    TIME_CPU_STOPDIFF(load_init);
    TIME_WALL_STOPDIFF(load_init);

    TIME_WALL_START(processing);
    TIME_CPU_START(processing);

    /* bulk of processing performed in ProcessDataGroups () */
    PROC_RETURN_CODE rc_processing = ProcessDataGroups (&sp, &InCurr, &InCurrStatus, &InHist, &InHistStatus,
        &OutData, &OutStatus, &OutRandomError, &OutAcceptable, &OutEstimParms,
        &OutEstimEF, &OutEstimLR, EstimatorList, SymbolTable, PositivityOption,
        VerifySpecsOption, ByGroupLoggingLevel);
    TIME_CPU_STOPDIFF(processing);
    TIME_WALL_STOPDIFF(processing);

    proc_exit_code = convert_processing_rc(rc_processing);

    if (proc_exit_code == BRC_SUCCESS) {
        IO_PRINT("\n");
        mem_usage("Before SP_wrap");
        if (PRC_SUCCESS != SP_wrap(&sp)) {
            proc_exit_code = BRC_FAIL_WRITE_GENERIC;
            goto error_cleanup;
        }
        mem_usage("After SP_wrap");
    }

    TIME_WALL_BEGIN(cleanup);
    TIME_CPU_BEGIN(cleanup);
    EI_SymbolTableFree (SymbolTable);

    /* always unload estimator before algorithm */
    UnloadEstimator (EstimatorList);
    UnloadAlgorithm (AlgorithmList);

    UnsetUpDataSets (&sp, &InCurr, &InCurrStatus, &InHist, &InHistStatus,
        &OutData, &OutStatus, &OutRandomError, &OutAcceptable, &OutEstimParms,
        &OutEstimEF, &OutEstimLR, VerifySpecsOption);

    ShowTime ("Procedure completed");

error_cleanup:
	if ((EI_mMessageList_Message_is_NULL() != 1)
		) {
		EI_FreeMessageList();
		EI_mMessageList_Message_to_NULL();
	}

    /* free Statcan Procedureand children */
    mem_usage("Before SPG_free");
    SPG_free((SP_generic* )&sp);
    mem_usage("After SPG_free");

    /* TIME MEASUREMENT */
    TIME_CPU_STOPDIFF(cleanup);
    TIME_WALL_STOPDIFF(cleanup);
    TIME_CPU_STOPDIFF(main);
    TIME_WALL_STOPDIFF(main);

    deinit_runtime_env();

    return proc_exit_code;
}


/*********************************************************************
Allocates unique variables to read from DATA, HIST or to write to OUTDATA
*********************************************************************/
static void AllocateDataDataSetVariables (
    tSList * Variable,
    tDataDataSet * Data)
{
    int i;

    SList_Sort (Variable, eSListSortAscending);
    Data->NumberOfVariables = SList_NumEntries (Variable);

    Data->Variable = STC_AllocateMemory (
        Data->NumberOfVariables * sizeof *Data->Variable);

    Data->VariableName = STC_AllocateMemory (
        Data->NumberOfVariables * sizeof *Data->VariableName);

    for (i = 0; i < Data->NumberOfVariables; i++) {
        Data->VariableName[i] = STC_StrDup (SList_Entry (Variable, i));
        IOUtil_copy_varname(Data->Variable[i].Name, Data->VariableName[i]);
        Data->Variable[i].Position = -1;
        Data->Variable[i].Size = sizeof Data->Variable[i].Value;
        Data->Variable[i].Value = EIM_MISSING_VALUE;
    }
}
/*********************************************************************
checks if at least one estimator computes a random error
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN AreRandomErrorRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++)
        if (EstimatorList->Estimator[i].RandomError[0] == 'Y')
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
Create 2 mappings between current input var positions and output var positions.
These mappings are necessary when writing results to easily find info from
one to the other.

InToOutMapping is an array of the dimension of InCurr.
The value in position i in current input data set will be found at
position Mapping[i] in output data set

OutToInMapping is an array of the dimension of OutData.
The value in position i in output data set will be found at
position Mapping[i] in current input data set
*********************************************************************/
static void CreateMappings (
    tDataDataSet * InCurr,
    tDataDataSet * OutData,
    int ** InToOutMapping,
    int ** OutToInMapping)
{
    int i;
    int j;
    int * Mapping;

    /* create out to in mapping */
    Mapping = STC_AllocateMemory (OutData->NumberOfVariables * sizeof *Mapping);
    for (i = 0; i < OutData->NumberOfVariables; i++) {
        for (j = 0; j < InCurr->NumberOfVariables; j++) {
            if (UTIL_StrICmp (OutData->Variable[i].Name,
                    InCurr->Variable[j].Name) == 0) {
                Mapping[i] = j;
                j = InCurr->NumberOfVariables; /* quit if found */
            }
        }
    }
/*    PrintIntArray ("OutToIn", Mapping, OutData->NumberOfVariables);*/
    *OutToInMapping = Mapping;

    Mapping = STC_AllocateMemory (InCurr->NumberOfVariables * sizeof *Mapping);
    for (i = 0; i < InCurr->NumberOfVariables; i++) {
        Mapping[i] = -1;
        for (j = 0; j < OutData->NumberOfVariables; j++) {
            if (UTIL_StrICmp (InCurr->Variable[i].Name,
                    OutData->Variable[j].Name) == 0) {
                Mapping[i] = j;
                j = OutData->NumberOfVariables; /* quit if found */
            }
        }
    }
/*    PrintIntArray ("InToOut", Mapping, InCurr->NumberOfVariables);*/
    *InToOutMapping = Mapping;
}
/*********************************************************************
*********************************************************************/
static void PrintIntArray (
    char * msg,
    int * I,
    int n)
{
    int i;

    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s: ", msg);
    for (i=0; i < n; i++) {
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%2d, ", I[i]);
    }
}
/*********************************************************************
Get original names of variables (other than "key") read in Current and
Historical data sets. Used to write in OUTESTEF and OUTESTLR.
*********************************************************************/
static void GetAllOriginalVarNames (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tDataDataSet * InHist,
    tAllOriginalVarNames * AllOriginalVarNames)
{
    int i;

    AllOriginalVarNames->CurrentVarName = NULL;
    AllOriginalVarNames->NumberOfCurrentVars = 0;
    AllOriginalVarNames->HistVarName = NULL;
    AllOriginalVarNames->NumberOfHistVars = 0;

    if (sp->dsr_indata.dsr.is_specified == IOSV_SPECIFIED) {
        AllOriginalVarNames->CurrentVarName = STC_AllocateMemory (
            InCurr->NumberOfVariables *
                sizeof *AllOriginalVarNames->CurrentVarName);

        AllOriginalVarNames->NumberOfCurrentVars = InCurr->NumberOfVariables;
        for (i = 0; i < InCurr->NumberOfVariables; i++) {
             AllOriginalVarNames->CurrentVarName[i] =
                 STC_StrDup (InCurr->VariableName[i]);
             SUtil_GetOriginalName(&sp->dsr_indata.dsr, AllOriginalVarNames->CurrentVarName[i]);
        }
    }

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        AllOriginalVarNames->HistVarName = STC_AllocateMemory (
            InHist->NumberOfVariables *
                sizeof *AllOriginalVarNames->HistVarName);

        AllOriginalVarNames->NumberOfHistVars = InHist->NumberOfVariables;
        for (i = 0; i < InHist->NumberOfVariables; i++) {
             AllOriginalVarNames->HistVarName[i] =
                 STC_StrDup (InHist->VariableName[i]);
             SUtil_GetOriginalName(&sp->dsr_indata_hist.dsr, AllOriginalVarNames->HistVarName[i]);
        }
    }
}
/*********************************************************************
Define gather write for output OUTACEPTABLE= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteAcceptable (
    DSW_generic* dsw,
    tOutAcceptableDataSet * OutAcceptable)
{
    /* initialize and allocate */
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTACCEPTABLE_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */

    IOUtil_copy_varname(OutAcceptable->EstimatorId.Name, OUTACCEPTABLE_ESTIMID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutAcceptable->EstimatorId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutAcceptable->AlgorithmName.Name, OUTACCEPTABLE_ALGORITHMNAME);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutAcceptable->AlgorithmName)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutAcceptable->Key)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output data OUT= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteData (
    DSW_generic* dsw,
    tDataDataSet * OutData)
{
    /* initialize and allocate */ /* + 1 for the key variable */
    if (IORC_SUCCESS != DSW_allocate(dsw, OutData->NumberOfVariables + 1)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */

    int i;

    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutData->Key)) {
        return PRC_FAIL_WRITE_DATA;
    }

    for (i = 0; i < OutData->NumberOfVariables; i++){
        if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutData->Variable[i])) {
            return PRC_FAIL_WRITE_DATA;
        }
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output OUTESTEF= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteEstimEF (
    DSW_generic* dsw,
    tOutEstimEFDataSet * OutEstimEF)
{
    /* initialize and allocate */
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTESTEF_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */

    IOUtil_copy_varname(OutEstimEF->EstimatorId.Name, OUTESTEF_ESTIMID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimEF->EstimatorId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimEF->AlgorithmName.Name, OUTESTEF_ALGORITHMNAME);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimEF->AlgorithmName)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimEF->FieldId.Name, OUTESTEF_FIELDID);
    OutEstimEF->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimEF->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimEF->Period.Name, OUTESTEF_PERIOD);
    OutEstimEF->Period.Size = OUTEST_PERIODSIZE;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimEF->Period)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimEF->AverageValue.Name, OUTESTEF_AVERAGEVALUE);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimEF->AverageValue)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimEF->Count.Name, OUTESTEF_COUNT);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimEF->Count)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output OUTESTLR= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteEstimLR (
    DSW_generic* dsw,
    tOutEstimLRDataSet * OutEstimLR)
{
    /* initialize and allocate */
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTESTLR_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */


    IOUtil_copy_varname(OutEstimLR->EstimatorId.Name, OUTESTLR_ESTIMID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimLR->EstimatorId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->AlgorithmName.Name, OUTESTLR_ALGORITHMNAME);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimLR->AlgorithmName)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->FieldId.Name, OUTESTLR_FIELDID);
    OutEstimLR->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimLR->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->Exponent.Name, OUTESTLR_EXPONENT);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimLR->Exponent)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->Period.Name, OUTESTLR_PERIOD);
    OutEstimLR->Period.Size = OUTEST_PERIODSIZE;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimLR->Period)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->BetaValue.Name, OUTESTLR_BETAVALUE);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimLR->BetaValue)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimLR->Count.Name, OUTESTLR_COUNT);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimLR->Count)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output OUTESTPARMS= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteEstimParms (
    DSW_generic* dsw,
    tOutEstimParmsDataSet * OutEstimParms,
    EIPT_POSITIVITY_OPTION PositivityOption) /* for NEGATIVE variable */
{
    /* initialize and allocate */
    if (PositivityOption == EIPE_ACCEPT_NEGATIVE) {
        if (IORC_SUCCESS != DSW_allocate(dsw, OUTESTPARMS_NBVARS - 1)) {
            return PRC_FAIL_WRITE_DATA;
        }
    } 
    else{
        if (IORC_SUCCESS != DSW_allocate(dsw, OUTESTPARMS_NBVARS)) {
            return PRC_FAIL_WRITE_DATA;
        }
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */

    IOUtil_copy_varname(OutEstimParms->EstimatorId.Name, OUTESTPARMS_ESTIMID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimParms->EstimatorId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimParms->AlgorithmName.Name, OUTESTPARMS_ALGORITHMNAME);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimParms->AlgorithmName)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimParms->FieldId.Name, OUTESTPARMS_FIELDID);
    OutEstimParms->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEstimParms->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimParms->Fti.Name, OUTESTPARMS_FTI);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimParms->Fti)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimParms->Imputed.Name, OUTESTPARMS_IMP);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimParms->Imputed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEstimParms->DivisionByZero.Name, OUTESTPARMS_DIVISIONBYZERO);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimParms->DivisionByZero)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (PositivityOption != EIPE_ACCEPT_NEGATIVE) {
        IOUtil_copy_varname(OutEstimParms->NegativeImputation.Name, OUTESTPARMS_NEGATIVE);
        if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutEstimParms->NegativeImputation)) {
            return PRC_FAIL_WRITE_DATA;
        }
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output OUTRANDOMERROR= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteRandomError (
    DSW_generic* dsw,
    tOutRandomErrorDataSet * OutRandomError)
{
    /* initialize and allocate */
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTRANDOMERROR_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* If we decide to add By variables, update the DSW_allocate() call above & add BY vars here */

    IOUtil_copy_varname(OutRandomError->EstimatorId.Name, OUTRANDOMERROR_ESTIMID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutRandomError->EstimatorId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->AlgorithmName.Name, OUTRANDOMERROR_ALGORITHMNAME);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutRandomError->AlgorithmName)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->Recipient.Name, OUTRANDOMERROR_RECIPIENT);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutRandomError->Recipient)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->Donor.Name, OUTRANDOMERROR_DONOR);
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutRandomError->Donor)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->FieldId.Name, OUTRANDOMERROR_FIELDID);
    OutRandomError->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutRandomError->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->Residual.Name, OUTRANDOMERROR_RESIDUAL);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutRandomError->Residual)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->RandomError.Name, OUTRANDOMERROR_RANDOMERROR);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutRandomError->RandomError)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->OriginalValue.Name, OUTRANDOMERROR_ORIGINALVALUE);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutRandomError->OriginalValue)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutRandomError->ImputedValue.Name, OUTRANDOMERROR_IMPUTEDVALUE);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++, &OutRandomError->ImputedValue)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Define gather write for output OUTSTATUS= data set
*********************************************************************/
static PROC_RETURN_CODE DefineGatherWriteStatus (
    DSW_generic* dsw,
    tStatusDataSet * OutStatus)
{
    /* allocate outstatus variables and then setup outstatus */
    VariableCharAllocate(&OutStatus->Key);

    IOUtil_copy_varname(OutStatus->FieldId.Name, DSC_FIELD_ID);
    OutStatus->FieldId.Size = LEGACY_MAXNAME;
    VariableCharAllocate(&OutStatus->FieldId);

    IOUtil_copy_varname(OutStatus->Status.Name, DSC_STATUS);
    OutStatus->Status.Size = EIM_STATUS_SIZE;
    VariableCharAllocate(&OutStatus->Status);

    return banff_setup_dsw_outstatus(
        dsw,
        OutStatus->Key.Name,
        (void*)OutStatus->Key.Value,
        (void*)OutStatus->FieldId.Value,
        (void*)OutStatus->Status.Value,
        (void*)&OutStatus->FieldValue
    );
}
/*********************************************************************
Define scatter read for algorithm data set
*********************************************************************/
static void DefineScatterReadAlgorithm (
    DSR_inalgorithm* dsr,
    tInAlgorithmDataSet * InAlgorithm)
{
    dsr->VL_algorithm_name.ptrs[0] = InAlgorithm->Name.Value;
    dsr->VL_status.ptrs[0] = InAlgorithm->Status.Value;
    dsr->VL_type.ptrs[0] =InAlgorithm->Type.Value ;
    dsr->VL_formula.ptrs[0] = InAlgorithm->Formula.Value;
    dsr->VL_description.ptrs[0] = InAlgorithm->Description.Value;
}
/*********************************************************************
Define scatter read for DATA and HIST data sets
*********************************************************************/
static IO_RETURN_CODE DefineScatterReadData (
    DSR_indata* dsw_indata,
    tDataDataSet * InData)
{
    DSR_generic* dsr = &dsw_indata->dsr;

    int i;
    int NumberOfVariables;

    NumberOfVariables = InData->NumberOfVariables;

    /* Define key variable for scatter read */
    dsr->VL_unit_id.ptrs[0] = InData->Key.Value;

    /* Exclusion Variable */
    if (InData->ExclusionInUse == EIE_TRUE) {
        dsw_indata->VL_data_excl.ptrs[0] = InData->ExclusionIndicator.Value;
    }

    /* "in var" variables - selected based on estimator dataset? */
    // create list of positions
    int* temp_vl_position_list = STC_AllocateMemory(sizeof(*temp_vl_position_list) * InData->NumberOfVariables);
    for (i = 0; i < InData->NumberOfVariables; i++) {
        temp_vl_position_list[i] = InData->Variable[i].Position;
    }
    // initialize Variable List
    IO_RETURN_CODE rc_vl_init = VL_init_from_position_list(&dsw_indata->dsr.VL_in_var, &dsw_indata->dsr, VL_NAME_in_var, IOVT_NUM, InData->NumberOfVariables, temp_vl_position_list);
    /* define other variables for scatter read */
    STC_FreeMemory(temp_vl_position_list);
    
    if (IORC_SUCCESS != rc_vl_init) {
        return rc_vl_init;
    }

    // set pointers
    for (i = 0; i < InData->NumberOfVariables; i++) {
        dsw_indata->dsr.VL_in_var.ptrs[i] = (void*)&InData->Variable[i].Value;
    }

    return IORC_SUCCESS;
}
/*********************************************************************
Define scatter read for ESTIMATOR data set
*********************************************************************/
static void DefineScatterReadEstimator (
    DSR_inestimator* dsr,
    tInEstimatorDataSet * InEstimator)
{
    tVariableChar * VariableChar;
    tVariableNumeric * VariableNumeric;

    VariableChar = &InEstimator->AlgorithmName;
    dsr->VL_algorithm_name.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->FieldId;
    dsr->VL_fieldid.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->AuxVariables;
    dsr->VL_aux_vars.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->WeightVariable;
    dsr->VL_weight_var.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->VarianceVariable;
    dsr->VL_variance_var.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->VariancePeriod;
    dsr->VL_variance_period.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->ExcludeImputed;
    dsr->VL_exclude_imputed.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->ExcludeOutliers;
    dsr->VL_exclude_outliers.ptrs[0] = VariableChar->Value;

    VariableChar = &InEstimator->RandomError;
    dsr->VL_random_error.ptrs[0] = VariableChar->Value;

    VariableNumeric = &InEstimator->VarianceExponent;
    dsr->VL_variance_exponent.ptrs[0] = (void*)&VariableNumeric->Value;

    VariableNumeric = &InEstimator->CountCriteria;
    dsr->VL_count_criteria.ptrs[0] = (void*)&VariableNumeric->Value;

    VariableNumeric = &InEstimator->PercentCriteria;
    dsr->VL_percent_criteria.ptrs[0] = (void*)&VariableNumeric->Value;
}
/*********************************************************************
Define scatter read for input status data set
*********************************************************************/
static void DefineScatterReadStatus (
    DSR_instatus* dsr,
    tStatusDataSet * InStatus)
{
    banff_setup_dsr_instatus(
        dsr,
        (void*)InStatus->Key.Value,
        (void*)InStatus->FieldId.Value,
        (void*)InStatus->Status.Value
    );
}
/*********************************************************************
Verify that the same variable do not appear in more then one place:
- ID vs DATAEXCLVARSTMT (character variable)
- ID vs HISTEXCLVARSTMT (character variable)
- ID vs BY (character variable)
- DATAEXCLVARSTMT vs BY (character variable)
- HISTEXCLVARSTMT vs BY (character variable)
- BY vs Estimator specification (numeric variables)
*********************************************************************/
static EIT_RETURNCODE ExclusivityBetweenLists (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tDataDataSet * InHist)
{
    int * ByCurrList;
    int * ByHistList;
    int crc;
    int * DataExclVarList;
    int * HistExclVarList;
    int i;
    int * IdCurrList;
    int IdHistList[1];
    int List[1];
    int NumberOfByVariables;
    int NumberOfCurrIdVariables;
    int NumberOfDataExclVarVariables;
    int NumberOfHistExclVarVariables;
    int VariablePosition;

    crc = EIE_SUCCEED;

    ByCurrList = (int *) sp->dsr_indata.dsr.VL_by_var.positions;
    NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;

    DataExclVarList = (int *) sp->dsr_indata.VL_data_excl.positions;
    NumberOfDataExclVarVariables = sp->dsr_indata.VL_data_excl.count;

    /* grammar: lookup data set is "InHist" for HISTEXCLVARSTMT */
    HistExclVarList = (int *)sp->dsr_indata_hist.VL_data_excl.positions;
    NumberOfHistExclVarVariables = sp->dsr_indata_hist.VL_data_excl.count;

    IdCurrList = (int *)sp->dsr_indata.dsr.VL_unit_id.positions;
    NumberOfCurrIdVariables = sp->dsr_indata.dsr.VL_unit_id.count;

    /* Validation for InData */
    VariablePosition = Intersect (IdCurrList, NumberOfCurrIdVariables,
        DataExclVarList, NumberOfDataExclVarVariables);
    if (VariablePosition != NOTFOUND) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
            sp->dsr_indata.dsr.col_names[VariablePosition], BPN_UNIT_ID, BPN_DATA_EXCL_VAR);
        crc = EIE_FAIL;
    }

    if (NumberOfByVariables > 0) {

        VariablePosition = Intersect (IdCurrList, NumberOfCurrIdVariables,
            ByCurrList, NumberOfByVariables);
        if (VariablePosition != NOTFOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
                sp->dsr_indata.dsr.col_names[VariablePosition], BPN_UNIT_ID, BPN_BY);
            crc = EIE_FAIL;
        }

        VariablePosition = Intersect (DataExclVarList,
            NumberOfDataExclVarVariables, ByCurrList, NumberOfByVariables);
        if (VariablePosition != NOTFOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
                sp->dsr_indata.dsr.col_names[VariablePosition], BPN_DATA_EXCL_VAR, BPN_BY);
            crc = EIE_FAIL;
        }

        for (i = 0; i < InCurr->NumberOfVariables; i++) {
            List[0] = InCurr->Variable[i].Position;

            VariablePosition = Intersect (List, 1, ByCurrList,
                NumberOfByVariables);
            if (VariablePosition != NOTFOUND) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInByAndCurrentEstimatorSpecs,
                    sp->dsr_indata.dsr.col_names[VariablePosition]);
                crc = EIE_FAIL;
            }
        }
    }

    /* Validation for InHist */
    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        /* (at this step, already validate that Key variable exists
            in HIST data set) */
        IdHistList[0] = InHist->Key.Position;

        VariablePosition = Intersect (IdHistList, 1,
            HistExclVarList, NumberOfHistExclVarVariables);
        if (VariablePosition != NOTFOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
                sp->dsr_indata_hist.dsr.col_names[VariablePosition], BPN_UNIT_ID, BPN_DATA_EXCL_VAR_HIST);
            crc = EIE_FAIL;
        }

        if (NumberOfByVariables > 0) {
            /* Position of BY variables in HIST data set
               (at this step, already validate that all BY variables are in HIST
               data set but we need their position) */
            ByHistList = GetHistPositionsOfByVars (&sp->dsr_indata_hist,
                NumberOfByVariables);

            VariablePosition = Intersect (HistExclVarList,
                NumberOfHistExclVarVariables, ByHistList, NumberOfByVariables);
            if (VariablePosition != NOTFOUND) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
                    sp->dsr_indata_hist.dsr.col_names[VariablePosition], BPN_DATA_EXCL_VAR_HIST, BPN_BY);
                crc = EIE_FAIL;
            }

            for (i = 0; i < InHist->NumberOfVariables; i++) {
                List[0] = InHist->Variable[i].Position;

                VariablePosition = Intersect (List, 1, ByHistList,
                    NumberOfByVariables);
                if (VariablePosition != NOTFOUND) {
                    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInByAndHistoricalEstimatorSpecs,
                        sp->dsr_indata_hist.dsr.col_names[VariablePosition]);
                    crc = EIE_FAIL;
                }
            }

            STC_FreeMemory (ByHistList);

        } /* NumberOfByVariables > 0 */

    } /* InHist->File != NULL */

    return crc;
}
/*********************************************************************
Free space allocated for structure tAllOriginalVarNames
*********************************************************************/
static void FreeAllOriginalVarNames (
    tAllOriginalVarNames * AllOriginalVarNames)
{
    int i;

    for (i = 0; i< AllOriginalVarNames->NumberOfCurrentVars; i++)
        STC_FreeMemory (AllOriginalVarNames->CurrentVarName[i]);
    STC_FreeMemory (AllOriginalVarNames->CurrentVarName);

    for (i = 0; i< AllOriginalVarNames->NumberOfHistVars; i++)
        STC_FreeMemory (AllOriginalVarNames->HistVarName[i]);
    STC_FreeMemory (AllOriginalVarNames->HistVarName);

}
/*********************************************************************
Free InAlgorithm space and stuff
*********************************************************************/
static void FreeInAlgorithmDataSet (
    tInAlgorithmDataSet * InAlgorithm)
{
    VariableCharFree (&InAlgorithm->Name);
    VariableCharFree (&InAlgorithm->Type);
    VariableCharFree (&InAlgorithm->Status);
    VariableCharFree (&InAlgorithm->Formula);
    VariableCharFree (&InAlgorithm->Description);
}
/*********************************************************************
Free space and stuff for DATA and HIST data sets
*********************************************************************/
static void FreeInDataDataSet (
    tDataDataSet * InData)
{
    int i;
    VariableCharFree (&InData->Key);

    if (InData->ExclusionInUse == EIE_TRUE)
        VariableCharFree (&InData->ExclusionIndicator);

    for (i = 0; i < InData->NumberOfVariables; i++)
        STC_FreeMemory (InData->VariableName[i]);
    STC_FreeMemory (InData->VariableName);

    STC_FreeMemory (InData->Variable);
}
/*********************************************************************
Free InEstimator space and stuff
*********************************************************************/
static void FreeInEstimatorDataSet (
    tInEstimatorDataSet * InEstimator)
{
    VariableCharFree (&InEstimator->AlgorithmName);
    VariableCharFree (&InEstimator->FieldId);
    VariableCharFree (&InEstimator->AuxVariables);
    VariableCharFree (&InEstimator->WeightVariable);
    VariableCharFree (&InEstimator->VarianceVariable);
    VariableCharFree (&InEstimator->VariancePeriod);
    VariableCharFree (&InEstimator->ExcludeImputed);
    VariableCharFree (&InEstimator->ExcludeOutliers);
    VariableCharFree (&InEstimator->RandomError);
}
/*********************************************************************
Free InStatus space and stuff
*********************************************************************/
static void FreeInStatusDataSet (
    tStatusDataSet * InStatus)
{
    VariableCharFree (&InStatus->Key);
    VariableCharFree (&InStatus->FieldId);
    VariableCharFree (&InStatus->Status);
}
/*********************************************************************
Free OutAcceptable space and stuff
*********************************************************************/
static void FreeOutAcceptableDataSet (
    tOutAcceptableDataSet * OutAcceptable)
{
    VariableCharFree (&OutAcceptable->AlgorithmName);
    VariableCharFree (&OutAcceptable->Key);
}
/*********************************************************************
Free OutData space and stuff
*********************************************************************/
static void FreeOutDataDataSet (
    tDataDataSet * OutData)
{
    int i;
    VariableCharFree (&OutData->Key);

    for (i = 0; i < OutData->NumberOfVariables; i++)
        STC_FreeMemory (OutData->VariableName[i]);
    STC_FreeMemory (OutData->VariableName);

    STC_FreeMemory (OutData->Variable);
}
/*********************************************************************
Free OutEstimEF space and stuff
*********************************************************************/
static void FreeOutEstimEFDataSet (
    tOutEstimEFDataSet * OutEstimEF)
{
    VariableCharFree (&OutEstimEF->AlgorithmName);
    VariableCharFree (&OutEstimEF->FieldId);
    VariableCharFree (&OutEstimEF->Period);
}
/*********************************************************************
Free OutEstimLR space and stuff
*********************************************************************/
static void FreeOutEstimLRDataSet (
    tOutEstimLRDataSet * OutEstimLR)
{
    VariableCharFree (&OutEstimLR->AlgorithmName);
    VariableCharFree (&OutEstimLR->FieldId);
    VariableCharFree (&OutEstimLR->Period);
}
/*********************************************************************
Free OutEstimParms space and stuff
*********************************************************************/
static void FreeOutEstimParmsDataSet (
    tOutEstimParmsDataSet * OutEstimParms)
{
    VariableCharFree (&OutEstimParms->AlgorithmName);
    VariableCharFree (&OutEstimParms->FieldId);
}
/*********************************************************************
Free OutRandomError space and stuff
*********************************************************************/
static void FreeOutRandomErrorDataSet (
    tOutRandomErrorDataSet * OutRandomError)
{
    VariableCharFree (&OutRandomError->AlgorithmName);
    VariableCharFree (&OutRandomError->Recipient);
    VariableCharFree (&OutRandomError->Donor);
    VariableCharFree (&OutRandomError->FieldId);
}
/*********************************************************************
Free OutStatus space and stuff
*********************************************************************/
static void FreeOutStatusDataSet (
    tStatusDataSet * OutStatus)
{
    VariableCharFree (&OutStatus->Key);
    VariableCharFree (&OutStatus->FieldId);
    VariableCharFree (&OutStatus->Status);
}
/*********************************************************************
Get position of BY variables in HIST data set
- already validated that BY variables exist in HIST data set
- NumberOfByVariables > 0
*********************************************************************/
static int * GetHistPositionsOfByVars (
    DSR_indata* dsr,
    int NumberOfByVariables)
{
    int * ByHistList;
    int i;

    ByHistList = STC_AllocateMemory (
        NumberOfByVariables * sizeof *ByHistList);

    /* already validated that all BY variables are in HIST data set */
    for (i = 0; i < NumberOfByVariables; i++) {
         ByHistList[i] = dsr->dsr.VL_by_var.positions[i];
    }

    return ByHistList;
}
/*********************************************************************
Get procedure parameters.
Set default value to parameter not specified.
*********************************************************************/
static IO_RETURN_CODE GetParms (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tDataDataSet * InHist,
    tInAlgorithmDataSet * InAlgorithm,
    tInEstimatorDataSet * InEstimator,
    EIT_ALGORITHM_LIST * AlgorithmList,
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    tDataDataSet * OutData,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    EIT_BOOLEAN * VerifySpecsOption,
    EIPT_POSITIVITY_OPTION * PositivityOption,
	int * ByGroupLoggingLevel)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;

    if (sp->verify_specs.meta.is_specified == IOSV_SPECIFIED && sp->verify_specs.value == IOB_TRUE)
        *VerifySpecsOption = EIE_TRUE;
    else
        *VerifySpecsOption = EIE_FALSE;

    /* Get the positivity option set by the user */
    if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
        *PositivityOption = EIPE_REJECT_NEGATIVE_DEFAULT;
    else if (sp->accept_negative.value == IOB_FALSE)
        *PositivityOption = EIPE_REJECT_NEGATIVE;
    else if (sp->accept_negative.value == IOB_TRUE)
        *PositivityOption = EIPE_ACCEPT_NEGATIVE;

	/* Get the reduce logging value set by the user */
    if (
        sp->no_by_stats.meta.is_specified == IOSV_SPECIFIED
        && sp->no_by_stats.value == IOB_TRUE
        && VL_is_specified(&sp->dsr_indata.dsr.VL_by_var)
        ) {
        *ByGroupLoggingLevel = 1;
    }
    else {
        *ByGroupLoggingLevel = 0;
    }

    if (VL_is_specified(&sp->dsr_indata.VL_data_excl))
        InCurr->ExclusionInUse = EIE_TRUE;
    else
        InCurr->ExclusionInUse = EIE_FALSE;

    if (VL_is_specified(&sp->dsr_indata_hist.VL_data_excl))
        InHist->ExclusionInUse = EIE_TRUE;
    else
        InHist->ExclusionInUse = EIE_FALSE;

    OutData->ExclusionInUse = EIE_FALSE; /* irrelevant for OutData */

    /* always load algorithm before estimator */
    rc = LoadAlgorithm (&sp->dsr_inalgorithm, InAlgorithm, AlgorithmList);
    if (rc != IORC_SUCCESS) {
        return rc;
    }
    
    rc = LoadEstimator (&sp->dsr_inestimator, InEstimator, EstimatorList);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    /* set AlgorithmName.Size in out report data set */
    OutRandomError->AlgorithmName.Size = InEstimator->AlgorithmName.Size;
    OutAcceptable->AlgorithmName.Size  = InEstimator->AlgorithmName.Size;
    OutEstimParms->AlgorithmName.Size  = InEstimator->AlgorithmName.Size;
    OutEstimEF->AlgorithmName.Size     = InEstimator->AlgorithmName.Size;
    OutEstimLR->AlgorithmName.Size     = InEstimator->AlgorithmName.Size;

    return rc;
}
/*********************************************************************
Finds unique variables to read from DATA, HIST and to write to OUTDATA
*********************************************************************/
static void InitDataSets (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    tDataDataSet * InCurr,
    tDataDataSet * InHist,
    tDataDataSet * OutData)
{
    int i;
    int j;
    tSList * ListData; /* var to read from DATA data set */
    tSList * ListHist; /* var to read from HIST data set */
    tSList * ListOutData; /* var to write to OUTDATA data set */
    EIT_STACK * Stack;

    /* create list of variable to read (one for each period) and write */
    SList_New (&ListData);
    SList_New (&ListHist);
    SList_New (&ListOutData);

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        Stack = EstimatorList->Estimator[i].Stack;

        /* read fieldid from DATA data set */
        SList_AddNoDup (EstimatorList->Estimator[i].FieldId, ListData);

        /* write fieldid to OUTDATA data set */
        SList_AddNoDup (EstimatorList->Estimator[i].FieldId, ListOutData);

        /* read variables from either DATA or HIST data set */
        for (j = 0; j < Stack->NumberEntries; j++) {
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                    strncmp (Stack->Entry[j].Symbol->Name, EIM_BETA_PREFIX,
                        strlen (EIM_BETA_PREFIX)) != 0) {  // nosemgrep
                if (Stack->Entry[j].Period == EIE_PERIOD_CURRENT)
                    SList_AddNoDup (Stack->Entry[j].Symbol->Name, ListData);
                else
                    SList_AddNoDup (Stack->Entry[j].Symbol->Name, ListHist);
            }
        }

        /* read weight var from either DATA or HIST data set */
        if (EstimatorList->Estimator[i].WeightVariable[0] != '\0') {
            if (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_LR) {
                /* read weight var from DATA data set */
                SList_AddNoDup (EstimatorList->Estimator[i].WeightVariable,
                    ListData);
            }
            else {
                /*
                read weight var from DATA and/or HIST data set
                we must check the stack to know where to read the weight
                variable from
                */
                if (EstimatorList->Estimator[i].WeightVariable[0] != '\0') {
                    for (j = 0; j < Stack->NumberEntries; j++) {
                        if(Stack->Entry[j].Operation==EIE_OPERATION_PUSHSYMBOL&&
                              Stack->Entry[j].Aggregate==EIE_AGGREGATE_AVERAGE){
                            if (Stack->Entry[j].Period == EIE_PERIOD_CURRENT) {
                                SList_AddNoDup (
                                    EstimatorList->Estimator[i].WeightVariable,
                                    ListData);
                            }
                            else {
                                SList_AddNoDup (
                                    EstimatorList->Estimator[i].WeightVariable,
                                    ListHist);
                            }
                        }
                    }
                }
            }
        }

        /* read variance var from either DATA or HIST data set */
        if (EstimatorList->Estimator[i].VarianceVariable[0] != '\0')
            if (EstimatorList->Estimator[i].VariancePeriod[0] == 'C')
                SList_AddNoDup (EstimatorList->Estimator[i].VarianceVariable,
                    ListData);
            else
                SList_AddNoDup (EstimatorList->Estimator[i].VarianceVariable,
                    ListHist);
    }

    AllocateDataDataSetVariables (ListData, InCurr);
    AllocateDataDataSetVariables (ListHist, InHist);
    AllocateDataDataSetVariables (ListOutData, OutData);

    /* free list of variable */
    SList_Free (ListData);
    SList_Free (ListHist);
    SList_Free (ListOutData);
}
/*********************************************************************
Looks for duplicate value that appear in both lists.
Returns the duplicate value, if it finds one,
NOTFOUND otherwise.
*********************************************************************/
static int Intersect (
    int * List1,
    int nList1,
    int * List2,
    int nList2)
{
    int i;
    int j;

    for (i = 0; i < nList1; i++)
        for (j = 0; j < nList2; j++)
            if (List1[i] == List2[j])
                return List1[i];
    return NOTFOUND;
}
/*********************************************************************
*********************************************************************/
static IO_RETURN_CODE LoadAlgorithm (
    DSR_inalgorithm* dsr,
    tInAlgorithmDataSet * InAlgorithm,
    EIT_ALGORITHM_LIST * AlgorithmList)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;
    EIT_RETURNCODE rc_EI = EIE_SUCCEED;

    rc_EI = EI_AlgorithmAddPredefined (AlgorithmList);
    if (rc_EI != EIE_SUCCEED) return IORC_ERROR;

    if (dsr->dsr.is_specified == IOSV_SPECIFIED) {
        rc = SetUpInAlgorithmDataSet (dsr, InAlgorithm);

        if (rc == IORC_SUCCESS){
            PROC_RETURN_CODE rc_read = ReadAlgorithm (dsr, InAlgorithm, AlgorithmList);
            /* This function's caller doesn't really care what went wrong.  
            Return `JRC_ERROR` for all `ReadAlgorithm()` errors           */
            if (rc_read != PRC_SUCCESS_NO_MORE_DATA) {
                rc = IORC_ERROR;
            }
        }

        FreeInAlgorithmDataSet (InAlgorithm);
        if (DEBUG) EI_AlgorithmPrint (AlgorithmList);
    }

    return rc;
}
/*********************************************************************
*********************************************************************/
static IO_RETURN_CODE LoadEstimator (
    DSR_inestimator* dsr,
    tInEstimatorDataSet * InEstimator,
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;

    if (dsr->dsr.is_specified == IOSV_SPECIFIED) {
        rc = SetUpInEstimatorDataSet (dsr, InEstimator);

        if (rc == IORC_SUCCESS){
            PROC_RETURN_CODE rc_read = ReadEstimator (dsr, InEstimator, EstimatorList);
            /* This function's caller doesn't really care what went wrong. 
               Return `JRC_ERROR` for all `ReadEstimator()` errors         */
            if (rc_read != PRC_SUCCESS_NO_MORE_DATA) {
                rc = IORC_ERROR;
            }
        }

        FreeInEstimatorDataSet (InEstimator);
        if (DEBUG) EI_EstimatorListPrint (EstimatorList);
    }

    return rc;
}

/*********************************************************************
Print set of estimator specifications to the LOG
*********************************************************************/
static void PrintEstimatorSpecifications (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_ALGORITHM_LIST * AlgorithmList)
{
    int i;
    int Index;
    char Space[101];

#define PRINT_MAX_DESCRIPTOR_NAME    24

/* PRINT_DESCRIPTOR() is used to print one estimator descriptor */
#define PRINT_DESCRIPTOR(Msg, DescriptorName, DescriptorValue) \
IO_PRINT_LINE (Msg, \
DescriptorName, \
PRINT_MAX_DESCRIPTOR_NAME - (int)strlen(DescriptorName), \
"                                                                                                                      ", \
DescriptorValue);

    if (EstimatorList->NumberEntries > 0) {
        IO_PRINT_LINE ("");
        IO_PRINT_LINE (MsgHeaderPrintEstimatorSpecifications);
        PRINT_UNDERLINE_HEADER
            (strlen(MsgHeaderPrintEstimatorSpecifications) - 1);  // nosemgrep
        IO_PRINT_LINE ("");
    }

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        IO_PRINT_LINE (MsgEstimatorNumber, i);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorAlgorithmName,
            EstimatorList->Estimator[i].AlgorithmName);

        Index = EI_AlgorithmFind (AlgorithmList,
            EstimatorList->Estimator[i].AlgorithmName);

        if (Index != -1) {
            char * Type = MsgTypeInitInvalid;

            EstimatorList->Estimator[i].Algorithm =
                 &AlgorithmList->Algorithm[Index];
            if (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_EF)
                Type = EIM_ALGORITHM_TYPE_EF;
            else if (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_LR)
                Type = EIM_ALGORITHM_TYPE_LR;

            PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
                MsgDescriptorDescription,
                EstimatorList->Estimator[i].Algorithm->Description);

            PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
                MsgDescriptorType, Type);

            PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
                MsgDescriptorStatus,
                EstimatorList->Estimator[i].Algorithm->Status);

            PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
                MsgDescriptorFormula,
                EstimatorList->Estimator[i].Algorithm->Formula);
        }
        else {
            EstimatorList->Estimator[i].Algorithm = NULL;
            IO_PRINT_LINE (MsgAlgorithmNameNotFound SAS_NEWLINE,
                PRINT_MAX_DESCRIPTOR_NAME + 2, ""); /* add 2 to replace ": " before string */
        }

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorFieldid, EstimatorList->Estimator[i].FieldId);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorAuxiliaryVariables,
            EstimatorList->Estimator[i].AuxVariables);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorWeightVariable,
            EstimatorList->Estimator[i].WeightVariable);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorVarianceVariable,
            EstimatorList->Estimator[i].VarianceVariable);

        strcpy (Space, "");
        if (EstimatorList->Estimator[i].VarianceExponent != EIM_MISSING_VALUE)
            sprintf (Space, "%f", EstimatorList->Estimator[i].VarianceExponent);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorVarianceExponent, Space);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorVariancePeriod,
            EstimatorList->Estimator[i].VariancePeriod);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorExcludeImputed,
            EstimatorList->Estimator[i].ExcludeImputed);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorExcludeOutliers,
            EstimatorList->Estimator[i].ExcludeOutliers);

        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorRandomError,
            EstimatorList->Estimator[i].RandomError);

        strcpy (Space, "");
        if (EstimatorList->Estimator[i].CountCriteria !=
                EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED)
            sprintf (Space, "%d", EstimatorList->Estimator[i].CountCriteria);
        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorCountCriteria,
            Space);

        strcpy (Space, "");
        if (EstimatorList->Estimator[i].PercentCriteria !=
                EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED)
            sprintf (Space, "%f", EstimatorList->Estimator[i].PercentCriteria);
        PRINT_DESCRIPTOR (MsgDescriptorNameStringValue,
            MsgDescriptorPercentCriteria,
            Space);

        IO_PRINT_LINE ("");
    }
    /*IO_PRINT_LINE ("");*/
}
/*********************************************************************
prints formula with placeholders replaced with the real variable names
*********************************************************************/
static void PrintFormulas (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    char FormattedFormula[1001];

    if (EstimatorList->NumberEntries > 0) {
        IO_PRINT_LINE ("");
        IO_PRINT_LINE (MsgHeaderPrintFormulas);
        PRINT_UNDERLINE_HEADER
            (strlen(MsgHeaderPrintFormulas) - 1);  // nosemgrep
        IO_PRINT_LINE ("");
    }

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
         Estimator = &EstimatorList->Estimator[i];

         IO_PRINT_LINE (MsgEstimatorNumber, i);

         EI_EstimatorFormatFormula (Estimator->Algorithm->Formula,
             Estimator->FieldId, Estimator->AuxVariables, FormattedFormula);

         if (Estimator->RandomError[0] == 'Y')
             IO_PRINT_LINE (MsgFieldidEqualFormulaRandomError SAS_NEWLINE,
                 Estimator->FieldId, FormattedFormula);
         else
             IO_PRINT_LINE (MsgFieldidEqualFormula SAS_NEWLINE,
                 Estimator->FieldId, FormattedFormula);
    }
}
/*********************************************************************
prints statistics on imputation
*********************************************************************/
static void PrintImputationStatistics (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    char Line[101];

    IO_PRINT_LINE ("");
    IO_PRINT_LINE (MsgHeaderPrintImputationStatistics);
    PRINT_UNDERLINE_HEADER
        (strlen(MsgHeaderPrintImputationStatistics) - 1);  // nosemgrep
    IO_PRINT_LINE ("");

    IO_PRINT_LINE ("Est Variable                            FTI    Imp    */0");
    for (i = 0; i < EstimatorList->NumberEntries; i++) {
         Estimator = &EstimatorList->Estimator[i];

         if (Estimator->Active == EIE_TRUE) {
             sprintf (Line, "%-3d %-32s %6d %6d %6d",
                 i,
                 Estimator->FieldId,
                 Estimator->FTI,
                 Estimator->Imputed,
                 Estimator->DivisionByZero);
             IO_PRINT_LINE ("%s", Line);
         }
         else {
             sprintf (Line, "%-3d %-32s " MsgEstimatorNotActive,
                 i, Estimator->FieldId);
             IO_PRINT_LINE ("%s", Line);
         }
    }
    IO_PRINT_LINE ("");
}
/*********************************************************************
prints statistics on imputation when reject negative
*********************************************************************/
static void PrintImputationStatisticsRejectNegative (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    char Line[101];

    IO_PRINT_LINE ("");
    IO_PRINT_LINE (MsgHeaderPrintImputationStatistics);
    PRINT_UNDERLINE_HEADER
        (strlen(MsgHeaderPrintImputationStatistics) - 1);  // nosemgrep
    IO_PRINT_LINE ("");

    IO_PRINT_LINE (
        "Est Variable                            FTI    Imp    */0    "
        MsgColumnNegative);
    for (i = 0; i < EstimatorList->NumberEntries; i++) {
         Estimator = &EstimatorList->Estimator[i];

         if (Estimator->Active == EIE_TRUE) {
             sprintf (Line, "%-3d %-32s %6d %6d %6d %6d",
                 i,
                 Estimator->FieldId,
                 Estimator->FTI,
                 Estimator->Imputed,
                 Estimator->DivisionByZero,
                 Estimator->NegativeImputation);
             IO_PRINT_LINE ("%s", Line);
         }
         else {
             sprintf (Line, "%-3d %-32s " MsgEstimatorNotActive,
                 i, Estimator->FieldId);
             IO_PRINT_LINE ("%s", Line);
         }
    }
    IO_PRINT_LINE ("");
}
/*********************************************************************
Prints count of missing key and/or fieldid in DATASTATUS and HISTSTATUS
data sets.
*********************************************************************/
static void PrintMissingStatusCount (
    char * KeyName,
    tCounter * Counter)
{
    if (Counter->MissingStatus[EIE_PERIOD_CURRENT] > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedMissingKeyNameOrFieldidInDataSet SAS_NEWLINE,
            Counter->MissingStatus[EIE_PERIOD_CURRENT], DSN_INSTATUS,
            KeyName);

    if (Counter->MissingStatus[EIE_PERIOD_HISTORICAL] > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedMissingKeyNameOrFieldidInDataSet SAS_NEWLINE,
            Counter->MissingStatus[EIE_PERIOD_HISTORICAL], DSN_INSTATUS_HIST,
            KeyName);
}
/*********************************************************************
Print the values of the calculated parameters to the LOG
Parameters are averages for EF and betas for LR
*********************************************************************/
static void PrintParameters (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    tCounter * Counter)
{
    EIT_AVERAGE * Average;
    EIT_AUX * Aux;
    int i;
    int j;
    char Exponent[101];
    char Line[1001];
    EIT_REGRESSION * Regression;

#define NB_DECIMAL 2
#define TITLEFORMAT "%-10s %-8s %-10s %-32s"
#define LINEFORMAT  "%-010.5f %-8s %-10s %-32s"

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        if (EstimatorList->Estimator[i].Active == EIE_FALSE) {
            continue;
        }

        IO_PRINT_LINE (MsgEstimatorNumber, i);

        if (EstimatorList->Estimator[i].Algorithm->Type ==
                EIE_ALGORITHM_TYPE_EF) {

            if (EstimatorList->Estimator[i].Average == NULL) {
                IO_PRINT_LINE (MsgNoParameters SAS_NEWLINE);
                continue;
            }

            Average = EstimatorList->Estimator[i].Average;

            /* Acceptable observations for EF:
               Count = %d Percent = %.*f %sweighted. */
            sprintf (Line, MsgAcceptableObsCountPercentForEF,
                Average->Count,
                NB_DECIMAL,
                (double) Average->Count /
                    Counter->ValidObs[EIE_PERIOD_CURRENT] * 100,
                Average->WeightIndex == -1 ? MsgIndicatorNotWeighted : "");
            IO_PRINT_LINE ("%s", Line);

            /* Title and lines of values:
               "Average  Period          Variable " */
            sprintf (Line, TITLEFORMAT, MsgHeaderAverage, MsgHeaderPeriod, "",
                "Variable");
            IO_PRINT_LINE ("%s", Line);


            for (; Average != NULL; Average = Average->Next) {
                sprintf (Line, LINEFORMAT,
                    Average->Average,
                    Average->Period == EIE_PERIOD_CURRENT ?
                        MsgPeriodCurr : DSN_INDATA_HIST,
                    "",
                    Average->Name);
                IO_PRINT_LINE ("%s", Line);
            }
            IO_PRINT_LINE ("");
        }
        else {
            Regression = EstimatorList->Estimator[i].Regression;

            /* Acceptable observations for LR:
               Count = %d Percent = %.*f %sweighted and %s a variance variable*/
            sprintf (Line, MsgAcceptableObsCountPercentForLR,
                Regression->Count,
                NB_DECIMAL,
                (double) Regression->Count /
                    Counter->ValidObs[EIE_PERIOD_CURRENT] * 100,
                Regression->WeightIndex == -1 ? MsgIndicatorNotWeighted : "",
                Regression->VarianceIndex == -1 ?
                    MsgIndicatorHasNotVariance : MsgIndicatorHasVariance);
            IO_PRINT_LINE ("%s", Line);

            /* Title and lines of values:
               "Beta  Period  Exponent  Variable" */
            sprintf (Line, TITLEFORMAT, "Beta", MsgHeaderPeriod,
                MsgHeaderExponent, "Variable");
            IO_PRINT_LINE ("%s", Line);

            if (Regression->Intercept == EIE_TRUE) {
                sprintf (Line, LINEFORMAT,
                    Regression->Betas != NULL ? Regression->Betas[0] : 0,
                    "----",
                    "",
                    MsgIntercept);
                IO_PRINT_LINE ("%s", Line);
            }

            for (Aux=Regression->Aux, j=0; Aux != NULL; Aux = Aux->Next, j++) {
                sprintf (Exponent, "%-10.2f", Aux->ExponentValue);
                sprintf (Line, LINEFORMAT,
                    Regression->Betas != NULL ?
                        Regression->Betas[j+Regression->Intercept] : 0,
                    Aux->Period == EIE_PERIOD_CURRENT ?
                        MsgPeriodCurr : DSN_INDATA_HIST,
                    Exponent,
                    Aux->VariableName);
                IO_PRINT_LINE ("%s", Line);
            }
            IO_PRINT_LINE ("");
        }
    }
    IO_PRINT_LINE ("");
}
/*********************************************************************
Print the values of the parameters to the LOG
*********************************************************************/
static void PrintParms (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tDataDataSet * InHist,
    EIT_ALGORITHM_LIST * AlgorithmList,
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_BOOLEAN VerifySpecsOption,
    EIPT_POSITIVITY_OPTION PositivityOption,
	int ByGroupLoggingLevel)
{
    double DoubleSeed;
    char Name[LEGACY_MAXNAME+1];
    int RandSeed;

    SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_instatus.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_indata_hist.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_instatus_hist.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_inalgorithm.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_inestimator.dsr);

    SUtil_PrintOutputDataSetInfo(&sp->dsw_outdata);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outrand_err);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outacceptable);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outest_parm);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outest_ef);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outest_lr);

    /* Seed: get/print it and start the generator number.
       Take time() if none is given or a value < 1 or out of range;
       print also a warning if the given value is negative, 0 or out of range.*/
    if (sp->seed.meta.is_specified == IOSV_NOT_SPECIFIED) {
        RandSeed = (int) time (NULL);
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
    }
    else {
        DoubleSeed = (double)sp->seed.value;
        if (DoubleSeed >= 1.0 && DoubleSeed <= EIM_MAX_INT) {
            RandSeed = (int) DoubleSeed;
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_SEED, RandSeed);
        }
        else if (DoubleSeed > EIM_MAX_INT) {
            RandSeed = (int) time (NULL);
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgSeedOutOfRangeGeneratedNumberTaken);
        }
        else {
            RandSeed = (int) time (NULL);
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
            if (EIM_DBL_EQ (DoubleSeed, 0.0))
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgSeedZeroGeneratedNumberTaken);
            else if (EIM_DBL_LT (DoubleSeed, 0.0))
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgSeedNegativeGeneratedNumberTaken);
        }
    }
    UTIL_SRand (RandSeed);
    UTIL_Rand (); /* kick starting the generator. do not remove. */

    if (VerifySpecsOption == EIE_TRUE)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE BPN_VERIFY_SPECS);

    if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOption == EIPE_REJECT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IOUtil_copy_varname(Name, sp->dsr_indata.dsr.VL_unit_id.names[0]);
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, Name);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

    if (InCurr->ExclusionInUse == EIE_TRUE) { // no need to check `.count`, checked in `GetParms()`
        IOUtil_copy_varname(Name, sp->dsr_indata.VL_data_excl.names[0]);
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_DATA_EXCL_VAR, Name);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_DATA_EXCL_VAR);

    if (InHist->ExclusionInUse == EIE_TRUE) { // no need to check `.count`, checked in `GetParms()`
        IOUtil_copy_varname(Name, sp->dsr_indata_hist.VL_data_excl.names[0]);
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_DATA_EXCL_VAR_HIST, Name);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_DATA_EXCL_VAR_HIST);

    DSR_generic* dsr = &sp->dsr_indata.dsr;
    SUtil_PrintStatementVars(&sp->by.meta, dsr, &dsr->VL_by_var, BPN_BY);
    IO_PRINT("\n");

    PrintEstimatorSpecifications (EstimatorList, AlgorithmList);

    /*print WARNING if algorithmname have been redefined in algorithm data set*/
    EI_PrintMessages ();
}
/*********************************************************************
prints stat on what was read
*********************************************************************/
static void PrintStatistics (
    SP_estimato* sp,
    tCounter * Counter,
    char * KeyName)
{

/* PRINT_COUNT_DATASET() is used to print the count of observations
   with the name of the data set.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT_DATASET(Msg, LengthPlaceHolders, DataSetName, Count) \
IO_PRINT_LINE (Msg, DataSetName, \
PRINT_STATISTICS_WIDTH - (int)strlen (Msg) + LengthPlaceHolders - (int)strlen (DataSetName) - \
PRINT_PRECISION - 1, \
"............................................................", \
PRINT_PRECISION, Count);

    IO_PRINT_LINE (MsgHeaderPrintStatistics);
    PRINT_UNDERLINE_HEADER (strlen(MsgHeaderPrintStatistics) - 1);  // nosemgrep

    IO_PRINT_LINE ("");
    PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH, DSN_INDATA,
        Counter->Obs[EIE_PERIOD_CURRENT]);

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH, DSN_INDATA_HIST,
            Counter->Obs[EIE_PERIOD_HISTORICAL]);

/*        IO_PRINT_LINE ("Number of observations in HIST only (skipped)..: %*d",*/
/*            PRINT_PRECISION, Counter->UnpairedHistObs);*/
    }

    PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH, DSN_INDATA,
        Counter->ValidObs[EIE_PERIOD_CURRENT]);

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH, DSN_INDATA_HIST,
            Counter->ValidObs[EIE_PERIOD_HISTORICAL]);

/*        IO_PRINT_LINE ("Number of valid observations in DATA only......: %*d",*/
/*            PRINT_PRECISION, Counter->ValidObs[EIE_PERIOD_CURRENT] -*/
/*                Counter->ValidObs[EIE_PERIOD_HISTORICAL]);*/
    }

    IO_PRINT_LINE ("");

    if (Counter->MissingKeys[EIE_PERIOD_CURRENT] > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameOneDataSet SAS_NEWLINE,
            Counter->MissingKeys[EIE_PERIOD_CURRENT], KeyName, DSN_INDATA);

    if (Counter->MissingValues[EIE_PERIOD_CURRENT] > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2 SAS_NEWLINE,
                Counter->MissingValues[EIE_PERIOD_CURRENT], DSN_INDATA,
                DSN_INSTATUS);

    if (Counter->NegativeValues[EIE_PERIOD_CURRENT] > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2 SAS_NEWLINE,
                Counter->NegativeValues[EIE_PERIOD_CURRENT], DSN_INDATA,
                DSN_INSTATUS);

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        if (Counter->MissingKeys[EIE_PERIOD_HISTORICAL] > 0)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameOneDataSet SAS_NEWLINE,
                Counter->MissingKeys[EIE_PERIOD_HISTORICAL], KeyName, DSN_INDATA_HIST);

        if (Counter->MissingValues[EIE_PERIOD_HISTORICAL] > 0)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
                MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2 SAS_NEWLINE,
                    Counter->MissingValues[EIE_PERIOD_HISTORICAL], DSN_INDATA_HIST,
                    DSN_INSTATUS_HIST);

        if (Counter->NegativeValues[EIE_PERIOD_HISTORICAL] > 0)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
                MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2 SAS_NEWLINE,
                    Counter->NegativeValues[EIE_PERIOD_HISTORICAL], DSN_INDATA_HIST,
                    DSN_INSTATUS_HIST);
    }
}
/*********************************************************************
*********************************************************************/
static PROC_RETURN_CODE ProcessDataGroups (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tStatusDataSet * InCurrStatus,
    tDataDataSet * InHist,
    tStatusDataSet * InHistStatus,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable,
    EIPT_POSITIVITY_OPTION PositivityOption,
    EIT_BOOLEAN VerifySpecsOption,
	int ByGroupLoggingLevel)
{
    tAllOriginalVarNames AllOriginalVarNames;
    tCounter Counter;
    EIT_DATATABLE * DataTable;
    int * InToOutMapping;
    long long NumberOfObservations;
    int * OutToInMapping;
    EIT_RETURNCODE rc;

    PROC_RETURN_CODE rc_function = PRC_SUCCESS;

    ShowTime ("Entering ProcessDataGroups()");

    /*
    Decide what size to give to DataTable:
    if NumberOfObservations is
    below 10000, use NumberOfObservations
    above 10000, use 10000
    if BY group specified, divide by arbitrary number
    */
    NumberOfObservations = sp->dsr_indata.dsr.num_records;

    if (NumberOfObservations == -1)
        /* sequential file. set NumberOfObservations to an arbitrary number */
        NumberOfObservations = 1000;
    else if (NumberOfObservations <= 10000)
        NumberOfObservations = NumberOfObservations + 10;
    else
        NumberOfObservations = 10000;

    /* if by group cut size by arbitrary number */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_by_var))
        NumberOfObservations /= 5;

    DataTable = EI_DataTableAllocate ((int) NumberOfObservations);

    CreateMappings (InCurr, OutData, &InToOutMapping, &OutToInMapping);

    GetAllOriginalVarNames (sp, InCurr, InHist, &AllOriginalVarNames);

    /* check if DATASTATUS data set has the by variables */
    rc = SUtil_AreAllByVariablesInDataSet(&sp->dsr_indata.dsr, &sp->dsr_instatus.dsr);
	EI_PrintMessages ();
    if (rc == EIE_SUCCEED) {
        /* BY variables were found in DATASTATUS data set */
        InCurrStatus->HasAllByVariables = EIE_TRUE;

        /* build BY list for DATASTATUS data set */
        /* call to SAS_XBYLIST(...) does not need to be replaced. 
        * in SAS, this performed a check and did some setup. 
        * Now however, setup happens elsewhere and  replacement
        * SUtil_AreAllByVariablesInDataSet (called above) performs the check
        */
    }
    else {
        /* One or more BY variable was not found in DATASTATUS data set */
        InCurrStatus->HasAllByVariables = EIE_FALSE;

        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet SAS_NEWLINE,
            DSN_INSTATUS);
    }

    /* check if HISTSTATUS data set has the by variables */
    if (sp->dsr_instatus_hist.dsr.is_specified == IOSV_SPECIFIED) {
        rc = SUtil_AreAllByVariablesInDataSet(&sp->dsr_indata.dsr, &sp->dsr_instatus_hist.dsr);
		EI_PrintMessages ();
		if (rc == EIE_SUCCEED) {
            /* BY variables were found in HISTSTATUS data set */
            InHistStatus->HasAllByVariables = EIE_TRUE;

            /* build BY list for HISTSTATUS data set */
            // no longer requiredSAS_XBYLIST (InCurr->File, InHistStatus->File);
        }
        else {
            /*One or more BY variable was not found in HISTSTATUS data set*/
            InHistStatus->HasAllByVariables = EIE_FALSE;

            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet SAS_NEWLINE,
                DSN_INSTATUS_HIST);
        }
    }

    PROC_RETURN_CODE rcReadByGroup = ReadByGroup (sp, InCurr, InCurrStatus, InHist, InHistStatus,
        DataTable, &Counter, ByGroupLoggingLevel);

    ShowTime (" ReadBygroup() completed");

    /* Every iteration process one BY group */
    int loops_entered = 0;
    while (rcReadByGroup == PRC_SUCCESS) {
        if (++loops_entered == 1) {
            /*
            Print this one time only for the first by group!
            If we do not do this, this message will appear for every by group,
            but it will refer to the same status data set obs.
            */
            PrintMissingStatusCount(InCurrStatus->Key.Name, &Counter);
        }

        /* make sure weight and variance variables before proceeding */
        rc = EI_ValidateWeightAndVarianceVariables (EstimatorList, DataTable);
        if (rc == EIE_FAIL) {
            rc_function = PRC_FAIL_ESTIMATO_INVALID_WEIGHT_VARIANCE;
            goto error_cleanup;
        }
        ShowTime (" EI_ValidateWeightAndVarianceVariables() completed");

        RemoveUnflagged (DataTable, PositivityOption, &Counter);
        ShowTime (" RemoveUnflagged() completed");

		if (ByGroupLoggingLevel != 1) PrintStatistics (sp, &Counter, InCurrStatus->Key.Name);

/*        EI_DataTablePrintStatistics (DataTable);*/

        /* rene: find a better test please */
		if (ByGroupLoggingLevel != 1 && Counter.ValidObs[EIE_PERIOD_CURRENT] == 0) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNotEnoughValidObsToProcessEstimators);
        }
        else {

/*            EI_DataTablePrint (DataTable);*/

            /* make all estimator active for this by group, init counters */
            EI_Reactivate (EstimatorList);

			if (ByGroupLoggingLevel != 1) IO_PRINT_LINE (MsgHeaderPrintParameters);
			if (ByGroupLoggingLevel != 1) {
				PRINT_UNDERLINE_HEADER
					(strlen(MsgHeaderPrintParameters) - 1);  // nosemgrep
			}
			if (ByGroupLoggingLevel != 1) IO_PRINT_LINE ("");

            EI_ComputeAverages (EstimatorList, DataTable);
			if (ByGroupLoggingLevel != 1) EI_PrintMessages ();
			else EI_ClearMessageList();

            rc = EI_ComputeBetas (EstimatorList, DataTable);
			if (ByGroupLoggingLevel != 1) EI_PrintMessages ();
			else EI_ClearMessageList();
            if (rc == EIE_FAIL) {
                rc_function = PRC_FAIL_ESTIMATO_COMPUTE_BETAS;
                goto error_cleanup;
            }

            /* check if the count and percent criteria are fullfilled */
            EI_ValidateCriteria (EstimatorList,
                Counter.ValidObs[EIE_PERIOD_CURRENT]);
			if (ByGroupLoggingLevel != 1) EI_PrintMessages ();
			else EI_ClearMessageList();

			if (ByGroupLoggingLevel != 1) PrintParameters (EstimatorList, &Counter);

            ShowTime (" Parameters computation completed");

            if (VerifySpecsOption == EIE_FALSE) {

                if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
                    rc = EI_DoEstimation (EstimatorList, SymbolTable,
                        DataTable);
                else
                    rc = EI_DoEstimationRejectNegative (EstimatorList,
                        SymbolTable, DataTable);
				if (ByGroupLoggingLevel != 1) EI_PrintMessages ();
				else EI_ClearMessageList();
                if (rc == EIE_FAIL) {
                    rc_function = PRC_FAIL_ESTIMATO_DO_ESTIMATION;
                    goto error_cleanup;
                }

                ShowTime (" Estimation completed");
				if (ByGroupLoggingLevel != 1) {
					if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
						PrintImputationStatistics(EstimatorList);
					else
						PrintImputationStatisticsRejectNegative(EstimatorList);

                    DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
				}
                ShowTime (" Print statistics completed");

                PROC_RETURN_CODE rc_write = WriteResults (sp, EstimatorList, DataTable, OutData, OutStatus,
                    OutRandomError, OutAcceptable, OutEstimParms, OutEstimEF,
                    OutEstimLR, InToOutMapping, OutToInMapping,
                    AllOriginalVarNames, PositivityOption);
                if (rc_write != PRC_SUCCESS) {
                    rc_function = rc_write;
                    goto error_cleanup;
                }

                ShowTime (" WriteResults completed");

#ifdef NEVERTOBEDEFINED
                for (i = 0; i < EstimatorList->NumberEntries; i++) {
                    if (EstimatorList->Estimator[i].Residual != NULL) {
                        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Estimator %d:\n", i);
                        EI_ResidualPrintStatistics (
                            EstimatorList->Estimator[i].Residual);
                        EI_ResidualPrint (EstimatorList->Estimator[i].
                            Residual);
                        EI_RandomErrorResultPrint (
                            EstimatorList->Estimator[i]. RandomErrorResult);
                    }
                }
#endif
            }
        }
        ShowTime ("BY group completed");

        EI_DataTableEmpty (DataTable);

        ShowTime (" Empty DataTable completed");

        rcReadByGroup = ReadByGroup (sp, InCurr, InCurrStatus, InHist, InHistStatus,
            DataTable, &Counter, ByGroupLoggingLevel);
        ShowTime (" ReadBygroup() completed");
    }

    // error check handled by caller
    rc_function = rcReadByGroup;

error_cleanup:
    EI_PrintMessages();
    EI_DataTableFree (DataTable);
    STC_FreeMemory (InToOutMapping);
    STC_FreeMemory (OutToInMapping);
    FreeAllOriginalVarNames(&AllOriginalVarNames);
    ShowTime ("Leaving ProcessDataGroups()");

    return rc_function;
}
/*********************************************************************
Reads ALGORITHM data set
    A dataset filled with parameters, processed in its entirety 
    during procedure setup.  
*********************************************************************/
static PROC_RETURN_CODE ReadAlgorithm (
    DSR_inalgorithm* dsr,
    tInAlgorithmDataSet * InAlgorithm,
    EIT_ALGORITHM_LIST * AlgorithmList)
{
    char * Description;
    char * Formula;
    char * Name;
    int NumberOfAlgorithms;
    char * Status;
    EIT_ALGORITHM_TYPE Type;
    IO_DATASET_RC rc_next_by = DSRC_SUCCESS;
    IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;

    while ((rc_next_by = DSR_cursor_next_by(&dsr->dsr)) == DSRC_NEXT_BY_SUCCESS) {
        while ((rc_next_rec = DSR_cursor_next_rec(&dsr->dsr)) == DSRC_NEXT_REC_SUCCESS) {
            IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*) dsr);
            if (rc_get_rec != IORC_SUCCESS) { 
                return PRC_FAIL_GET_REC;
            }

            SUtil_NullTerminate (InAlgorithm->Name.Value,
                InAlgorithm->Name.Size);
            SUtil_NullTerminate (InAlgorithm->Status.Value,
                InAlgorithm->Status.Size);
            SUtil_NullTerminate (InAlgorithm->Type.Value,
                InAlgorithm->Type.Size);
            SUtil_NullTerminate (InAlgorithm->Formula.Value,
                InAlgorithm->Formula.Size);
            SUtil_NullTerminate (InAlgorithm->Description.Value,
                InAlgorithm->Description.Size);

            UTIL_StrUpper (InAlgorithm->Name.Value,
                InAlgorithm->Name.Value);
            UTIL_StrUpper (InAlgorithm->Status.Value,
                InAlgorithm->Status.Value);
            UTIL_StrUpper (InAlgorithm->Type.Value,
                InAlgorithm->Type.Value);
            UTIL_StrUpper (InAlgorithm->Formula.Value,
                InAlgorithm->Formula.Value);

            UTIL_DropBlanks (InAlgorithm->Formula.Value);

            Name = InAlgorithm->Name.Value;
            if (strcmp (InAlgorithm->Type.Value, EIM_ALGORITHM_TYPE_EF) == 0)
                Type = EIE_ALGORITHM_TYPE_EF;
            else if (strcmp (InAlgorithm->Type.Value, EIM_ALGORITHM_TYPE_LR)==0)
                Type = EIE_ALGORITHM_TYPE_LR;
            else
                Type = EIE_ALGORITHM_TYPE_INVALID;
            Status = InAlgorithm->Status.Value;
            Formula = InAlgorithm->Formula.Value;
            Description = InAlgorithm->Description.Value;

            /*
            if adding an algorithm does not change the size of the algoritm
            list it means that the last algorithm added replaced an algorithm
            already in the list
            */
            NumberOfAlgorithms = AlgorithmList->NumberEntries;
            EI_AlgorithmAdd (AlgorithmList, Name, Type, Status, Formula,
                Description);
            if (NumberOfAlgorithms == AlgorithmList->NumberEntries) {
                EI_AddMessage (MsgHeaderEstimator, EIE_WARNING,
                    MsgRedefinitionOfPredefinedAlgorithmName "\n",
                    Name);
            }
        }

        // check for errors
        if (rc_next_rec != DSRC_NO_MORE_REC_IN_BY) { // error
            return PRC_FAIL_ADVANCE_REC;
        } // else continue (no error)
    }

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_BY;
    }
}
/*********************************************************************
Reads DATA, DATASTATUS, HIST and HISTSTATUS data sets.
return E_READBYGROUP_FOUND when data is read successfully.
return E_READBYGROUP_NOTFOUND when end of DATA data set is reached.
return E_READBYGROUP_FAIL when NOTSORTED option is specified, and the BY
                          variables do not match exactly.
*********************************************************************/
static PROC_RETURN_CODE ReadByGroup (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tStatusDataSet * InCurrStatus,
    tDataDataSet * InHist,
    tStatusDataSet * InHistStatus,
    EIT_DATATABLE * DataTable,
    tCounter * Counter,
	int ByGroupLoggingLevel)
{
    EIT_EXCLUSION HistoricalExclusionFlag;

    Counter->Obs[EIE_PERIOD_CURRENT] = 0;
    Counter->Obs[EIE_PERIOD_HISTORICAL] = 0;
    Counter->ValidObs[EIE_PERIOD_CURRENT] = 0;
    Counter->ValidObs[EIE_PERIOD_HISTORICAL] = 0;

    Counter->MissingKeys[EIE_PERIOD_CURRENT] = 0;
    Counter->MissingKeys[EIE_PERIOD_HISTORICAL] = 0;
    Counter->MissingStatus[EIE_PERIOD_CURRENT] = 0;
    Counter->MissingStatus[EIE_PERIOD_HISTORICAL] = 0;
    Counter->MissingValues[EIE_PERIOD_CURRENT] = 0;
    Counter->MissingValues[EIE_PERIOD_HISTORICAL] = 0;
    Counter->NegativeValues[EIE_PERIOD_CURRENT] = 0;
    Counter->NegativeValues[EIE_PERIOD_HISTORICAL] = 0;

    IO_DATASET_RC rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr);
	if (ByGroupLoggingLevel != 1) IO_PRINT_LINE ("");

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else if (rc_next_by == DSRC_BY_NOT_SORTED) {
        return PRC_FAIL_BY_NOT_SORTED;
    }
    else if (rc_next_by != DSRC_NEXT_BY_SUCCESS) {
        return PRC_FAIL_ADVANCE_BY;
    } // else continue (no error)

    if (InHist->ExclusionInUse == EIE_TRUE)
        HistoricalExclusionFlag = EIE_EXCLUDED;
    else
        HistoricalExclusionFlag = EIE_INCLUDED;

    PROC_RETURN_CODE rc_ReadData = PRC_FAIL_UNHANDLED;
    rc_ReadData = ReadData (&sp->dsr_indata, InCurr, HistoricalExclusionFlag, DataTable, Counter);
    if (rc_ReadData != PRC_SUCCESS_NO_MORE_DATA) {
        return rc_ReadData;
    }

    PROC_RETURN_CODE rc_ReadStatus = PRC_FAIL_UNHANDLED;
    rc_ReadStatus = ReadStatus (&sp->dsr_indata, &sp->dsr_instatus, InCurr, InCurrStatus, EIE_PERIOD_CURRENT, DataTable, Counter);
    if (rc_ReadStatus != PRC_SUCCESS_NO_MORE_DATA) {
        return rc_ReadStatus;
    }

    PROC_RETURN_CODE rc_ReadHist = PRC_FAIL_UNHANDLED;
    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        IO_DATASET_RC rc_sync_by = DSR_cursor_sync_by(&sp->dsr_indata.dsr, &sp->dsr_indata_hist.dsr);
        if (rc_sync_by == DSRC_DIFFERENT_GROUPS) {
            /* eof for InHist or no matching by variables */
            return PRC_SUCCESS;
        }
        else if (rc_sync_by == DSRC_SUCCESS) {
            rc_ReadHist = ReadHist (&sp->dsr_indata_hist, InHist, DataTable, Counter);
            if (rc_ReadHist != PRC_SUCCESS_NO_MORE_DATA) {
                return rc_ReadHist;
            }

            if (sp->dsr_instatus_hist.dsr.is_specified == IOSV_SPECIFIED) {
                rc_ReadStatus = ReadStatus (&sp->dsr_indata_hist, &sp->dsr_instatus_hist, InHist, InHistStatus, EIE_PERIOD_HISTORICAL,
                    DataTable, Counter);
                if (rc_ReadStatus != PRC_SUCCESS_NO_MORE_DATA) {
                    return rc_ReadStatus;
                }
            }
        }
        else {
            // arriving here likely indicates a bug in the code
            return PRC_FAIL_SYNC_GENERIC;
        }
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Reads DATA data set
*********************************************************************/
static PROC_RETURN_CODE ReadData (
    DSR_indata* dsr,
    tDataDataSet * InCurr,
    EIT_EXCLUSION HistoricalExclusionFlag,
    EIT_DATATABLE * DataTable,
    tCounter * Counter)
{
    EIT_DATA * Data;
    int i;
    EIT_DATATABLELOOKUP_RETURNCODE rcLookup;

    IO_DATASET_RC rc_next_rec;

    while ((rc_next_rec = DSR_cursor_next_rec(&dsr->dsr)) == DSRC_NEXT_REC_SUCCESS) {

        Counter->Obs[EIE_PERIOD_CURRENT]++;

        IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*)dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }

        SUtil_NullTerminate (InCurr->Key.Value, InCurr->Key.Size);

        if (InCurr->ExclusionInUse == EIE_TRUE)
            SUtil_NullTerminate (InCurr->ExclusionIndicator.Value,
                InCurr->ExclusionIndicator.Size);

        if (InCurr->Key.Value[0] == '\0') {
            Counter->MissingKeys[EIE_PERIOD_CURRENT]++;
        }
        else {
            rcLookup = EI_DataTableLookup (DataTable, InCurr->Key.Value,
                EIE_DATATABLELOOKUPTYPE_CREATE, &Data);

            switch (rcLookup) {
            case EIE_DATATABLELOOKUP_NOTFOUND:
                /* Cool... continuons */

                Counter->ValidObs[EIE_PERIOD_CURRENT]++;

                if (InCurr->ExclusionInUse == EIE_TRUE &&
                        (strcmp (InCurr->ExclusionIndicator.Value, "E") == 0))
                    Data->ExcludedObs[EIE_PERIOD_CURRENT] = EIE_EXCLUDED;
                else
                    Data->ExcludedObs[EIE_PERIOD_CURRENT] = EIE_INCLUDED;

                Data->ExcludedObs[EIE_PERIOD_HISTORICAL] =
                    HistoricalExclusionFlag;

                Data->DataRec[EIE_PERIOD_CURRENT] = STC_AllocateMemory (
                    sizeof *Data->DataRec[EIE_PERIOD_CURRENT]);
                EI_DataRecAllocate (InCurr->VariableName,
                    InCurr->NumberOfVariables,
                    Data->DataRec[EIE_PERIOD_CURRENT]);
                for (i = 0; i < InCurr->NumberOfVariables; i++)
                    if (IOUtil_is_missing (InCurr->Variable[i].Value))
                        Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] =
                            EIM_MISSING_VALUE;
                    else
                        Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] =
                            InCurr->Variable[i].Value;

                /* set datarec flag to ok or missing */
                EI_DataRecSetMissingFlag (Data->DataRec[EIE_PERIOD_CURRENT]);
                break;

            case EIE_DATATABLELOOKUP_FOUND:
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgKeyNameWithDuplicateValueInDataSet SAS_NEWLINE,
                    InCurr->Key.Name, InCurr->Key.Value, DSN_INDATA);
                return PRC_FAIL_DUPLICATE_REC;
            }
        }
    }

    // check for errors
    if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_REC;
    }
}
/*********************************************************************
Reads ESTIMATOR data set
    A dataset filled with parameters, processed in its entirety 
    during procedure setup.  
*********************************************************************/
static PROC_RETURN_CODE ReadEstimator (
    DSR_inestimator* dsr,
    tInEstimatorDataSet * InEstimator,
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int CountCriteria;
    double PercentCriteria;
    double VarianceExponent;

    IO_DATASET_RC rc_next_by = DSRC_SUCCESS;
    IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;

    while ((rc_next_by = DSR_cursor_next_by(&dsr->dsr)) == DSRC_NEXT_BY_SUCCESS){
        
        while ((rc_next_rec = DSR_cursor_next_rec(&dsr->dsr)) == DSRC_NEXT_REC_SUCCESS) {
            IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*)dsr);
            if (rc_get_rec != IORC_SUCCESS) {
                return PRC_FAIL_GET_REC;
            }

            SUtil_NullTerminate (InEstimator->AlgorithmName.Value,
                InEstimator->AlgorithmName.Size);
            SUtil_NullTerminate (InEstimator->FieldId.Value,
                InEstimator->FieldId.Size);
            SUtil_NullTerminate (InEstimator->AuxVariables.Value,
                InEstimator->AuxVariables.Size);
            SUtil_NullTerminate (InEstimator->WeightVariable.Value,
                InEstimator->WeightVariable.Size);
            SUtil_NullTerminate (InEstimator->VarianceVariable.Value,
                InEstimator->VarianceVariable.Size);
            SUtil_NullTerminate (InEstimator->VariancePeriod.Value,
                InEstimator->VariancePeriod.Size);
            SUtil_NullTerminate (InEstimator->ExcludeImputed.Value,
                InEstimator->ExcludeImputed.Size);
            SUtil_NullTerminate (InEstimator->ExcludeOutliers.Value,
                InEstimator->ExcludeOutliers.Size);
            SUtil_NullTerminate (InEstimator->RandomError.Value,
                InEstimator->RandomError.Size);

            UTIL_StrUpper (InEstimator->FieldId.Value,
                InEstimator->FieldId.Value);
            UTIL_StrUpper (InEstimator->AuxVariables.Value,
                InEstimator->AuxVariables.Value);
            UTIL_StrUpper (InEstimator->WeightVariable.Value,
                InEstimator->WeightVariable.Value);
            UTIL_StrUpper (InEstimator->VarianceVariable.Value,
                InEstimator->VarianceVariable.Value);
            UTIL_StrUpper (InEstimator->VariancePeriod.Value,
                InEstimator->VariancePeriod.Value);
            UTIL_StrUpper (InEstimator->ExcludeImputed.Value,
                InEstimator->ExcludeImputed.Value);
            UTIL_StrUpper (InEstimator->ExcludeOutliers.Value,
                InEstimator->ExcludeOutliers.Value);
            UTIL_StrUpper (InEstimator->RandomError.Value,
                InEstimator->RandomError.Value);

            if (IOUtil_is_missing (InEstimator->VarianceExponent.Value))
                VarianceExponent = EIM_MISSING_VALUE;
            else
                VarianceExponent = InEstimator->VarianceExponent.Value;

            if (IOUtil_is_missing (InEstimator->CountCriteria.Value))
                CountCriteria = EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED;
            else {
                CountCriteria = (int) InEstimator->CountCriteria.Value;
                if ((double)CountCriteria != InEstimator->CountCriteria.Value)
                    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgCountCriteriaDoubleReplacedByInteger
                        SAS_NEWLINE, InEstimator->CountCriteria.Value,
                        CountCriteria);
            }

            if (IOUtil_is_missing (InEstimator->PercentCriteria.Value))
                PercentCriteria = EIM_ESTIM_PERCENTCRITERIA_NOT_SPECIFIED;
            else
                PercentCriteria = InEstimator->PercentCriteria.Value;

            EI_EstimatorAdd (EstimatorList, InEstimator->AlgorithmName.Value,
                InEstimator->FieldId.Value, InEstimator->AuxVariables.Value,
                InEstimator->WeightVariable.Value,
                InEstimator->VarianceVariable.Value, VarianceExponent,
                InEstimator->VariancePeriod.Value,
                InEstimator->ExcludeImputed.Value,
                InEstimator->ExcludeOutliers.Value,
                InEstimator->RandomError.Value, CountCriteria, PercentCriteria);
        }

        // check for errors
        if (rc_next_rec != DSRC_NO_MORE_REC_IN_BY) { // error
            return PRC_FAIL_ADVANCE_REC;
        } // else continue (no error)
    }

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_BY;
    }
}
/*********************************************************************
Reads HIST data set
*********************************************************************/
static PROC_RETURN_CODE ReadHist (
    DSR_indata* dsr,
    tDataDataSet * InHist,
    EIT_DATATABLE * DataTable,
    tCounter * Counter)
{
    EIT_DATA * Data;
    int i;
    int rcLookup;
    IO_DATASET_RC rc_next_rec;

    while ((rc_next_rec = DSR_cursor_next_rec(&dsr->dsr)) == DSRC_NEXT_REC_SUCCESS) {

        Counter->Obs[EIE_PERIOD_HISTORICAL]++;

        IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*)dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }

        SUtil_NullTerminate (InHist->Key.Value, InHist->Key.Size);

        if (InHist->ExclusionInUse == EIE_TRUE)
            SUtil_NullTerminate (InHist->ExclusionIndicator.Value,
                InHist->ExclusionIndicator.Size);

        if (InHist->Key.Value[0] == '\0') {
            Counter->MissingKeys[EIE_PERIOD_HISTORICAL]++;
        }
        else {
            rcLookup = EI_DataTableLookup (DataTable, InHist->Key.Value,
                EIE_DATATABLELOOKUPTYPE_FIND, &Data);

            switch (rcLookup) {
            case EIE_DATATABLELOOKUP_NOTFOUND:
                /*
                if there is no current data,
                we do not care to keep historical data.
                */
/*                Counter->UnpairedHistObs++;*/
                break;

            case EIE_DATATABLELOOKUP_FOUND:
                if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL) {
                    IO_PRINT_LINE ("1z" MsgKeyNameWithDuplicateValueInDataSet SAS_NEWLINE,
                        InHist->Key.Name, InHist->Key.Value, DSN_INDATA_HIST);
                    return PRC_FAIL_DUPLICATE_REC;
                }

                /* Cool... continuons */

                Counter->ValidObs[EIE_PERIOD_HISTORICAL]++;

                if (InHist->ExclusionInUse == EIE_TRUE &&
                        (strcmp (InHist->ExclusionIndicator.Value, "E") == 0))
                    Data->ExcludedObs[EIE_PERIOD_HISTORICAL] = EIE_EXCLUDED;
                else
                    Data->ExcludedObs[EIE_PERIOD_HISTORICAL] = EIE_INCLUDED;

                Data->DataRec[EIE_PERIOD_HISTORICAL] = STC_AllocateMemory (
                    sizeof *Data->DataRec[EIE_PERIOD_HISTORICAL]);
                EI_DataRecAllocate (InHist->VariableName,
                    InHist->NumberOfVariables,
                    Data->DataRec[EIE_PERIOD_HISTORICAL]);
                for (i = 0; i < InHist->NumberOfVariables; i++)
                    if (IOUtil_is_missing (InHist->Variable[i].Value))
                        Data->DataRec[EIE_PERIOD_HISTORICAL]->FieldValue[i] =
                            EIM_MISSING_VALUE;
                    else
                        Data->DataRec[EIE_PERIOD_HISTORICAL]->FieldValue[i] =
                            InHist->Variable[i].Value;

                /* set datarec flag to ok or missing */
                EI_DataRecSetMissingFlag (Data->DataRec[EIE_PERIOD_HISTORICAL]);
            }
        }
    }

    // check for errors
    if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_REC;
    }
}
/*********************************************************************
Reads status for DATASTATUS and HISTSTATUS data sets
*********************************************************************/
static PROC_RETURN_CODE ReadStatus (
    DSR_indata* dsr_indata, 
    DSR_instatus* dsr_instatus,
    tDataDataSet * InData,
    tStatusDataSet * InStatus,
    int Period,
    EIT_DATATABLE * DataTable,
    tCounter * Counter) /* used to synchronize the status data set */
{
    EIT_DATA * Data;
    int i;
    int rcLookup;

    /* synchronize STATUS */
    if (InStatus->HasAllByVariables) {
        /* STATUS has all the by variables. synchronize STATUS. */
        IO_DATASET_RC rc_sync_by= DSR_cursor_sync_by(&dsr_indata->dsr, &dsr_instatus->dsr);
        if (rc_sync_by == DSRC_DIFFERENT_GROUPS) {
            /* no matching by variables in status data set */
            return PRC_SUCCESS; /* do not read status */
        }
        else if (rc_sync_by != DSRC_SUCCESS) {
            // arriving here likely indicates a bug in the code
            return PRC_FAIL_SYNC_GENERIC;
        }
    }
    else {
        /* STATUS does not have all the by variables.
           rewind STATUS to read it all. */
        if (DSRC_SUCCESS != DSR_cursor_rewind(&dsr_instatus->dsr, IOCA_REWIND_DATASET)) {
            return PRC_FAIL_SYNC_GENERIC;
        }
    }
    
    IO_DATASET_RC rc_next_rec;
    while ((rc_next_rec = DSR_cursor_next_rec(&dsr_instatus->dsr)) == DSRC_NEXT_REC_SUCCESS) {
        IO_RETURN_CODE rc_get_rec = DSR_rec_get(&dsr_instatus->dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }

        SUtil_NullTerminate (InStatus->Key.Value, InStatus->Key.Size);
        SUtil_NullTerminate (InStatus->FieldId.Value, InStatus->FieldId.Size);
        SUtil_NullTerminate (InStatus->Status.Value, InStatus->Status.Size);

        /* remove leading blanks from fieldid */
        UTIL_DropBlanks (InStatus->FieldId.Value);
/*        SUtil_GetOriginalName (InStatus->FieldId.Value, InData->File);*/  // no longer needed?
        UTIL_StrUpper (InStatus->FieldId.Value, InStatus->FieldId.Value);

        if (InStatus->Key.Value[0] == '\0' ||
                InStatus->FieldId.Value[0] == '\0') {
            Counter->MissingStatus[Period]++;
        }
        else {
            rcLookup = EI_DataTableLookup (DataTable, InStatus->Key.Value,
                EIE_DATATABLELOOKUPTYPE_FIND, &Data);

            switch (rcLookup) {
            case EIE_DATATABLELOOKUP_NOTFOUND:
                /* variable not in data table */
                break;

            case EIE_DATATABLELOOKUP_FOUND:
                /* HIST could be NULL, but not current */
                if (Data->DataRec[Period] != NULL) {
                    for (i = 0; i < InData->NumberOfVariables; i++) {

                        /* rene:compare quoi? OriginalName ou case insensitive*/
                        if (strcmp (InData->VariableName[i],
                                InStatus->FieldId.Value) == 0) {

                            /* the only status of concern are:
                            FTI, FTE, Ianything (but not IDE) */
                            if (strcmp (InStatus->Status.Value,
                                    EIM_STATUS_FTI) == 0)
                                Data->DataRec[Period]->StatusFlag[i] = FIELDFTI;
                            else if (strcmp (InStatus->Status.Value,
                                    EIM_STATUS_FTE) == 0)
                                Data->DataRec[Period]->StatusFlag[i] = FIELDFTE;
                            else if (strcmp (InStatus->Status.Value,
                                    EIM_STATUS_IDE) == 0)
                                /*
                                NOTHING TO DO, IDE var are considered good
                                but we need this because it is an exception
                                to the next if.
                                */
                                i=i; /* shut up lint */
                            else if (InStatus->Status.Value[0] == 'I')
                                Data->DataRec[Period]->StatusFlag[i] =
                                    FIELDIMPUTED;

                            /* skip remaining variables */
                            i = InData->NumberOfVariables;
                        }
                    }
                }
                break;
            }
        }
    }

    // check for errors
    if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_REC;
    }
}
/*********************************************************************
Check if missing data fields are flagged as FTI.
If not flagged, the obs is removed.
Set counters.
*********************************************************************/
static void RemoveUnflagged (
    EIT_DATATABLE * DataTable,
    EIPT_POSITIVITY_OPTION PositivityOption,
    tCounter * Counter)
{
    EIT_DATA * Data;
    EIT_DATA * DataNext;
    EIT_DATAREC * DataRec;
    int i;
    int j;
    EIT_DATA * PreviousData = NULL;
    int Remove;
/*
    EI_DataTablePrintStatistics (DataTable);
    EI_DataTablePrint (DataTable);
    EI_PrintMessages ();
*/
	for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = DataNext) {

            Remove = EIE_FALSE;

            /* check current ... */
            DataRec = Data->DataRec[EIE_PERIOD_CURRENT];
            for (j = 0; j < DataRec->NumberofFields; j++) {
                if (DataRec->StatusFlag[j] != FIELDFTI) {
                    if (DataRec->FieldValue[j] == EIM_MISSING_VALUE) {
                        Counter->MissingValues[EIE_PERIOD_CURRENT]++;
                        Remove = EIE_TRUE;
                        j = DataRec->NumberofFields; /* skip remaining */
                    }
                    else if (PositivityOption != EIPE_ACCEPT_NEGATIVE &&
                            DataRec->FieldValue[j] < 0.0) {
                        Counter->NegativeValues[EIE_PERIOD_CURRENT]++;
                        Remove = EIE_TRUE;
                        j = DataRec->NumberofFields; /* skip remaining */
                    }
                }
            }

            /* ... and then check historical, if current was good */
            DataRec = Data->DataRec[EIE_PERIOD_HISTORICAL];
            if (!Remove && DataRec != NULL) {
                for (j = 0; j < DataRec->NumberofFields; j++) {
                    if (DataRec->StatusFlag[j] != FIELDFTI) {
                        if (DataRec->FieldValue[j] == EIM_MISSING_VALUE) {
                            Counter->MissingValues[EIE_PERIOD_HISTORICAL]++;
                            Remove = EIE_TRUE;
                            j = DataRec->NumberofFields; /* skip remaining */
                        }
                        else if (PositivityOption != EIPE_ACCEPT_NEGATIVE &&
                                DataRec->FieldValue[j] < 0.0) {
                            Counter->NegativeValues[EIE_PERIOD_HISTORICAL]++;
                            Remove = EIE_TRUE;
                            j = DataRec->NumberofFields; /* skip remaining */
                        }
                    }
                }
            }

			/* save the Next pointer now, after the free it is too late */
			DataNext = Data->Next;

            if (Remove == EIE_TRUE) {
                Counter->ValidObs[EIE_PERIOD_CURRENT]--;
                if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL)
                    Counter->ValidObs[EIE_PERIOD_HISTORICAL]--;

                if (Data == DataTable->Data[i])
                    DataTable->Data[i] = Data->Next;
                else
                    PreviousData->Next = Data->Next;

                EI_DataTableFreeItem (Data);
            }
            else
                PreviousData = Data;
        }
    }

/*    EI_DataTablePrintStatistics (DataTable);*/
/*    EI_DataTablePrint (DataTable);*/
}
/*********************************************************************
Sets all residual to TRUE or FALSE
*********************************************************************/
static void SetAllRandomErrorSpecifications (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    char * RandomError)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        strcpy (EstimatorList->Estimator[i].RandomError, RandomError);
    }
}
/*********************************************************************
Sets up DataSets structure
*********************************************************************/
static IO_RETURN_CODE SetUpDataSets (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tStatusDataSet * InCurrStatus,
    tDataDataSet * InHist,
    tStatusDataSet * InHistStatus,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    EIPT_POSITIVITY_OPTION PositivityOption,
    EIT_BOOLEAN VerifySpecsOption)
{
    int i;
    IO_RETURN_CODE rc = IORC_ERROR;

    InCurr->Key.Position = sp->dsr_indata.dsr.VL_unit_id.positions[0];
    IOUtil_copy_varname(InCurr->Key.Name, sp->dsr_indata.dsr.col_names[InCurr->Key.Position]);
    IOUtil_copy_varname(InCurrStatus->Key.Name, InCurr->Key.Name);

    if (InCurr->ExclusionInUse == EIE_TRUE) {
        InCurr->ExclusionIndicator.Position = sp->dsr_indata.VL_data_excl.positions[0];
        IOUtil_copy_varname(InCurr->ExclusionIndicator.Name, sp->dsr_indata.dsr.col_names[InCurr->ExclusionIndicator.Position]);
    }

    rc = SetUpInDataDataSet (&sp->dsr_indata, InCurr);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = SetUpInStatusDataSet (&sp->dsr_instatus, InCurrStatus);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        IOUtil_copy_varname(InHist->Key.Name, InCurr->Key.Name);
        if (InHist->ExclusionInUse == EIE_TRUE) {
            InHist->ExclusionIndicator.Position = sp->dsr_indata_hist.VL_data_excl.positions[0];
            IOUtil_copy_varname(InHist->ExclusionIndicator.Name, sp->dsr_indata_hist.dsr.col_names[InHist->ExclusionIndicator.Position]);
        }

        rc = SetUpInDataDataSet (&sp->dsr_indata_hist, InHist);
        if (rc != IORC_SUCCESS) {
            return rc;
        }

        if (sp->dsr_instatus_hist.dsr.is_specified == IOSV_SPECIFIED) {
            IOUtil_copy_varname(InHistStatus->Key.Name, InCurr->Key.Name);
            rc = SetUpInStatusDataSet (&sp->dsr_instatus_hist, InHistStatus);
            if (rc != IORC_SUCCESS) {
                return rc;
            }
        }
    }

    /* should be in ValidateParms() but I need info from SetUpDataSets() */
	if (EIE_SUCCEED != ExclusivityBetweenLists(sp, InCurr, InHist)) {
        return IORC_FAIL_LIST_EXCLUSIVITY;
	}

    if (VerifySpecsOption == EIE_FALSE) {

        /* set variable name case as they are in input data set*/
        for (i = 0; i < OutData->NumberOfVariables; i++) {
            SUtil_GetOriginalName(&sp->dsr_indata.dsr, OutData->VariableName[i]);
            SUtil_GetOriginalName(&sp->dsr_indata.dsr, OutData->Variable[i].Name);
        }

        IOUtil_copy_varname(OutData->Key.Name, InCurr->Key.Name);
        OutData->Key.Size = DSR_get_column_length(&sp->dsr_indata.dsr,
            InCurr->Key.Position);
        if (PRC_SUCCESS != DefineGatherWriteData(&sp->dsw_outdata, OutData)) {
            return IORC_FAIL_INIT_OUT_DATASET;
        }

        IOUtil_copy_varname(OutStatus->Key.Name, InCurr->Key.Name);
        OutStatus->Key.Size = DSR_get_column_length(&sp->dsr_indata.dsr,
            InCurr->Key.Position);
        if (PRC_SUCCESS != DefineGatherWriteStatus (&sp->dsw_outstatus, OutStatus)) {
            return IORC_FAIL_INIT_OUT_DATASET;
        }

        if (sp->dsw_outrand_err.is_requested == IOB_TRUE) {
            OutRandomError->Recipient.Size = OutData->Key.Size;
            OutRandomError->Donor.Size = OutData->Key.Size;
            if (PRC_SUCCESS != DefineGatherWriteRandomError (&sp->dsw_outrand_err, OutRandomError)) {
                return IORC_FAIL_INIT_OUT_DATASET;
            }
        }

        if (sp->dsw_outacceptable.is_requested == IOB_TRUE) {
            IOUtil_copy_varname(OutAcceptable->Key.Name, InCurr->Key.Name);
            OutAcceptable->Key.Size = OutData->Key.Size;
            if (PRC_SUCCESS != DefineGatherWriteAcceptable (&sp->dsw_outacceptable, OutAcceptable)) {
                return IORC_FAIL_INIT_OUT_DATASET;
            }
        }

        if (sp->dsw_outest_parm.is_requested == IOB_TRUE){
            if (PRC_SUCCESS != DefineGatherWriteEstimParms (&sp->dsw_outest_parm, OutEstimParms,
                PositivityOption)) {
                return IORC_FAIL_INIT_OUT_DATASET;
            }
        }

        if (sp->dsw_outest_ef.is_requested == IOB_TRUE){
            if (PRC_SUCCESS != DefineGatherWriteEstimEF (&sp->dsw_outest_ef, OutEstimEF)) {
                return IORC_FAIL_INIT_OUT_DATASET;
            }
        }

        if (sp->dsw_outest_lr.is_requested == IOB_TRUE){
            if (PRC_SUCCESS != DefineGatherWriteEstimLR (&sp->dsw_outest_lr, OutEstimLR)) {
                return IORC_FAIL_INIT_OUT_DATASET;
            }
        }
    }

    return rc;
}
/*********************************************************************
Verify mandatory variables exist in Algorithm file.
If they exist get their corresponding position and size.
Allocate the necessary space to read them.
*********************************************************************/
static IO_RETURN_CODE SetUpInAlgorithmDataSet (
    DSR_inalgorithm* dsr,
    tInAlgorithmDataSet * InAlgorithm)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;

    rc = VariableCharDefineForInput (&dsr->dsr,
        ALGORITHM_NAME, &InAlgorithm->Name);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ALGORITHM_TYPE, &InAlgorithm->Type);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ALGORITHM_STATUS, &InAlgorithm->Status);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ALGORITHM_FORMULA, &InAlgorithm->Formula);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ALGORITHM_DESCRIPTION, &InAlgorithm->Description);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    DefineScatterReadAlgorithm (dsr, InAlgorithm);

    return rc;
}
/*********************************************************************
Verify variables exist in DATA and HIST data sets.
If they exist get their corresponding position and size.
Allocate the necessary space to read them.
*********************************************************************/
static IO_RETURN_CODE SetUpInDataDataSet (
    DSR_indata* dsr,
    tDataDataSet * InData)
{
    int i;
    IO_RETURN_CODE rc = IORC_SUCCESS;

    rc = VariableCharDefineForInput (&dsr->dsr,
        InData->Key.Name, &InData->Key);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    if (InData->ExclusionInUse == EIE_TRUE) {
        rc = VariableCharDefineForInput(&dsr->dsr,
            InData->ExclusionIndicator.Name, &InData->ExclusionIndicator);
        if (rc != IORC_SUCCESS) {
            return rc;
        }
    }


    for (i = 0; i < InData->NumberOfVariables; i++) {
        rc = VariableNumericDefineForInput (&dsr->dsr,
            InData->Variable[i].Name, &InData->Variable[i]);
        if (rc != IORC_SUCCESS) {
            return rc;
        }
    }

    rc = DefineScatterReadData (dsr, InData);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    return rc;
}
/*********************************************************************
Verify mandatory variables exist in estimator data set.
If they exist get their corresponding position and size.
Allocate the necessary space to read them.
*********************************************************************/
static IO_RETURN_CODE SetUpInEstimatorDataSet (
    DSR_inestimator* dsr,
    tInEstimatorDataSet * InEstimator)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_ALGORITHMNAME, &InEstimator->AlgorithmName);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_FIELDID, &InEstimator->FieldId);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_AUXVARIABLES, &InEstimator->AuxVariables);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_WEIGHTVARIABLE, &InEstimator->WeightVariable);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_VARIANCEVARIABLE, &InEstimator->VarianceVariable);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_VARIANCEPERIOD, &InEstimator->VariancePeriod);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_EXCLUDEIMPUTED, &InEstimator->ExcludeImputed);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_EXCLUDEOUTLIERS, &InEstimator->ExcludeOutliers);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        ESTIMATOR_RANDOMERROR, &InEstimator->RandomError);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableNumericDefineForInput (&dsr->dsr,
        ESTIMATOR_VARIANCEEXPONENT, &InEstimator->VarianceExponent);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableNumericDefineForInput (&dsr->dsr,
        ESTIMATOR_COUNTCRITERIA, &InEstimator->CountCriteria);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableNumericDefineForInput (&dsr->dsr,
        ESTIMATOR_PERCENTCRITERIA, &InEstimator->PercentCriteria);
    if (rc != IORC_SUCCESS) {
        return rc;
    }


    DefineScatterReadEstimator (dsr, InEstimator);

    return rc;
}
/*********************************************************************
Verify 3 mandatory variables exist in Status file.
If they exist get their corresponding position and size.
Allocate the necessary space to read them.
*********************************************************************/
static IO_RETURN_CODE SetUpInStatusDataSet (
    DSR_instatus* dsr,
    tStatusDataSet * InStatus)
{
    IO_RETURN_CODE rc = IORC_SUCCESS;

    rc = VariableCharDefineForInput (&dsr->dsr,
        InStatus->Key.Name, &InStatus->Key);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        SUTIL_FIELDSTAT_FIELDID, &InStatus->FieldId);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    rc = VariableCharDefineForInput (&dsr->dsr,
        SUTIL_FIELDSTAT_STATUS, &InStatus->Status);
    if (rc != IORC_SUCCESS) {
        return rc;
    }

    DefineScatterReadStatus (dsr, InStatus);
    
    return rc;
}
/*********************************************************************
prints time.
help to analyse performance.
*********************************************************************/
static void ShowTime (
    char * Message)
{
#ifdef _DEBUG
	if (DEBUG) UTIL_ShowTime (Message);
#endif
}
/*********************************************************************
*********************************************************************/
static void UnloadAlgorithm (
    EIT_ALGORITHM_LIST * AlgorithmList)
{
    EI_AlgorithmFree (AlgorithmList);
}
/*********************************************************************
*********************************************************************/
static void UnloadEstimator (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EI_EstimatorFree (EstimatorList);
}
/*********************************************************************
*********************************************************************/
static void UnsetUpDataSets (
    SP_estimato* sp,
    tDataDataSet * InCurr,
    tStatusDataSet * InCurrStatus,
    tDataDataSet * InHist,
    tStatusDataSet * InHistStatus,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    EIT_BOOLEAN VerifySpecsOption)
{
    FreeInDataDataSet (InCurr);
    FreeInStatusDataSet (InCurrStatus);

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        FreeInDataDataSet (InHist);
        if (sp->dsr_instatus_hist.dsr.is_specified == IOSV_SPECIFIED)
            FreeInStatusDataSet (InHistStatus);
    }

    if (VerifySpecsOption == EIE_FALSE) {
        FreeOutDataDataSet (OutData);
        FreeOutStatusDataSet (OutStatus);
        if (sp->dsw_outrand_err.is_requested == IOB_TRUE)
            FreeOutRandomErrorDataSet (OutRandomError);
        if (sp->dsw_outacceptable.is_requested == IOB_TRUE)
            FreeOutAcceptableDataSet (OutAcceptable);
        if (sp->dsw_outest_parm.is_requested == IOB_TRUE)
            FreeOutEstimParmsDataSet (OutEstimParms);
        if (sp->dsw_outest_ef.is_requested == IOB_TRUE)
            FreeOutEstimEFDataSet (OutEstimEF);
        if (sp->dsw_outest_lr.is_requested == IOB_TRUE)
            FreeOutEstimLRDataSet (OutEstimLR);
    }
}
/*********************************************************************
Checks if DATAEXCLVAR and HISTEXCLVAR statements were specified when not needed.
statements is needed when at least one estimator is a estimator function
containing an average or is a linear regression.
*********************************************************************/
static void ValidateExclVarStatement (
    SP_estimato* sp,
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_BOOLEAN AreParametersRequested;
    EIT_BOOLEAN AreRandomErrorsRequested;
    EIT_BOOLEAN IsDATAEXCLVARSpecified;
    EIT_BOOLEAN IsHISTEXCLVARSpecified;

    AreParametersRequested = EI_AreParametersRequested (EstimatorList);
    AreRandomErrorsRequested = EI_AreRandomErrorsRequested (EstimatorList);

    IsDATAEXCLVARSpecified = VL_is_specified(&sp->dsr_indata.VL_data_excl) ? 1 : 0;
    IsHISTEXCLVARSpecified = VL_is_specified(&sp->dsr_indata_hist.VL_data_excl) ? 1 : 0;

    if (IsDATAEXCLVARSpecified &&
            AreParametersRequested == EIE_FALSE &&
            AreRandomErrorsRequested == EIE_FALSE)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgExclVarStatementIgnored SAS_NEWLINE, BPN_DATA_EXCL_VAR);

    if (IsHISTEXCLVARSpecified &&
            AreParametersRequested == EIE_FALSE &&
            AreRandomErrorsRequested == EIE_FALSE)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgExclVarStatementIgnored SAS_NEWLINE, BPN_DATA_EXCL_VAR_HIST);
}
/*********************************************************************
validates proc parameters
does all validations before returning
*********************************************************************/
static EIT_RETURNCODE ValidateParms (
    SP_estimato* sp,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    EIT_BOOLEAN AreHistVariablesRequested;
    int crc; /* cumulative return code */
    int rc;

    crc = EIE_SUCCEED;

    if (sp->dsr_indata.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INDATA);
        crc = EIE_FAIL;
    }
    if (sp->dsr_instatus.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INSTATUS);
        crc = EIE_FAIL;
    }
    if (sp->dsr_inestimator.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INESTIMATOR);
        crc = EIE_FAIL;
    }

    if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
        /* make sure HIST data set has the by groups variables */
        rc = SUtil_AreAllByVariablesInDataSet(&sp->dsr_indata.dsr, &sp->dsr_indata_hist.dsr);
        if (rc != EIE_SUCCEED) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgByVarsNotFoundInDataSet, DSN_INDATA_HIST);
            crc = EIE_FAIL;
        }
    }

    if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_UNIT_ID);
        crc = EIE_FAIL;
    }

    if (PositivityOption == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        crc = EIE_FAIL;
    }

    if (EstimatorList->NumberEntries == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgMustSpecifyEstimatorSpecs);
        crc = EIE_FAIL;
    }
    else {
        rc = EI_ValidateEstimator (EstimatorList, SymbolTable);
        if (rc != EIE_SUCCEED) {
            EI_PrintMessages ();
            IO_PRINT_LINE ("");
            return EIE_FAIL; /* must leave NOW! */
        }

        ValidateExclVarStatement (sp, EstimatorList);

        AreHistVariablesRequested = EI_AreHistVariablesRequested (
            EstimatorList);

        if (sp->dsr_indata_hist.dsr.is_specified == IOSV_NOT_SPECIFIED) {
            if (AreHistVariablesRequested == EIE_TRUE) {
                /* stop if HIST is required */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAlgorithmWithHistVarAndNoHistDataSet);
                crc = EIE_FAIL;
            }

            if (sp->dsr_instatus_hist.dsr.is_specified == IOSV_SPECIFIED)
                /* warn if HISTSTATUS data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHistStatusOptionIgnored);
        }
        else { /* InHist->File != NULL */
            if (AreHistVariablesRequested == EIE_FALSE)
                /* warn if HIST data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHistOptionIgnored);
        }

        OutRandomError->Requested = EIE_FALSE;
        if (sp->dsw_outrand_err.is_requested == IOB_TRUE) {
            if (AreRandomErrorRequested (EstimatorList) == EIE_FALSE)
                /* warn if OUTRANDOMERROR data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgOutRandomErrorOptionIgnored);
            else
                OutRandomError->Requested = EIE_TRUE;
        }

        OutAcceptable->Requested = EIE_FALSE;
        if (sp->dsw_outacceptable.is_requested == IOB_TRUE) {
            if (EI_AreParametersRequested (EstimatorList) == EIE_FALSE)
                /* warn if OutAcceptable data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgOutAcceptableOptionIgnored);
            else
                OutAcceptable->Requested = EIE_TRUE;
        }

        OutEstimParms->Requested = EIE_FALSE;
        if (sp->dsw_outest_parm.is_requested == IOB_TRUE)
            OutEstimParms->Requested = EIE_TRUE;

        OutEstimEF->Requested = EIE_FALSE;
        if (sp->dsw_outest_ef.is_requested == IOB_TRUE) {
            if (EI_AreEFRequestedWithParameters (EstimatorList) == EIE_FALSE)
                /* warn if OutEstimEF data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgOutEstEFOptionIgnored);
            else
                OutEstimEF->Requested = EIE_TRUE;
        }

        OutEstimLR->Requested = EIE_FALSE;
        if (sp->dsw_outest_lr.is_requested == IOB_TRUE) {
            if (EI_AreLRRequested (EstimatorList) == EIE_FALSE)
                /* warn if OutEstimLR data set was specified for nothing */
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgOutEstLROptionIgnored);
            else
                OutEstimLR->Requested = EIE_TRUE;
        }

    }

    return crc;
}
/*********************************************************************
Allocate the `.Value` member of a `tVariableChar` struct.
Caller must set `.Size` prior to calling this function.
*********************************************************************/
static void VariableCharAllocate(
    tVariableChar* Variable)
{
    Variable->Value = STC_AllocateMemory(Variable->Size + 1);
}
/*********************************************************************
Checks that the variable VariableName exist in File and
is a character variable.
Allocates the space needed.
*********************************************************************/
static IO_RETURN_CODE VariableCharDefineForInput (
    DSR_generic* dsr,
    char * VariableName,
    tVariableChar * Variable)
{
    int VariableType;

    IOUtil_copy_varname(Variable->Name, VariableName);
    int num_vars = 1;
    DSR_get_pos_from_names(dsr, num_vars, &VariableName, &Variable->Position);
    if (Variable->Position == IOSV_VAR_MISSING) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet SAS_NEWLINE,
            Variable->Name, dsr->dataset_name);
        return IORC_VARLIST_NOT_FOUND;
    }
    VariableType = DSR_get_col_type(dsr, Variable->Position);
    if (VariableType != IOVT_CHAR) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet SAS_NEWLINE,
            Variable->Name, dsr->dataset_name);
        return IORC_FAIL_TYPE_INCORRECT;
    }

    Variable->Size = DSR_get_column_length(dsr, Variable->Position);
    VariableCharAllocate(Variable);

    return IORC_SUCCESS;
}
/*********************************************************************
Checks that the variable VariableName exist in File and
is a character variable.
Allocates the space needed.
*********************************************************************/
static PROC_RETURN_CODE VariableCharDefineForOutput (
    DSW_generic * dsw,
    int destination_index,
    tVariableChar * Variable)
{
    VariableCharAllocate(Variable);
    if (IORC_SUCCESS != DSW_define_column(dsw, destination_index, Variable->Name, Variable->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Frees tVariableChar.
*********************************************************************/
static void VariableCharFree (
    tVariableChar * Variable)
{
    STC_FreeMemory (Variable->Value);
}
/*********************************************************************
Prints content of tVariableChar
*********************************************************************/
static void VariableCharPrint (
    tVariableChar * Variable)
{
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "N=%s S=%d P=%d V=%s\n",
        Variable->Name, Variable->Size, Variable->Position, Variable->Value);
}
/*********************************************************************
set a tVariableChar, value must be null terminated.
*********************************************************************/
static void VariableCharSet (
    tVariableChar * Variable,
    char * Value)
{
    SUtil_CopyPad (Variable->Value, Value, Variable->Size);
}
/*********************************************************************
Checks that the variable VariableName exist in File and
is a numeric variable
*********************************************************************/
static IO_RETURN_CODE VariableNumericDefineForInput (
    DSR_generic* dsr,
    char * VariableName,
    tVariableNumeric * Variable)
{
    int VariableType;

    IOUtil_copy_varname(Variable->Name, VariableName);

    int num_vars = 1;
    DSR_get_pos_from_names(dsr, num_vars, &VariableName, &Variable->Position);
    if (Variable->Position == IOSV_VAR_MISSING) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgNumericVarNotInDataSet SAS_NEWLINE,
            Variable->Name, dsr->dataset_name);
        return IORC_VARLIST_NOT_FOUND;
    }

    VariableType = DSR_get_col_type(dsr, Variable->Position);
    if (VariableType != SUTIL_VARIABLE_TYPE_NUMERIC) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotNumericInDataSet SAS_NEWLINE,
            Variable->Name, dsr->dataset_name);
        return IORC_FAIL_TYPE_INCORRECT;
    }

    Variable->Size = sizeof Variable->Value;

    return IORC_SUCCESS;
}
/*********************************************************************
Define numeric variable for output
*********************************************************************/
static PROC_RETURN_CODE VariableNumericDefineForOutput (
    DSW_generic* dsw,
    int destination_index,
    tVariableNumeric * Variable)
{
    if (IORC_SUCCESS != DSW_define_column(dsw, destination_index, Variable->Name, &Variable->Value, IOVT_NUM)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/*********************************************************************
Prints content of tVariableNumeric
*********************************************************************/
static void VariableNumericPrint (
    tVariableNumeric * Variable)
{
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "N=%s S=%d P=%d V=%f\n",
        Variable->Name, Variable->Size, Variable->Position, Variable->Value);
}
/*********************************************************************
set a tVariableNumeric, takes care of missing values
*********************************************************************/
static void VariableNumericSet (
    tVariableNumeric * Variable,
    double Value)
{
    if (Value == EIM_MISSING_VALUE)
        Variable->Value = IOUtil_missing_value('.');
    else
        Variable->Value = Value;
}
/*********************************************************************
Writes result to output data sets: OUTDATA, OUTSTATUS and OUTRANDOMERROR
Also, write result to output report data sets: OUTESTPARMS, OUTESTEF,
OUTESTLR and OUTACCEPTABLE
*********************************************************************/
static PROC_RETURN_CODE WriteResults (
    SP_estimato* sp,
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_DATATABLE * DataTable,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tOutRandomErrorDataSet * OutRandomError,
    tOutAcceptableDataSet * OutAcceptable,
    tOutEstimParmsDataSet * OutEstimParms,
    tOutEstimEFDataSet * OutEstimEF,
    tOutEstimLRDataSet * OutEstimLR,
    int * InToOutMapping,
    int * OutToInMapping,
    tAllOriginalVarNames AllOriginalVarNames,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    EIT_BOOLEAN AtLeastOneImputation;
    EIT_AUX * Aux;
    EIT_AVERAGE * Average;
    EIT_DATA * Data;
    EIT_DATAREC * DataRec;
    int i;
	int IntStatusFlag;
    int j;
    EIT_REGRESSION * Regression;
    EIT_RANDOMERRORRESULT * RER;
    char Status[EIM_STATUS_SIZE+1];

    /* write out outdata and status data sets */
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {

            DataRec = Data->DataRec[EIE_PERIOD_CURRENT];

            VariableCharSet (&OutData->Key, Data->Key);
            VariableCharSet (&OutStatus->Key, Data->Key);

            AtLeastOneImputation = EIE_FALSE;
            for (j = 0; j < OutData->NumberOfVariables; j++) {
                VariableNumericSet (&OutData->Variable[j],
                    DataRec->FieldValue[OutToInMapping[j]]);

                /*
                We use to do the test
                if (DataRec->StatusFlag[OutToInMapping[j]] < 0) {
                but under gcc which promotes enumeration types to unsigned
                integer types the test was always false.
                So by assigning the StatusFlag to a signed int we can safely
                compare the value of StatusFlag agains 0.
                */
                IntStatusFlag = DataRec->StatusFlag[OutToInMapping[j]];

                if (IntStatusFlag < 0) {
                    AtLeastOneImputation = EIE_TRUE;

                    VariableCharSet (&OutStatus->FieldId,
                        OutData->Variable[j].Name);

                    sprintf (Status, "I%s",
                        EstimatorList->Estimator[-1-DataRec->
                            StatusFlag[OutToInMapping[j]]].Algorithm->Status);
                    VariableCharSet (&OutStatus->Status, Status);
                    // copy value from `outdata` dataset
                    OutStatus->FieldValue = OutData->Variable[j].Value;

                    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                        return PRC_FAIL_WRITE_DATA;
                    }
                }
            }

            if (AtLeastOneImputation == EIE_TRUE) {
                if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdata)) {
                    return PRC_FAIL_WRITE_DATA;
                }
            }
        }
    }

    /* write out info on imputation with "estimator[i]" ... */
    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        /* write out OUTRANDOMERROR= data set */
        if (OutRandomError->Requested == EIE_TRUE) {
            if (EstimatorList->Estimator[i].RandomErrorResult != NULL) {
                for (j = 0; j < EstimatorList->Estimator[i].RandomErrorResult->
                        NumberEntries; j++) {

                    RER = &EstimatorList->Estimator[i].RandomErrorResult->
                        RandomErrorResult[j];

                    VariableNumericSet (&OutRandomError->EstimatorId, i);
                    VariableCharSet (&OutRandomError->AlgorithmName,
                        EstimatorList->Estimator[i].AlgorithmName);
                    VariableCharSet (&OutRandomError->Recipient,
                        RER->Recipient);
                    VariableCharSet (&OutRandomError->Donor, RER->Donor);
                    VariableCharSet (&OutRandomError->FieldId,
                        OutData->VariableName[InToOutMapping[
                            EstimatorList->Estimator[i].FieldIdIndex]]);
                    VariableNumericSet (&OutRandomError->Residual,
                         RER->Residual);
                    VariableNumericSet (&OutRandomError->RandomError,
                        RER->RandomError);
                    VariableNumericSet (&OutRandomError->OriginalValue,
                        RER->OriginalValue);
                    VariableNumericSet (&OutRandomError->ImputedValue,
                        RER->ImputedValue);

                    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outrand_err)){
                        return PRC_FAIL_WRITE_DATA;
                    }
                }
            }
        }

        /* write out OUTACCEPTABLE= data set
           ("requested" is true if at least one estimator
            with parameters is in the list specified) */
        if (OutAcceptable->Requested == EIE_TRUE) {
            if (EstimatorList->Estimator[i].Active) {
                if (EstimatorList->Estimator[i].AcceptableKeysResult != NULL) {
                    for (j = 0; j < EstimatorList->Estimator[i].
                            AcceptableKeysResult->NumberEntries; j++) {
                        VariableNumericSet (&OutAcceptable->EstimatorId, i);
                        VariableCharSet (&OutAcceptable->AlgorithmName,
                            EstimatorList->Estimator[i].AlgorithmName);
                        VariableCharSet (&OutAcceptable->Key,
                            EstimatorList->Estimator[i].AcceptableKeysResult->
                            AcceptableKeys[j]);

                        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outacceptable)) {
                            return PRC_FAIL_WRITE_DATA;
                        }
                    }
                }
            }
        }

        /* write out OUTESTPARMS= data set
           ("requested" is true if at least one estimator
            with parameters is in the list specified) */
        if (OutEstimParms->Requested == EIE_TRUE) {
            if (EstimatorList->Estimator[i].Active) {
                VariableNumericSet (&OutEstimParms->EstimatorId, i);
                VariableCharSet (&OutEstimParms->AlgorithmName,
                    EstimatorList->Estimator[i].AlgorithmName);
                VariableCharSet (&OutEstimParms->FieldId,
                    OutData->VariableName[InToOutMapping[
                        EstimatorList->Estimator[i].FieldIdIndex]]);
                VariableNumericSet (&OutEstimParms->Fti,
                    EstimatorList->Estimator[i].FTI);
                VariableNumericSet (&OutEstimParms->Imputed,
                    EstimatorList->Estimator[i].Imputed);
                VariableNumericSet (&OutEstimParms->DivisionByZero,
                    EstimatorList->Estimator[i].DivisionByZero);
                if (PositivityOption != EIPE_ACCEPT_NEGATIVE)
                    VariableNumericSet (&OutEstimParms->NegativeImputation,
                        EstimatorList->Estimator[i].NegativeImputation);

                if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outest_parm)) {
                    return PRC_FAIL_WRITE_DATA;
                }
            }
        }

        /* write out OUTESTEF= data set
           ("requested" is true if at least one EF estimator
            with parameters is in the list specified)*/
        if (OutEstimEF->Requested == EIE_TRUE) {
            if ((EstimatorList->Estimator[i].Active) &&
                (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_EF)){
               Average = EstimatorList->Estimator[i].Average;
               for (; Average != NULL; Average = Average->Next) {
                   VariableNumericSet (&OutEstimEF->EstimatorId, i);
                   VariableCharSet (&OutEstimEF->AlgorithmName,
                       EstimatorList->Estimator[i].AlgorithmName);
                   if (Average->Period == EIE_PERIOD_CURRENT) {
                       VariableCharSet (&OutEstimEF->FieldId,
                           AllOriginalVarNames.CurrentVarName
                               [Average->VariableIndex]);
                       VariableCharSet (&OutEstimEF->Period, "C");
                   }
                   else {
                       VariableCharSet (&OutEstimEF->FieldId,
                           AllOriginalVarNames.HistVarName
                               [Average->VariableIndex]);
                       VariableCharSet (&OutEstimEF->Period, "H");
                   }
                   VariableNumericSet (&OutEstimEF->AverageValue,
                       Average->Average);
                   VariableNumericSet (&OutEstimEF->Count, Average->Count);

                   if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outest_ef)) {
                       return PRC_FAIL_WRITE_DATA;
                   }
               }
            }
        }

        /* write out OUTESTLR= data set
           ("requested" is true if at least one LR estimator
            is in the list specified) */
        if (OutEstimLR->Requested == EIE_TRUE) {
            if ((EstimatorList->Estimator[i].Active) &&
                (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_LR)){
               Regression = EstimatorList->Estimator[i].Regression;
               /* write out info on "intercept" */
               if (Regression->Intercept == EIE_TRUE) {
                   VariableNumericSet (&OutEstimLR->EstimatorId, i);
                   VariableCharSet (&OutEstimLR->AlgorithmName,
                       EstimatorList->Estimator[i].AlgorithmName);
                   VariableCharSet (&OutEstimLR->FieldId, "Intercept");
                   VariableNumericSet (&OutEstimLR->Exponent,
                       EIM_MISSING_VALUE);
                   VariableCharSet (&OutEstimLR->Period, "");
                   if (Regression->Betas != NULL)
                       VariableNumericSet (&OutEstimLR->BetaValue,
                           Regression->Betas[0]);
                   else
                       VariableNumericSet (&OutEstimLR->BetaValue, 0);
                   VariableNumericSet (&OutEstimLR->Count, Regression->Count);

                   if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outest_lr)) {
                       return PRC_FAIL_WRITE_DATA;
                   }
              }

              /* write out info on "regressors" */
              for (Aux=Regression->Aux, j=0; Aux != NULL; Aux = Aux->Next, j++){
                   VariableNumericSet (&OutEstimLR->EstimatorId, i);
                   VariableCharSet (&OutEstimLR->AlgorithmName,
                       EstimatorList->Estimator[i].AlgorithmName);
                   if (Aux->Period == EIE_PERIOD_CURRENT) {
                       VariableCharSet (&OutEstimLR->FieldId,
                           AllOriginalVarNames.CurrentVarName[Aux->AuxIndex]);
                       VariableCharSet (&OutEstimLR->Period, "C");
                   }
                   else {
                       VariableCharSet (&OutEstimLR->FieldId,
                           AllOriginalVarNames.HistVarName[Aux->AuxIndex]);
                       VariableCharSet (&OutEstimLR->Period, "H");
                   }
                   VariableNumericSet (&OutEstimLR->Exponent,
                       Aux->ExponentValue);
                   if (Regression->Betas != NULL)
                       VariableNumericSet (&OutEstimLR->BetaValue,
                           Regression->Betas[j+Regression->Intercept]);
                   else
                       VariableNumericSet (&OutEstimLR->BetaValue, 0);
                   VariableNumericSet (&OutEstimLR->Count, Regression->Count);

                   if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outest_lr)) {
                       return PRC_FAIL_WRITE_DATA;
                   }
              }
            } /* "EstimatorList->Estimator[i]" : active and LR */
         } /* OutEstimLR->Requested is TRUE */

    } /* increment "i" for "EstimatorList->Estimator[i]" */

    return PRC_SUCCESS;
}
