/*********************************************************************
NAME:         OUTLIER
DESCRIPTION:  identifies outlying observations.
*********************************************************************/

/* these must be define before including <uwproc.h> */
#define MAINPROC 1
#define SASPROC  1

#include <stdio.h>
#include <string.h>

#include "outlier_JIO.h"

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Message.h"
#include "EI_Outlier.h"
#include "EI_UserData.h"
#include "EIP_Common.h"
#include "STC_Memory.h"

/* statements and options name */
#define DATA_GRM_NAME          DSN_INDATA
#define HIST_GRM_NAME          DSN_INDATA_HIST
#define AUX_GRM_NAME           "AUX"
#define OUTSTATUS_GRM_NAME     DSN_OUTSTATUS
#define OUTSUM_GRM_NAME        DSN_OUTSUMMARY
#define METHOD_GRM_NAME        BPN_METHOD
#define MEI_GRM_NAME           BPN_MEI
#define MII_GRM_NAME           BPN_MII
#define BETAE_GRM_NAME         BPN_BETA_E
#define BETAI_GRM_NAME         BPN_BETA_I
#define MDM_GRM_NAME           BPN_MDM
#define EXPONENT_GRM_NAME      BPN_EXPONENT
#define MINOBS_GRM_NAME        BPN_MIN_OBS
#define OUTLIERSTAT_GRM_NAME   BPN_OUTLIER_STATS /* called BOUNDSTAT prior to Banff 2.06 */
#define ID_GRM_NAME            BPN_UNIT_ID
#define VAR_GRM_NAME           BPN_VAR
#define WITH_GRM_NAME          BPN_WITH
#define WEIGHT_GRM_NAME        BPN_WEIGHT
#define ACCEPTNEGATIVE_GRM_NAME BPN_ACCEPT_NEGATIVE
#define ACCEPTZERO_GRM_NAME    BPN_ACCEPT_ZERO
#define STARTCENTILE_GRM_NAME  BPN_START_CENTILE
#define SIGMA_GRM_NAME         BPN_SIGMA
#define SIDE_GRM_NAME          BPN_SIDE

/* related to METHOD parameter */
#define MAX_LENGTH_NAME8 8 /* as the name suggest */
#define CURRENT_WORD     "CURRENT"
#define CURRENT_ABBREV1  "C"
#define CURRENT_ABBREV2  "CU"
#define HIST_WORD        "HISTORIC"
#define HIST_ABBREV1     "H"
#define HIST_ABBREV2     "HT"
#define RATIO_WORD       "RATIO"
#define RATIO_ABBREV1    "R"
#define SIGMAGAP_WORD    "SIGMAGAP"
#define SIGMAGAP_ABBREV1 "S"
#define SIGMAGAP_ABBREV2 "SG"

#define SIGMA_STD        "STD"
#define SIGMA_MAD        "MAD"
#define SIGMA_DEFAULT    SIGMA_MAD

#define SIDE_LEFT        "LEFT"
#define SIDE_BOTH        "BOTH"
#define SIDE_RIGHT       "RIGHT"
#define SIDE_DEFAULT     SIDE_BOTH

/* related to 4th field on FIELDSTAT */
#define STRLENGTH_STATUS 3  /* compare only the first 3 letters    */
#define ODE_ABBREV       "ODE"

/* related to outstatus_detailed dataset, depending on OUTLIERSTAT_OPTION and METHOD */
#define OUTSTATUS_DETAILED_MIN_VARIABLES   3 /* Id variable name, FIELDID, OUTSTATUS */
#define FIELDSTAT_OUTSTATUS          "OUTLIER_STATUS"
#define FIELDSTAT_CURVALUE           "CURRENT_VALUE"
#define FIELDSTAT_HISTAUX_FIELDID    "HIST_AUX"
#define FIELDSTAT_HISTAUX_VALUE      "HIST_AUX_VALUE"
#define FIELDSTAT_EFFECT             "EFFECT"
#define FIELDSTAT_LEFT_ODI_BOUNDARY  "IMP_BND_L"
#define FIELDSTAT_LEFT_ODE_BOUNDARY  "EXCL_BND_L"
#define FIELDSTAT_RIGHT_ODE_BOUNDARY "EXCL_BND_R"
#define FIELDSTAT_RIGHT_ODI_BOUNDARY "IMP_BND_R"
#define FIELDSTAT_EXCL_SIGMAGAP      "EXCL_SIGMAGAP"
#define FIELDSTAT_IMP_SIGMAGAP       "IMP_SIGMAGAP"
#define FIELDSTAT_GAP                "GAP"
#define FIELDSTAT_METHOD             "METHOD"
#define FIELDSTAT_METHOD_SIZE        2
#define FIELDSTAT_WEIGHT             "WEIGHT"

#define FIELDSTAT_METHOD_HB          "HB"
#define FIELDSTAT_METHOD_SG          "SG"

/* Variable (column) names for the OutSummary dataset */
#define OUTSUM_MIN_VARS              10 //FIELDID + NOBS + VALIDOBS + NUsed + NRej + NRej_VarMiss + NRej_VarZero + NRej_VarNeg + NFTI + NFTE
#define OUTSUM_VN_NOBS               "NObs"
#define OUTSUM_VN_NVALID             "NValid"
#define OUTSUM_VN_AUXID              "AuxVarID"
#define OUTSUM_VN_NREJ_NOMATCH       "NRejected_NoMatch"
#define OUTSUM_VN_NUSED              "NUsed"
#define OUTSUM_VN_NREJ_TOTAL         "NRejected"
#define OUTSUM_VN_NREJ_MISS          "NRejected_VarMissing"
#define OUTSUM_VN_NREJ_ZERO          "NRejected_VarZero"
#define OUTSUM_VN_NREJ_NEG           "NRejected_VarNegative"
#define OUTSUM_VN_NREJ_AUXMISS       "NRejected_AuxVarMissing"
#define OUTSUM_VN_NREJ_AUXZERO       "NRejected_AuxVarZero"
#define OUTSUM_VN_NREJ_AUXNEG        "NRejected_AuxVarNegative"
#define OUTSUM_VN_NFTI               "NFTI"
#define OUTSUM_VN_NFTE               "NFTE"
#define OUTSUM_VN_QUANTILE_FIRST     "Q1"
#define OUTSUM_VN_QUANTILE_MEDIAN    "M"
#define OUTSUM_VN_QUANTILE_THIRD     "Q3"
#define OUTSUM_VN_SIGMA              "DEVIATION"

#define ISNOTFOUND  -1

#define STARTCENTILE_SIDE_BOTH_MINIMUM_VALUE   50.0
#define STARTCENTILE_SIDE_OTHER_MINIMUM_VALUE   0.0
#define STARTCENTILE_SIDE_BOTH_MAXIMUM_VALUE  100.0
#define STARTCENTILE_SIDE_OTHER_MAXIMUM_VALUE 100.0
#define STARTCENTILE_SIDE_BOTH_DEFAULT_VALUE   75.0
#define STARTCENTILE_SIDE_OTHER_DEFAULT_VALUE   0.0

/* number of digit to print in report before IO_PRINT_LINE will start to use
scientific notation */
#define PRINT_PRECISION    7
#define PRINT_STATISTICS_WIDTH PRINT_STATISTICS_WIDTH_DEFAULT + 7

/*
4 cases for OUTLIER input mode:
Case 1: One data set  (DATA=)           - VAR statement
Case 2: One data set  (DATA=)           - VAR and WITH statements
Case 3: Two data sets (DATA= and HIST=) - VAR statement
Case 4: Two data sets (DATA= and HIST=) - VAR and WITH statements
*/
enum T_OUTLIER_INPUTMODE {
	E_OUTLIER_INPUTMODE_1DS_NOWITH,
	E_OUTLIER_INPUTMODE_1DS_WITH,
	E_OUTLIER_INPUTMODE_2DS_NOWITH,
	E_OUTLIER_INPUTMODE_2DS_WITH
};
typedef enum T_OUTLIER_INPUTMODE T_OUTLIER_INPUTMODE;

enum T_OUTLIER_METHOD {
	E_OUTLIER_METHOD_CURRENT,
	E_OUTLIER_METHOD_HISTORICAL,
	E_OUTLIER_METHOD_SIGMAGAP,
	E_OUTLIER_METHOD__ERROR_,
};
typedef enum T_OUTLIER_METHOD T_OUTLIER_METHOD;

/* info needed for execution statistics */
struct T_OUTLIER_COUNTER {
	int UnpairedHistObsInByGroup; /* # of HIST not paired on ID with DATA     */
	int EmptyHistObsInByGroup;    /* # of DATA not paired on ID with HIST     */

	/* 2 of each counters. 2 periods each.                                    */
	int ObsInByGroup[2];      /* # of obs in data set                         */
	int ValidObsInByGroup[2]; /* # of valid obs in data set                   */
							  /* (before substracting EmptyHistObsInByGroup   */
							  /*  for E_OUTLIER_METHOD_HISTORICAL)            */
	int MissingId[2];         /* # of missing id in data data set             */
	int MissingWeight;        /* # of missing weight in data data set         */
};
typedef struct T_OUTLIER_COUNTER T_OUTLIER_COUNTER;

/*
structure to hold info needed by the read and write callback functions.
all obs are read at once for one BY group and the information is put in
the EIT_DATATABLE.
the read callbacks use that data.
T_OUTLIERCALLBACKINFO structure:

READ callback
	EIT_DATATABLE structure
		contain a pointer to the EIT_DATA array of pointers to linked list
		of DATA the "size" of the EIT_DATA array,
		the total number of DATA structure in all the linked lists
	EIT_DATATABLE * DataTable;
		index for the EIT_DATA array (from 0 to DataTable->Size - 1)
	int DataTableIndex;
		number of DATA structure processed (0 to DataTable->NumberOfData - 1)
	int NumberOfDataRead;
		pointer to the next DATA to process
	EIT_DATA * Data;
		hold the id
	char * Id;
		index for variable processed (from 0 to (NumberOfVariables - 1)

WRITE callback
		output data set
	ptr Outfile;
		for SAS_XVPUT? calls
	ptr XVPut;
	char * Id;

		if OUTLIERSTAT option is specified

BOTH callbacks
		length of id in data set
	int IdLength;
*/
struct T_OUTLIERCALLBACKINFO {
	EIT_DATATABLE * DataTable;
	int DataTableIndex; /* 0 to (DataTable->Size - 1) */
	EIT_DATA * Data; /* Next DATA to process */
	int NumberOfDataRead; /* 0 to (DataTable->NumberOfData - 1) */
	int VarIndex; /* 0 to (NumberOfVarVariables - 1) */
	int WeightIndex; /* once set, it does not change. it is equal to NumberOfVarVariables. */

	int IdLength;
	int NumberOfWeightVariables; // 0 or 1
	//some options
	EIPT_POSITIVITY_OPTION PositivityOption;
	EIT_BOOLEAN AcceptZero;
	//counters
	int MissingValue[2];//current + historical
	int NegativeValue[2];//current + historical
	int ZeroValue[2];//current + historical
	int NegativeWeight;
	int ZeroWeight;

	T_OUTLIER_METHOD Method;
	char MethodString[3];

	//the id value of the outlier variable, allocated once and reused
	char * Id;
	char Status[EIM_STATUS_SIZE+1];  /* FTI, FTE */
    double FieldValue;
	char OutlierStatus[EIM_STATUS_SIZE+1]; /* ODIL, ODIR, ODEL, ODER */
	EIT_BOOLEAN OutlierstatOption;
	double CurrentValue;
	double PreviousValue;
	double Weight;
	double Effect;
	double LeftODIBoundary;  /* Intervals:                                    */
	double LeftODEBoundary;  /* <------|------|---------------|------|------> */
	double RightODEBoundary; /*   ODIL | ODEL |  Acceptance   | ODER | ODIR   */
	double RightODIBoundary;
	double Excl_Sigmagap;// Deviation * BetaE
	double Imp_Sigmagap;// Deviation * BetaI
	double Gap;// Difference between this record and the previous one
};
typedef struct T_OUTLIERCALLBACKINFO T_OUTLIERCALLBACKINFO ;


static EIT_DATATABLE * AllocateDataTable (DSR_indata* dsr);
static void CountNoneHistObs (EIT_DATATABLE *, T_OUTLIER_COUNTER *);
static PROC_RETURN_CODE DefineCharForOutput (DSW_generic* dsw, int dest_index, char *, char *, int);
static PROC_RETURN_CODE DefineDoubleForOutput (DSW_generic* dsw, int dest_index, char *, double *, int);
static PROC_RETURN_CODE DefineGatherWrite (SP_outlier* sp, T_OUTLIERCALLBACKINFO *, T_OUTSUMINFO *, T_OUTLIER_INPUTMODE, EIT_OUTLIER_SIDE Side,
	double BetaE, double BetaI, char *, char *);
static void DefineScatterRead (DSR_indata* dsr, char *, int, double *, int, double *);
static EIT_RETURNCODE DuplicateBetweenListsInData (SP_outlier* sp, T_OUTLIER_INPUTMODE);
static EIT_RETURNCODE DuplicateBetweenWithByInHist (SP_outlier* sp, int, int *);
static T_OUTLIER_METHOD GetOutlierMethod (SP_outlier* sp, char *);
static IO_RETURN_CODE GetParms (SP_outlier* sp, T_OUTLIER_INPUTMODE * InputMode, T_OUTLIER_METHOD *, char *,  char *,
	double *, double *, double *, double *, double *, double *,
	int *, int *, char * IdVariableName, int *, int **, /*int *, */int **, int *, int **,
	EIT_BOOLEAN *, EIPT_POSITIVITY_OPTION *,
	EIT_BOOLEAN * AcceptZero, double * StartCentile, EIT_OUTLIER_SIDE * Side, char * SideString,
	EIT_OUTLIER_SIGMA * Sigma, char * SigmaString, int *);
static EIT_BOOLEAN Intersect (int *, int, int *, int);
static double MissingToSasMissing (double d);
static void PrintCountBadDataByVar (T_OUTLIER_INPUTMODE, int, char *, int *, int *, int *,
	int NegativeWeight, int ZeroWeight, char *, char *);
static void PrintParms (SP_outlier* sp, T_OUTLIER_INPUTMODE, T_OUTLIER_METHOD, char *, char *,
	T_OUTSUMINFO *, double, double, double, double, double, double, int, 
	EIT_BOOLEAN OutlierstatOption, EIPT_POSITIVITY_OPTION,
	double StartCentile, char * SideString, char * SigmaTypeString, int ByGroupLoggingLevel);
static void PrintStatistics (T_OUTLIER_INPUTMODE, SP_outlier* sp, T_OUTLIER_COUNTER *, char *);
static EIT_RETURNCODE ProcessByGroup (SP_outlier* sp, T_OUTLIER_INPUTMODE, int, char *, int *, int *, char *, int *,
	T_OUTLIERCALLBACKINFO *, T_OUTSUMINFO *, double, double, double, double, double, double, int, EIT_OUTLIER_SIDE Side, EIT_OUTLIER_SIGMA, double, char *, int);
static PROC_RETURN_CODE ReadByGroup (SP_outlier* sp, T_OUTLIER_INPUTMODE, char *, int, double *,
	double * WeightVariableValue, char *, double *, T_OUTLIERCALLBACKINFO *, T_OUTLIER_COUNTER *, char *, int);
static EIT_READCALLBACK_RETURNCODE ReadCDData (void *, char **, double *, double *);
static PROC_RETURN_CODE ReadData (SP_outlier* sp, char *, int, double *, double * WeightVariableValue, T_OUTLIERCALLBACKINFO *, T_OUTLIER_COUNTER *);
static PROC_RETURN_CODE ReadHist (SP_outlier* sp, char *, int, double *, T_OUTLIERCALLBACKINFO *, T_OUTLIER_COUNTER *, char *);
static EIT_READCALLBACK_RETURNCODE ReadHTData (void *, char **, double *, double *, double *);
static void SetDefaultVarList (SP_outlier* sp);
static EIT_RETURNCODE ValidateForTwoDatasetsAndWith (
	SP_outlier* sp,
	char *, char *, int *, int, int *);
static EIT_RETURNCODE ValidateForTwoDatasetsNoWith (SP_outlier* sp, char *, int,
		int *, char *, int *, int *);
static EIT_RETURNCODE ValidateIDInHist (SP_outlier* sp, char *, char *, int *);
static EIT_RETURNCODE ValidateParms (SP_outlier* sp, T_OUTLIER_INPUTMODE InputMode, T_OUTLIER_METHOD Method,
	double, double, double, double, double, double, int, EIPT_POSITIVITY_OPTION, char *,
	EIT_BOOLEAN AcceptZero, double StartCentile,
	EIT_OUTLIER_SIDE Side, char * SideString, char * SigmaTypeString);
static EIT_RETURNCODE ValidateVARInHist (SP_outlier* sp, int, int *, char *, int *);
static EIT_BOOLEAN VerifyEnoughObsByGroup (SP_outlier* sp, T_OUTLIER_INPUTMODE, int, char *,
	T_OUTLIER_COUNTER *, EIT_DATATABLE *, int);
static EIT_WRITECALLBACK_RETURNCODE WriteOutSum (DSW_generic* dsw, T_OUTSUMINFO * OutSumData);
static EIT_WRITECALLBACK_RETURNCODE WriteStatus (DSW_generic* dsw_outstatus, DSW_generic* dsw_outstatus_detailed,
	void *, char *, EIT_STATUS,
	double, double, double, double, double, double, double, double, double, double, double);


/*********************************************************************
*********************************************************************/
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int outlier(
	T_in_parm in_parms,

	T_in_ds in_ds_indata,
	T_in_ds in_ds_indata_hist,

    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outstatus_detailed,
    T_out_ds out_arr_outstatus_detailed,
    T_out_ds out_sch_outsummary,
    T_out_ds out_arr_outsummary
	)
{
	double BetaE;
	double BetaI;
	T_OUTLIER_COUNTER Counter				= { 0 };
	EIT_BOOLEAN EnoughObs					= { 0 };
	double Exponent;
	char HistAuxKeyword[12]					= { 0 }; /* keyword name used by user to specify secondary input data set: HIST or AUX */
	int IdPositionData; /* Position of ID var in DATA  */
	int IdPositionHist = -1; /* Position of ID var in HIST  */
	char IdVariableName[LEGACY_MAXNAME+1]		= { 0 };
	char * IdValueData						= { 0 };
	char * IdValueHist						= { 0 };
	T_OUTLIER_INPUTMODE InputMode			= { 0 };
	double MEI;
	char MethodString[MAX_LENGTH_NAME8+1]	= { 0 };
	double MII;
	double MinimumDistanceMultiplier;
	int MinObs;
	int NumberOfVarVariables;
	EIT_RETURNCODE rc						= { 0 };
	EIT_OUTLIER_SIDE Side					= { 0 };
	char SideString[MAX_LENGTH_NAME8+1]		= { 0 };
	EIT_OUTLIER_SIGMA Sigma					= { 0 };
	char SigmaString[MAX_LENGTH_NAME8+1]	= { 0 };
	double StartCentile;
	T_OUTLIERCALLBACKINFO UserData			= { 0 };
    T_OUTSUMINFO OutSumData					= { 0 };
	char VarVariableName[LEGACY_MAXNAME+1]		= { 0 };
	int * VarVariablesPosition			= { 0 }; /* Position of VAR variables in DATA */
	double * VarVariablesValue				= { 0 }; /* Values of VAR variables in DATA */
	int * WeightVariablePosition		= { 0 }; /* Position of WEIGHT variable in DATA */
	double WeightVariableValue; /* Value of WEIGHT variables in DATA */
	char WithVariableName[LEGACY_MAXNAME+1]		= { 0 };
	int * WithVariablesPosition		= { 0 }; /* Position of WITH variables (2 cases) - in DATA or - in AUX/HIST */
	double * WithVariablesValue				= { 0 };
	int ByGroupLoggingLevel; // 1- suppress BY group logging 0||else- don't suppress it ... what an uninformitive flag

	BANFF_RETURN_CODE proc_exit_code = BRC_SUCCESS;

	/* Initialize runtime environment */
	init_runtime_env();

	/* TIME MEASUREMENT */
	TIME_WALL_DECLARE(cleanup);
	TIME_CPU_DECLARE(cleanup);
	TIME_WALL_START(main);
	TIME_CPU_START(main);

#ifdef _CRTDBG_MAP_ALLOC
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	STC_AllocateMemorySetCB(SUtil_AllocateMemory);
	STC_ReallocationMemorySetCB(SUtil_ReallocateMemory);
	STC_FreeMemorySetCB(SUtil_FreeMemory);

	EI_PrintMessagesSetCB (SUtil_PrintMessages);

	EI_AllocateMessageList ();

	SUtil_PrintSystemInfo (SI_SYSTEMNAME, SI_SYSTEMVERSION, BANFF_PROC_NAME,
		SI_PROCVERSION, SI_EMAIL, NULL);

	/* Initialize JSON input/output related variables */
	// master: Statcan Procedurestructure
	SP_outlier			sp					= { 0 };

	TIME_WALL_START(load_init);
	TIME_CPU_START(load_init);
	mem_usage("before SP_init");
	IO_RETURN_CODE rc_sp_init = SP_init(
		&sp,
		in_parms,
		in_ds_indata, in_ds_indata_hist,
    	out_sch_outstatus,
    	out_arr_outstatus,
    	out_sch_outstatus_detailed,
    	out_arr_outstatus_detailed,
    	out_sch_outsummary,
    	out_arr_outsummary
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

	IO_RETURN_CODE rc_get_parms = IORC_SUCCESS;

	rc_get_parms = GetParms (&sp, &InputMode, &UserData.Method, MethodString, HistAuxKeyword,
		&MEI, &MII, &MinimumDistanceMultiplier, &Exponent, &BetaE, &BetaI,
		&MinObs, &IdPositionData, IdVariableName,
		&NumberOfVarVariables, &VarVariablesPosition,
		&WithVariablesPosition,
		&UserData.NumberOfWeightVariables, &WeightVariablePosition,
		&UserData.OutlierstatOption, &UserData.PositivityOption,
		&UserData.AcceptZero, &StartCentile, &Side, SideString, &Sigma, SigmaString, &ByGroupLoggingLevel);
	if (rc_get_parms != IORC_SUCCESS) {
		proc_exit_code = BRC_FAIL_READ_PARMS_LEGACY;
		goto error_cleanup;
	}
	/******************************************/
	/* Enable OutSummary out dataset if user  */
	/* provided a dataset name, else disable  */
	/******************************************/
	if (sp.dsw_outsummary.is_requested == IOB_FALSE)
		OutSumData.IsEnabled = 0;
	else
		OutSumData.IsEnabled = 1;

	PrintParms (&sp, InputMode, UserData.Method, MethodString, HistAuxKeyword,
		&OutSumData, MEI, MII, MinimumDistanceMultiplier, Exponent, BetaE, BetaI,
		MinObs, UserData.OutlierstatOption, UserData.PositivityOption,
		StartCentile, SideString, SigmaString, ByGroupLoggingLevel);

	rc = ValidateParms (&sp, InputMode, UserData.Method, MEI, MII, MinimumDistanceMultiplier, Exponent,
		BetaE, BetaI, MinObs, UserData.PositivityOption, HistAuxKeyword,
		UserData.AcceptZero, StartCentile, Side, SideString, SigmaString);
	if (rc == EIE_FAIL) {
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
	}

	/* Set up other variables depending on the input mode */
	switch (InputMode) {
	case E_OUTLIER_INPUTMODE_1DS_NOWITH:
		/* nothing to do */
		break;

	case E_OUTLIER_INPUTMODE_1DS_WITH:
		// same data set, replicate information
		IdPositionHist = IdPositionData;
		strcpy (HistAuxKeyword, DATA_GRM_NAME);
		break;

	case E_OUTLIER_INPUTMODE_2DS_NOWITH:
		WithVariablesPosition = STC_AllocateMemory (NumberOfVarVariables * sizeof *WithVariablesPosition);
		rc = ValidateForTwoDatasetsNoWith (&sp, IdVariableName, NumberOfVarVariables,
			VarVariablesPosition, HistAuxKeyword, &IdPositionHist,
			WithVariablesPosition);
		if (rc == EIE_FAIL) {
			STC_FreeMemory (WithVariablesPosition);
			proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
			goto error_cleanup;
		}
		break;

	case E_OUTLIER_INPUTMODE_2DS_WITH:
		rc = ValidateForTwoDatasetsAndWith (&sp, IdVariableName, HistAuxKeyword,
			&IdPositionHist, NumberOfVarVariables, WithVariablesPosition);
		if (rc == EIE_FAIL) {
			proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
			goto error_cleanup;
		}
		break;
	}

	/* Allocate the id and the array filled by SAS_XVGET () */
	UserData.IdLength = DSR_get_column_length(&sp.dsr_indata.dsr, IdPositionData);
	UserData.Id = STC_AllocateMemory (UserData.IdLength + 1);//why twice?
	IdValueData = STC_AllocateMemory (UserData.IdLength + 1);//why twice?
	VarVariablesValue = STC_AllocateMemory (NumberOfVarVariables * sizeof *VarVariablesValue);

	UserData.WeightIndex = NumberOfVarVariables;
	strcpy (UserData.MethodString, (UserData.Method == E_OUTLIER_METHOD_SIGMAGAP ? FIELDSTAT_METHOD_SG : FIELDSTAT_METHOD_HB));

	DefineScatterRead (&sp.dsr_indata, IdValueData,
		NumberOfVarVariables, VarVariablesValue,
		UserData.NumberOfWeightVariables, &WeightVariableValue);

	if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		int IdLengthHist;
		IdLengthHist = DSR_get_column_length(&sp.dsr_indata_hist.dsr, IdPositionHist);
		IdValueHist = STC_AllocateMemory (IdLengthHist + 1);
		WithVariablesValue = STC_AllocateMemory (NumberOfVarVariables * sizeof *WithVariablesValue);
		DefineScatterRead (&sp.dsr_indata_hist, IdValueHist,
			NumberOfVarVariables, WithVariablesValue,
			0, NULL);//No weight variable in hist data set, thus 0, NULL, NULL
	}

	PROC_RETURN_CODE rc_write_init = PRC_FAIL_UNHANDLED;

	if (UserData.Method == E_OUTLIER_METHOD_SIGMAGAP)
		rc_write_init = DefineGatherWrite (&sp, &UserData, &OutSumData, InputMode, Side, BetaE, BetaI, VarVariableName, WithVariableName);
	else
		rc_write_init = DefineGatherWrite (&sp, &UserData, &OutSumData, InputMode, Side, MEI, MII, VarVariableName, WithVariableName);

	if (rc_write_init != PRC_SUCCESS) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}

	EI_SetUserData ((void *) &UserData);

	UserData.DataTable = AllocateDataTable (&sp.dsr_indata);

	TIME_CPU_STOPDIFF(load_init);
	TIME_WALL_STOPDIFF(load_init);

	TIME_WALL_START(processing);
	TIME_CPU_START(processing);

	PROC_RETURN_CODE rc_processing = ReadByGroup (&sp, InputMode,
		IdValueData, NumberOfVarVariables, VarVariablesValue, &WeightVariableValue,
		IdValueHist, WithVariablesValue, &UserData, &Counter, HistAuxKeyword, ByGroupLoggingLevel);

	while (rc_processing == PRC_SUCCESS) {
		/* Every iteration process one BY group */
		EnoughObs = VerifyEnoughObsByGroup (&sp, InputMode, MinObs, HistAuxKeyword, &Counter, UserData.DataTable, ByGroupLoggingLevel);
		OutSumData.NObs			= (double) Counter.ObsInByGroup[EIE_PERIOD_CURRENT];
		OutSumData.NValid		= (double) (Counter.ValidObsInByGroup[EIE_PERIOD_CURRENT] - Counter.EmptyHistObsInByGroup);
		OutSumData.NRej_NoMatch	= (double) Counter.EmptyHistObsInByGroup;
		if (EnoughObs == EIE_TRUE) {
			/* find outlier for the BY group */
			if (ByGroupLoggingLevel != 1) PrintStatistics (InputMode, &sp, &Counter, HistAuxKeyword);

			rc = ProcessByGroup (&sp, InputMode, NumberOfVarVariables, VarVariableName,
				VarVariablesPosition, WeightVariablePosition, WithVariableName, WithVariablesPosition,
				&UserData, &OutSumData, MEI, MII, MinimumDistanceMultiplier, Exponent, BetaE, BetaI, MinObs,
				Side, Sigma, StartCentile, HistAuxKeyword, ByGroupLoggingLevel);
			if (ByGroupLoggingLevel != 1) EI_PrintMessages ();
			else EI_ClearMessageList();
			if (rc == EIE_FAIL) {
				proc_exit_code = BRC_FAIL_PROCESSING_GENERIC;
				goto error_cleanup;
			}
		}
		else {
			/* Arriving here means we skipped VAR-level processin for this BY group
			  => Set all VAR-level values to missing, Write observation to Out Summary */
			if (OutSumData.IsEnabled) {
				memset(VarVariableName, '\0', LEGACY_MAXNAME + 1);
				memset(WithVariableName, '\0', LEGACY_MAXNAME + 1);
				/*valid/reject*/
				OutSumData.NUsed			= EIM_MISSING_VALUE;
				OutSumData.NRej_Total		= EIM_MISSING_VALUE;
				OutSumData.NRej_VarMiss		= EIM_MISSING_VALUE;
				OutSumData.NRej_VarZero		= EIM_MISSING_VALUE;
				OutSumData.NRej_VarNegative	= EIM_MISSING_VALUE;
				OutSumData.NRej_AuxMiss		= EIM_MISSING_VALUE;
				OutSumData.NRej_AuxZero		= EIM_MISSING_VALUE;
				OutSumData.NRej_AuxNegative	= EIM_MISSING_VALUE;
				/*NFT(I|E)*/
				OutSumData.NFTI				= EIM_MISSING_VALUE;
				OutSumData.NFTE				= EIM_MISSING_VALUE;
				/*bounds*/
				OutSumData.IMP_BND_L		= EIM_MISSING_VALUE;
				OutSumData.EXCL_BND_L		= EIM_MISSING_VALUE;
				OutSumData.EXCL_BND_R		= EIM_MISSING_VALUE;
				OutSumData.IMP_BND_R		= EIM_MISSING_VALUE;
				/*HB specific*/
				OutSumData.Q1				= EIM_MISSING_VALUE;
				OutSumData.M				= EIM_MISSING_VALUE;
				OutSumData.Q3				= EIM_MISSING_VALUE;
				/*SG specific*/
				OutSumData.SIGMA			= EIM_MISSING_VALUE;
				OutSumData.IMP_SIGMAGAP		= EIM_MISSING_VALUE;
				OutSumData.EXCL_SIGMAGAP	= EIM_MISSING_VALUE;

				/*write observation*/
				WriteOutSum(&sp.dsw_outsummary, &OutSumData);
			}
		}
		EI_DataTableEmpty (UserData.DataTable);
		rc_processing = ReadByGroup (&sp, InputMode,
			IdValueData, NumberOfVarVariables, VarVariablesValue, &WeightVariableValue,
			IdValueHist, WithVariablesValue, &UserData, &Counter, HistAuxKeyword, ByGroupLoggingLevel);
	}
	
	proc_exit_code = convert_processing_rc(rc_processing);
	if (proc_exit_code != BRC_SUCCESS) {
		goto error_cleanup;
	}

	TIME_CPU_STOPDIFF(processing); // skipped when `goto error_cleanup;`?
	TIME_WALL_STOPDIFF(processing);

	IO_PRINT("\n");
	mem_usage("Before SP_wrap");
	if (PRC_SUCCESS != SP_wrap(&sp)) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}
	mem_usage("After SP_wrap");

error_cleanup:
	TIME_WALL_BEGIN(cleanup);
	TIME_CPU_BEGIN(cleanup);

	if (IdValueData != NULL)
		STC_FreeMemory (IdValueData);
	if (UserData.Id != NULL)
		STC_FreeMemory (UserData.Id);
	if (VarVariablesValue != NULL)
		STC_FreeMemory (VarVariablesValue);
	EI_DataTableFree (UserData.DataTable);

	/* free memory depending on the input mode */
	switch (InputMode) {
	case E_OUTLIER_INPUTMODE_1DS_NOWITH:
		/* nothing to do */
		break;

	case E_OUTLIER_INPUTMODE_2DS_NOWITH:
		// CAUTION: `WithVariablesPosition` shared with a varlist position list in some cases
		if (WithVariablesPosition != NULL && WithVariablesPosition != sp.dsr_indata_hist.VL_var.positions)
			STC_FreeMemory (WithVariablesPosition);
		break;

	case E_OUTLIER_INPUTMODE_1DS_WITH:
	case E_OUTLIER_INPUTMODE_2DS_WITH:
		break;
	}

	if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		if (IdValueHist != NULL)
			STC_FreeMemory (IdValueHist);
		if (WithVariablesValue != NULL)
			STC_FreeMemory (WithVariablesValue);

	}
	EI_FreeMessageList ();

	/* free Statcan Procedureand children */
	mem_usage("Before SPG_free");
	SPG_free((SP_generic*) &sp);
	mem_usage("After SPG_free");

	/* TIME MEASUREMENT */
	TIME_CPU_STOPDIFF(cleanup);
	TIME_WALL_STOPDIFF(cleanup);
	TIME_CPU_STOPDIFF(main);
	TIME_WALL_STOPDIFF(main);

	deinit_runtime_env();

	return proc_exit_code;
} /*fin du Main */


/*
Decide what size to give to DataTable
and allocate memory.
*/
static EIT_DATATABLE * AllocateDataTable (
	DSR_indata* dsr)
{
	long long NumberOfObservations;

	/*
	Decide what size to give to DataTable:
	if NumberOfObservations is
	below 10000, use NumberOfObservations
	above 10000, use 10000
	if BY group specified, divide by 5
	*/

	NumberOfObservations = dsr->dsr.num_records;

	if (NumberOfObservations == 0) {
		/* Nothing to do */
	}
	else if (NumberOfObservations == -1)
		/* sequential file. set NumberOfObservations to an arbitrary number */
		NumberOfObservations = 1000;
	else if (NumberOfObservations <= 10000)
		NumberOfObservations = NumberOfObservations;
	else
		NumberOfObservations = 10000;

	if (VL_is_specified(&dsr->dsr.VL_by_var))
		NumberOfObservations /= 5;

	return EI_DataTableAllocate ((int) NumberOfObservations);
}
/*
Count the number of observations read in the current period
without a match in the historical period.
*/
static void CountNoneHistObs (
	EIT_DATATABLE * DataTable,
	T_OUTLIER_COUNTER * Counter)
{
	EIT_DATA * Data;
	int i;

	Counter->EmptyHistObsInByGroup = 0;

	for (i = 0; i < DataTable->Size; i++) {
		for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
			if ((Data->DataRec[EIE_PERIOD_CURRENT] != NULL) &&
				(Data->DataRec[EIE_PERIOD_HISTORICAL] == NULL))
				/* current obs not "paired" */
				Counter->EmptyHistObsInByGroup++;
		}
	}
}
/*
define a character variable in an output dataset
*/
static PROC_RETURN_CODE DefineCharForOutput (
	DSW_generic* dsw,
	int dest_index,
	char * VarName,
	char * VarValue,
	int VarSize)
{
	// `VarSize` reserved for future use, the `++` only servers to suppress build warnings
	VarSize++;

	if (IORC_SUCCESS != DSW_define_column(dsw, dest_index, VarName, (void*)VarValue, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}
/*
define a numeric variable in an output dataset
*/
static PROC_RETURN_CODE DefineDoubleForOutput (
	DSW_generic* dsw,
	int dest_index,
	char * VarName,
	double * VarValue,
	int VarSize)
{
	// `VarSize` reserved for future use, the `++` only servers to suppress build warnings
	VarSize++;

	if (IORC_SUCCESS != DSW_define_column(dsw, dest_index, VarName, (void*)VarValue, IOVT_NUM)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}
/*
Define gather write.
That data set contains a number of variables

4 variables: Id variable name, Fieldid, Status (FTI, FTE) and OutStatus (ODIL, ODIR, ODEL, ODER).

BY Variables not included: in SAS if a BY was specified, we copied those variables too.

If OUTLIERSTAT is specified, more variables are written in output data set.
For 1 variable Hidiroglou-Berthelot method (6 + 1):
	- Method (HB or SG)
	- Current_Value
	- 4 bound values
	- Weight (if specified)
For 2 variables H-B methods (9 + 1):
	- Method (HB or SG)
	- Current_Value
	- 4 bound values
	- Hist_Aux
	- Hist_Aux_Value
	- Effect
	- Weight (if specified)
For 1 variable SIGMAGAP method (8 + 1):
	- Method (HB or SG)
	- Current_Value
	- 4 bound values
	- 2 gap values
	- Weight (if specified)
For 2 variables SIGMAGAP methods (10 + 1):
	- Method (HB or SG)
	- Current_Value
	- 2 gap values
	- 4 bound values
	- Hist_Aux
	- Hist_Aux_Value
	- Weight (if specified)
*/
static PROC_RETURN_CODE DefineGatherWrite (
	SP_outlier* sp,
	T_OUTLIERCALLBACKINFO * UserData,
	T_OUTSUMINFO * OutSumData,
	T_OUTLIER_INPUTMODE InputMode,
	EIT_OUTLIER_SIDE Side,
	double E, // MEI or BetaE
	double I, // MII or BetaI
	char * VarVariableName,
	char * WithVariableName)
{
	int NumberOfVariables;

	/* OutStatus dataset: initialize gather write definition */
	int key_var_position = sp->dsr_indata.dsr.VL_unit_id.positions[0];
	PROC_RETURN_CODE rc_setup_outstatus = banff_setup_dsw_outstatus(
		&sp->dsw_outstatus,
		sp->dsr_indata.dsr.col_names[key_var_position],
		(void*)UserData->Id,
		(void*)VarVariableName,
		(void*)UserData->Status,
		(void*)&UserData->FieldValue
	);

	if (rc_setup_outstatus != PRC_SUCCESS) {
		return rc_setup_outstatus;
	}

	EIT_BOOLEAN BndLE, BndLI, BndRE, BndRI;

	/* outstatus_detailed: initialize and set flags for the exclusion and imputation boundary variables*/
	BndLE = EIE_FALSE;
	BndLI = EIE_FALSE;
	BndRE = EIE_FALSE;
	BndRI = EIE_FALSE;
	if (I != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT))
		BndLI = EIE_TRUE;
	if (E != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT))
		BndLE = EIE_TRUE;
	if (E != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT))
		BndRE = EIE_TRUE;
	if (I != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT))
		BndRI = EIE_TRUE;

	/* OutStatus_detailed: define number of variables to write in output status */ 
	NumberOfVariables = OUTSTATUS_DETAILED_MIN_VARIABLES; // <unit-id>, FIELDID, OUTSTATUS
	if (UserData->OutlierstatOption) {
		NumberOfVariables += 2; // METHOD, CURRENT_VALUE
		if (UserData->NumberOfWeightVariables == 1)
			NumberOfVariables += 1; // WEIGHT
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH)
			NumberOfVariables += 2; // HIST_AUX, HIST_AUX_VALUE
		if (UserData->Method == E_OUTLIER_METHOD_SIGMAGAP) {
			NumberOfVariables += 1; // GAP
			if (I != EIM_MISSING_VALUE)
				NumberOfVariables += 1; // IMP_SIGMAGAP
			if (E != EIM_MISSING_VALUE)
				NumberOfVariables += 1; // EXCL_SIGMAGAP
		}
		else // UserData->Method != E_OUTLIER_METHOD_SIGMAGAP
			if ( InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH)
				NumberOfVariables += 1; // EFFECT
		// the 4 bounds
		if (BndLI == EIE_TRUE)
			NumberOfVariables += 1; // IMP_BND_L
		if (BndLE == EIE_TRUE)
			NumberOfVariables += 1; // EXCL_BND_L
		if (BndRE == EIE_TRUE)
			NumberOfVariables += 1; // EXCL_BND_R
		if (BndRI == EIE_TRUE)
			NumberOfVariables += 1; // IMP_BND_R
	}

	DSW_generic* dsw_outstatus_detailed = &sp->dsw_outstatus_detailed;
	if (IORC_SUCCESS != DSW_allocate(dsw_outstatus_detailed, NumberOfVariables)) {
		return PRC_FAIL_WRITE_DATA;
	}
	int var_added = 0;
	if (IORC_SUCCESS != DSW_define_column(dsw_outstatus_detailed, var_added++, sp->dsr_indata.dsr.VL_unit_id.names[0], (void*)UserData->Id, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}
    /* fieldid, status and "outlier status" */
	if (PRC_SUCCESS != DefineCharForOutput(dsw_outstatus_detailed, var_added++, DSC_FIELD_ID, VarVariableName, LEGACY_MAXNAME)) {
		return PRC_FAIL_WRITE_DATA;
	}
	if (PRC_SUCCESS != DefineCharForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_OUTSTATUS, UserData->OutlierStatus, EIM_STATUS_SIZE)) {
		return PRC_FAIL_WRITE_DATA;
	}
	/* extra info if Outlierstatus option set */
	if (UserData->OutlierstatOption) {
		if (PRC_SUCCESS != DefineCharForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_METHOD, UserData->MethodString, FIELDSTAT_METHOD_SIZE)) {
			return PRC_FAIL_WRITE_DATA;
		}
		if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_CURVALUE, &UserData->CurrentValue, sizeof (double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		if (UserData->NumberOfWeightVariables == 1){
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_WEIGHT, &UserData->Weight, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			if (PRC_SUCCESS != DefineCharForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_HISTAUX_FIELDID, WithVariableName, LEGACY_MAXNAME)) {
				return PRC_FAIL_WRITE_DATA;
			}
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_HISTAUX_VALUE, &UserData->PreviousValue, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (UserData->Method == E_OUTLIER_METHOD_SIGMAGAP) {
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_GAP, &UserData->Gap, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			if (I != EIM_MISSING_VALUE){
				if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_IMP_SIGMAGAP, &UserData->Imp_Sigmagap, sizeof (double))) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
			if (E != EIM_MISSING_VALUE){
				if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_EXCL_SIGMAGAP, &UserData->Excl_Sigmagap, sizeof (double))) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
		}
		else // UserData->Method != E_OUTLIER_METHOD_SIGMAGAP
			if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH){
				if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_EFFECT, &UserData->Effect, sizeof (double))) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
		if (BndLI == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_LEFT_ODI_BOUNDARY, &UserData->LeftODIBoundary, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndLE == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_LEFT_ODE_BOUNDARY, &UserData->LeftODEBoundary, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndRE == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_RIGHT_ODE_BOUNDARY, &UserData->RightODEBoundary, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndRI == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput (dsw_outstatus_detailed, var_added++, FIELDSTAT_RIGHT_ODI_BOUNDARY, &UserData->RightODIBoundary, sizeof (double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
	}

	if (IORC_SUCCESS != DSW_start_appending(dsw_outstatus_detailed)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* If OutSummary parameter provided */
	if (OutSumData->IsEnabled){
		/******************************************************
		OutSummary dataset: calculate number of variables */
		NumberOfVariables = OUTSUM_MIN_VARS;
		/* NRejected_NoMatch if 2 datasets */
		if (InputMode == E_OUTLIER_INPUTMODE_2DS_WITH
			|| InputMode == E_OUTLIER_INPUTMODE_2DS_NOWITH) {
			NumberOfVariables += 1; 
		}
		/* REJECT(NEGATIVE|ZERO): 
			if 2 variable, add ID and Missing count for Aux (always), 
				plus negative & zero (if applicable) */
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			NumberOfVariables += 4; //AuxVarID, NRejected_AuxMiss, NRej_AuxZero, NRej_AuxNeg
		}
		/* add method specific variables */
		if ((UserData->Method == E_OUTLIER_METHOD_CURRENT) || (UserData->Method == E_OUTLIER_METHOD_HISTORICAL))
			NumberOfVariables += 3;//3 quantiles
		else if (UserData->Method == E_OUTLIER_METHOD_SIGMAGAP){
			NumberOfVariables += 1;// sigma (deviation)
			if (I != EIM_MISSING_VALUE)
				NumberOfVariables += 1;// imp_sigmagap, excl_sigmagap
			if (E != EIM_MISSING_VALUE)
				NumberOfVariables += 1;// excl_sigmagap
		}

		/* add bounds */
		if (BndLI == EIE_TRUE)
			NumberOfVariables += 1; // IMP_BND_L
		if (BndLE == EIE_TRUE)
			NumberOfVariables += 1; // EXCL_BND_L
		if (BndRE == EIE_TRUE)
			NumberOfVariables += 1; // EXCL_BND_R
		if (BndRI == EIE_TRUE)
			NumberOfVariables += 1; // IMP_BND_R

		/******************************************************
		OutSummary dataset: initialize gather write definition */
		DSW_generic* dsw = &sp->dsw_outsummary;
		if (IORC_SUCCESS != DSW_allocate(dsw, NumberOfVariables)) {
			return PRC_FAIL_WRITE_DATA;
		}

		/******************************************************
		OutSummary dataset: define each output variable
			WARNING: The order (left->right) of variables in output dataset matches
					the order (first->last) variables added with Define<type>ForOutput(...) calls
		      Note: argument 3 of Define<Char|Double>ForOutput(...) is the fixed location in memory that SAS pulls from when creating an observation */
		var_added = 0;
        /* NObs */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NOBS, &OutSumData->NObs, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
        /* NValid */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NVALID, &OutSumData->NValid, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		if (InputMode == E_OUTLIER_INPUTMODE_2DS_WITH
			|| InputMode == E_OUTLIER_INPUTMODE_2DS_NOWITH) {
			/* NRej_NoMatch */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_NOMATCH, &OutSumData->NRej_NoMatch, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		/* fieldID */
		if (PRC_SUCCESS != DefineCharForOutput(dsw, var_added++, DSC_FIELD_ID, VarVariableName, LEGACY_MAXNAME)) {
			return PRC_FAIL_WRITE_DATA;
		}

		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			/* AuxVarID */
			if (PRC_SUCCESS != DefineCharForOutput(dsw, var_added++, OUTSUM_VN_AUXID, WithVariableName, LEGACY_MAXNAME)) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		/* NUsed */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NUSED, &OutSumData->NUsed, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		/* NRej_Total */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_TOTAL, &OutSumData->NRej_Total, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		/* NRej_Miss */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_MISS, &OutSumData->NRej_VarMiss, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}

		if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			/* NRej_VarNegative */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_NEG, &OutSumData->NRej_VarNegative, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			/* NRej_VarZero */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_ZERO, &OutSumData->NRej_VarZero, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}

		}else {
			/* NRej_AuxMiss */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_AUXMISS, &OutSumData->NRej_AuxMiss, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			/* NRej_VarZero */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_ZERO, &OutSumData->NRej_VarZero, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			/* NRej_AuxZero */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_AUXZERO, &OutSumData->NRej_AuxZero, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}

			/* NRej_VarNegative */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_NEG, &OutSumData->NRej_VarNegative, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			/* NRej_AuxNegative */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NREJ_AUXNEG, &OutSumData->NRej_AuxNegative, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}

		}
		/* NFTI */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NFTI, &OutSumData->NFTI, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		/* NFTE */
		if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_NFTE, &OutSumData->NFTE, sizeof(double))) {
			return PRC_FAIL_WRITE_DATA;
		}
		/* method specific fields */
		if ((UserData->Method == E_OUTLIER_METHOD_CURRENT) || (UserData->Method == E_OUTLIER_METHOD_HISTORICAL)){
			/* HB quantiles */
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_QUANTILE_FIRST, &OutSumData->Q1, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_QUANTILE_MEDIAN, &OutSumData->M, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_QUANTILE_THIRD, &OutSumData->Q3, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		else if (UserData->Method == E_OUTLIER_METHOD_SIGMAGAP) {
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, OUTSUM_VN_SIGMA, &OutSumData->SIGMA, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
			if (I != EIM_MISSING_VALUE){
				if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_IMP_SIGMAGAP, &OutSumData->IMP_SIGMAGAP, sizeof(double))) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
			if (E != EIM_MISSING_VALUE){
				if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_EXCL_SIGMAGAP, &OutSumData->EXCL_SIGMAGAP, sizeof(double))) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
		}
		/* applicable imputation and exclusion boundaries */
		if (BndLI == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_LEFT_ODI_BOUNDARY, &OutSumData->IMP_BND_L, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndLE == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_LEFT_ODE_BOUNDARY, &OutSumData->EXCL_BND_L, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndRE == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_RIGHT_ODE_BOUNDARY, &OutSumData->EXCL_BND_R, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		if (BndRI == EIE_TRUE){
			if (PRC_SUCCESS != DefineDoubleForOutput(dsw, var_added++, FIELDSTAT_RIGHT_ODI_BOUNDARY, &OutSumData->IMP_BND_R, sizeof(double))) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
		
		if (IORC_SUCCESS != DSW_start_appending(dsw)) {
			return PRC_FAIL_WRITE_DATA;
		}
	}

	return PRC_SUCCESS;
}
/*
Define scatter read data data set.
I used the same function to read hist data set even if there is no weight variable in hist data set.
I just make sure to set NumberOfWeightVariables equal to 0 in that case.
*/
static void DefineScatterRead (
	DSR_indata* dsr,
	char * Id,
	int NumberOfVariables,//current or historical
	double * VariablesValue,//current or historical
	int NumberOfWeightVariables, // 0 or 1
	double * WeightVariableValue)
{
	int i;

	/* KEY variable */
	dsr->dsr.VL_unit_id.ptrs[0] = (void*) Id;
	
	/* VAR (WITH) variable(s) */
	for (i = 0; i < NumberOfVariables; i++) {
		/* Define var. for scatter read */
		dsr->VL_var.ptrs[i] = (VariablesValue + i);
	}

	/* WEIGHT variable */
	if (NumberOfWeightVariables != 0) {
		dsr->VL_weight.ptrs[0] = WeightVariableValue;
	}

	/* NOTE: outlier doesn't use the DSR_generic member `VL_in_var`
		It is used in many other procedures, so this one might look different.  
		In this procedure we determine exactly which variables to read earlier in 
		the process.  */
}
/*
Check that no variable were repeated between lists
(with positions in DATA)
*/
static EIT_RETURNCODE DuplicateBetweenListsInData (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode)
{
	int * ByVariablesPosition;
	int * IdVariablesPosition;
	int * VarVariablesPosition;
	int * WeightVariablesPosition;
	int * WithVariablesPosition;

	int NumberOfByVariables;
	int NumberOfIdVariables;
	int NumberOfVarVariables;
	int NumberOfWeightVariables;
	int NumberOfWithVariables;

	EIT_RETURNCODE rc = EIE_SUCCEED;

	/* Reminder
		- IdPosition : one character variable
		- ByVariablesPosition : character and/or numeric variable(s)
		- VarVariablesPosition and WithVariablesPosition : numeric variable(s)
		- WeightVariablesPosition : one numeric variable
	*/
	ByVariablesPosition = (int*)sp->dsr_indata.dsr.VL_by_var.positions;
	NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;

	IdVariablesPosition = (int *) sp->dsr_indata.dsr.VL_unit_id.positions;
	NumberOfIdVariables = sp->dsr_indata.dsr.VL_unit_id.count;

	VarVariablesPosition = (int *) sp->dsr_indata.VL_var.positions;
	NumberOfVarVariables = sp->dsr_indata.VL_var.count;

	WeightVariablesPosition = (int *) sp->dsr_indata.VL_weight.positions;
	NumberOfWeightVariables = sp->dsr_indata.VL_weight.count;

	WithVariablesPosition = (int*)sp->dsr_indata_hist.VL_var.positions;
	NumberOfWithVariables = sp->dsr_indata_hist.VL_var.count;

	/* ID vs BY : ID (being character) can only intersect with BY */
	if (Intersect (IdVariablesPosition, NumberOfIdVariables, ByVariablesPosition, NumberOfByVariables)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, ID_GRM_NAME, BPN_BY);
		rc = EIE_FAIL;
	}
	/* VAR vs BY / VAR vs WEIGHT / BY vs WEIGHT */
	if (Intersect (VarVariablesPosition, NumberOfVarVariables, ByVariablesPosition, NumberOfByVariables)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, VAR_GRM_NAME, BPN_BY);
		rc = EIE_FAIL;
	}
	if (Intersect (VarVariablesPosition, NumberOfVarVariables, WeightVariablesPosition, NumberOfWeightVariables)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, VAR_GRM_NAME, WEIGHT_GRM_NAME);
		rc = EIE_FAIL;
	}
	if (Intersect (ByVariablesPosition, NumberOfByVariables, WeightVariablesPosition, NumberOfWeightVariables)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, BPN_BY, WEIGHT_GRM_NAME);
		rc = EIE_FAIL;
	}

	/* verification only if no AUX/HIST data set: WEIGHT vs WITH / BY vs WITH */
	if (InputMode == E_OUTLIER_INPUTMODE_1DS_WITH) {
		if (Intersect (WeightVariablesPosition, NumberOfWeightVariables, WithVariablesPosition, NumberOfWithVariables)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, WEIGHT_GRM_NAME, WITH_GRM_NAME);
			rc = EIE_FAIL;
		}
		if (Intersect (ByVariablesPosition, NumberOfByVariables, WithVariablesPosition, NumberOfWithVariables)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, BPN_BY, WITH_GRM_NAME);
			rc = EIE_FAIL;
		}
	}

	return rc;
}
/*
Check that no variable were repeated between lists BY and WITH
(positions in AUX/HIST)
Case: two data sets - BY and WITH
*/
static EIT_RETURNCODE DuplicateBetweenWithByInHist (
	SP_outlier* sp,
	int NumberOfVarVariables,
	int * WithVariablesPosition)
{
	int * ByInHist;
	int NumberOfByVariables;
	EIT_RETURNCODE rc = EIE_SUCCEED;

	NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;
	ByInHist = sp->dsr_indata_hist.dsr.VL_by_var.positions;

	if (Intersect (ByInHist, NumberOfByVariables, WithVariablesPosition, NumberOfVarVariables)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, WITH_GRM_NAME, BPN_BY);
		rc = EIE_FAIL;
	}

	return rc;
}
/*
Get method from parameter provided by user.
*/
static T_OUTLIER_METHOD GetOutlierMethod (
	SP_outlier* sp,
	char * MethodString)
{
	MethodString[0]= '\0';
	if (sp->method.meta.is_specified == IOSV_SPECIFIED) {
		memcpy (MethodString, (char *) sp->method.value, MAX_LENGTH_NAME8);
		SUtil_NullTerminate (MethodString, MAX_LENGTH_NAME8);

		/* validate string method */
		if ((strcmp (MethodString, CURRENT_ABBREV1) == 0) ||
				(strcmp (MethodString, CURRENT_ABBREV2) == 0) ||
				(strcmp (MethodString, CURRENT_WORD) == 0)) {
			strcpy (MethodString, CURRENT_WORD);
			return E_OUTLIER_METHOD_CURRENT;
		}
		else if ((strcmp (MethodString, HIST_ABBREV1) == 0) ||
				(strcmp (MethodString, HIST_ABBREV2) == 0) ||
				(strcmp (MethodString, HIST_WORD) == 0)) {
			strcpy (MethodString, HIST_WORD);
			return E_OUTLIER_METHOD_HISTORICAL;
		}
		else if ((strcmp (MethodString, RATIO_ABBREV1) == 0) ||
				(strcmp (MethodString, RATIO_WORD) == 0)) {
			strcpy (MethodString, RATIO_WORD);
			return E_OUTLIER_METHOD_HISTORICAL;
		}
		else if ((strcmp (MethodString, SIGMAGAP_ABBREV1) == 0) ||
				(strcmp (MethodString, SIGMAGAP_ABBREV2) == 0) ||
				(strcmp (MethodString, SIGMAGAP_WORD) == 0)) {
			strcpy (MethodString, SIGMAGAP_WORD);
			return E_OUTLIER_METHOD_SIGMAGAP;
		}
		else {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, METHOD_GRM_NAME, MethodString);
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidMethod, METHOD_GRM_NAME);
			return E_OUTLIER_METHOD__ERROR_;
		}
	}
	IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, METHOD_GRM_NAME);
	IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, METHOD_GRM_NAME);
	return E_OUTLIER_METHOD__ERROR_;

//	return E_OUTLIER_METHOD_SIGMAGAP; //for lint, will not be reached
}
/*
Reads procedure parameters.
Parameters not set are either set to their default value or mark as not set.
*/
static IO_RETURN_CODE GetParms (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE * InputMode,
	T_OUTLIER_METHOD * Method,
	char * MethodString,
	char * HistAuxKeyword,
	double * MEI,
	double * MII,
	double * MinimumDistanceMultiplier,
	double * Exponent,
	double * BetaE,
	double * BetaI,
	int * MinObs,
	int * IdPositionData,
	char * IdVariableName,
	int * NumberOfVarVariables,
	int ** VarVariablesPositionData,
	int ** WithVariablesPosition,
	int * NumberOfWeightVariables,
	int ** WeightVariablePosition,
	EIT_BOOLEAN * OutlierstatOption,
	EIPT_POSITIVITY_OPTION * PositivityOption,
	EIT_BOOLEAN * AcceptZero,
	double * StartCentile,
	EIT_OUTLIER_SIDE * Side,
	char * SideString,
	EIT_OUTLIER_SIGMA * Sigma,
	char * SigmaString,
	int * ByGroupLoggingLevel)
{
	//if method is wrong, quit.
	//default values and validation of values depend on the method.
	*Method = GetOutlierMethod (sp, MethodString);
	if (*Method == E_OUTLIER_METHOD__ERROR_) {
		return IORC_ERROR; // caller doesn't care what went wrong, so return `JRC_ERROR`
	}

	if (sp->dsr_indata_hist.dsr.is_specified == IOSV_NOT_SPECIFIED) // 1 data set
		if (sp->with_var.meta.is_specified == IOSV_NOT_SPECIFIED) // use same variable name
			*InputMode = E_OUTLIER_INPUTMODE_1DS_NOWITH;
		else // two different variable names
			*InputMode = E_OUTLIER_INPUTMODE_1DS_WITH;
	else // 2 data sets
		if (sp->with_var.meta.is_specified == IOSV_NOT_SPECIFIED) // use same variable name
			*InputMode = E_OUTLIER_INPUTMODE_2DS_NOWITH;
		else // two different variable names
			*InputMode = E_OUTLIER_INPUTMODE_2DS_WITH;

	if (*Method == E_OUTLIER_METHOD_CURRENT)
		//if Method is CURRENT, set InputMode to 1DS_NOWITH
		*InputMode = E_OUTLIER_INPUTMODE_1DS_NOWITH;

	/* Initialize "HistAuxKeyword" given to specify secondary input data set */
	/* `AUX` explained: 
	     Previously `AUX` and `HIST` were both aliases for the secondary input dataset.  Chosing
		 one alias of another only affected console messages, so `AUX` was eliminated.  
		`HIST` is now the only supported name for this dataset.   */
	int hardcoded_as_false = (1 == 0);
	if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) // in SAS: if `HIST` was specified
		strcpy (HistAuxKeyword, HIST_GRM_NAME);
	else if (hardcoded_as_false) // in SAS: if `AUX` was specified
		strcpy (HistAuxKeyword, AUX_GRM_NAME);
	else
		strcpy (HistAuxKeyword, "");

	if (sp->exponent.meta.is_specified == IOSV_NOT_SPECIFIED)
		*Exponent = EIM_OUTLIER_DEFAULT_EXPONENT;
	else
		*Exponent = sp->exponent.value;

	if (sp->mei.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MEI = EIM_MISSING_VALUE;
	else
		*MEI = sp->mei.value;

	if (sp->mii.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MII = EIM_MISSING_VALUE;
	else
		*MII = sp->mii.value;

	if (sp->mdm.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MinimumDistanceMultiplier = EIM_OUTLIER_DEFAULT_MDM;
	else
		*MinimumDistanceMultiplier = sp->mdm.value;

	if (sp->beta_e.meta.is_specified == IOSV_NOT_SPECIFIED)
		*BetaE = EIM_MISSING_VALUE;
	else
		*BetaE = sp->beta_e.value;

	if (sp->beta_i.meta.is_specified == IOSV_NOT_SPECIFIED)
		*BetaI = EIM_MISSING_VALUE;
	else
		*BetaI = sp->beta_i.value;

	if (sp->min_obs.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MinObs = EIM_OUTLIER_DEFAULT_MINOBS;
	else
		*MinObs = (int)sp->min_obs.value;

	if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
		*IdPositionData = sp->dsr_indata.dsr.VL_unit_id.positions[0];
		IOUtil_copy_varname(IdVariableName, sp->dsr_indata.dsr.col_names[*IdPositionData]);
	}

	if (*InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		/* NOTE: previously, in SAS based procedures, determining the "default VAR list"
		 	was performed here, however in the converted procedure this takes place
			during `SP_init`. 
			This empty IF body will remain to demonstrate how it used to work*/

		//SetDefaultVarList(*InData); // taken care of in SP_init, using new `DSR_default_var_list`
	}

	*NumberOfVarVariables = sp->dsr_indata.VL_var.count;
	*VarVariablesPositionData = sp->dsr_indata.VL_var.positions;

	*WithVariablesPosition = (int *)sp->dsr_indata_hist.VL_var.positions;

	*NumberOfWeightVariables = sp->dsr_indata.VL_weight.count;
	*WeightVariablePosition = (int *) sp->dsr_indata.VL_weight.positions;

	*OutlierstatOption = sp->outlier_stats.meta.is_specified == IOSV_SPECIFIED && sp->outlier_stats.value == IOB_TRUE ? EIE_TRUE : EIE_FALSE;

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

	if (sp->accept_zero.meta.is_specified == IOSV_NOT_SPECIFIED) {  // accept_zero not specified, use default
		if (*InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH)
			*AcceptZero = EIE_TRUE;
		else
			*AcceptZero = EIE_FALSE;
	}
	else if (sp->accept_zero.value == IOB_FALSE) { // only rejectzero specified
		*AcceptZero = EIE_FALSE;
	}
	else if (sp->accept_zero.value == IOB_TRUE) {  // only acceptzero specified
		*AcceptZero = EIE_TRUE;
	}

	*Sigma = EIE_OUTLIER_SIGMA_DEFAULT;
	if (sp->sigma.meta.is_specified == IOSV_NOT_SPECIFIED)
		strcpy (SigmaString, SIGMA_MAD);
	else {
		memcpy (SigmaString, (char *) sp->sigma.value, MAX_LENGTH_NAME8);
		SUtil_NullTerminate (SigmaString, MAX_LENGTH_NAME8);
		if (strcmp (SigmaString, SIGMA_STD) == 0)
			*Sigma = EIE_OUTLIER_SIGMA_STD;
	}

	*Side = EIE_OUTLIER_SIDE_DEFAULT;
	if (sp->side.meta.is_specified == IOSV_NOT_SPECIFIED)
		strcpy (SideString, SIDE_DEFAULT);
	else {
		memcpy (SideString, (char *) sp->side.value, MAX_LENGTH_NAME8);
		SUtil_NullTerminate (SideString, MAX_LENGTH_NAME8);
		if (strcmp (SideString, SIDE_RIGHT) == 0)
			*Side = EIE_OUTLIER_SIDE_RIGHT;
		else if (strcmp (SideString, SIDE_LEFT) == 0)
			*Side = EIE_OUTLIER_SIDE_LEFT;
	}

	if (sp->start_centile.meta.is_specified == IOSV_NOT_SPECIFIED) {
		if (*Side == EIE_OUTLIER_SIDE_BOTH)
			*StartCentile = STARTCENTILE_SIDE_BOTH_DEFAULT_VALUE;
		else
			*StartCentile = STARTCENTILE_SIDE_OTHER_DEFAULT_VALUE;
	}
	else
		*StartCentile = sp->start_centile.value;

	return IORC_SUCCESS;
}
/*
Looks for values that appear in both array.
Returns EIE_TRUE, if it finds one, EIE_FALSE otherwise.
*/
static EIT_BOOLEAN Intersect (
	int * List1, int nList1,
	int * List2, int nList2)
{
	int i;
	int j;

	for (i = 0; i < nList1; i++)
		for (j = 0; j < nList2; j++)
			if (List1[i] == List2[j])
				return EIE_TRUE;
	return EIE_FALSE;
}

/*
replace a Banff missing value with a SAS missing value
*/
static double MissingToSasMissing (
	double d)
{
	return d == EIM_MISSING_VALUE ? IOUtil_missing_value('.') : d;
}
/*
Print warning for "bad data" by VAR :
- for current method:
		number of missing values in DATA
		number of negative values in DATA
- for historical trend mathod:
		number of missing values in DATA or HIST/AUX
		number of negative or 0 values in DATA or HIST/AUX
*/
static void PrintCountBadDataByVar (
	T_OUTLIER_INPUTMODE InputMode,
	int NumberOfWeightVariables,
	char * HistAuxKeyword,
	int * MissingValue,
	int * NegativeValue,
	int * ZeroValue,
	int NegativeWeight,
	int ZeroWeight,
	char * VarVariableName,
	char * WithVariableName)
{
	char * htvn; //hist Variable Name
	int svn;     //Size Variable Name

	svn = (int) strlen(VarVariableName);
	if (MissingValue[EIE_PERIOD_CURRENT] > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetMissingValueForVar,
			MissingValue[EIE_PERIOD_CURRENT], DATA_GRM_NAME, svn, VarVariableName);
	if (NegativeValue[EIE_PERIOD_CURRENT] > 0)
		if (NumberOfWeightVariables != 0)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetNegativeValueForWeightedVar,
				NegativeValue[EIE_PERIOD_CURRENT], DATA_GRM_NAME, svn, VarVariableName);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetNegativeValueForVar,
				NegativeValue[EIE_PERIOD_CURRENT], DATA_GRM_NAME, svn, VarVariableName);
	if (ZeroValue[EIE_PERIOD_CURRENT] > 0)
		if (NumberOfWeightVariables != 0)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetZeroValueForWeightedVar,
				ZeroValue[EIE_PERIOD_CURRENT], DATA_GRM_NAME, svn, VarVariableName);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetZeroValueForVar,
				ZeroValue[EIE_PERIOD_CURRENT], DATA_GRM_NAME, svn, VarVariableName);

	if (InputMode == E_OUTLIER_INPUTMODE_1DS_WITH || InputMode == E_OUTLIER_INPUTMODE_2DS_WITH)
		htvn = WithVariableName;
	else
		htvn = VarVariableName;
	svn = (int) strlen(htvn);
	if (MissingValue[EIE_PERIOD_HISTORICAL] > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetMissingValueForVar,
			MissingValue[EIE_PERIOD_HISTORICAL], HistAuxKeyword, svn, htvn);
	if (NegativeValue[EIE_PERIOD_HISTORICAL] > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetNegativeValueForVar,
			NegativeValue[EIE_PERIOD_HISTORICAL], HistAuxKeyword, svn, htvn);
	if (ZeroValue[EIE_PERIOD_HISTORICAL] > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedInDataSetZeroValueForVar,
			ZeroValue[EIE_PERIOD_HISTORICAL], HistAuxKeyword, svn, htvn);

	if (NegativeWeight > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNegativeWeightWarning);
	if (ZeroWeight > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgZeroWeightWarning);

	IO_PRINT_LINE ("");
}
/*
Prints parameters.
Needed parameters:				HIST/
	Method				CURRENT RATIO	SIGMAGAP
ID						y		y		y
VAR						y		y		y
WEIGHT					y		y		y
BY						y		y		y
DATA					y		y		y
OUTSTATUS				y		y		y
ACCEPT|REJECTNEGATIVE	y		y		y
ACCEPT|REJECTZERO		y		y		y
MINOBS					y		y		y
OUTLIERSTAT				y		y		y
SIDE					y		y		y

WITH					n		y		y
HIST|AUX				n		y		y

MEI						y		y		n
MII						y		y		n
MDM						y		y		n
EXPONENT				n		y		n

BETAE					n		n		y
BETAI					n		n		y
SIGMA					n		n		y
STARTCENTILE			n		n		y
*/
static void PrintParms (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode,
	T_OUTLIER_METHOD Method,
	char * MethodString,
	char * HistAuxKeyword, // unused, but leaving for future use (see commented code)
	T_OUTSUMINFO * OutSumData, // unused, but leaving for future use (see commented code)
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	double Exponent,
	double BetaE,
	double BetaI,
	int MinObs,
	EIT_BOOLEAN OutlierstatOption,
	EIPT_POSITIVITY_OPTION PositivityOption,
	double StartCentile,
	char * SideString,
	char * SigmaString,
	int ByGroupLoggingLevel)
{
	char s[101];

	SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);
	SUtil_PrintInputDataSetInfo(&sp->dsr_indata_hist.dsr);

	SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
	SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus_detailed);
	SUtil_PrintOutputDataSetInfo(&sp->dsw_outsummary);

	IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, METHOD_GRM_NAME, MethodString);

	if (sp->side.meta.is_specified == IOSV_SPECIFIED)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, SIDE_GRM_NAME, SideString);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, SIDE_GRM_NAME, SIDE_BOTH);

	if (Method == E_OUTLIER_METHOD_SIGMAGAP)
		if (sp->sigma.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, SIGMA_GRM_NAME, SigmaString);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, SIGMA_GRM_NAME, SIGMA_MAD);
	else
		if (sp->sigma.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, SIGMA_GRM_NAME, SigmaString);

	sprintf (s, "%.*g", 6, MII);
	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->mii.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, MII_GRM_NAME, s);
	}
	else {
		if (sp->mii.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, MII_GRM_NAME);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, MII_GRM_NAME, s);
	}

	sprintf (s, "%.*g", 6, MEI);
	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->mei.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, MEI_GRM_NAME, s);
	}
	else {
		if (sp->mei.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, MEI_GRM_NAME);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, MEI_GRM_NAME, s);
	}

	sprintf (s, "%.*g", 6, MinimumDistanceMultiplier);
	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->mdm.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, MDM_GRM_NAME, s);
	}
	else
		if (sp->mdm.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, MDM_GRM_NAME, s);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, MDM_GRM_NAME, s);

	sprintf (s, "%.*g", 6, Exponent);
	if (Method != E_OUTLIER_METHOD_HISTORICAL) {
		if (sp->exponent.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, EXPONENT_GRM_NAME, s);
	}
	else
		if (sp->exponent.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, EXPONENT_GRM_NAME, s);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, EXPONENT_GRM_NAME, s);

	sprintf (s, "%.*g", 6, BetaI);
	if (Method != E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->beta_i.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, BETAI_GRM_NAME, s);
	}
	else {
		if (sp->beta_i.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BETAI_GRM_NAME);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BETAI_GRM_NAME, s);
	}

	sprintf (s, "%.*g", 6, BetaE);
	if (Method != E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->beta_e.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, BETAE_GRM_NAME, s);
	}
	else {
		if (sp->beta_e.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BETAE_GRM_NAME);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BETAE_GRM_NAME, s);
	}

	sprintf (s, "%.*g", 6, StartCentile);
	if (Method != E_OUTLIER_METHOD_SIGMAGAP) {
		if (sp->start_centile.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgNotUsed, STARTCENTILE_GRM_NAME, s);
	}
	else {
		if (sp->start_centile.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, STARTCENTILE_GRM_NAME, s);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, STARTCENTILE_GRM_NAME, s);
	}

	if (sp->min_obs.meta.is_specified == IOSV_NOT_SPECIFIED)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, MINOBS_GRM_NAME, MinObs);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, MINOBS_GRM_NAME, MinObs);

	if (OutlierstatOption)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE OUTLIERSTAT_GRM_NAME);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, OUTLIERSTAT_GRM_NAME);

	if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
	else if (PositivityOption == EIPE_REJECT_NEGATIVE)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
	else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

	if (sp->accept_zero.meta.is_specified == IOSV_NOT_SPECIFIED) {  // accept_zero not specified, use default
		if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH)
			IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptZeroDefault);
		else
			IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectZeroDefault);
	}
	else if (sp->accept_zero.value == IOB_TRUE) {  // only acceptzero specified
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptZero);
	}
	else if (sp->accept_zero.value == IOB_FALSE) { // only rejectzero specified
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectZero);
	}

	/* List of ID variables */
	SUtil_PrintStatementVars(&sp->unit_id.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_unit_id, ID_GRM_NAME);

	/* List of VAR_GRM_NB variables */
	SUtil_PrintStatementVars(&sp->var.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.VL_var, VAR_GRM_NAME);

	if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		/* WITH statement is dependant on the input data set */
		if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED)
			SUtil_PrintStatementVars(&sp->with_var.meta, &sp->dsr_indata_hist.dsr, &sp->dsr_indata_hist.VL_var, WITH_GRM_NAME);
		else
			SUtil_PrintStatementVars(&sp->with_var.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.VL_var, WITH_GRM_NAME);
	}

	/* List of WEIGHT variables */
	SUtil_PrintStatementVars(&sp->weight.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.VL_weight, WEIGHT_GRM_NAME);

	/* List of BY-variables - not mandatory */
	SUtil_PrintStatementVars(&sp->by.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_by_var, BPN_BY);
}
/*
Print general information on data sets for the by-group processed
or for the whole data sets.
*/
static void PrintStatistics (
	T_OUTLIER_INPUTMODE InputMode,
	SP_outlier* sp,
	T_OUTLIER_COUNTER * Counter,
	char * HistAuxKeyword)
{
	char TempKeyword[12];

	/* align the keyword "layout" (HIST: 4 char, AUX: 3 char */
	strcpy(TempKeyword, HistAuxKeyword);
	if (strcmp(TempKeyword, AUX_GRM_NAME) == 0)
		strcpy(TempKeyword, "AUX.");

	DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroup_SAS_FREE, 2);

#define  ONETAB  4

/* PRINT_COUNT_DATASET() is used to print the count of observations
	with the name of the data set.
	- parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT_DATASET(Msg, LengthPlaceHolders, DataSetName, Count) \
IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE Msg, DataSetName, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - strlen (DataSetName) - PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

/* PRINT_SUBTOTAL_DATASET() is used to print subtotal of observations
	with the name of the data set,
	adding some heading spaces before the message.
	- parm LengthPlaceHolders is related to Msg
	- parm LengthHeadingSpace is ONETAB */
#define PRINT_SUBTOTAL_DATASET(Msg, LengthPlaceHolders, LengthHeadingSpace, DataSetName, Count) \
IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE Msg, \
LengthHeadingSpace, \
"",\
DataSetName, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - \
strlen (DataSetName) - PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

/* PRINT_SUBTOTAL_NODATASET() is used to print subtotal of observations
	without the name of the data set,
	adding some heading spaces before the message.
	- parm LengthPlaceHolders is related to Msg
	- parm LengthHeadingSpace is ONETAB */
#define PRINT_SUBTOTAL_NODATASET(Msg, LengthPlaceHolders, LengthHeadingSpace, Count) \
IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE Msg, \
LengthHeadingSpace, \
"",\
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - PRINT_PRECISION),\
"............................................................", \
PRINT_PRECISION, Count);


	PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH, DATA_GRM_NAME,
		Counter->ObsInByGroup[EIE_PERIOD_CURRENT]);

	if (Counter->MissingId[EIE_PERIOD_CURRENT] > 0)
		PRINT_SUBTOTAL_NODATASET (MsgNumberDroppedMissingID,
			MsgNumberDroppedMissingID_LPH, ONETAB, Counter->MissingId[EIE_PERIOD_CURRENT]);

	if (Counter->MissingWeight > 0)
		PRINT_SUBTOTAL_NODATASET (MsgNumberDroppedMissingWeight,
			MsgNumberDroppedMissingWeight_LPH, ONETAB, Counter->MissingWeight);

	if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH, DATA_GRM_NAME,
			Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT]);
	}
	else {
		if (Counter->EmptyHistObsInByGroup > 0)
			PRINT_SUBTOTAL_DATASET (MsgNumberDroppedIDOnlyInDataSet,
				MsgNumberDroppedIDOnlyInDataSet_LPH, ONETAB,
				DATA_GRM_NAME, Counter->EmptyHistObsInByGroup);

		PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH,
			DATA_GRM_NAME, Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT] -
			Counter->EmptyHistObsInByGroup);
		/* Stephen Arsenault: The above seems hacky.  When using 2 vars, data is 
		read and validated giving NValid good obs.  Then hist is read, validated, and 
		linked with data, giving N2Valid good obs.  Finally, data is linked with hist 
		and revalidated to find data without a matchin gobservation in 
		hist (Coutner.EmptyHistObsInByGroup), but NValid is not updated.  
		Now here, we just subtract the value in the print statement.  
		Why?*/
	}

	IO_PRINT_LINE ("");

	if (InputMode == E_OUTLIER_INPUTMODE_2DS_NOWITH || InputMode == E_OUTLIER_INPUTMODE_2DS_WITH) {

		PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH,
			TempKeyword, Counter->ObsInByGroup[EIE_PERIOD_HISTORICAL]);

		if (Counter->MissingId[EIE_PERIOD_HISTORICAL] > 0)
			PRINT_SUBTOTAL_NODATASET (MsgNumberDroppedMissingID,
				MsgNumberDroppedMissingID_LPH, ONETAB,
				Counter->MissingId[EIE_PERIOD_HISTORICAL]);

		if (Counter->UnpairedHistObsInByGroup > 0)
			PRINT_SUBTOTAL_DATASET (MsgNumberDroppedIDOnlyInDataSet,
				MsgNumberDroppedIDOnlyInDataSet_LPH, ONETAB,
				TempKeyword, Counter->UnpairedHistObsInByGroup);

		PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH,
			TempKeyword, Counter->ValidObsInByGroup[EIE_PERIOD_HISTORICAL]);

		IO_PRINT_LINE ("");
	}
}
/*
Process all variable for this BY group
*/
static EIT_RETURNCODE ProcessByGroup (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode,
	int NumberOfVariables,
	char * VarVariableName,
	int * VarVariablesPosition,
	int * WeightVariablePosition,
	char * WithVariableName,
	int * WithVariablesPosition,
	T_OUTLIERCALLBACKINFO * UserData,
	T_OUTSUMINFO * OutSumData,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	double Exponent,
	double BetaE,
	double BetaI,
	int MinObs,
	EIT_OUTLIER_SIDE Side,
	EIT_OUTLIER_SIGMA Sigma,
	double StartCentile,
	char * HistAuxKeyword,
	int ByGroupLoggingLevel)
{
	EIT_RETURNCODE rc = EIE_FAIL;
	char WeightVariableName[LEGACY_MAXNAME+1];

	for (UserData->VarIndex = 0; UserData->VarIndex < NumberOfVariables; UserData->VarIndex++) {
		/*
		set VarVariableName for WriteStatus ()
		*/
		IOUtil_copy_varname(VarVariableName, sp->dsr_indata.dsr.col_names[VarVariablesPosition[UserData->VarIndex]]);

		/*
		if (With statement):
		set WithVariableName
		- for WriteStatus () when "Outlierstat"
		- also use for messages print in the "log"
		*/
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			IOUtil_copy_varname(WithVariableName, sp->dsr_indata_hist.dsr.col_names[WithVariablesPosition[UserData->VarIndex]]);
		}

		/* if weight variables is used: set WeightVariableName */
		if (UserData->NumberOfWeightVariables != 0) {
			IOUtil_copy_varname(WeightVariableName, sp->dsr_indata.dsr.col_names[WeightVariablePosition[0]]);
		}

		UserData->MissingValue[EIE_PERIOD_CURRENT] = 0;
		UserData->MissingValue[EIE_PERIOD_HISTORICAL] = 0;
		UserData->NegativeValue[EIE_PERIOD_CURRENT] = 0;
		UserData->NegativeValue[EIE_PERIOD_HISTORICAL] = 0;
		UserData->ZeroValue[EIE_PERIOD_CURRENT] = 0;
		UserData->ZeroValue[EIE_PERIOD_HISTORICAL] = 0;
		UserData->NegativeWeight = 0;
		UserData->ZeroWeight = 0;

		/* initialize index in the linked list of DATA's */
		UserData->DataTableIndex = 0;
		UserData->Data = UserData->DataTable->Data[UserData->DataTableIndex];
		UserData->NumberOfDataRead = 0;

		if (UserData->Method == E_OUTLIER_METHOD_CURRENT)
			rc = EI_CDOutlier (&sp->dsw_outstatus, &sp->dsw_outstatus_detailed, ReadCDData, WriteStatus, OutSumData, Side, MEI, MII,
				MinimumDistanceMultiplier, MinObs, (EIT_BOOLEAN) (UserData->NumberOfWeightVariables != 0));
		else if (UserData->Method == E_OUTLIER_METHOD_HISTORICAL)
			rc = EI_HTOutlier (&sp->dsw_outstatus, &sp->dsw_outstatus_detailed, ReadHTData, WriteStatus, OutSumData, Side, MEI, MII,
				MinimumDistanceMultiplier, MinObs, Exponent, (EIT_BOOLEAN) (UserData->NumberOfWeightVariables != 0));
		else { // UserData->Method == E_OUTLIER_METHOD_SIGMAGAP
			if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH)
				rc = EI_OutlierSigmagap (&sp->dsw_outstatus, &sp->dsw_outstatus_detailed, ReadCDData, WriteStatus, OutSumData, Side, Sigma, BetaE, BetaI, StartCentile,
				   MinObs, (EIT_BOOLEAN) (UserData->NumberOfWeightVariables != 0));
			else
				rc = EI_OutlierSigmagapRatio (&sp->dsw_outstatus, &sp->dsw_outstatus_detailed, ReadHTData, WriteStatus, OutSumData, Side, Sigma, BetaE, BetaI,
					StartCentile, MinObs, (EIT_BOOLEAN) (UserData->NumberOfWeightVariables != 0));
		}

		if (rc == EIE_SUCCEED) {
			/* This iteration of VAR-level processing complete, 
				set OutSummary rejection counters and write observation */
			OutSumData->NRej_VarMiss		= UserData->MissingValue[EIE_PERIOD_CURRENT];
			OutSumData->NRej_VarZero		= UserData->ZeroValue[EIE_PERIOD_CURRENT];
			OutSumData->NRej_VarNegative	= UserData->NegativeValue[EIE_PERIOD_CURRENT];
			OutSumData->NRej_AuxMiss		= UserData->MissingValue[EIE_PERIOD_HISTORICAL];
			OutSumData->NRej_AuxZero		= UserData->ZeroValue[EIE_PERIOD_HISTORICAL];
			OutSumData->NRej_AuxNegative	= UserData->NegativeValue[EIE_PERIOD_HISTORICAL];
			OutSumData->NRej_Total		= OutSumData->NRej_VarMiss  + OutSumData->NRej_VarZero  + OutSumData->NRej_VarNegative
										+ OutSumData->NRej_AuxMiss + OutSumData->NRej_AuxZero + OutSumData->NRej_AuxNegative;
			WriteOutSum(&sp->dsw_outsummary, OutSumData);
			
		}

		if (ByGroupLoggingLevel != 1) {
			/* print variable name if a message has to be printed */
			if (EI_GetNumberOfMessages() > 0) {
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgHeaderForVariable_SAS_FREE "\n",
					(int) strlen(VarVariableName), VarVariableName);
				EI_PrintMessages();
			}

			PrintCountBadDataByVar(InputMode,
				UserData->NumberOfWeightVariables, HistAuxKeyword,
				UserData->MissingValue, UserData->NegativeValue, UserData->ZeroValue,
				UserData->NegativeWeight, UserData->ZeroWeight,
				VarVariableName, WithVariableName);

			EI_PrintMessages();
		}
		else {
			EI_ClearMessageList();
		}
	}
	if (ByGroupLoggingLevel != 1) {
		DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
	}
	return rc;
}
/*********************************************************************
Reads DATA and HIST data sets.
Returns `RETURN_CODE_PROCESSING` enumeration
*********************************************************************/
static PROC_RETURN_CODE ReadByGroup (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode,
	char * IdValueData,
	int NumberOfVarVariables,
	double * VarVariablesValue,
	double * WeightVariableValue,
	char * IdValueHist,
	double * WithVariablesValue,
	T_OUTLIERCALLBACKINFO * UserData,
	T_OUTLIER_COUNTER * Counter,
	char * HistAuxKeyword,
	int ByGroupLoggingLevel)
{
	int rc;

	Counter->ObsInByGroup[EIE_PERIOD_CURRENT] = 0;
	Counter->ObsInByGroup[EIE_PERIOD_HISTORICAL] = 0;
	Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT] = 0;
	Counter->ValidObsInByGroup[EIE_PERIOD_HISTORICAL] = 0;
	Counter->UnpairedHistObsInByGroup = 0;
	Counter->MissingId[EIE_PERIOD_CURRENT] = 0;
	Counter->MissingId[EIE_PERIOD_HISTORICAL] = 0;
	Counter->MissingWeight = 0;
	Counter->EmptyHistObsInByGroup = 0;

	IO_DATASET_RC rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr);

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

	if (ByGroupLoggingLevel != 1) IO_PRINT_LINE ("");

	PROC_RETURN_CODE rc_ReadData = PRC_FAIL_UNHANDLED;
	
	rc_ReadData = ReadData(sp, IdValueData, NumberOfVarVariables, VarVariablesValue,
		WeightVariableValue, UserData, Counter);

	if (rc_ReadData != PRC_SUCCESS_NO_MORE_DATA) {
		return rc_ReadData;
	}

	IO_DATASET_RC rc_sync_by = DSRC_SUCCESS;
	PROC_RETURN_CODE rc_ReadHist = PRC_FAIL_UNHANDLED;
	switch (InputMode) {
	case E_OUTLIER_INPUTMODE_1DS_WITH:
		/* InData and InHist are the same pointer when only DATA data set specified */
		if (DSRC_SUCCESS != DSR_cursor_rewind(&sp->dsr_indata_hist.dsr, IOCA_REWIND_BY_GROUP)) {
			return PRC_FAIL_SYNC_GENERIC;
		}
		rc_ReadHist = ReadHist (sp, IdValueHist, NumberOfVarVariables, WithVariablesValue,
			UserData, Counter, HistAuxKeyword);
		break;

	case E_OUTLIER_INPUTMODE_2DS_NOWITH:
	case E_OUTLIER_INPUTMODE_2DS_WITH:
		rc_sync_by = DSR_cursor_sync_by(&sp->dsr_indata.dsr, &sp->dsr_indata_hist.dsr);
		if (rc_sync_by == DSRC_SUCCESS) {
			rc_ReadHist = ReadHist(sp, IdValueHist, NumberOfVarVariables, WithVariablesValue,
				UserData, Counter, HistAuxKeyword);
		}
		else if (rc_sync_by == DSRC_DIFFERENT_GROUPS) {
			return PRC_SUCCESS;
		}
		else {
			// arriving here likely indicates a bug in the code
			return PRC_FAIL_SYNC_GENERIC;
		}
		break;
	case E_OUTLIER_INPUTMODE_1DS_NOWITH:
		// no historic dataset to read
		rc_ReadHist = PRC_SUCCESS_NO_MORE_DATA;
	}

	if (rc_ReadHist != PRC_SUCCESS_NO_MORE_DATA) {
		return rc_ReadHist;
	}
	
	return PRC_SUCCESS;
}
/*
Reads DATA data set information. It reads it from the DataTable.
It is called by the outlier API as a callback function.
if a weight is specified, its value is position at the end of the DataRec
*/
static EIT_READCALLBACK_RETURNCODE ReadCDData (
	void * UserData,
	char ** Id,
	double * Value,
	double * Weight)
{
	T_OUTLIERCALLBACKINFO * LocalUserData = (T_OUTLIERCALLBACKINFO *) UserData;
	EIT_READCALLBACK_RETURNCODE rc;
	double WeightedValue;

	rc = EIE_READCALLBACK_NOTFOUND;

	while (LocalUserData->NumberOfDataRead < LocalUserData->DataTable->NumberOfData && rc == EIE_READCALLBACK_NOTFOUND) {

		//Data = LocalUserData->Data;
		if (LocalUserData->Data == NULL) {
			/* no data in this datatable slot */
			LocalUserData->DataTableIndex++;
			LocalUserData->Data = LocalUserData->DataTable->Data[LocalUserData->DataTableIndex];
			continue; // goto while
		}

		*Value = LocalUserData->Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[LocalUserData->VarIndex];
		if (LocalUserData->NumberOfWeightVariables != 0) {
			*Weight = LocalUserData->Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[LocalUserData->WeightIndex];
			WeightedValue = *Value * *Weight;
		}
		else {
			*Weight = 1.0;
			WeightedValue = *Value;
		}

		if (IOUtil_is_missing (*Value)) {
			/* Skip this obs. if variable is missing */
			LocalUserData->MissingValue[EIE_PERIOD_CURRENT]++;
		}
		else if (LocalUserData->PositivityOption != EIPE_ACCEPT_NEGATIVE &&
				EIM_DBL_LT (WeightedValue, 0.0)) {
			/* Skip this obs. if weighted variable is negative and PositivityOption is REJECTNEGATIVE */
			LocalUserData->NegativeValue[EIE_PERIOD_CURRENT]++;
		}
		else if (!LocalUserData->AcceptZero &&
				EIM_DBL_EQ (WeightedValue, 0.0)) {
			/* Skip this obs. if weighted variable is zero and RejectZero is TRUE */
			LocalUserData->ZeroValue[EIE_PERIOD_CURRENT]++;
		}
		else {
			/* data is valid: use it */
			*Id = LocalUserData->Data->Key;
			if (EIM_DBL_EQ (*Weight, 0.0))
				LocalUserData->ZeroWeight++;
			else if (EIM_DBL_LT (*Weight, 0.0))
				LocalUserData->NegativeWeight++;
			rc = EIE_READCALLBACK_FOUND;
		}

		if (LocalUserData->Data->Next != NULL)
			LocalUserData->Data = LocalUserData->Data->Next;
		else {
			LocalUserData->DataTableIndex++;
			LocalUserData->Data = LocalUserData->DataTable->Data[LocalUserData->DataTableIndex];
		}
		LocalUserData->NumberOfDataRead++;
	}
	return rc;
}
/*
Reads DATA data set. It puts the data in a DataTable.
It is called by the procedure.
*/
static PROC_RETURN_CODE ReadData (
	SP_outlier* sp,
	char * Id,
	int NumberOfVarVariables,
	double * VarVariablesValue,
	double * WeightVariableValue,
	T_OUTLIERCALLBACKINFO * UserData,
	T_OUTLIER_COUNTER * Counter)
{
	EIT_DATA * Data;
	int i;
	int rcLookup;

	IO_RETURN_CODE rc_next_rec = IORC_SUCCESS;

	while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {

		Counter->ObsInByGroup[EIE_PERIOD_CURRENT]++;

		IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*) &sp->dsr_indata);
		if (rc_get_rec != IORC_SUCCESS) {
			return PRC_FAIL_GET_REC;
		}
		SUtil_NullTerminate (Id, UserData->IdLength);

		if (Id[0] == '\0') {
			/* Skip this obs. if id is missing */
			Counter->MissingId[EIE_PERIOD_CURRENT]++;
		}
		else if (UserData->NumberOfWeightVariables != 0 &&
				IOUtil_is_missing (*WeightVariableValue)) {
			/* Skip this obs. if weight is missing */
			Counter->MissingWeight++;
		}
		else {
			rcLookup = EI_DataTableLookup (UserData->DataTable, Id,
				EIE_DATATABLELOOKUPTYPE_CREATE, &Data);

			switch (rcLookup) {
			case EIE_DATATABLELOOKUP_NOTFOUND: /* not previously there */

				Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT]++;

				Data->DataRec[EIE_PERIOD_CURRENT] = STC_AllocateMemory (sizeof *Data->DataRec[EIE_PERIOD_CURRENT]);

				// when a weight variable is not specified, we allocate space for NumberOfVarVariables
				// when a weight variable is specified, we allocate space for (NumberOfVarVariables + 1)
				// the first NumberOfVarVariables variables are for the VAR variables, the last variable is for the WEIGHT variable.
				// NumberOfWeightVariables is either 0 or 1
				// the Weight is positioned at the end of the DataRec
				//don't send variable names, they are not used anyway
				EI_DataRecAllocate (NULL, NumberOfVarVariables + UserData->NumberOfWeightVariables, Data->DataRec[EIE_PERIOD_CURRENT]);

				for (i = 0; i < NumberOfVarVariables; i++)
					Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] = VarVariablesValue[i];
				if (UserData->NumberOfWeightVariables != 0)
					Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] = *WeightVariableValue;
				break;

			case EIE_DATATABLELOOKUP_FOUND:
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgIdValueDuplicatedInDataSet "\n", Id, DATA_GRM_NAME);
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
/*
Reads HIST data set. It puts the data in a DataTable.
It is called by the procedure.
*/
static PROC_RETURN_CODE ReadHist (
	SP_outlier* sp,
	char * Id,
	int NumberOfVarVariables,
	double * WithVariablesValue,
	T_OUTLIERCALLBACKINFO * UserData,
	T_OUTLIER_COUNTER * Counter,
	char * HistAuxKeyword)
{
	EIT_DATA * Data;
	int i;
	int rcLookup;

	IO_RETURN_CODE rc_next_rec = IORC_SUCCESS;

	while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata_hist.dsr)) == DSRC_NEXT_REC_SUCCESS) {

		Counter->ObsInByGroup[EIE_PERIOD_HISTORICAL]++;

		IO_RETURN_CODE rc_get_rec = DSR_rec_get((DSR_generic*) &sp->dsr_indata_hist);
		if (rc_get_rec != IORC_SUCCESS) {
			return PRC_FAIL_GET_REC;
		}
		SUtil_NullTerminate (Id, UserData->IdLength);

		if (Id[0] == '\0') {
			Counter->MissingId[EIE_PERIOD_HISTORICAL]++;
		}
		else {
			rcLookup = EI_DataTableLookup (UserData->DataTable, Id,
				EIE_DATATABLELOOKUPTYPE_FIND, &Data);

			switch (rcLookup) {
			case EIE_DATATABLELOOKUP_NOTFOUND:
				/*
				if there is no current data,
				we do not care to keep previous data.
				*/
				Counter->UnpairedHistObsInByGroup++;
				break;

			case EIE_DATATABLELOOKUP_FOUND:
				if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL) {
					IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgIdValueDuplicatedInDataSet "\n",
						Id, HistAuxKeyword);
					return PRC_FAIL_DUPLICATE_REC;
				}

				Counter->ValidObsInByGroup[EIE_PERIOD_HISTORICAL]++;

				Data->DataRec[EIE_PERIOD_HISTORICAL] = STC_AllocateMemory (
					sizeof *Data->DataRec[EIE_PERIOD_HISTORICAL]);

				EI_DataRecAllocate (NULL, NumberOfVarVariables, Data->DataRec[EIE_PERIOD_HISTORICAL]);

				for (i = 0; i < NumberOfVarVariables; i++)
					Data->DataRec[EIE_PERIOD_HISTORICAL]->FieldValue[i] = WithVariablesValue[i];
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
/*
Reads HIST data set information. It reads it from the DataTable.
It is called by the outlier API as a callback function.
*/
static EIT_READCALLBACK_RETURNCODE ReadHTData (
	void * UserData,
	char ** Id,
	double * CurrentValue,
	double * PreviousValue,
	double * Weight)
{
	T_OUTLIERCALLBACKINFO * LocalUserData = (T_OUTLIERCALLBACKINFO *) UserData;
	EIT_READCALLBACK_RETURNCODE rc;
	double WeightedCurrentValue;

	rc = EIE_READCALLBACK_NOTFOUND;

	while (LocalUserData->NumberOfDataRead < LocalUserData->DataTable->NumberOfData && rc == EIE_READCALLBACK_NOTFOUND) {

		if (LocalUserData->Data == NULL) {
			/* no data in this datatable slot */
			LocalUserData->DataTableIndex++;
			LocalUserData->Data = LocalUserData->DataTable->Data[LocalUserData->DataTableIndex];
			continue; // goto while
		}
		else if ((LocalUserData->Data->DataRec[EIE_PERIOD_CURRENT] != NULL) &&
				(LocalUserData->Data->DataRec[EIE_PERIOD_HISTORICAL] == NULL)) {
			/* Skip this obs. if no historical data */
			if (LocalUserData->Data->Next != NULL)
				LocalUserData->Data = LocalUserData->Data->Next;
			else {
				LocalUserData->DataTableIndex++;
				LocalUserData->Data = LocalUserData->DataTable->Data[LocalUserData->DataTableIndex];
			}
			LocalUserData->NumberOfDataRead++;
			continue; // goto while
		}

		*CurrentValue = LocalUserData->Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[LocalUserData->VarIndex];
		*PreviousValue = LocalUserData->Data->DataRec[EIE_PERIOD_HISTORICAL]->FieldValue[LocalUserData->VarIndex];
		if (LocalUserData->NumberOfWeightVariables != 0) {
			*Weight = LocalUserData->Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[LocalUserData->WeightIndex];
			WeightedCurrentValue = *CurrentValue * *Weight;
		}
		else {
			*Weight = 1.0;
			WeightedCurrentValue = *CurrentValue;
		}

		if (IOUtil_is_missing (*CurrentValue) ||
				IOUtil_is_missing (*PreviousValue) ||
				EIM_DBL_LE (WeightedCurrentValue, 0.0) ||
				EIM_DBL_LE (*PreviousValue, 0.0)) {
			/* Skip this obs. if current  is missing or variable (weighted current or hist) is negative or 0. */
			if (IOUtil_is_missing (*CurrentValue))
				LocalUserData->MissingValue[EIE_PERIOD_CURRENT]++;
			else if (IOUtil_is_missing (*PreviousValue))
				LocalUserData->MissingValue[EIE_PERIOD_HISTORICAL]++;
			else if (EIM_DBL_EQ (WeightedCurrentValue, 0.0))
				LocalUserData->ZeroValue[EIE_PERIOD_CURRENT]++;
			else if (EIM_DBL_EQ (*PreviousValue, 0.0))
				LocalUserData->ZeroValue[EIE_PERIOD_HISTORICAL]++;
			else if (EIM_DBL_LT (WeightedCurrentValue, 0.0))
				LocalUserData->NegativeValue[EIE_PERIOD_CURRENT]++;
			else /*if (EIM_DBL_LT (*PreviousValue, 0.0))*/
				LocalUserData->NegativeValue[EIE_PERIOD_HISTORICAL]++;
		}
		else {
			/* data is valid: use it */
			*Id = LocalUserData->Data->Key;
			if (EIM_DBL_EQ (*Weight, 0.0))
				LocalUserData->ZeroWeight++;
			else if (EIM_DBL_LT (*Weight, 0.0))
				LocalUserData->NegativeWeight++;
			rc = EIE_READCALLBACK_FOUND;
		}

		if (LocalUserData->Data->Next != NULL)
			LocalUserData->Data = LocalUserData->Data->Next;
		else {
			LocalUserData->DataTableIndex++;
			LocalUserData->Data = LocalUserData->DataTable->Data[LocalUserData->DataTableIndex];
		}
		LocalUserData->NumberOfDataRead++;
	}
	return rc;
}
/*
Create default variable list, if none is given.
If there is no var statement specified, create a default var list
by selecting all numeric variable NOT in the BY list.
Should be called only for current data and SIGMAGAP methods.
This function has NO EFFECT if a VAR statement has been specified.
UPDATE 2023, this was rewritten: see `IOUtil.c::DSR_default_var_list()`
*/
//static void SetDefaultVarList (
//	SP_outlier* sp)
//{
//	printf("\n\nERROR: SetDefaultVarList NOT IMPLEMENTED\n\n");
//}
/*
DATA and HIST data sets specified.
VAR and WITH statements given.
- validate ID in AUX/HIST,
- for WITH variables, validation done by the grammar,
- check duplicates between WITH and BY in AUX/HIST
Keep positions in AUX/HIST:
	- ID in "IdPositionHist"
	- WITH in "WithVariablesPosition"
*/
static EIT_RETURNCODE ValidateForTwoDatasetsAndWith (
	SP_outlier* sp,
	char * IdVariableName,
	char * HistAuxKeyword,
	int * IdPositionHist,
	int NumberOfVarVariables,
	int * WithVariablesPosition)
{
	EIT_RETURNCODE rc = EIE_SUCCEED;

	if (ValidateIDInHist (sp, IdVariableName, HistAuxKeyword, IdPositionHist) == EIE_FAIL)
		rc = EIE_FAIL;

	if ((VL_is_specified(&sp->dsr_indata.dsr.VL_by_var)) &&
		(DuplicateBetweenWithByInHist (sp, NumberOfVarVariables, WithVariablesPosition) == EIE_FAIL))
		rc = EIE_FAIL;

	return rc;
}
/*
DATA and HIST data sets specified.
Only VAR statement given.
- validate ID and VAR in AUX/HIST
Keep positions in AUX/HIST:
	- ID in "IdPositionHist"
	- VAR in "WithVariablesPosition"
*/
static EIT_RETURNCODE ValidateForTwoDatasetsNoWith (
	SP_outlier* sp,
	char * IdVariableName,
	int NumberOfVarVariables,
	int * VarVariablesPosition,
	char * HistAuxKeyword,
	int * IdPositionHist,
	int * WithVariablesPosition)
{
	EIT_RETURNCODE rc = EIE_SUCCEED;

	if ((ValidateIDInHist (sp, IdVariableName, HistAuxKeyword, IdPositionHist) == EIE_FAIL) ||
		(ValidateVARInHist (sp, NumberOfVarVariables, VarVariablesPosition, HistAuxKeyword, WithVariablesPosition) == EIE_FAIL))
		rc = EIE_FAIL;

	return rc;
}

/*
Check that ID is in AUX/HIST data set (character type).
Keep it's position.
*/
static EIT_RETURNCODE ValidateIDInHist (
	SP_outlier* sp,
	char * IdVariableName,
	char * HistAuxKeyword,
	int * IdPositionHist)
{
	EIT_RETURNCODE rc = EIE_SUCCEED;
	int VariableType;

	IO_RETURN_CODE rc_io =  DSR_get_pos_from_names(&sp->dsr_indata_hist.dsr, 1, &IdVariableName, IdPositionHist);
	if (rc_io == IORC_VARLIST_NOT_FOUND) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameNotInDataSet, ID_GRM_NAME, HistAuxKeyword);
		rc = EIE_FAIL;
	}
	else {
		VariableType = DSR_get_col_type(&sp->dsr_indata_hist.dsr, *IdPositionHist);
		if (VariableType != IOVT_CHAR) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet, ID_GRM_NAME, HistAuxKeyword);
			rc = EIE_FAIL;
		}
	}

	return rc;
}
/*
validate parameters
*/
static EIT_RETURNCODE ValidateParms (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode,
	T_OUTLIER_METHOD Method,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	double Exponent,
	double BetaE,
	double BetaI,
	int MinObs,
	EIPT_POSITIVITY_OPTION PositivityOption,
	char * HistAuxKeyword,
	EIT_BOOLEAN AcceptZero,
	double StartCentile,
	EIT_OUTLIER_SIDE Side,
	char * SideString,
	char * SigmaString)
{
	int bylistrc;
	EIT_RETURNCODE rc = EIE_SUCCEED;

	if (sp->dsr_indata.dsr.VL_unit_id.count > 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgMultipleStatements, ID_GRM_NAME);
		rc = EIE_FAIL;
	}

	if (sp->dsr_indata.VL_weight.count > 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgMultipleStatements, WEIGHT_GRM_NAME);
		rc = EIE_FAIL;
	}

	if (Method == E_OUTLIER_METHOD_HISTORICAL) {
		if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgHTRatioMethodBadSpecifications);
			rc = EIE_FAIL;
		}
		if (!(0 <= Exponent && Exponent <= 1)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidExponent, EXPONENT_GRM_NAME);
			rc = EIE_FAIL;
		}
	}
	if (Method != E_OUTLIER_METHOD_CURRENT) {
		/* Number of variables in VAR and WITH are equal */
		if ((sp->dsr_indata_hist.VL_var.count != 0) && (sp->dsr_indata.VL_var.count != sp->dsr_indata_hist.VL_var.count)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgHTRatioMethodDifferentSizeForVarAndWith);
			rc = EIE_FAIL;
		}
	}
	if (InputMode == E_OUTLIER_INPUTMODE_2DS_NOWITH || InputMode == E_OUTLIER_INPUTMODE_2DS_WITH) {
		/* By groups variables */
		if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED) {
			bylistrc = EIE_SUCCEED;
			bylistrc = SUtil_AreAllByVariablesInDataSet(&sp->dsr_indata.dsr, &sp->dsr_indata_hist.dsr);
			if (bylistrc != EIE_SUCCEED) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgByVarsNotFoundInDataSet, HistAuxKeyword);
				rc = EIE_FAIL;
			}
		}
	}

	if (strcmp (SideString, SIDE_LEFT) && strcmp (SideString, SIDE_BOTH) && strcmp (SideString, SIDE_RIGHT)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidSide, SIDE_GRM_NAME);
		rc = EIE_FAIL;
	}

	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (BetaI == EIM_MISSING_VALUE && BetaE == EIM_MISSING_VALUE) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAtLeastBETAIorBETAEMustBeSpecified);
			rc = EIE_FAIL;
		}
		else {
			if (BetaI != EIM_MISSING_VALUE && BetaE != EIM_MISSING_VALUE && BetaI <= BetaE) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgBETAIMustBeGreaterThanBETAE);
				rc = EIE_FAIL;
			}
			if (BetaE != EIM_MISSING_VALUE && BetaE <= EIM_OUTLIER_MINIMUM_MULTIPLIER) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgParmGreater0, BETAE_GRM_NAME);
				rc = EIE_FAIL;
			}
			if (BetaI != EIM_MISSING_VALUE && BetaI <= EIM_OUTLIER_MINIMUM_MULTIPLIER) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgParmGreater0, BETAI_GRM_NAME);
				rc = EIE_FAIL;
			}
		}
	}
	else {
		if (MII == EIM_MISSING_VALUE && MEI == EIM_MISSING_VALUE) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAtLeastMIIorMEIMustBeSpecified);
			rc = EIE_FAIL;
		}
		else {
			if (MII != EIM_MISSING_VALUE && MEI != EIM_MISSING_VALUE && MII <= MEI) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgMIIMustBeGreaterThanMEI);
				rc = EIE_FAIL;
			}
			if (MEI != EIM_MISSING_VALUE && MEI <= EIM_OUTLIER_MINIMUM_MULTIPLIER) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgParmGreater0, MEI_GRM_NAME);
				rc = EIE_FAIL;
			}
			if (MII != EIM_MISSING_VALUE && MII <= EIM_OUTLIER_MINIMUM_MULTIPLIER) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgParmGreater0, MII_GRM_NAME);
				rc = EIE_FAIL;
			}
		}
	}
	if (Method != E_OUTLIER_METHOD_SIGMAGAP) {
		if (MinimumDistanceMultiplier < 0.0) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgParmGreaterEqual0, MDM_GRM_NAME);
			rc = EIE_FAIL;
		}
	}

	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (MinObs < EIM_OUTLIER_MINIMUM_MINOBS_SIGMAGAP) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgMINOBSInvalidValue, EIM_OUTLIER_MINIMUM_MINOBS_SIGMAGAP);
			rc = EIE_FAIL;
		}
	}
	else { /* HB */
		if (MinObs < EIM_OUTLIER_MINIMUM_MINOBS_HB) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgMINOBSInvalidValue, EIM_OUTLIER_MINIMUM_MINOBS_HB);
			rc = EIE_FAIL;
		}
	}

	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (Side == EIE_OUTLIER_SIDE_BOTH) {
			if (StartCentile < STARTCENTILE_SIDE_BOTH_MINIMUM_VALUE || StartCentile >= STARTCENTILE_SIDE_BOTH_MAXIMUM_VALUE) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidStartCentile, STARTCENTILE_GRM_NAME, STARTCENTILE_SIDE_BOTH_MINIMUM_VALUE, STARTCENTILE_SIDE_BOTH_MAXIMUM_VALUE);
				rc = EIE_FAIL;
			}
		}
		else {
			if (StartCentile < STARTCENTILE_SIDE_OTHER_MINIMUM_VALUE || StartCentile >= STARTCENTILE_SIDE_OTHER_MAXIMUM_VALUE) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidStartCentile, STARTCENTILE_GRM_NAME, STARTCENTILE_SIDE_OTHER_MINIMUM_VALUE, STARTCENTILE_SIDE_OTHER_MAXIMUM_VALUE);
				rc = EIE_FAIL;
			}
		}
	}

	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (strcmp (SigmaString, SIGMA_MAD) && strcmp (SigmaString, SIGMA_STD)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid " " MsgInvalidSigma, SIGMA_GRM_NAME);
			rc = EIE_FAIL;
		}
	}

	if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementMandatory, ID_GRM_NAME);
		rc = EIE_FAIL;
	}

	if (sp->dsr_indata.VL_var.count == 0) {
        if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH){
            if (sp->var.meta.is_specified == IOSV_NOT_SPECIFIED){
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgNoDefaultForVar); //no numeric variable in data set
            }else{
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementEmpty, VAR_GRM_NAME); //treat VAR; as an error
			}
        }else{
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementMandatory, VAR_GRM_NAME); //(2 data sets or a WITH) specified, VAR is mandatory
        }
		rc = EIE_FAIL;
    }
	else { // VL_var.count != 0
		if (VL_is_specified(&sp->dsr_indata.VL_var)) {
			/* Check if VAR variables are repeated, ex : VAR INCOME INCOME; */
			if (SUtil_AreDuplicateInListPosition (sp->dsr_indata.VL_var.count, sp->dsr_indata.VL_var.positions)) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmWithDuplicateVariable, VAR_GRM_NAME);
				rc = EIE_FAIL;
			}
		}
	}

	if (sp->with_var.meta.is_specified == IOSV_SPECIFIED) {
		if (sp->dsr_indata_hist.VL_var.count == 0) {
			//treat WITH; as an error
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementEmpty, WITH_GRM_NAME);
			rc = EIE_FAIL;
		}
	}

	//treat WEIGHT; as an error /* Following conversion, this check may be pointless, retain anyway */
	if ((sp->weight.meta.is_specified == IOSV_SPECIFIED) && (sp->dsr_indata.VL_weight.count == 0)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementEmpty, WEIGHT_GRM_NAME);
		rc = EIE_FAIL;
	}

	//treat BY; as an error /* Following conversion, this check may be pointless, retain anyway */
	if ((sp->by.meta.is_specified == IOSV_SPECIFIED) && (sp->dsr_indata.dsr.VL_by_var.count == 0)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementEmpty, BPN_BY);
		rc = EIE_FAIL;
	}

	if (DuplicateBetweenListsInData (sp, InputMode) == EIE_FAIL)
		rc = EIE_FAIL;

	if (PositivityOption == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
		rc = EIE_FAIL;
	}

	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHTRatioMethodAcceptNegativeIgnored);
			if (AcceptZero == EIE_TRUE)
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHTRatioMethodAcceptZeroIgnored);
		}
	}
	else {/* HB */
		if (InputMode != E_OUTLIER_INPUTMODE_1DS_NOWITH) {
			if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHTRatioMethodAcceptNegativeIgnored);
			if (AcceptZero == EIE_TRUE)
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHTRatioMethodAcceptZeroIgnored);
		}
	}

	if (Method == E_OUTLIER_METHOD_CURRENT) {
		if (sp->dsr_indata_hist.dsr.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgHISTDataSetIgnored, HistAuxKeyword);
		if (sp->with_var.meta.is_specified == IOSV_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgWithIgnored);
	}
	/* - Comment out Exponent message *-*-June 10, 2013 -
	if (SFTYPE (proc.head, EXPONENT_GRM_NB) != 0 && (Method == E_OUTLIER_METHOD_SIGMAGAP || Method == E_OUTLIER_METHOD_CURRENT))
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgExponentIgnored);
    */
	if (Method == E_OUTLIER_METHOD_SIGMAGAP) {
		if (MinObs >= EIM_OUTLIER_MINIMUM_MINOBS_SIGMAGAP && MinObs < EIM_OUTLIER_DEFAULT_MINOBS)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgMINOBSWarningValue, EIM_OUTLIER_DEFAULT_MINOBS, EIM_OUTLIER_DEFAULT_MINOBS);
	}
	else {
		if (MinObs >= EIM_OUTLIER_MINIMUM_MINOBS_HB && MinObs < EIM_OUTLIER_DEFAULT_MINOBS)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgMINOBSWarningValue, EIM_OUTLIER_DEFAULT_MINOBS, EIM_OUTLIER_DEFAULT_MINOBS);
	}

	return rc;
}
/*
Check VAR list in AUX/HIST data set (numeric type).
Keep their positions.
*/
static EIT_RETURNCODE ValidateVARInHist (
	SP_outlier* sp,
	int NumberOfVarVariables,
	int * VarVariablesPosition,
	char * HistAuxKeyword,
	int * WithVariablesPosition)
{
	int i;
	EIT_RETURNCODE rc;
	char* VariableName;
	int VariableType;

	rc = EIE_SUCCEED;

	for (i = 0; i < NumberOfVarVariables; i++) {
		VariableName = (char*) sp->dsr_indata.dsr.col_names[VarVariablesPosition[i]];
		IO_RETURN_CODE rc_io = DSR_get_pos_from_names(&sp->dsr_indata_hist.dsr, 1, &VariableName, &WithVariablesPosition[i]);
		if (rc_io == IORC_VARLIST_NOT_FOUND) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameNotInDataSet, VariableName, HistAuxKeyword);
			rc = EIE_FAIL;
		}
		else {
			VariableType = DSR_get_col_type(&sp->dsr_indata_hist.dsr, WithVariablesPosition[i]);
			if (VariableType != IOVT_NUM) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotNumericInDataSet, VariableName, HistAuxKeyword);
				rc = EIE_FAIL;
			}
		}
	}
	return rc;
}
/*
Verify if number of observations in data set is "enough"
ie equal or more to 10 observations (MINIMUM_OBS).
Print statistics and message if not enough observations.
*/
static EIT_BOOLEAN VerifyEnoughObsByGroup (
	SP_outlier* sp,
	T_OUTLIER_INPUTMODE InputMode,
	int MinObs,
	char * HistAuxKeyword,
	T_OUTLIER_COUNTER * Counter,
	EIT_DATATABLE * DataTable,
	int ByGroupLoggingLevel)
{
	if (InputMode == E_OUTLIER_INPUTMODE_1DS_NOWITH) {
		if (Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT] < MinObs) {
			if (ByGroupLoggingLevel != 1) {
				PrintStatistics(InputMode, sp, Counter, HistAuxKeyword);
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgMinimumValueOfValidObsNotReached, MinObs);
				IO_PRINT_LINE("");
			}
			return EIE_FALSE;
		}
	}
	else {
		CountNoneHistObs (DataTable, Counter);
		if ((Counter->ValidObsInByGroup[EIE_PERIOD_CURRENT] - Counter->EmptyHistObsInByGroup) < MinObs) {
			if (ByGroupLoggingLevel != 1) {
				PrintStatistics(InputMode, sp, Counter, HistAuxKeyword);
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgMinimumValueOfValidObsNotReached, MinObs);
				IO_PRINT_LINE("");
			}
			return EIE_FALSE;
		}
	}
	return EIE_TRUE;	
}
/*
Write Out Summary dataset observation
*/
static EIT_WRITECALLBACK_RETURNCODE WriteOutSum (
	DSW_generic* dsw,
    T_OUTSUMINFO * OutSumData
)
{
	if (OutSumData->IsEnabled){
		/* BY-level counters */
		OutSumData->NObs             = MissingToSasMissing(OutSumData->NObs);
		OutSumData->NValid           = MissingToSasMissing(OutSumData->NValid);
		OutSumData->NRej_NoMatch     = MissingToSasMissing(OutSumData->NRej_NoMatch);
		/* VAR-level counters */
		OutSumData->NUsed            = MissingToSasMissing(OutSumData->NUsed);
		OutSumData->NRej_Total       = MissingToSasMissing(OutSumData->NRej_Total);
		OutSumData->NRej_VarMiss     = MissingToSasMissing(OutSumData->NRej_VarMiss);
		OutSumData->NRej_VarZero     = MissingToSasMissing(OutSumData->NRej_VarZero);
		OutSumData->NRej_VarNegative = MissingToSasMissing(OutSumData->NRej_VarNegative);
		OutSumData->NRej_AuxMiss     = MissingToSasMissing(OutSumData->NRej_AuxMiss);
		OutSumData->NRej_AuxZero     = MissingToSasMissing(OutSumData->NRej_AuxZero);
		OutSumData->NRej_AuxNegative = MissingToSasMissing(OutSumData->NRej_AuxNegative);
		/* NFT(I|E) */
		OutSumData->NFTI             = MissingToSasMissing(OutSumData->NFTI);
		OutSumData->NFTE             = MissingToSasMissing(OutSumData->NFTE);
		/* boundaries */
		OutSumData->IMP_BND_L        = MissingToSasMissing(OutSumData->IMP_BND_L);
		OutSumData->EXCL_BND_L       = MissingToSasMissing(OutSumData->EXCL_BND_L);
		OutSumData->EXCL_BND_R       = MissingToSasMissing(OutSumData->EXCL_BND_R);
		OutSumData->IMP_BND_R        = MissingToSasMissing(OutSumData->IMP_BND_R);
		/* HB Method specific */
		OutSumData->Q1               = MissingToSasMissing(OutSumData->Q1);
		OutSumData->M                = MissingToSasMissing(OutSumData->M);
		OutSumData->Q3               = MissingToSasMissing(OutSumData->Q3);
		/* SG method specific */
		OutSumData->SIGMA            = MissingToSasMissing(OutSumData->SIGMA);
		OutSumData->IMP_SIGMAGAP     = MissingToSasMissing(OutSumData->IMP_SIGMAGAP);
		OutSumData->EXCL_SIGMAGAP    = MissingToSasMissing(OutSumData->EXCL_SIGMAGAP);
		
		if (IORC_SUCCESS != DSW_add_record(dsw)) {
			return EIE_WRITECALLBACK_FAIL;
		}
	}
	return EIE_WRITECALLBACK_SUCCEED;
}
/*
Write outcome to outstatus data set.
This function is used by all outlier detection methods.
*/
static EIT_WRITECALLBACK_RETURNCODE WriteStatus (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	void * UserData,
	char * Id,
	EIT_STATUS Status,  /* can be ODI(L/R), ODE(L/R) */
	double CurrentValue,     /* used if OutlierstatOption */
	double PreviousValue,    /* used if OutlierstatOption and 2 variables */
	double Weight,
	double Effect,           /* used if OutlierstatOption and 2 variables */
	double LeftODIBoundary,  /* used if OutlierstatOption */
	double LeftODEBoundary,  /* used if OutlierstatOption */
	double RightODEBoundary, /* used if OutlierstatOption */
	double RightODIBoundary, /* used if OutlierstatOption */
	double Excl_Sigmagap,    /* used if OutlierstatOption */
	double Imp_Sigmagap,     /* used if OutlierstatOption */
	double Gap)              /* used if OutlierstatOption */
{
	T_OUTLIERCALLBACKINFO * LocalUserData = (T_OUTLIERCALLBACKINFO *) UserData;

	if (strcmp (Status, EIM_STATUS_GOOD) != 0) {
		/*
		variable written to dataset cannot be null terminated.
		the string must be at least the length specified when XVPUTD was called
		and it must be space padded to that length.
		*/
		SUtil_CopyPad (LocalUserData->Id, Id, LocalUserData->IdLength);

		if (strncmp (Status, ODE_ABBREV, STRLENGTH_STATUS) == 0) // check first 3 characters
			SUtil_CopyPad (LocalUserData->Status, EIM_STATUS_FTE, EIM_STATUS_SIZE);
		else
			SUtil_CopyPad (LocalUserData->Status, EIM_STATUS_FTI, EIM_STATUS_SIZE);

		SUtil_CopyPad (LocalUserData->OutlierStatus, Status, EIM_STATUS_SIZE);

		// copy value from `indata` dataset
		LocalUserData->FieldValue = CurrentValue;

		/* Conditional writing: OUTLIERSTAT_OPTION */
		if (LocalUserData->OutlierstatOption) {
			/*
			I set all variables, but only those variables which were defined in
			DefineGatherWrite() will be written to the data set.
			*/
			LocalUserData->CurrentValue = CurrentValue;
			LocalUserData->PreviousValue = PreviousValue;
			LocalUserData->Weight = Weight;
			LocalUserData->Effect = Effect;
			LocalUserData->LeftODIBoundary = MissingToSasMissing (LeftODIBoundary);
			LocalUserData->LeftODEBoundary = MissingToSasMissing (LeftODEBoundary);
			LocalUserData->RightODEBoundary = MissingToSasMissing (RightODEBoundary);
			LocalUserData->RightODIBoundary = MissingToSasMissing (RightODIBoundary);
			LocalUserData->Excl_Sigmagap = MissingToSasMissing (Excl_Sigmagap);
			LocalUserData->Imp_Sigmagap = MissingToSasMissing (Imp_Sigmagap);
			LocalUserData->Gap = Gap;
		}

		if (IORC_SUCCESS != DSW_add_record(dsw_outstatus)) {
			return EIE_WRITECALLBACK_FAIL;
		}
		
		if (IORC_SUCCESS != DSW_add_record(dsw_outstatus_detailed)) {
			return EIE_WRITECALLBACK_FAIL;
		}
	}

	return EIE_WRITECALLBACK_SUCCEED;
}
