/**************************************************************************/
/* NAME:         DONORIMPUTATION                                          */
/* PRODUCT:      SAS/TOOLKIT                                              */
/* TYPE:         PROCEDURE                                                */
/* DESCRIPTION:  This function implements the donor imputation method     */
/*               in the form of a SAS Procedure                           */
/*               (accept negative values data)                            */
/**************************************************************************/
#define MAINPROC 1
#define SASPROC  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "donorimp_JIO.h"

#include "Donor.h"
#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Donor.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"
#include "EI_EditsConsistency.h"
#include "EI_EditsRedundancy.h"
#include "EI_Message.h"
#include "EI_SortRespondents.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "slist.h"
#include "adtlist.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageBanff.h"

/* To do with DONORMAP out dataset  */
#define DONORMAP_RECIPIENT    "RECIPIENT"
#define DONORMAP_DONOR        "DONOR"
#define DONORMAP_NB_ATTEMPTS  "NUMBER_OF_ATTEMPTS"
#define DONORMAP_DONORLIMIT   "DONORLIMIT"

/* To do with parameter ELIGDON */
#define ELIGDON_FLAG_ANY         0
#define ELIGDON_FLAG_ORIGINAL    1
#define ELIGDON_LETTER_ANY       'A'
#define ELIGDON_LETTER_ORIGINAL  'O'
#define ELIGDON_MAX_LENGTH        8
#define ELIGDON_STRING_ANY       "ANY"
#define ELIGDON_STRING_DEFAULT   "ORIGINAL"
#define ELIGDON_STRING_ORIGINAL  "ORIGINAL"

#define NB_VARS_DONORMAP 4
#define NB_VARS_INSTATUS 3
#define NB_VARS_OUTMATCHING_FIELDS 3

/* To do with parameter DATAEXCLVAR */
/* (Acceptable string value)        */
#define RESPONDENT_TO_EXCLUDE    "E"

#define DEFAULT_MIN_DONORS   30
#define DEFAULT_PCENT_DONORS 30.0
#define MAX_PCENT_DONORS 100.0
#define MIN_PCENT_DONORS   1.0
#define NB_DECIMAL  2
#define VAR_NOT_FOUND -1
#define VARSTMT_LEX BPN_MUST_MATCH
#define INDEX_LIST(l,index) (((l)->listItems) + ((index) * (l)->itemSize))

/* number of digit to print in report before IO_PRINT will start to use
scientific notation */
#define PRINT_PRECISION 7
#define PRINT_STATISTICS_WIDTH PRINT_STATISTICS_WIDTH_DEFAULT

typedef struct {
	char Name[LEGACY_MAXNAME+1];
	int Size;
	int Position;
	char * Value;
} T_CHAR_VARIABLE;

/* info needed for execution statistics */
typedef struct {
	int ObsInByGroup;
	int ValidObsInByGroup;
	int DonorsReachedDonorLimit;
	int WithMatchingFields;
	int WithMatchingFieldsImputed;
	int WithMatchingFieldsNotImputed;
	int WithoutMatchingFields;
	int WithoutMatchingFieldsImputed;
	int WithoutMatchingFieldsNotImputed;
	int WithInsufficientDonors; /* Counter to keep track of recipients*/
								/* in a "by group" with not enough    */
								/* donors.                            */
	int MissingKey;			/* Counter to keep track of respondents     */
							/* with missing value for the key.          */
	int MissingData;		/* Counter to keep track of respondents     */
							/* where an edit field or one user matching */
							/* field outside the edit is missing but    */
							/* has no FTI status.                       */
	int MissingStatus;		/* Counter to keep track of entries on the  */
							/* status data set where either the key     */
							/* field or the FIELDID field or both are   */
							/* missing.                                 */
	int Mixed;				/* Counter to keep track of "mixed"         */
							/* respondents.                             */
	int ExcludedDonors;		/* Counter to keep track of respondents     */
							/* qualified as a donor but excluded by     */
							/* DATAEXCLVAR.                             */
	int NoEligibleDonors;	/* Counter to keep track of respondents     */
							/* qualified as a donor but excluded by     */
							/* ELIGDON.                                 */
	int NegativeData;		/* Counter to keep track of respondents     */
							/* where an edit field or one user matching */
							/* field outside the edit is negative but   */
							/* has no FTI status, AND the               */
							/* REJECTNEGATIVE option is YES.            */
} T_INFO;

/* structure to hold info needed by the write function.*/
typedef struct {
	char * DonorKey;
	double NumberOfAttempts;
	double DonorLimit;
	char OutFieldid[LEGACY_MAXNAME+1];
	char OutStatus[EIM_STATUS_SIZE+1];
	double FieldValue;
	double * OutVarValue;
	char * RecipientKey;
	char * RespondentKey;
	int LengthKeyData;
	int EditsNumberOfFields;
	int MatchingFieldsSelection;
} T_WRITEINFO;


static int SList_FindInsertionPointBinary (tSList *, char *);
static int SList_InsertStringAfter (tSList *, int, char *);
static void AdjustRecipientsStatusIfMFU (SP_donorimp* sp, EIT_ALL_RECIPIENTS *, int *, int);
static EIT_DATATABLE * AllocateDataTable (SP_donorimp* sp);
static void AllocateDataVariables (char  **, int, double **, int);
static void AllocateWriteVariables (char  **, double **, int, int);
static EIT_RETURNCODE BuildNumericList (DSR_generic* dsr, char **, int, int *);
static EIT_RETURNCODE CheckCharVarInStatus (DSR_generic* dsr, char *, T_CHAR_VARIABLE *);
static void CleanMustMatchVariables (int *, int, int *, int *);
static void CopyDataTableToAllRespondents (EIT_DATATABLE *, EIT_ALL_RESPONDENTS *);
static void CopyAllRespondentsToAllRecipients (EIT_ALL_RESPONDENTS *, EIT_ALL_RECIPIENTS *, int);
static EIT_BOOLEAN DataNonEligible (EIT_DATAREC *);
static EIT_BOOLEAN DataMixed (EIT_DATAREC *, int);
static EIT_BOOLEAN DataUnflaggedMissing (EIT_DATAREC *);
static EIT_BOOLEAN DataUnflaggedNegative (EIT_DATAREC *);
static PROC_RETURN_CODE DefineGatherWriteData (DSW_generic* dsw, T_CHAR_VARIABLE, char *, char **, int, double *);
static PROC_RETURN_CODE DefineGatherWriteDonorMap (DSW_generic* dsw, char *, char *, double *, double *);
static PROC_RETURN_CODE DefineGatherWriteStatus (DSW_generic* dsw, T_CHAR_VARIABLE, T_WRITEINFO* UserData);
static PROC_RETURN_CODE DefineGatherWriteMatchingFields (DSW_generic* dsw, T_CHAR_VARIABLE, char *, char *, char *);
static EIT_RETURNCODE DefineScatterReadData (DSR_indata* dsr, int, int,
	T_CHAR_VARIABLE *, int, int, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, int *,
	double *);
static EIT_RETURNCODE DefineScatterReadStatus (DSR_instatus* dsr, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *);
static EIT_RETURNCODE DefineVarsINSTATUS (DSR_generic* dsr, T_CHAR_VARIABLE,
	T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *);
static int DoImputation (SP_donorimp* sp, EIT_ALL_RESPONDENTS *, EIT_ALL_RECIPIENTS *,
	EIT_TRANSFORMED *, EIT_ALL_DONORS *, EIT_FIELDNAMES *, int, EIT_EDITS *,
	int, int, int, EIT_KDTREE *, T_INFO *, T_WRITEINFO *, char **, int, tSList *, tADTList *, int);
static void EliminateCommonFields (char **, int, char **, int,
	EIT_FIELDNAMES *);
static EIT_RETURNCODE ExclusivityBetweenLists (DSR_generic* dsr, int *, int, int *,
	int, int, int, int, int, int);
static EIT_READCALLBACK_RETURNCODE GetMatchingFieldData (char *, int *, char **,
	double *);
static void GetNamesofUserMatchingFields (SP_donorimp* sp, EIT_FIELDNAMES *, int);
static void GetParms (SP_donorimp* sp, char **, char **, int *, int **, int *, double *, int *, char *, int *,
	T_CHAR_VARIABLE *, int *, int *, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, int *,
	EIPT_POSITIVITY_OPTION *, int *, int *, double *, int *);
static void IncrementTotalCounter (T_INFO *, T_INFO *);
static void InitCounter (T_INFO *);
static int IsRecipient (int, EIT_STATUS_OF_FIELDS *);
static int IsUserMatchingField (int, int *, int);
static void LpFreeEnv (void);
static void PrintInfoCounters (char *, char *, T_INFO *);
static void PrintParms (SP_donorimp* sp, int, double,
	int, char *, int, char *, int, int, char *, char *, char *, char *,
	EIPT_POSITIVITY_OPTION, int, int, double, int ByGroupLoggingLevel);
static void PrintStatistics (int, int, T_INFO *, int);
static PROC_RETURN_CODE ReadByGroup (SP_donorimp* sp, EIT_FIELDNAMES *, T_CHAR_VARIABLE *,
	int, int, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, double *, T_CHAR_VARIABLE *, T_CHAR_VARIABLE *,
	T_CHAR_VARIABLE *, EIT_DATATABLE *, T_INFO *, EIT_BOOLEAN, tSList *, tADTList *);
static PROC_RETURN_CODE ReadData (SP_donorimp* sp, EIT_FIELDNAMES *, T_CHAR_VARIABLE *, int, int, T_CHAR_VARIABLE *,
	T_CHAR_VARIABLE *, double *, EIT_DATATABLE *, T_INFO *, tSList *, tADTList *);
static PROC_RETURN_CODE ReadStatus (SP_donorimp* sp, EIT_FIELDNAMES *, T_CHAR_VARIABLE *,
	T_CHAR_VARIABLE *, T_CHAR_VARIABLE *, EIT_DATATABLE *, T_INFO *, 
	EIT_BOOLEAN);
static void RemoveSomeData (EIT_DATATABLE *, int, int, int, T_INFO *,
	EIPT_POSITIVITY_OPTION);
static EIT_RETURNCODE ValidateEligDonParameter (char *, int *);
static EIT_RETURNCODE ValidateParms (SP_donorimp* sp, char *,
	char *, int, double, int, char *, int *, int, int *,
	EIPT_POSITIVITY_OPTION, int, int, double);
static PROC_RETURN_CODE WriteResults (SP_donorimp* sp, T_WRITEINFO *, EIT_DATAREC *, char *, char *, int, int, char **);
static void ShowTime (char *);

/* These variables are used by callback functions. */
/* This is why they are global.                    */
static EIT_ALL_RESPONDENTS mAllRespondents = {0, 0, NULL, NULL, NULL};

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int donorimp(
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
)
{
	EIT_ALL_DONORS AllDonors = {NULL, NULL, NULL, NULL, 0, 0}; 
	EIT_FIELDNAMES AllFields = {NULL, 0};
	char ** AllFieldsNameOriginal = NULL;	/* Used by "WriteResults" (all the   */
									/* process is done with "uppercase") */
	EIT_ALL_RECIPIENTS AllRecipients = {0, 0, NULL, NULL};
	EIT_TRANSFORMED AllTransformed = {NULL, 0, NULL};
	EIT_DATATABLE * DataTable = NULL;	/* To hold "all "data" for one "by group" */
								/* and copy them in "mAllRespondents"     */
								/* before processing. Overhead but:       */
								/* - no need to sorted datasets before    */
								/*   calling this procedure               */
								/* - no need to change the API's          */
	EIT_EDITS Edits = {NULL, NULL, NULL, 0, 0, 0};

	int EditsNumberOfEquations;
	int EditsNumberOfFields;	/* Number of fields involved in the Edits */
								/* without the user must match fields.    */
	char ** EditsOriginalFieldNames = NULL;
	char * EditsString = {0}; /* String containing all edits input */
						/* at procedure call.                */

	int * EditVariablesList = NULL;
	int EligDonorFlag; /* 0:any  1:original */
	char EligDonorString[ELIGDON_MAX_LENGTH+1];
	int EnoughDonors;
	T_CHAR_VARIABLE ExclVarDATA = { 0 }; /* For "exclusion variable" in DATA */
	T_CHAR_VARIABLE RandNumDATA = { 0 }; /* For "randomnum variable" in DATA */
	int ExclVarFlag = { 0 }; /* One exclusion var (1) or none (0) */
	int RandNumFlag = { 0 }; /* One randomnum var (1) or none (0) */
	tSList * randkey_list = NULL;   /* One record key    (char *) value per record in the current by group (also called the current "data group") or NULL (if RandNumFlag == 0) */
	tADTList * randnum_list = NULL; /* One random number (double) value per record in the current by group (also called the current "data group") or NULL (if RandNumFlag == 0) */
	T_CHAR_VARIABLE FieldidINSTATUS = { 0 }; /* For "fieldid" in INSTATUS */
	int i;
	T_INFO Info;
	EIT_BOOLEAN InStatusHasAllByVariables;
	double * InVarValue = NULL; /* Values read from DATA. */
	EIT_KDTREE * KDTree = NULL;
	EIT_KDTREE_RETURNCODE KDTREErcode;
	T_CHAR_VARIABLE KeyDATA = { 0 };     /* For "key" in DATA     */
	T_CHAR_VARIABLE KeyINSTATUS = { 0 }; /* For "key" in INSTATUS */
	char * lpversion = NULL;
	char * lpname = NULL;
	EIT_FIELDNAMES MatchingFields = {NULL, 0};
	EIT_MF_RETURNCODE MFrcode;
	int MinimumNumberOfDonors;
	double MinimumPercentOfDonors;
	double Mrl;
	int * MustMatchVariablesList = { 0 }; /* For all the "must match" vars.  */
	int * MustMatchVariablesListToRead = NULL;	/* Only for the "must match" */
												/* vars outside EDITS'ones.  */
	int NLimit;
	int NumberOfDonorsToFind;
	int NumberOfMustMatchVars = { 0 };
	int NumberOfMustMatchVarsToRead;
	EIPT_POSITIVITY_OPTION PositivityOptionSet;
	int LoggingVerbosityLevel;
	int ByGroupLoggingLevel;
	EIT_EDITS PostEdits = {NULL, NULL, NULL, 0, 0, 0};
	char * PostEditsString = {0}; /* String containing all post imputation  */
							/* edits input at procedure call.         */
	int RandomSelection;
	EIT_RETURNCODE  rcLocal;
	EIT_RETURNCODE rcParser;
	T_CHAR_VARIABLE StatusINSTATUS = { 0 }; /* For "status" in INSTATUS */
	EIT_FIELDNAMES SystemMatchingFields = {NULL, 0};
	T_INFO TotalInfo;
	int TotalRecipients;
	int TotalRespondents;
	EIT_TRF_RETURNCODE TRFrcode;
	T_WRITEINFO UserData = { 0 };
	

	EIT_FIELDNAMES UserMatchingFields = {NULL, 0}; /* Put in uppercase after printing */
	int index_for_debugging_statements;


	int rc;

	BANFF_RETURN_CODE proc_exit_code = BRC_SUCCESS;

	mAllRespondents.Key					= NULL;              
	mAllRespondents.RespondentData		= NULL;
	mAllRespondents.RespondentExclusion = NULL;

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

	UTIL_ResetTime ();

	/* SI : System Info */
	SUtil_PrintSystemInfo (SI_SYSTEMNAME, SI_SYSTEMVERSION, BANFF_PROC_NAME,
        SI_PROCVERSION, SI_EMAIL, NULL);

	SP_donorimp sp = { 0 };
	TIME_WALL_START(load_init);
	TIME_CPU_START(load_init);
	mem_usage("before SP_init");
	IO_RETURN_CODE rc_sp_init = SP_init(
		&sp,
		in_parms,
		in_ds_indata,
		in_ds_instatus,
		out_sch_outdata,
		out_arr_outdata,
		out_sch_outstatus,
		out_arr_outstatus,
		out_sch_outdonormap,
		out_arr_outdonormap,
		out_sch_outmatching_fields,
		out_arr_outmatching_fields
	);
	mem_usage("after SP_init");

	proc_exit_code = SP_validate_init(rc_sp_init);
	if (proc_exit_code != BRC_SUCCESS) {
		goto error_cleanup;
	}

	IO_RETURN_CODE rc_validation = SPG_validate(&sp.spg);
	if (rc_validation != IORC_SUCCESS) {
		proc_exit_code = BRC_FAIL_VALIDATION_NEW;
		goto error_cleanup;
	}

	/*******************************************/
	/* Get procedure parameters except "seed". */
	/* (it'll be get later, inside PrintParms) */
	/*******************************************/
	GetParms (&sp, &EditsString,
		&PostEditsString, &NumberOfMustMatchVars, &MustMatchVariablesList,
		&MinimumNumberOfDonors, &MinimumPercentOfDonors, &NumberOfDonorsToFind,
		EligDonorString, &RandomSelection, &KeyDATA, &ExclVarFlag, &RandNumFlag,
		&ExclVarDATA, &RandNumDATA, &UserData.MatchingFieldsSelection, &PositivityOptionSet, &LoggingVerbosityLevel,
		&NLimit, &Mrl, &ByGroupLoggingLevel);
	/****************************************************/
	/* Get the names of MUSTMATCH fields (no mandatory) */
	/****************************************************/
	GetNamesofUserMatchingFields (&sp, &UserMatchingFields,
		NumberOfMustMatchVars);
	/*********************************************************/
	/* Print procedure parameters in the LOG                 */
	/* (for "seed": get/print it and start the generator...) */
	/*********************************************************/
	PrintParms (&sp, MinimumNumberOfDonors,
		MinimumPercentOfDonors, NumberOfDonorsToFind, EligDonorString,
		RandomSelection, KeyDATA.Name,
		ExclVarFlag,RandNumFlag,ExclVarDATA.Name,RandNumDATA.Name, EditsString, PostEditsString,
		PositivityOptionSet, LoggingVerbosityLevel, NLimit, Mrl, ByGroupLoggingLevel);

	/* Initializes the LP environment */
	if (LPI_InitLpEnv (&lpname, &lpversion) != LPI_SUCCESS) {
		IO_PRINT (SAS_MESSAGE_PREFIX_ERROR MsgInitLPFail "\n\n", "");
		proc_exit_code = BRC_FAIL_LPI_INIT;
		goto error_cleanup;
	}
	/***************************/
	/* Validate parameters     */
	/***************************/
	rcLocal = ValidateParms (&sp, EditsString,
		PostEditsString, MinimumNumberOfDonors, MinimumPercentOfDonors,
		NumberOfDonorsToFind, EligDonorString, &EligDonorFlag,
		NumberOfMustMatchVars, MustMatchVariablesList,
		PositivityOptionSet, LoggingVerbosityLevel, NLimit, Mrl);
	if (rcLocal == EIE_FAIL) {
		EI_PrintMessages ();
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
	}

	/*****************/
	/* Get the Edits */
	/*****************/
	rcParser = EI_EditsParse (EditsString, &Edits);
	if (rcParser != EIE_SUCCEED) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_EDITS ".");
		EI_PrintMessages ();
		proc_exit_code = BRC_FAIL_EDITS_PARSE;
		goto error_cleanup;
	}

	EI_PrintMessages (); /* print warnings of EI_EditsParse */

	IO_PRINT_LINE (SAS_NEWLINE MsgHeaderEdits);
	if ((PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT) ||
		(PositivityOptionSet == EIPE_REJECT_NEGATIVE)) {
		if (EIE_SUCCEED == EI_EditsAddPositivity (&Edits))
			IO_PRINT_LINE (MsgPositivityEditsAdded );
		else {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAddingPositivityEditsFail);
			EI_PrintMessages ();
			proc_exit_code = BRC_FAIL_EDITS_OTHER;
			goto error_cleanup;
		}
	}

	SUtil_PrintEdits (&Edits);

	/***********************************************/
	/* Get the Post Imputation Edits if any.       */
	/* (take Edits as PostEdits if none are given) */
	/***********************************************/
	if (PostEditsString != NULL) {
		rcParser = EI_EditsParse (PostEditsString, &PostEdits);
		if (rcParser != EIE_SUCCEED) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_POST_EDITS);
			EI_PrintMessages ();
			proc_exit_code = BRC_FAIL_EDITS_PARSE;
			goto error_cleanup;
		}
		EI_PrintMessages (); /* print warnings of EI_EditsParse */

		IO_PRINT_LINE (SAS_NEWLINE MsgHeaderPostEdits);
		if ((PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT) ||
			(PositivityOptionSet == EIPE_REJECT_NEGATIVE)) {
			if (EIE_SUCCEED == EI_EditsAddPositivity (&PostEdits))
				IO_PRINT_LINE (MsgPositivityPostEditsAdded);
			else {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAddingPositivityPostEditsFail);
				EI_PrintMessages ();
				proc_exit_code = BRC_FAIL_EDITS_OTHER;
				goto error_cleanup;
			}
		}
		SUtil_PrintEdits (&PostEdits);
	}
	else {
		/* Edits are taken as PostEdits */
		rcParser = EI_EditsParse (EditsString, &PostEdits);
		if (rcParser != EIE_SUCCEED) {
			/* should not fail, edits already parsed previously */
			EI_PrintMessages ();
			proc_exit_code = BRC_FAIL_EDITS_PARSE;
			goto error_cleanup;
		}
	}

	/*************************************/
	/* Edits:                            */
	/* - check consistency               */
	/* - check redundancy                */
	/* - print reduced set               */
	/*************************************/
	if (EI_EditsConsistency (&Edits) != EIE_EDITSCONSISTENCY_CONSISTENT) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInconsistentEdits SAS_NEWLINE);
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_EDITS_CONSISTENCY;
		goto error_cleanup;
	}

	EditsNumberOfEquations = Edits.NumberofEquations;

	if (EI_EditsRedundancy (&Edits) != EIE_EDITSREDUNDANCY_SUCCESS) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_EDITS_REDUNDANCY;
		goto error_cleanup;
	}

	if (EditsNumberOfEquations == Edits.NumberofEquations)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgReducedEditsEquivalent SAS_NEWLINE);
	else {
		IO_PRINT_LINE (SAS_NEWLINE MsgHeaderReducedEdits);
		SUtil_PrintEdits (&Edits);
	}

	/*************************************/
	/* If any PostEdits are given        */
	/* explicitly:                       */
	/* - check consistency               */
	/* - check redundancy                */
	/* - print reduced set               */
	/*************************************/
	if (sp.post_edits.meta.is_specified == IOSV_SPECIFIED) {
		if (EI_EditsConsistency (&PostEdits)!= EIE_EDITSCONSISTENCY_CONSISTENT){
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInconsistentPostEdits SAS_NEWLINE);
			/* LpFreeEnv (); */
			proc_exit_code = BRC_FAIL_EDITS_CONSISTENCY;
			goto error_cleanup;
		}

		EditsNumberOfEquations = PostEdits.NumberofEquations;

		if (EI_EditsRedundancy (&PostEdits) != EIE_EDITSREDUNDANCY_SUCCESS) {
			EI_PrintMessages ();
			/* LpFreeEnv (); */
			proc_exit_code = BRC_FAIL_EDITS_REDUNDANCY;
			goto error_cleanup;
		}
		if (EditsNumberOfEquations == PostEdits.NumberofEquations)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgReducedPostEditsEquivalent SAS_NEWLINE);
		else {
			IO_PRINT_LINE (SAS_NEWLINE MsgHeaderReducedPostEdits);
			SUtil_PrintEdits (&PostEdits);
		}
	}

	/*********************************************/
	/* Check that edits and post edits contains  */
	/* the exact same fields, only if post edits */
	/* are given explicitly.                     */
	/*********************************************/
	if (sp.post_edits.meta.is_specified == IOSV_SPECIFIED) {
		if (EI_EditsCheckFieldsSet (&Edits, &PostEdits) != 0) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDifferentSetVarsInEditsPostEdits SAS_NEWLINE);
			/* LpFreeEnv (); */
			proc_exit_code = BRC_FAIL_EDITS_OTHER;
			goto error_cleanup;
		}
	}

	/***********************************************************/
	/* Check and get info on mandatory variables in INSTATUS   */
	/***********************************************************/
	rcLocal = DefineVarsINSTATUS (&sp.dsr_instatus.dsr, KeyDATA, &KeyINSTATUS,
		&FieldidINSTATUS, &StatusINSTATUS);
	if (rcLocal == EIE_FAIL) {
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
		goto error_cleanup;
	}

	/*****************************************************************/
	/* Get Numeric List of SAS Variables corresponding to FieldNames */
	/* involved in the edits.                                        */
	/* This list represents the position of each field on the input  */
	/* SAS Data set.                                                 */
	/* If the variable is not found or is not numeric, return error. */
	/*****************************************************************/
	EditVariablesList = STC_AllocateMemory (
		(Edits.NumberofColumns-1) * sizeof *EditVariablesList);
	rcLocal = BuildNumericList (&sp.dsr_indata.dsr, Edits.FieldName, Edits.NumberofColumns-1,
		EditVariablesList);
	if (rcLocal == EIE_FAIL) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_SETUP_OTHER;
		goto error_cleanup;
	}

	/*******************************************************/
	/* Validate the exclusivity of variables between lists.*/
	/* (using variables position in InData)                */
	/*  - for num variables:                               */
	/*        EDITS vs BY-num sublist                      */
	/*        MUSTMATCH vs BY-num sublist                  */
	/*  - for char variables:                              */
	/*        ID vs EXCLVAR                                */
	/*        BY-char sublist vs ID                        */
	/*        BY-char sublist vs EXCLVAR                   */
	/*******************************************************/
	rcLocal = ExclusivityBetweenLists (&sp.dsr_indata.dsr, EditVariablesList,
		Edits.NumberofColumns-1, MustMatchVariablesList, NumberOfMustMatchVars,
		KeyDATA.Position, ExclVarFlag, RandNumFlag, ExclVarDATA.Position, RandNumDATA.Position);
	if (rcLocal == EIE_FAIL) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
	}

	/*******************************************************************/
	/* The parser returns the variable names in upper case.  The names */
	/* must return to their original state (bring back lower case      */
	/* letters if any) so that they are output the same as they were   */
	/* input.                                                          */
	/*******************************************************************/
	EditsOriginalFieldNames = STC_AllocateMemory (
		(Edits.NumberofColumns-1) * sizeof *EditsOriginalFieldNames);
	if (EditsOriginalFieldNames == NULL) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_ALLOCATE_MEMORY;
		goto error_cleanup;
	}

	for (i = 0; i < Edits.NumberofColumns-1; i++) {
		EditsOriginalFieldNames[i] = STC_StrDup (Edits.FieldName[i]);
		SUtil_GetOriginalName(&sp.dsr_indata.dsr, EditsOriginalFieldNames[i]);
	}

	/* The user matching fields must be put in uppercase. */
	for (i = 0; i < UserMatchingFields.NumberofFields; i++)
		UTIL_StrUpper (UserMatchingFields.FieldName[i],
			UserMatchingFields.FieldName[i]);

	/*************************************************************/
	/* Get the total number of fields (in uppercase) involved in */
	/* this run which is the total number of fields involved in  */
	/* the edits plus the total number of user defined matching  */
	/* fields that are not in the edits.                         */
	/*************************************************************/
	UserData.EditsNumberOfFields = Edits.NumberofColumns - 1;
	EditsNumberOfFields = Edits.NumberofColumns - 1;

	EliminateCommonFields (Edits.FieldName, EditsNumberOfFields,
		UserMatchingFields.FieldName, UserMatchingFields.NumberofFields,
		&AllFields);

	/* Get "original" names of all fields: for "WriteResult" */
	if (AllFields.NumberofFields > 0) {
		AllFieldsNameOriginal = STC_AllocateMemory (
			AllFields.NumberofFields * sizeof *AllFieldsNameOriginal);
		if (AllFieldsNameOriginal == NULL) {
			EI_PrintMessages ();
			/* LpFreeEnv (); */
			proc_exit_code = BRC_FAIL_ALLOCATE_MEMORY;
			goto error_cleanup;
		}
	}
	for (i = 0; i < AllFields.NumberofFields; i++) {
		AllFieldsNameOriginal[i] = STC_StrDup (AllFields.FieldName[i]);
		SUtil_GetOriginalName(&sp.dsr_indata.dsr, AllFieldsNameOriginal[i]);
	}

	/*******************************************************/
	/* Create MustMatchVariablesListToRead to be used by   */
	/* DefineScatterReadData :                             */
	/* removes variables from MustMatchVariablesList       */
	/* that are in EditVariablesList                       */
	/*******************************************************/
	MustMatchVariablesListToRead = STC_AllocateMemory (
		NumberOfMustMatchVars * sizeof *MustMatchVariablesListToRead);
	for (i = 0; i < NumberOfMustMatchVars; i++)
		MustMatchVariablesListToRead[i] = MustMatchVariablesList[i];
	NumberOfMustMatchVarsToRead = NumberOfMustMatchVars;

	CleanMustMatchVariables (EditVariablesList, EditsNumberOfFields,
		MustMatchVariablesListToRead, &NumberOfMustMatchVarsToRead);
		
	/* Reinitialize MUST_MATCH variable list?
		`CleanMustMatchVariables` removes any member which is also listed in the
		edits.  After this cleaning, if any variables have been removed, the list
		must be reinitialized.  If all variables have been removed, free the list*/
	if (NumberOfMustMatchVarsToRead == 0) {
		VL_free(&sp.dsr_indata.VL_must_match);
	}
	else if (NumberOfMustMatchVarsToRead < NumberOfMustMatchVars) {
		VL_free(&sp.dsr_indata.VL_must_match);
		IO_RETURN_CODE rc_vl_reinit = VL_init_from_position_list(&sp.dsr_indata.VL_must_match, &sp.dsr_indata.dsr, BPN_MUST_MATCH, IOVT_NUM, NumberOfMustMatchVarsToRead, MustMatchVariablesListToRead);
		if (IORC_SUCCESS != rc_vl_reinit){
			proc_exit_code = BRC_FAIL_VARLIST_SYSTEM_GENERATED;
			goto error_cleanup;
		}
	}

	/************************************/
	/* Define scatter read for InData   */
	/************************************/
	UserData.LengthKeyData = KeyDATA.Size;
	AllocateDataVariables (&UserData.RespondentKey, KeyDATA.Size, &InVarValue,
		AllFields.NumberofFields);

	rcLocal = DefineScatterReadData (&sp.dsr_indata, 
		AllFields.NumberofFields, EditsNumberOfFields,
		&KeyDATA, ExclVarFlag, RandNumFlag, &ExclVarDATA, &RandNumDATA, EditVariablesList,
		InVarValue);
	if (rcLocal == EIE_FAIL) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
		goto error_cleanup;
	}

	/************************************/
	/* Define scatter read for InStatus */
	/************************************/
	rcLocal = DefineScatterReadStatus (&sp.dsr_instatus, &KeyINSTATUS,
		&FieldidINSTATUS, &StatusINSTATUS);
	if (rcLocal == EIE_FAIL) {
		EI_PrintMessages ();
		/* LpFreeEnv (); */
		proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
		goto error_cleanup;
	}

	/*******************************************************/
	/* Define gather write for writing updated respondents */
	/*******************************************************/
	AllocateWriteVariables (&UserData.RecipientKey, &UserData.OutVarValue,
		EditsNumberOfFields, KeyDATA.Size);
	
	PROC_RETURN_CODE rc_write_init = PRC_FAIL_UNHANDLED;

	rc_write_init = DefineGatherWriteData (&sp.dsw_outdata, KeyDATA, UserData.RecipientKey, EditsOriginalFieldNames,
		EditsNumberOfFields, UserData.OutVarValue);
	if (rc_write_init != PRC_SUCCESS) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}

	rc_write_init = DefineGatherWriteStatus (&sp.dsw_outstatus, KeyDATA, &UserData);
	if (rc_write_init != PRC_SUCCESS) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}

	rc_write_init = DefineGatherWriteMatchingFields (&sp.dsw_outmatching_fields, KeyDATA, UserData.RespondentKey, UserData.OutFieldid,
		UserData.OutStatus);
	if (rc_write_init != PRC_SUCCESS) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}

	UserData.DonorKey = STC_AllocateMemory (KeyDATA.Size+1);

	rc_write_init = DefineGatherWriteDonorMap (&sp.dsw_outdonormap, UserData.RespondentKey, UserData.DonorKey,
		&UserData.NumberOfAttempts, &UserData.DonorLimit);
	if (rc_write_init != PRC_SUCCESS) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}

	/***********************************************/
	/* FOR INDATAFILE:                             */
	/* Allocate memory for structures to hold      */
	/* all recipients and all respondents.         */
	/***********************************************/

	mAllRespondents.NumberofRespondents = 0;
	mAllRespondents.NumberofAllocatedRespondents = 0;
	mAllRespondents.Key = NULL;
	mAllRespondents.RespondentData = NULL;
	mAllRespondents.RespondentExclusion = NULL; /* not used in this proc */

	AllRecipients.NumberofRecipients = 0;
	AllRecipients.NumberofAllocatedRecipients = 0;
	AllRecipients.Key = NULL;
	AllRecipients.RecipientData = NULL;

	EI_AllocateRespondents (&mAllRespondents);
	EI_AllocateRecipients (&AllRecipients);
	DataTable = AllocateDataTable (&sp);
	TIME_CPU_STOPDIFF(load_init);
	TIME_WALL_STOPDIFF(load_init);

	TIME_WALL_START(processing);
	TIME_CPU_START(processing);
	InitCounter (&Info);
	InitCounter (&TotalInfo);
	TotalRespondents = 0;
	TotalRecipients = 0;

	/* check if INSTATUS data set has all the by variables */
	rcLocal = SUtil_AreAllByVariablesInDataSet(&sp.dsr_indata.dsr, &sp.dsr_instatus.dsr);
	EI_PrintMessages ();
	if (rcLocal == EIE_SUCCEED) {
		/* all BY variables were found in INSTATUS data set */
		InStatusHasAllByVariables = EIE_TRUE;

		/* build BY list for status data set */
        /* call to SAS_XBYLIST(...) does not need to be replaced. 
        * in SAS, this performed a check and did some setup. 
        * Now however, setup happens elsewhere and  replacement
        * SUtil_AreAllByVariablesInDataSet (called above) performs the check
        */
	}
	else {
		/* One or more BY variable was not found in INSTATUS data set */
		InStatusHasAllByVariables = EIE_FALSE;
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet SAS_NEWLINE,
			DSN_INSTATUS);
	}

	if (RandNumFlag != 0) {
		SList_New (&randkey_list);
		randnum_list = ADTList_Create(NULL, NULL, sizeof(double), 20);
	} else {
		randkey_list = NULL;
		randnum_list = NULL;
	}

	/******************************************/
	/* PROCESS: read and keep valid data      */
	/*          impute and write output data  */
	/******************************************/
	PROC_RETURN_CODE rc_processing = ReadByGroup (&sp, &AllFields, &KeyDATA, ExclVarFlag, RandNumFlag, &ExclVarDATA, &RandNumDATA, InVarValue,
		&KeyINSTATUS, &FieldidINSTATUS, &StatusINSTATUS, DataTable,
		&Info, InStatusHasAllByVariables, randkey_list, randnum_list);

	/* Every iteration process one BY group */
	int loops_entered = 0;
	while (rc_processing == PRC_SUCCESS) {
		if (++loops_entered == 1) {
			/* Status data set is read for each "by group"
				Print  statistics on it one time only. */
			if (Info.MissingStatus > 0) {
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameOrFieldidInDataSet SAS_NEWLINE,
					Info.MissingStatus, DSN_INSTATUS, KeyINSTATUS.Name);
			}
		}

	if (LoggingVerbosityLevel > 0) {
		IO_PRINT_LINE("DataTable->Size = %d", DataTable->Size);
		IO_PRINT_LINE("DataTable->NumberOfData = %d", DataTable->NumberOfData);
		IO_PRINT_LINE("RandNumFlag = %d", RandNumFlag);
		if (RandNumFlag) {
			IO_PRINT_LINE("%d items in randkey_list: [", randkey_list->ne       );
			for (index_for_debugging_statements = 0; index_for_debugging_statements < randkey_list->ne       ; index_for_debugging_statements++) {IO_PRINT_LINE("\"%s\",", (randkey_list->l[index_for_debugging_statements]));}
			IO_PRINT_LINE("]");
			IO_PRINT_LINE("%d items in randnum_list: [", randnum_list->itemsUsed);
			for (index_for_debugging_statements = 0; index_for_debugging_statements < randnum_list->itemsUsed; index_for_debugging_statements++) {IO_PRINT_LINE("%f,", ((double *)(randnum_list->listItems))[index_for_debugging_statements]);}
			IO_PRINT_LINE("]");
		}
	}

		/* remove DATA where:
			- at least one missing field is not flagged as FTI
			- a mixed record
			- a potential donor flagged EXCLUDED or non eligible (ELIGDON) */
		RemoveSomeData (DataTable, EditsNumberOfFields, ExclVarFlag,
			EligDonorFlag, &Info, PositivityOptionSet);
		if (ByGroupLoggingLevel != 1) IO_PRINT_LINE ("");
		if ((Info.MissingKey || Info.MissingData || Info.NegativeData ||
			Info.Mixed || Info.ExcludedDonors || Info.NoEligibleDonors) && (ByGroupLoggingLevel != 1))
			PrintInfoCounters (KeyDATA.Name, ExclVarDATA.Name, &Info);

		if (Info.ValidObsInByGroup == 0) {
			if (ByGroupLoggingLevel != 1) {
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNoValidObs);
				PrintStatistics(mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
					&Info, RandomSelection);
			}
			IncrementTotalCounter (&TotalInfo, &Info);
			TotalRecipients += AllRecipients.NumberofRecipients;
			TotalRespondents += mAllRespondents.NumberofRespondents;
		}
		else { /* some "valid observations" */
			/* Copy "DATATABLE" to "AllRespondents" and sort it.*/
			CopyDataTableToAllRespondents (DataTable, &mAllRespondents);

			/* About SORT:
			   - need to sort because "UTIL_Binary" is used in EI_SearchKDTree
			   - best result with "Shellsort" instead of "Quicksort"... */
			/* ShowTime ("Before Shellsort");*/
			EI_SortRespondents_Shellsort (&mAllRespondents);
			/* ShowTime ("After Shellsort");*/

			/* ... keep code for "Quicksort"
			rcSortRespondents = EI_SortRespondents_Quicksort (&mAllRespondents);
			if (rcSortRespondents == EIE_SORTRESPONDENTS_QUICKSORT_FAIL ||
				rcSortRespondents == EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK){
				EI_PrintMessages ();
				if (rcSortRespondents==EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK)
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE "Technical problem. Call your administrator.");
				proc_exit_code = BRC_FAIL_UNHANDLED;
				goto error_cleanup;
			}
			*/

			/* Identify all recipients in the current BY Group
			   (Recipients are respondents that have at least
				one field from the edits that is flagged as a FTI) */
			CopyAllRespondentsToAllRecipients (&mAllRespondents, &AllRecipients,
				EditsNumberOfFields);

			/* If any recipients, verify if enough donors.
			   Otherwise, skip the current "by group". */
			if (!AllRecipients.NumberofRecipients) {
				/* SKIP the current BY Group */
				if (ByGroupLoggingLevel != 1) {
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNoRecipient);
					PrintStatistics(mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
						&Info, RandomSelection);
				}
				IncrementTotalCounter (&TotalInfo, &Info);
				TotalRecipients += AllRecipients.NumberofRecipients;
				TotalRespondents += mAllRespondents.NumberofRespondents;
			}
			else { /* some "Recipients" */
				EnoughDonors = EI_EnoughDonors (mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
					MinimumNumberOfDonors, MinimumPercentOfDonors);

				if (!EnoughDonors) {
					/* SKIP the current "by group" */
					if (ByGroupLoggingLevel != 1) IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNotEnoughDonors);
					Info.WithInsufficientDonors = AllRecipients.NumberofRecipients;
					if (ByGroupLoggingLevel != 1) PrintStatistics (mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
						&Info, RandomSelection);
					IncrementTotalCounter (&TotalInfo, &Info);
					TotalRecipients += AllRecipients.NumberofRecipients;
					TotalRespondents += mAllRespondents.NumberofRespondents;
				}
				else {

					int DonorLimit;

					/* For all recipients:
					   adjust status for all fields to FIELDMFU
					   if the field is a user matching one and
					   it's status wasn't FIELDFTI. */
					AdjustRecipientsStatusIfMFU (&sp, &AllRecipients,
						MustMatchVariablesList, NumberOfMustMatchVars);

					/* Allocate SystemMatchingFields */
					SystemMatchingFields.FieldName = STC_AllocateMemory (
						(Edits.NumberofColumns-1) * sizeof *SystemMatchingFields.FieldName);
					SystemMatchingFields.NumberofFields = 0;

					/* Find system matching fields */
					MFrcode = EI_FindMatchingFields (&AllRecipients,
						&SystemMatchingFields, &Edits);
					if (MFrcode == EIE_MATCHFIELDS_FAIL) {
						EI_PrintMessages ();
						/* LpFreeEnv (); */
						proc_exit_code = BRC_EIE_MATCHFIELDS_FAIL;
						goto error_cleanup;
					}

					/* Eliminate the user matching fields that
					   are already in the system matching fields. */
					EliminateCommonFields (SystemMatchingFields.FieldName,
						SystemMatchingFields.NumberofFields,
						UserMatchingFields.FieldName,
						UserMatchingFields.NumberofFields, &MatchingFields);

					/* Transform data.
					   Build KDtree.
					   Do imputation. */
					TRFrcode = EI_Transform (&MatchingFields,
						GetMatchingFieldData, &AllTransformed);
					if (TRFrcode == EIE_TRANSFORM_FAIL) {
						EI_PrintMessages ();
						/* LpFreeEnv (); */
						proc_exit_code = BRC_EIE_TRANSFORM_FAIL;
						goto error_cleanup;
					}

					KDTREErcode = EI_BuildKDTree (&AllDonors, &KDTree,
						&AllTransformed, &MatchingFields, &mAllRespondents);
					if (KDTREErcode == EIE_KDTREE_FAIL) {
						EI_PrintMessages ();
						/* LpFreeEnv (); */
						proc_exit_code = BRC_EIE_KDTREE_FAIL;
						goto error_cleanup;
					}

					/* DEBUG
					EI_PrintKDTree (KDTree, 0);
					SUtil_PrintMessages ();
					ENDDEBUG */

					DonorLimit = EI_CalculateDonorLimit (NLimit, Mrl,
						mAllRespondents.NumberofRespondents-AllDonors.NumberOfDonors, AllDonors.NumberOfDonors);

					rc = DoImputation (&sp, &mAllRespondents, &AllRecipients, &AllTransformed,
						&AllDonors, &MatchingFields, Edits.NumberofColumns,
						&PostEdits, NumberOfDonorsToFind, DonorLimit, RandomSelection,
						KDTree, &Info, &UserData, AllFieldsNameOriginal, RandNumFlag, randkey_list, randnum_list, LoggingVerbosityLevel);
					if (rc != 0) {
						proc_exit_code = BRC_FAIL_PROCESSING_GENERIC;
						goto error_cleanup;
					}

					Info.DonorsReachedDonorLimit = EI_CountDonorsReachedDonorLimit (&AllDonors, DonorLimit);
					if (ByGroupLoggingLevel != 1) {
						if (DonorLimit == INT_MAX)
							IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE "DONORLIMIT = %s" SAS_NEWLINE, MsgNoLimit);
						else
							IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE "DONORLIMIT = %d" SAS_NEWLINE, DonorLimit);

						if (DonorLimit != INT_MAX && (double)AllDonors.NumberOfDonors * DonorLimit < (double)(mAllRespondents.NumberofRespondents - AllDonors.NumberOfDonors))
							IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgDonorLimitation SAS_NEWLINE);

						PrintStatistics(mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
							&Info, RandomSelection);
						DSR_cursor_print_by_message(&sp.dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
					}
					IncrementTotalCounter (&TotalInfo, &Info);
					TotalRecipients += AllRecipients.NumberofRecipients;
					TotalRespondents += mAllRespondents.NumberofRespondents;

					EI_DestructKDTree (KDTree);
					KDTree = NULL;
					EI_FreeDonors (&AllDonors);
					AllDonors.Key = NULL;
					AllDonors.Value = NULL;
					AllDonors.DonorStatusFlag = NULL;
					EI_FreeTransformed (&AllTransformed);
					AllTransformed.TransformedValues = NULL;

					/* Free SystemMatchingFields */
					for (i = 0; i < SystemMatchingFields.NumberofFields; i++)
						STC_FreeMemory (SystemMatchingFields.FieldName[i]);
					STC_FreeMemory (SystemMatchingFields.FieldName);
					SystemMatchingFields.FieldName = NULL;

					for (i = 0; i < MatchingFields.NumberofFields; i++)
						STC_FreeMemory (MatchingFields.FieldName[i]);
					STC_FreeMemory (MatchingFields.FieldName);
					MatchingFields.FieldName = NULL;
				} /* enough "Donors" */
			} /* some "Recipients" */
		} /* some "valid observations" */

		InitCounter (&Info);
		AllRecipients.NumberofRecipients = 0;
		mAllRespondents.NumberofRespondents = 0;
		EI_DataTableEmpty (DataTable);

		if (VL_is_specified(&sp.dsr_indata.dsr.VL_by_var) > 0 && (ByGroupLoggingLevel != 1)) IO_PRINT_LINE ("");

		if (RandNumFlag) {
			ADTList_Empty(randnum_list);
			SList_Empty  (randkey_list);
		}
		rc_processing = ReadByGroup (&sp, &AllFields, &KeyDATA, ExclVarFlag, RandNumFlag, &ExclVarDATA, &RandNumDATA, InVarValue,
			&KeyINSTATUS, &FieldidINSTATUS, &StatusINSTATUS, DataTable, &Info,
			InStatusHasAllByVariables, randkey_list, randnum_list);
	}
	
	proc_exit_code = convert_processing_rc(rc_processing);
	if (proc_exit_code != BRC_SUCCESS) {
		goto error_cleanup;
	}


	TIME_CPU_STOPDIFF(processing);
	TIME_WALL_STOPDIFF(processing);

	IO_PRINT_LINE ("");
	if (VL_is_specified(&sp.dsr_indata.dsr.VL_by_var)) {
		IO_PRINT_LINE ("");
		PrintStatistics (TotalRespondents, TotalRecipients, &TotalInfo, RandomSelection);
		IO_PRINT_LINE ("");
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFooterAllByGroup SAS_NEWLINE);
	}
	IO_PRINT_LINE ("");

	mem_usage("Before SP_wrap");
	if (PRC_SUCCESS != SP_wrap(&sp)) {
		proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
	}
	mem_usage("After SP_wrap");

	TIME_WALL_BEGIN(cleanup);
	TIME_CPU_BEGIN(cleanup);

	goto normal_cleanup;

error_cleanup:
	if ((KDTree                          != NULL)
	) {
		EI_DestructKDTree  (KDTree         );
		KDTree = NULL;
	}
	if ((AllTransformed.TransformedValues != NULL )
	) {
		EI_FreeTransformed (&AllTransformed);
		AllTransformed.TransformedValues = NULL;
	}
	/* Free SystemMatchingFields */
	for (i = 0; i < SystemMatchingFields.NumberofFields; i++) {
		if ((SystemMatchingFields.FieldName    != NULL) &&
			(SystemMatchingFields.FieldName[i] != NULL)
		) {
			STC_FreeMemory (SystemMatchingFields.FieldName[i]);
			SystemMatchingFields.FieldName[i] = NULL;
		}
	}
	if ((SystemMatchingFields.FieldName   != NULL)
	) {
		STC_FreeMemory (SystemMatchingFields.FieldName);
		SystemMatchingFields.FieldName = NULL;
	}
	for (i = 0; i < MatchingFields.NumberofFields; i++) {
		if ((MatchingFields.FieldName          != NULL) &&
		    (MatchingFields.FieldName[i]       != NULL)
		) {
			STC_FreeMemory (MatchingFields.FieldName[i]);
			MatchingFields.FieldName[i] = NULL;
		}
	}
	if ((MatchingFields.FieldName         != NULL)
	) {
		STC_FreeMemory (MatchingFields.FieldName);
		MatchingFields.FieldName = NULL;
	}
normal_cleanup:
if ((AllDonors.Key != NULL) &&
(AllDonors.Value != NULL) &&
(AllDonors.DonorStatusFlag != NULL)
) {
	EI_FreeDonors(&AllDonors);
	AllDonors.Key = NULL;
	AllDonors.Value = NULL;
	AllDonors.DonorStatusFlag = NULL;
}
	/* free memory */
	if ((AllRecipients .Key                   != NULL) &&
	    (AllRecipients .RecipientData         != NULL)
	) {
		EI_FreeRecipients (&AllRecipients);
		AllRecipients.Key           = NULL;
		AllRecipients.RecipientData = NULL;
	}
	if ((mAllRespondents.Key                  != NULL) &&
	    (mAllRespondents.RespondentData       != NULL) &&
	    (mAllRespondents.RespondentExclusion  != NULL)
	) {
		EI_FreeRespondents (&mAllRespondents);
		mAllRespondents.Key                 = NULL;
		mAllRespondents.RespondentData      = NULL;
		mAllRespondents.RespondentExclusion = NULL;
	}
	if ((DataTable                            != NULL)
	) {
		EI_DataTableFree (DataTable);
		DataTable = NULL;
	}

	if (NumberOfMustMatchVars) {
		if ((MustMatchVariablesList               != NULL)
		) {
			STC_FreeMemory (MustMatchVariablesList);
			MustMatchVariablesList = NULL;
		}
		if ((MustMatchVariablesListToRead         != NULL)
		) {
			STC_FreeMemory (MustMatchVariablesListToRead);
			MustMatchVariablesListToRead = NULL;
		}
		for (i = 0; i < UserMatchingFields.NumberofFields; i++) {
			if ((UserMatchingFields.FieldName     != NULL) &&
			    (UserMatchingFields.FieldName [i] != NULL)
			) {
				STC_FreeMemory (UserMatchingFields.FieldName[i]);
				UserMatchingFields.FieldName[i] = NULL;
			}
		}
		if ((UserMatchingFields.FieldName     != NULL)
		) {
			STC_FreeMemory (UserMatchingFields.FieldName);
			UserMatchingFields.FieldName = NULL;
		}
	}

	for (i = 0; i < AllFields.NumberofFields; i++) {
		if ((AllFields.FieldName              != NULL) &&
		    (AllFields.FieldName          [i] != NULL)
		) {
			STC_FreeMemory (AllFields.FieldName[i]);
			AllFields.FieldName [i] = NULL;
		}
		if ((AllFieldsNameOriginal            != NULL) &&
		    (AllFieldsNameOriginal        [i] != NULL)
		) {
			STC_FreeMemory (AllFieldsNameOriginal[i]);
			AllFieldsNameOriginal[i] = NULL;
		}
	}
	if ((AllFields.FieldName              != NULL)
	) {
		STC_FreeMemory (AllFields.FieldName);
		AllFields.FieldName = NULL;
	}
	if ((AllFieldsNameOriginal            != NULL)
	) {
		STC_FreeMemory (AllFieldsNameOriginal);
		AllFieldsNameOriginal = NULL;
	}

	for (i = 0; i < Edits.NumberofColumns-1; i++) {
		if ((EditsOriginalFieldNames          != NULL) &&
		    (EditsOriginalFieldNames      [i] != NULL)
		) {
			STC_FreeMemory (EditsOriginalFieldNames[i]);
			EditsOriginalFieldNames[i] = NULL;
		}
	}
	if (EditsOriginalFieldNames          != NULL) {
		STC_FreeMemory (EditsOriginalFieldNames);
		EditsOriginalFieldNames = NULL;
	}

	if ((Edits    .FieldName              != NULL) &&
	    (Edits    .EditId                 != NULL) &&
	    (Edits    .Coefficient            != NULL)
	) {
		EI_EditsFree (&Edits);
		Edits.FieldName   = NULL;
		Edits.EditId      = NULL;
		Edits.Coefficient = NULL;
	}
	if ((PostEdits.FieldName              != NULL) &&
	    (PostEdits.EditId                 != NULL) &&
	    (PostEdits.Coefficient            != NULL)
	) {
		EI_EditsFree (&PostEdits);
		PostEdits.FieldName   = NULL;
		PostEdits.EditId      = NULL;
		PostEdits.Coefficient = NULL;
	}
	if ((EditVariablesList                != NULL)
	) {
		STC_FreeMemory (EditVariablesList);
		EditVariablesList = NULL;
	}

	if ((KeyDATA.Value                    != NULL)
	) {
		STC_FreeMemory (KeyDATA.Value);
		KeyDATA.Value = NULL;
	}
	if (ExclVarFlag) {
		if ((ExclVarDATA.Value                != NULL)
		) {
			STC_FreeMemory (ExclVarDATA.Value);
			ExclVarDATA.Value = NULL;
		}
	}

	if (RandNumFlag) {
		if ((RandNumDATA.Value                != NULL)
		) {
			STC_FreeMemory (RandNumDATA.Value);
			RandNumDATA.Value = NULL;
		}
		if ((randnum_list                     != NULL)
		) {
			ADTList_Delete(randnum_list);
			randnum_list = NULL;
		}
		if ((randkey_list                     != NULL)
		) {
			SList_Free    (randkey_list);
			randkey_list = NULL;
		}
	}

	if ((KeyINSTATUS.Value                != NULL)
	) {
		STC_FreeMemory (KeyINSTATUS.Value);
		KeyINSTATUS.Value = NULL;
	}
	if ((FieldidINSTATUS.Value            != NULL)
	) {
		STC_FreeMemory (FieldidINSTATUS.Value);
		FieldidINSTATUS.Value = NULL;
	}
	if ((StatusINSTATUS.Value             != NULL)
	) {
		STC_FreeMemory (StatusINSTATUS.Value);
		StatusINSTATUS.Value = NULL;
	}

	if ((InVarValue                       != NULL)
	) {
		STC_FreeMemory (InVarValue);
		InVarValue = NULL;
	}

	if ((UserData.DonorKey                != NULL)
	) {
		STC_FreeMemory (UserData.DonorKey);
		UserData.DonorKey = NULL;
	}
	if ((UserData.OutVarValue             != NULL)
	) {
		STC_FreeMemory (UserData.OutVarValue);
		UserData.OutVarValue = NULL;
	}
	if ((UserData.RecipientKey            != NULL)
	) {
		STC_FreeMemory (UserData.RecipientKey);
		UserData.RecipientKey = NULL;
	}
	if ((UserData.RespondentKey           != NULL)
	) {
		STC_FreeMemory (UserData.RespondentKey);
		UserData.RespondentKey = NULL;
	}

	if ((LPI_glpkpbtab_is_NULL()          != 1   )
	) {
		LpFreeEnv ();
	}
	if ((EI_mMessageList_Message_is_NULL()!= 1   )
	) {
		EI_FreeMessageList ();
		EI_mMessageList_Message_to_NULL();
	}
	/* free Statcan Procedureand children */
	mem_usage("Before SPG_free");
	SPG_free(&sp.spg);
	mem_usage("After SPG_free");

	/* TIME MEASUREMENT */
	TIME_CPU_STOPDIFF(cleanup);
	TIME_WALL_STOPDIFF(cleanup);
	TIME_CPU_STOPDIFF(main);
	TIME_WALL_STOPDIFF(main);

	deinit_runtime_env();

	return(proc_exit_code);
}

int SList_FindInsertionPointBinary (
    tSList * l,
    char * listItem)
{
    /*-finds (using binary search) the insertion point for the element "listItem" in the sorted list "l" that will keep the list sorted    */
    /*-SList_FindInsertionPointBinary(l, listItem) returns the index of the list element IMMEDIATELY FOLLOWING which it should be inserted */
    /* in order to keep the list sorted;                                                                                                   */
    /* if "listItem" is already in the list, the index returned is the index of the LAST occurrence of "listItem" (this means that if an   */
    /* item already ocurring one or more times in the sorted list is inserted immediately following the index returned by this function,   */
    /* it will be inserted at the END   of the run of (one or  more) ocurrences already in the list)                                       */
    char ** list;
    int low;
    int high;
    int mid;
    int rc;
    int rc0;
    int rc1;
    
    list = SList_TheList (l); /* list = l->l; */
    
    if (l->ne == 0) {
        /*-the list is empty, so listItem must be inserted at the start of the (empty) list: */
        return -1;
    }
    if (l->ne == 1) {
        rc = strcmp(listItem, list[0]);
        if   (rc <  0) {
            /*  listItem < list[0] */
            return -1;
        } else if (rc == 0) {
            /*  listItem == list[0] */
            return 0;
        } else { /* (rc > 0) */
            /* listItem >  list[0] */
            return 0;
        }
    }
    if (l->ne == 2) {
        rc0 = strcmp(listItem, list[0]);
        rc1 = strcmp(listItem, list[1]);
        if   (rc0 <  0) {
            /* listItem <  list[0] */
            if (rc1 < 0) {
                /* listItem <  list[0] and listItem <  list[1] */
                return -1;
            } else if (rc1 == 0) {
                /* listItem <  list[0] and listItem == list[1] (impossible because list is sorted) */
                return -2;
            } else { /* (rc1 > 0) */
                /* listItem <  list[0] and listItem >  list[1] (impossible because list is sorted) */
                return -2;
            }
        } else if (rc0 == 0) {
            /* listItem == list[0] */
            if (rc1 < 0) {
                /* listItem == list[0] and listItem <  list[1] */
                return 0;
            } else if (rc1 == 0) {
                /* listItem == list[0] and listItem == list[1] */
                return 1;
            } else { /* (rc1 > 0) */
                /* listItem == list[0] and listItem >  list[1] (impossible because list is sorted) */
                return -2;
            }
        } else { /* (rc0 >  0) */
            /* listItem >  list[0] */
            if (rc1 < 0) {
                /* listItem >  list[0] and listItem <  list[1] */
                return 0;
            } else if (rc1 == 0) {
                /* listItem >  list[0] and listItem == list[1] */
                return 1;
            } else { /* (rc1 > 0) */
                /* listItem >  list[0] and listItem >  list[1] */
                return 1;
            }
        }
    }
    /* l->ne >= 3: */
    low = 0;
    high = l->ne-1;
    
    /*-check whether "listItem" is greater than or equal to the last item in the list: */
    rc = strcmp(listItem, list[high]);
    if (rc >= 0) {
        /* listItem >= list[high]                                                              */
        /* so "listItem" can only be inserted immediately following the last item in the list: */
        return high;
    }
    /*-check whether "listItem" is either:                               */
    /*     less than first item in the list                              */
    /* or:                                                               */
    /*     equal to first item in the list but less than the second item */
    /* :                                                                 */
    rc = strcmp(listItem, list[0]);
    if (rc <= 0) {
        if (rc == 0) {
            /* listItem == list[0] */
            if (strcmp(listItem, list[1]) < 0) {
                /* listItem == list[0] and listItem < list[1],                                          */
                /* so list[0] == listItem < list[1],                                                    */
                /* so "listItem" can only be inserted immediately following the first item in the list: */
                return 0;
            }
        } else {
            /* listItem < list[0],                                                               */
            /* so "listItem" can only be inserted immediately before the first item in the list: */
            return (-1);
        }
    }
    /* (((list[low] < listItem) || (list[low] == listItem and list[low+1] == listItem)) and */
    /*  (listItem < list[high])                                                             */
    /* )                                                                                    */
    /* printf("list[low] <= listItem < list[high], and if list[low] == listItem then list[low+1] == listItem too"); */
    while (low <= high) {
        mid = (low + high) / 2;
        rc = strcmp(listItem, list[mid]);
        /* printf("[low, mid, high, rc] = [%d, %d, %d, %d]", low, mid, high, rc); */
        if (rc == 0) {
            /* listItem == list[mid] */
            if (((mid+1) == high) || (strcmp(listItem, list[mid+1]) < 0)) {
                /* listItem == list[mid] and listItem < list[mid+1]                                                     */
                /* (it's true when this loop starts and at every point during its execution that listItem < list[high], */
                /*  so if ((mid+1) == high) then listItem < list[mid+1]                                                 */
                /* ),                                                                                                   */
                /* so list[mid] == listItem < list[mid+1],                                                              */
                /* so "listItem" does occur in the list and list[mid] is the LAST occurrence of "listItem" in the list: */
                return mid;
            } else {
                /* listItem == list[mid] and listItem == list[mid+1]                                                                         */
                /* ("listItem" can't be greater than "list[mid+1]", because it equals "list[mid]" and the list is sorted in ascending order) */
                /* --the last occurrence in the list of "listItem" must be later in the list,                                                */
                /* so it must be between "mid" and "high", not between "low" and "mid",                                                      */
                /* so increase "low" to equal "mid" rather than decreasing "high" to equal "mid":                                            */
                low = mid;
            }
        } else if (rc > 0) {
            /* listItem > list[mid] */
            if (mid >= (high - 1)) {
                /* mid == high-1 or mid == high; but listItem < list[high], so mid == high-1                                            */
                /* so listItem > list[mid] and listItem < list[mid+1],                                                                  */
                /* so list[mid] < listItem < list[mid+1],                                                                               */
                /* so "listItem" doesn't occur in the list, and the only place where it could be inserted while keeping the list sorted */
                /* is immediately following the element at the index "mid":                                                             */
                return mid;
            }
            /* listItem > list[mid] (and mid < (high - 1), so the location where "listItem" should be inserted hasn't been completely narrowed down yet) */
            /* --the locations where any occurrences in the list of "listItem" would go must be after the element whose index is "mid",                  */
            /* so they must be between "mid" and "high", not between "low" and "mid",                                                                    */
            /* so increase "low" to equal "mid" rather than decreasing "high" to equal "mid":                                                            */
            low = mid; /* + 1 */
        } else { /* (rc < 0) */
            /* listItem < list[mid] */
            if (mid <= (low + 1)) {
                /* mid == low or mid == low+1; but list[low] <= listItem, so (since listItem < list[mid]) mid == low+1                  */
                /* so low == mid-1                                                                                                      */
                /* so listItem < list[mid] and listItem < list[mid-1],                                                                  */
                /* so list[mid-1] < listItem < list[mid],                                                                               */
                /* so "listItem" doesn't occur in the list, and the only place where it could be inserted while keeping the list sorted */
                /* is immediately following the element at the index "mid-1":                                                           */
                return mid-1;
            }
            /* listItem < list[mid] (and mid > (low + 1), so the location where "listItem" should be inserted hasn't been completely narrowed down yet) */
            /* --the locations where any occurrences in the list of "listItem" would go must be before the element whose index is "mid",                */
            /* so they must be between "low" and "mid", not between "mid" and "high",                                                                   */
            /* so decrease "high" to equal "mid" rather than increasing "low" to equal "mid":                                                           */
            high = mid; /* - 1 */
        }
    }
    /*-control should never reach here: */
    printf("error; control should never reach here (the last line of the function \"SList_FindInsertionPointBinary()\"");
    return -2;
}

int SList_InsertStringAfter (
    tSList * inlst,
    int insndx,
    char * elem)
{
    /*-insert the string "elem" in the list of strings "inlst" immediately following the element inlst[insndx] */
    /*-example call:                                                                                           */
    /*     SList_InsertStringAfter(["0", "1", "2", "3", "5", "6", "7", "8"], 3, "-4");                         */
    /*     #["0", "1", "2", "3", "-4", "5", "6", "7", "8"]                                                     */
    int rc;
    int i;
    char * temp_string;
    
    rc = SList_Add(elem, inlst);
    if (rc != eSListSucceed)
        return rc;
    
    temp_string = inlst->l[SList_NumEntries(inlst)-1];
    /* for ndx in range(len(inlst)-1, (insndx+1)-1, -1) */
    for (i = (SList_NumEntries(inlst)-1); i > ((insndx+1)-1); i = i-1) {
        inlst->l[i] = inlst->l[i - 1];
    }
    inlst->l[insndx + 1] = temp_string;
    return rc;
}

/**********************************************************************
 For RECIPIENT fields, if status is not FIELDFTI and it's a user
 matching field, reset the status to FIELDMFU.

 Potential modification:
   1. FIELDFTI -------------------> FIELDFTI (never changed)
   2. FIELDOK/FIELDFTE/
	  FIELDIMPUTED(no FIELDIDE) --> if "User Matching Field"
										changed to FIELDMFU
									else no changement.
**********************************************************************/
static void AdjustRecipientsStatusIfMFU (
	SP_donorimp* sp,
	EIT_ALL_RECIPIENTS * AllRecipients,
	int * MustMatchVariablesList,
	int NumberOfMustMatchVars)
{
	EIT_DATAREC * DataRec;
	int i;
	int * IsMustMatch;
	int j;
	int Position;

	IsMustMatch = NULL;
	DataRec = &AllRecipients->RecipientData[0];

	IsMustMatch = STC_AllocateMemory (DataRec->NumberofFields * sizeof *IsMustMatch);
	for (i = 0; i < DataRec->NumberofFields; i++) {
		DSR_get_pos_from_names(&sp->dsr_indata.dsr, 1, &DataRec->FieldName[i], &Position);
		IsMustMatch[i] = IsUserMatchingField (Position, MustMatchVariablesList, NumberOfMustMatchVars);
	}

	for (i = 0; i < AllRecipients->NumberofRecipients; i++) {
		DataRec = &AllRecipients->RecipientData[i];
		for (j = 0; j < DataRec->NumberofFields; j++) {
			if (DataRec->StatusFlag[j] != FIELDFTI && IsMustMatch[j])
				DataRec->StatusFlag[j] = FIELDMFU;
		}
	}
	STC_FreeMemory (IsMustMatch);
}

/******************************************
 Decide what size to give to DataTable
 and allocate memory.
******************************************/
static EIT_DATATABLE * AllocateDataTable (
	SP_donorimp* sp)
{
	long long NumberOfObservations = 0;

	/*
	Decide what size to give to DataTable:
	if NumberOfObservations is
	below 10000, use NumberOfObservations
	above 10000, use 10000
	if BY group specified, divide by 5
	*/
	NumberOfObservations = sp->dsr_indata.dsr.num_records;

	if (NumberOfObservations == 0) {
		/* Nothing to do */
	}
	else if (NumberOfObservations == -1)
		/* sequential file. set NumberOfObservations to an arbitrary number */
		NumberOfObservations = 1000;
	else if (NumberOfObservations <= 10000)
		NumberOfObservations = NumberOfObservations + 10;
	else
		NumberOfObservations = 10000;

	if (VL_is_specified(&sp->dsr_indata.dsr.VL_by_var))
		NumberOfObservations /= 5;

	return EI_DataTableAllocate ((int) NumberOfObservations);

}

/**************************************
 Allocate memory for Data Variables
**************************************/
static void AllocateDataVariables (
	char ** RespondentKey,
	int LengthKeyData,
	double ** InVarValue,
	int TotalNumberOfFields)
{
	*RespondentKey = STC_AllocateMemory (LengthKeyData+1);
	*InVarValue = STC_AllocateMemory (TotalNumberOfFields * sizeof **InVarValue);
}

/***************************************
 Allocate memory for Write Variables
***************************************/
static void AllocateWriteVariables (
	char ** RecipientKey,
	double ** OutVarValue,
	int EditsNumberOfFields,
	int LengthKeyData)
{
	*RecipientKey = STC_AllocateMemory (LengthKeyData+1);
	*OutVarValue = STC_AllocateMemory (EditsNumberOfFields * sizeof **OutVarValue);
}

/**********************************************************************
 Build a list containing the numbers of the variables corresponding
 to the EDITS fields that can be found on the data set pointed to by
 InData.
 Validates that the variables exist and are numeric.
**********************************************************************/
static EIT_RETURNCODE BuildNumericList (
	DSR_generic* dsr,
	char ** FieldName,
	int NumberOfFields,
	int * EditVariablesList)
{
	int i;
	int found;
	long VariableType;

	for (i = 0; i < NumberOfFields; i++) {
		if (IORC_VARLIST_NOT_FOUND == DSR_get_pos_from_names(dsr, 1, &FieldName[i], &found)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotInDataSet SAS_NEWLINE, FieldName[i], dsr->dataset_name);
			return EIE_FAIL;
		}
		else {
			EditVariablesList[i] = found;
			VariableType = DSR_get_col_type(dsr, EditVariablesList[i]);
			if (VariableType != IOVT_NUM) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotNumericInDataSet, FieldName[i], dsr->dataset_name);
				return EIE_FAIL;
			}
		}
	}

	return EIE_SUCCEED;
}
/*************************************************************
 Check that a VariableName exists in INSTATUS
 and it's a character variable.
 Fill the corresponding structure with it's position and name.
**************************************************************/
static EIT_RETURNCODE CheckCharVarInStatus (
	DSR_generic* dsr,
	char * VariableName,
	T_CHAR_VARIABLE * Variable)
{
	EIT_RETURNCODE rc = EIE_SUCCEED;
	int VariableType;

	IOUtil_copy_varname(Variable->Name, VariableName);
	if (IORC_VARLIST_NOT_FOUND == DSR_get_pos_from_names(dsr, 1, &VariableName, &Variable->Position)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet,
			Variable->Name, dsr->dataset_name);
		rc = EIE_FAIL;
	}
	else {
		VariableType = DSR_get_col_type(dsr, Variable->Position);
		if (VariableType != IOVT_CHAR) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet,
				Variable->Name, dsr->dataset_name);
			rc = EIE_FAIL;
		}
	}

	return rc;
}

/****************************************************************************
 Removes variables from MustMatchVariablesList that are in EditVariablesList.
 The MustMatchVariablesList contain the position in the SAS dataset of the
 variable to read. If it is not cleaned, we will read more data than necessary.

 Each list contains distinct values.
****************************************************************************/
static void CleanMustMatchVariables (
	int * EditVariablesList,
	int EditsNumberOfFields,
	int * MustMatchVariablesListToRead,
	int * NumberOfMustMatchVarsToRead)
{
	int i;
	int insertion;
	int j;

	/* find the duplicates. set them to -1 */
	for (i = 0; i < *NumberOfMustMatchVarsToRead; i++) {
		for (j = 0; j < EditsNumberOfFields; j++) {
			if (MustMatchVariablesListToRead[i] == EditVariablesList[j]) {
				MustMatchVariablesListToRead[i] = -1;
				j = EditsNumberOfFields; /* go to the next must match */
			}
		}
	}

	/* remove the duplicates */
	insertion = 0;
	for (i = 0; i < *NumberOfMustMatchVarsToRead; i++) {
		if (MustMatchVariablesListToRead[i] != -1) {
			MustMatchVariablesListToRead[insertion++] =
				MustMatchVariablesListToRead[i];
		}
	}
	*NumberOfMustMatchVarsToRead = insertion;
}

/***********************************************
Copy Respondent Data to Recipient Variables
***********************************************/
static void CopyAllRespondentsToAllRecipients (
	EIT_ALL_RESPONDENTS * AllRespondents,
	EIT_ALL_RECIPIENTS * AllRecipients,
	int EditsNumberOfFields) /* number of variables in edits */
{
	int i; /* respondents index */
	int IdxRecipient;
	int isRecipient; /* Respondent is a "recipient" or not. */

/*    if (DEBUG) EI_RespondentsPrint (AllRespondents);*/

	IdxRecipient = 0;
	for (i = 0; i < AllRespondents->NumberofRespondents; i++) {

		isRecipient = IsRecipient (EditsNumberOfFields,
			AllRespondents->RespondentData[i].StatusFlag);

		if (isRecipient == 1) {
			if (AllRecipients->NumberofRecipients ==
					AllRecipients->NumberofAllocatedRecipients) {
				EI_AllocateRecipients (AllRecipients);
			}

			AllRecipients->RecipientData[IdxRecipient].NumberofFields =
				AllRespondents->RespondentData[i].NumberofFields;
			AllRecipients->RecipientData[IdxRecipient].FieldName =
				AllRespondents->RespondentData[i].FieldName;
			AllRecipients->RecipientData[IdxRecipient].FieldValue =
				AllRespondents->RespondentData[i].FieldValue;
			AllRecipients->RecipientData[IdxRecipient].StatusFlag =
				AllRespondents->RespondentData[i].StatusFlag;
			AllRecipients->Key[IdxRecipient] = AllRespondents->Key[i];

			AllRecipients->NumberofRecipients++;
			IdxRecipient++;
		}
	}
/*    if (DEBUG) EI_RecipientsPrint (AllRecipients);*/
}

/***********************************************
Copy "DATATABLE" to "AllRespondents"
************************************************/
static void CopyDataTableToAllRespondents (
	EIT_DATATABLE * DataTable,
	EIT_ALL_RESPONDENTS * AllRespondents)
{
	EIT_DATA * Data;
	EIT_DATAREC * DataRec;
	int i;
	int IdxResp;

	IdxResp = 0;
	for (i = 0; i < DataTable->Size ; i++) {
		for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {

			DataRec = Data->DataRec[EIE_PERIOD_CURRENT];

			if (AllRespondents->NumberofRespondents == AllRespondents->NumberofAllocatedRespondents) {
				EI_AllocateRespondents (AllRespondents);
			}

			AllRespondents->Key[IdxResp] = Data->Key;
			AllRespondents->RespondentData[IdxResp].NumberofFields = DataRec->NumberofFields;
			AllRespondents->RespondentData[IdxResp].FieldName = DataRec->FieldName;
			AllRespondents->RespondentData[IdxResp].FieldValue = DataRec->FieldValue;
			AllRespondents->RespondentData[IdxResp].StatusFlag = DataRec->StatusFlag;

			AllRespondents->RespondentExclusion[IdxResp] = Data->ExcludedObs[EIE_PERIOD_CURRENT];

			/* Increment ... */
			AllRespondents->NumberofRespondents++;
			IdxResp++;
		} /* for (Data=DataTable->Data[i];...;...) */
	} /* for (i = 0;...;...;) */
}

/**********************************************************************
Check if a donor is non eligible.
A donor is non eligible if at least one field is flagged FIELDIMPUTED.
Called if ELIGDON = ORIGINAL (dft value)

Return: EIE_TRUE --> Data must be removed (non eligible donor)
		EIE_FALSE -> Data must be kept (eligible donor)
**********************************************************************/
static EIT_BOOLEAN DataNonEligible (
	EIT_DATAREC * DataRec)
{
	int i;
	EIT_BOOLEAN ToRemove;

	ToRemove = EIE_FALSE;

	for (i = 0; i < DataRec->NumberofFields; i++) {
		if (DataRec->StatusFlag[i] == FIELDIMPUTED) {
			ToRemove = EIE_TRUE;
			break;
		}
	}

	return ToRemove;
}

/********************************************************
 Check if data is a mixed record.
 To be mixed, a record must have:
 - all edits variables not flagged FTI
 - at least one user matching fields outside the
   edits is FTI
 Return: EIE_TRUE --> Data must be removed
		 EIE_FALSE -> Data must be kept
********************************************************/
static EIT_BOOLEAN DataMixed (
	EIT_DATAREC * DataRec,
	int EditsNumberOfFields)
{
	int i;
	EIT_BOOLEAN ToRemove;

	ToRemove  = EIE_FALSE;

	for (i = EditsNumberOfFields; i < DataRec->NumberofFields; i++) {
		if (DataRec->StatusFlag[i] == FIELDFTI) {
			ToRemove = EIE_TRUE;
			i = DataRec->NumberofFields; /* skip remaining */
		}
	}

	if (ToRemove == EIE_TRUE) {
		for (i = 0; i < EditsNumberOfFields; i++) {
			if (DataRec->StatusFlag[i] == FIELDFTI) {
				ToRemove = EIE_FALSE;
				i = DataRec->NumberofFields; /* skip remaining */
			}
		}
	}

	return ToRemove;
}

/********************************************************
Check if at least one missing field is unflagged as FTI.
Return: EIE_TRUE --> Data must be removed
		EIE_FALSE -> Data must be kept
********************************************************/
static EIT_BOOLEAN DataUnflaggedMissing (
	EIT_DATAREC * DataRec)
{
	int i;
	EIT_BOOLEAN ToRemove;

	ToRemove = EIE_FALSE;

	for (i = 0; i < DataRec->NumberofFields; i++) {
		if ((DataRec->StatusFlag[i] != FIELDFTI) && (DataRec->FieldValue[i] == EIM_MISSING_VALUE)) {
			ToRemove = EIE_TRUE;
			i = DataRec->NumberofFields; /* skip remaining */
		}
	}

	return ToRemove;
}

/********************************************************
Check if at least one negative field is unflagged as FTI.
Return: EIE_TRUE --> Data must be removed
		EIE_FALSE -> Data must be kept
This check is relevant only if user has set positivity
option to REJECTNEGATIVE (explicitely or by default)
********************************************************/
static EIT_BOOLEAN DataUnflaggedNegative (
	EIT_DATAREC * DataRec)
{
	int i;
	EIT_BOOLEAN ToRemove;

	ToRemove = EIE_FALSE;

	for (i = 0; i < DataRec->NumberofFields; i++) {
		if ((DataRec->StatusFlag[i] != FIELDFTI) && (DataRec->FieldValue[i] < 0.0)) {
			ToRemove = EIE_TRUE;
			i = DataRec->NumberofFields; /* skip remaining */
		}
	}

	return ToRemove;
}

/*******************************************
 Define Gather Write for new Observation
*******************************************/
static PROC_RETURN_CODE DefineGatherWriteData (
	DSW_generic* dsw,
	T_CHAR_VARIABLE KeyDATA,
	char * RecipientKey,
	char ** FieldName, /* original names */
	int NumberOfFields, /* in Edits */
	double * OutVarValue)
{
	if (IORC_SUCCESS != DSW_allocate(dsw, NumberOfFields + 1)) {
		return PRC_FAIL_WRITE_DATA;
	}

	int var_added = 0;

	/* Define gather write for the key */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, KeyDATA.Name, (void*)RecipientKey, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* Define gather write for the numeric variables */
	for (int i = NumberOfFields-1; i >= 0; i--) {
		if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, FieldName[i], (void*)(OutVarValue + i), IOVT_NUM)) {
			return PRC_FAIL_WRITE_DATA;
		}
	}

	if (IORC_SUCCESS != DSW_start_appending(dsw)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}

/******************************************
 Define gather write for OUTDONORMAPSET
******************************************/
static PROC_RETURN_CODE DefineGatherWriteDonorMap (
	DSW_generic* dsw,
	char * RespondentKey,
	char * DonorKey,
	double * NumberOfAttempts,
	double * DonorLimit)
{
	if (IORC_SUCCESS != DSW_allocate(dsw, NB_VARS_DONORMAP)) {
		return PRC_FAIL_WRITE_DATA;
	}

	int var_added = 0;

	/* For Recipient Key */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_RECIPIENT, (void*)RespondentKey, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* For Donor Key */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_DONOR, (void*)DonorKey, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* For Number of attempts */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_NB_ATTEMPTS, (void*)NumberOfAttempts, IOVT_NUM)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* For Donor Limit */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_DONORLIMIT, (void*)DonorLimit, IOVT_NUM)) {
		return PRC_FAIL_WRITE_DATA;
	}

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}

/****************************************
 Define gather write for OUTSTATUSSET
****************************************/
static PROC_RETURN_CODE DefineGatherWriteStatus (
	DSW_generic* dsw,
	T_CHAR_VARIABLE KeyDATA,
	T_WRITEINFO* UserData)
{
	return banff_setup_dsw_outstatus(
		dsw,
		KeyDATA.Name,
		(void*)UserData->RespondentKey,
		(void*)UserData->OutFieldid,
		(void*)UserData->OutStatus,
		(void*)&UserData->FieldValue
	);
}

/****************************************
 Define gather write for outmatching_fields
****************************************/
static PROC_RETURN_CODE DefineGatherWriteMatchingFields (
	DSW_generic* dsw,
	T_CHAR_VARIABLE KeyDATA,
	char * RespondentKey,
	char * OutFieldid,
	char * OutStatus)
{
	if(dsw->is_requested == IOB_FALSE){
		if (IORC_SUCCESS != DSW_allocate(dsw, 0)) {
			return PRC_FAIL_WRITE_DATA;
		}
		return PRC_SUCCESS;
	}

	if (IORC_SUCCESS != DSW_allocate(dsw, NB_VARS_OUTMATCHING_FIELDS)) {
		return PRC_FAIL_WRITE_DATA;
	}

	int var_added = 0;

	/* For Recipient Key */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, KeyDATA.Name, (void*)RespondentKey, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* For others variables */
	/* For FieldID */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DSC_FIELD_ID, (void*)OutFieldid, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* For Status */
	if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DSC_STATUS, (void*)OutStatus, IOVT_CHAR)) {
		return PRC_FAIL_WRITE_DATA;
	}

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}

/**************************************
 Define scatter read for InData
**************************************/
static EIT_RETURNCODE DefineScatterReadData (
	DSR_indata* dsr_indata,
	int NumberOfFields, /* AllFields: Edits + MustMatch */
	int NumberOfEditsFields,
	T_CHAR_VARIABLE * KeyDATA,
	int ExclVarFlag,
	int RandNumFlag,
	T_CHAR_VARIABLE * ExclVarDATA,
	T_CHAR_VARIABLE * RandNumDATA,
	int * EditVariablesList,
	double * InVarValue)
{
	int i;
	int j;
	int NumberOfVars;

	DSR_generic* dsr = &dsr_indata->dsr;

	NumberOfVars = NumberOfFields + 1;/* +1 for Id (mandatory) */
	if (ExclVarFlag)
		NumberOfVars++; /* +1 for DATAEXCLVAR (optional) */
	if (RandNumFlag)
		NumberOfVars++; /* +1 for DATARANDNUMVAR (optional) */

	/* Define key variable for scatter read */
	dsr->VL_unit_id.ptrs[0] = KeyDATA->Value;

	/* Define DATAEXCLVAR variable for scatter read */
	if (ExclVarFlag) {
		dsr_indata->VL_data_excl.ptrs[0] = ExclVarDATA->Value;
	}

	/* Define DATARANDNUMVAR variable for scatter read */
	if (RandNumFlag) {
		dsr_indata->VL_rand_num.ptrs[0] = RandNumDATA->Value;
	}

	/* initialize in var varlist with edits fields */
	IO_RETURN_CODE rc_vl_init = VL_init_from_position_list(&dsr->VL_in_var, dsr, VL_NAME_in_var, IOVT_NUM, NumberOfEditsFields, EditVariablesList);
	if (IORC_SUCCESS != rc_vl_init) {
		return EIE_FAIL;
	}
	/* Define edit variables for scatter read */
	for (i = 0; i < NumberOfEditsFields; i++) {
		dsr->VL_in_var.ptrs[i] = (char*)(InVarValue + i);
	}

	/* Define must match variables for scatter read */
	/* Do this only if there are must match fields  */
	if (NumberOfFields > NumberOfEditsFields) {
		for (i = 0; i < NumberOfFields-NumberOfEditsFields; i++) {
			j = i+NumberOfEditsFields;
			dsr_indata->VL_must_match.ptrs[i] = (char*)(InVarValue + j);
		}
	}


	return EIE_SUCCEED;
}

/******************************************************
 Define scatter read for INSTATUSSET

 (Executes after function "DefineVarsINSTATUS",
  so we got information about length and position
  of variables.)
******************************************************/
static EIT_RETURNCODE DefineScatterReadStatus (
	DSR_instatus* dsr,
	T_CHAR_VARIABLE * KeyINSTATUS,
	T_CHAR_VARIABLE * FieldidINSTATUS,
	T_CHAR_VARIABLE * StatusINSTATUS)
{
	banff_setup_dsr_instatus(
		dsr,
		(void*)KeyINSTATUS->Value,
		(void*)FieldidINSTATUS->Value,
		(void*)StatusINSTATUS->Value
	);

	return EIE_SUCCEED;
}

/*******************************************************
 Check/fill structures for KeyINSTATUS, FieldidINSTATUS
 and StatusINSTATUS with their characteristics and
 allocate memory for their value.
*******************************************************/
static EIT_RETURNCODE DefineVarsINSTATUS (
	DSR_generic* dsr,
	T_CHAR_VARIABLE KeyDATA,
	T_CHAR_VARIABLE * KeyINSTATUS,
	T_CHAR_VARIABLE * FieldidINSTATUS,
	T_CHAR_VARIABLE * StatusINSTATUS)
{
	EIT_RETURNCODE rc = EIE_SUCCEED;

	KeyINSTATUS->Value = NULL;
	FieldidINSTATUS->Value = NULL;
	StatusINSTATUS->Value = NULL;

	/* Var: "key" (idem to KeyDATA.Name) */
	if (CheckCharVarInStatus(dsr, KeyDATA.Name, KeyINSTATUS) == EIE_FAIL)
		rc = EIE_FAIL;
	else {
		KeyINSTATUS->Size = DSR_get_column_length(dsr, KeyINSTATUS->Position);
		if (KeyINSTATUS->Size < 0) {	//indicates an error occurred
			rc = EIE_FAIL;
		}else{
			KeyINSTATUS->Value = STC_AllocateMemory (KeyINSTATUS->Size+1);
		}
	}

	/* Var: FIELDID */
	if (CheckCharVarInStatus (dsr, DSC_FIELD_ID, FieldidINSTATUS) == EIE_FAIL)
		rc = EIE_FAIL;
	else {
		FieldidINSTATUS->Size = DSR_get_column_length(dsr, FieldidINSTATUS->Position);
		if (FieldidINSTATUS->Size < 0) {	//indicates an error occurred
			rc = EIE_FAIL;
		} else {
			FieldidINSTATUS->Value = STC_AllocateMemory(FieldidINSTATUS->Size + 1);
		}
	}

	/* Var: STATUS */
	if (CheckCharVarInStatus (dsr, DSC_STATUS, StatusINSTATUS) == EIE_FAIL)
		rc = EIE_FAIL;
	else {
		StatusINSTATUS->Size = DSR_get_column_length(dsr, StatusINSTATUS->Position);
		if (StatusINSTATUS->Size < 0) {		//indicates an error occurred
			rc = EIE_FAIL;
		} else {
			StatusINSTATUS->Value = STC_AllocateMemory(StatusINSTATUS->Size + 1);
		}
	}

	/* A variable is not in INSTATUS or  */
	/* wrong type: free memory.          */
	if (rc == EIE_FAIL) {
		if(KeyINSTATUS->Value != NULL)
			STC_FreeMemory (KeyINSTATUS->Value);
		if (FieldidINSTATUS->Value != NULL)
			STC_FreeMemory (FieldidINSTATUS->Value);
		if (StatusINSTATUS->Value != NULL)
			STC_FreeMemory (StatusINSTATUS->Value);
		KeyINSTATUS->Value = NULL;
		FieldidINSTATUS->Value = NULL;
		StatusINSTATUS->Value = NULL;
	}

	return rc;
}

/**********************************************************************
 Finds a donor (hopefully!) for every recipients.
 Calls EI_FindNearestDonors() when a recipient has matching fields and
 try them, one after one, selecting the one with smallest distance,
 until recipient pass post edits.
 Calls EI_RandomDonorWithPostEdits() when a recipient does not have
 matching field and "RandomSelection".
**********************************************************************/
static int DoImputation (
	SP_donorimp* sp,
	EIT_ALL_RESPONDENTS * AllRespondents,
	EIT_ALL_RECIPIENTS * AllRecipients,
	EIT_TRANSFORMED * AllTransformed,
	EIT_ALL_DONORS * AllDonors,
	EIT_FIELDNAMES * MatchingFields,
	int NumberOfColumnsInEdits, /* Number of fields + constant term */
	EIT_EDITS * PostEdits,
	int NumberOfDonorsToFind,
	int DonorLimit,
	int RandomSelection,
	EIT_KDTREE * KDTree,
	T_INFO * Info,
	T_WRITEINFO * UserData,
	char ** AllFieldsNameOriginal,
	int RandNumFlag,
	tSList   * randkey_list,
	tADTList * randnum_list,
	int LoggingVerbosityLevel)
{
	EIT_ONERECIPIENTALLOCATE_RETURNCODE ALLOCATEFTI_rcode;
	int DonorRow;
	EIT_EDITSAPPLY_RETURNCODE EDITSAPPLYrcode;
	int * FTIFields = NULL;
	int i;
	int j;
	int index_for_debugging_statements;
	EIT_NEARESTDONORS * NearestDonors = { 0 };
	EIT_FINDNEARESTDONORS_RETURNCODE NEARESTDONORSrcode;
	EIT_BOOLEAN notPassedPostEdits;
	EIT_RANDOMDONOR_RETURNCODE RANDOMDONORrcode;

	PROC_RETURN_CODE rc_write = PRC_FAIL_UNHANDLED;
	
	int rc;
	
	rc = 0;

	ALLOCATEFTI_rcode = EI_OneRecipient_AllocateFTIFields (&FTIFields, NumberOfColumnsInEdits);

	for (i = 0; i < AllRecipients->NumberofRecipients; i++) {

		EI_OneRecipient_GetFTIFields (FTIFields, NumberOfColumnsInEdits,
			&AllRecipients->RecipientData[i]);

		if (EI_OneRecipient_HasMatchingFields_AllTypes (&AllRecipients->RecipientData[i])) {

			/* recipient has matching fields:
			try to find a donor from the nearest donors */

			Info->WithMatchingFields++;
			/* find nearest donors */
			NEARESTDONORSrcode = EI_FindNearestDonors (NumberOfDonorsToFind,
				&AllRecipients->RecipientData[i], AllRecipients->Key[i],
				FTIFields, MatchingFields, AllTransformed, AllDonors,
				KDTree, &NearestDonors, DonorLimit);
			EI_PrintMessages ();

			switch (NEARESTDONORSrcode) {
			case EIE_FINDNEARESTDONORS_SUCCEED:
				/* try nearest donor(s) until recipient pass post edits */
				notPassedPostEdits = EIE_TRUE;
				j = 0;
				DonorRow = -1;
				if (LoggingVerbosityLevel > 0) {
					IO_PRINT_LINE("distances from recipient \"%s\" of its nearest donors = {", AllRecipients->Key[i]);
					for (index_for_debugging_statements = 0;
					     index_for_debugging_statements < NearestDonors->NumberofDonors;
					     index_for_debugging_statements = index_for_debugging_statements + 1) {
						IO_PRINT_LINE("\"%s\": %f", (AllDonors->Key)[NearestDonors->DonorsIndex[index_for_debugging_statements]], NearestDonors->Distances[index_for_debugging_statements]);
					}
					IO_PRINT_LINE("}");
					/* IO_PRINT_LINE("(j, NearestDonors->NumberofDonors, notPassedPostEdits)=(%d, %d, %d)", j, NearestDonors->NumberofDonors, notPassedPostEdits); */
				}
				while ((j < NearestDonors->NumberofDonors) && notPassedPostEdits) {
					/* IO_PRINT_LINE("j=%d", j); */
					if (RandNumFlag) {
						DonorRow = EI_OneRecipient_FindNextNearestDnAlt (j, NearestDonors, AllDonors, randkey_list, randnum_list, AllRecipients->Key[i]);
						if (DonorRow < 0) {
							/*-an error occurred in the call to "EI_OneRecipient_FindNextNearestDnAlt()": */
							rc = -1;
							goto error_cleanup;
						}
					} else {
						DonorRow = EI_OneRecipient_FindNextNearestDonor (j, NearestDonors);
					}
					if (LoggingVerbosityLevel > 0) {
						IO_PRINT_LINE("next donor considered for recipient \"%s\" (from among nearest donors): \"%s\"", AllRecipients->Key[i], (AllDonors->Key)[NearestDonors->DonorsIndex[j]]);
						/* IO_PRINT_LINE("next donor considered for recipient \"%s\" (from among nearest donors): \"%s\"", AllRecipients->Key[i], (AllDonors->Key)[DonorRow]); */
					}
					EI_OneRecipient_CopyDonorValues (DonorRow, AllDonors->Key,
						AllRespondents, FTIFields, &AllRecipients->RecipientData[i]);
					EDITSAPPLYrcode = EI_EditsApplyOnValues (PostEdits,
						AllRecipients->RecipientData[i].FieldValue);

					if (EDITSAPPLYrcode == EIE_EDITSAPPLY_SUCCEED) {
						/* the donor fits */

						AllDonors->Gave[DonorRow]++;

						rc_write = WriteResults (sp, UserData, &AllRecipients->RecipientData[i],
							AllRecipients->Key[i], AllDonors->Key[DonorRow], j+1, DonorLimit,
							AllFieldsNameOriginal);
						if (rc_write != PRC_SUCCESS) {
							rc = -1;
							goto error_cleanup;
						}
						Info->WithMatchingFieldsImputed++;
						notPassedPostEdits = EIE_FALSE;
					}
					else
						/* the donor doesn't fit */
						j++;
				}

				/* none "nearest donors" fits */
				if (notPassedPostEdits == EIE_TRUE)
					Info->WithMatchingFieldsNotImputed++;
				EI_OneRecipient_FreeNearestDonors (NearestDonors);
				NearestDonors = NULL;

				break;

			case EIE_FINDNEARESTDONORS_NOTFOUND:
				/* no donor */
				Info->WithMatchingFieldsNotImputed++;
				break;

			default:
				rc = -1;
				goto error_cleanup;
			}
		}
		else {
			/* recipient has no matching field:
			try to find a donor randomly */

			Info->WithoutMatchingFields++;
			if (RandomSelection == 1) {
				DonorRow = -1;
				RANDOMDONORrcode = EI_RandomDonorWithPostEdits (
					&AllRecipients->RecipientData[i], FTIFields, AllRespondents,
					PostEdits, AllDonors, &DonorRow, DonorLimit, RandNumFlag, randkey_list, randnum_list, AllRecipients->Key[i], LoggingVerbosityLevel);
				EI_PrintMessages ();

				switch (RANDOMDONORrcode) {
				case EIE_RANDOMDONOR_SUCCEED:
					/* the random donor fits */
					AllDonors->Gave[DonorRow]++;
					rc_write = WriteResults (sp, UserData, &AllRecipients->RecipientData[i],
						AllRecipients->Key[i], AllDonors->Key[DonorRow], 0, DonorLimit,
						AllFieldsNameOriginal);
					if (rc_write != PRC_SUCCESS) {
						rc = -1;
						goto error_cleanup;
					}
					Info->WithoutMatchingFieldsImputed++;
					break;

				case EIE_RANDOMDONOR_NOTFOUND:
					Info->WithoutMatchingFieldsNotImputed++;
					break;

				default: /* case EIE_RANDOMDONOR_FAIL */
					rc = -1;
					goto error_cleanup;
				}
			}
		} /* recipient has no matching field */
	}
	goto normal_cleanup;

error_cleanup:
	
normal_cleanup:
	if (NearestDonors != NULL && NearestDonors->Distances != NULL && NearestDonors->DonorsIndex != NULL) {
		EI_OneRecipient_FreeNearestDonors(NearestDonors);
	}
	
	if ((FTIFields != NULL)
	) {
		EI_OneRecipient_FreeFTIFields (FTIFields);
		FTIFields = NULL;
	}
	return rc;
}

/**********************************************************************
 Create the union of Array1 with Array2. Duplicates are eliminated.
 Array1 must be sorted in descending alphabetical order.
**********************************************************************/
static void EliminateCommonFields (
	char ** Array1,
	int NumberOfFieldsinArray1,
	char ** Array2,
	int NumberOfFieldsinArray2,
	EIT_FIELDNAMES * NewArray)
{
	int i;
	int rc;

	/* First copy all the fields from Array1 to NewArray */
	NewArray->FieldName = STC_AllocateMemory (
		(NumberOfFieldsinArray1+NumberOfFieldsinArray2) *
			sizeof *NewArray->FieldName);
	NewArray->NumberofFields = NumberOfFieldsinArray1;

	for (i = 0; i < NewArray->NumberofFields; i++) {
		NewArray->FieldName[i] = STC_StrDup (Array1[i]);
	}

	/* Then look for fields that are in Array2 but not in Array1 */
	/* and copy them to NewArray */
	for (i = 0; i < NumberOfFieldsinArray2; i++) {
		rc = UTIL_Binary_Reverse (Array2[i], Array1, NumberOfFieldsinArray1);
		if (rc == -1) { /* Field in Array2 not found in Array1 */
			NewArray->FieldName[NewArray->NumberofFields] =
				STC_StrDup (Array2[i]);
			NewArray->NumberofFields++;
		}
	}

/*    UTIL_PrintStrings (NewArray->FieldName, NewArray->NumberofFields);*/
}

/***********************************************************
Verify variables exclusivity between lists via position
of variables on InData:
  - BY-list of num variables vs EDITS
  - BY-list of num variables vs MUSTMATCH
  - ID vs BY-list of char variables
  - (if ExclVarFlag): EXCLVAR vs ID
					  EXCLVAR vs BY-list of char variables
Note: EDITS and MUSTMATCH are not mutually exclusive.
***********************************************************/
static EIT_RETURNCODE ExclusivityBetweenLists (
	DSR_generic* dsr,
	int * EditVariables,
	int EditNumberOfVar,
	int * MustMatchVariables,
	int MustMatchNumberOfVar,
	int IdPosition,
	int ExclVarFlag,
	int RandNumFlag,
	int ExclVarPosition,
	int RandNumPosition)
{
	int * ByList = { 0 };
	int * ByListTypeChar;
	int * ByListTypeNum;
	int found;
	int i;
	int j;
	int k;
	int NumberOfByVars;
	int NumberOfByVarsChar;
	int NumberOfByVarsNum;
	EIT_RETURNCODE rc;
	long VariableType;

	rc = EIE_SUCCEED;
	ByListTypeChar = NULL;
	ByListTypeNum = NULL;
	NumberOfByVarsNum  = 0;
	NumberOfByVarsChar = 0;

	/* If any BY-variables, identify their position */
	NumberOfByVars = dsr->VL_by_var.count;
	if (NumberOfByVars != 0)
		ByList = dsr->VL_by_var.positions;

	/* BY-variables: build array of position for num variables  */
	/*                     array of position for char variables */
	for (i = 0; i < NumberOfByVars; i++) {
		VariableType = DSR_get_col_type(dsr, ByList[i]);
		if (VariableType == IOVT_NUM)
			NumberOfByVarsNum++;
		else if (VariableType == IOVT_CHAR)
			NumberOfByVarsChar++;
		else
			return EIE_FAIL;
	}

	ByListTypeNum  = STC_AllocateMemory (NumberOfByVarsNum  * sizeof *ByListTypeNum);
	ByListTypeChar = STC_AllocateMemory (NumberOfByVarsChar * sizeof *ByListTypeChar);

	j=0;
	k=0;
	for (i = 0; i < NumberOfByVars; i++) {
		VariableType = DSR_get_col_type(dsr, ByList[i]);
		if (VariableType == IOVT_NUM) {
			ByListTypeNum[j] = ByList[i];
			j++;
		}
		else if (VariableType == IOVT_CHAR) {
			ByListTypeChar[k] = ByList[i];
			k++;
		}
		else {
			return EIE_FAIL;
		}
	}

	/* ByListTypeChar vs Id */
	i = 0;
	while (i < NumberOfByVarsChar) {
		if (ByListTypeChar[i] == IdPosition) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_UNIT_ID, BPN_BY);
			rc = EIE_FAIL;
			break;
		}
		i++;
	}

	/* If any ExclVar: ExclVar vs Id             */
	/*                 ExclVar vs ByListTypeChar */
	if (ExclVarFlag) {
		if (IdPosition == ExclVarPosition) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_UNIT_ID, BPN_DATA_EXCL_VAR);
			rc = EIE_FAIL;
		}

		i = 0;
		while (i < NumberOfByVarsChar) {
			if (ByListTypeChar[i] == ExclVarPosition) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_DATA_EXCL_VAR,
					BPN_BY);
				rc = EIE_FAIL;
				break;
			}
			i++;
		}
	}

	/* If any RandNum: RandNum vs ByListTypeChar */
	if (RandNumFlag) {
		i = 0;
		while (i < NumberOfByVarsNum) {
			if (ByListTypeNum[i] == RandNumPosition) {
				/* IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsRandomNum, "DATARANDNUMVAR", BPN_BY); */ /* <-- have to add new message, then uncomment this */
				rc = EIE_FAIL;
				break;
			}
			i++;
		}
	}

	/* ByListTypeNum vs Edits */
	found = 0;
	for (i = 0; i < NumberOfByVarsNum; i++) {
		for (j=0; j < EditNumberOfVar; j++) {
			if (ByListTypeNum[i] == EditVariables[j]) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_EDITS, BPN_BY);
				rc = EIE_FAIL;
				found = 1;
				break;
			}
		}
		if (found)
			break;
	}

	/* ByListTypeNum vs MustMatchVars */
	found = 0;
	for (i = 0; i < NumberOfByVarsNum; i++) {
		for (j=0; j < MustMatchNumberOfVar; j++) {
			if (ByListTypeNum[i] == MustMatchVariables[j]) {
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_MUST_MATCH, BPN_BY);
				rc = EIE_FAIL;
				found = 1;
				break;
			}
		}
		if (found)
			break;
	}

	STC_FreeMemory (ByListTypeNum);
	STC_FreeMemory (ByListTypeChar);

	return rc;
}
/***********************************************************
 This is the call back function (from EI_Transform) that
 gets the transformable data for one matching field at
 a time.
 ObsIndex should be set to 0 by the calling function
 before the first time GetMatchingFieldData () is called.
 This function does not read from the SAS data set.
 It will get the data from the AllRespondents structure.
************************************************************/
static EIT_READCALLBACK_RETURNCODE GetMatchingFieldData (
	char * VarName,
	int * ObsIndex,
	char ** Key,
	double * Value)
{
	int i;

	for (i = 0; i < mAllRespondents.RespondentData[0].NumberofFields; i++) {
		if (!strcmp (VarName, mAllRespondents.RespondentData[0].FieldName[i])) {
			break;
		}
	}

	while (*ObsIndex < mAllRespondents.NumberofRespondents) {
		if (mAllRespondents.RespondentData[*ObsIndex].StatusFlag[i]==FIELDFTI) {
			(*ObsIndex)++;
		}
		else {
			*Key = mAllRespondents.Key[*ObsIndex];
			*Value = mAllRespondents.RespondentData[*ObsIndex].FieldValue[i];
			(*ObsIndex)++;
			return EIE_READCALLBACK_FOUND;
		}
	}
	return EIE_READCALLBACK_NOTFOUND;
}

/********************************************
 Populate the UserMatchingFields structure
********************************************/
static void GetNamesofUserMatchingFields (
	SP_donorimp* sp,
	EIT_FIELDNAMES * UserMatchingFields, /* Original names */
	int NumberOfMustMatchVars)
{
	int i;

	UserMatchingFields->NumberofFields = NumberOfMustMatchVars;
	UserMatchingFields->FieldName = STC_AllocateMemory (
		UserMatchingFields->NumberofFields * sizeof *UserMatchingFields->FieldName);

	for (i = 0; i < UserMatchingFields->NumberofFields; i++) {
		UserMatchingFields->FieldName[i] = STC_AllocateMemory (LEGACY_MAXNAME+1);
		IOUtil_copy_varname(UserMatchingFields->FieldName[i], sp->dsr_indata.VL_must_match.names[i]);
	}
}

/*******************************************
 Get procedure parameters except "seed".
 (it'll be get later, inside PrintParms)
*******************************************/
static void GetParms (
	SP_donorimp* sp,
	char ** EditsString,
	char ** PostEditsString,
	int * NumberOfMustMatchVars,
	int ** MustMatchVariablesList,
	int * MinimumNumberOfDonors,
	double * MinimumPercentOfDonors,
	int * NumberOfDonorsToFind,
	char * EligDonorString,
	int * RandomSelection,
	T_CHAR_VARIABLE * KeyDATA,
	int * ExclVarFlag,
	int * RandNumFlag,
	T_CHAR_VARIABLE * ExclVarDATA,
	T_CHAR_VARIABLE * RandNumDATA,
	int * WriteMatchingFields,
	EIPT_POSITIVITY_OPTION * PositivityOptionSet,
	int * LoggingVerbosityLevel,
	int * NLimit,
	double * Mrl,
	int * ByGroupLoggingLevel)
{
	double DoubleMinimumNumberOfDonors;
	double DoubleNumberOfDonorsToFind;

	/* Edits */
	if (sp->edits.meta.is_specified == IOSV_SPECIFIED)
		*EditsString = (char*) sp->edits.value;
	else
		*EditsString = (char *) NULL;

	/* Post Edits */
	if (sp->post_edits.meta.is_specified == IOSV_SPECIFIED)
		*PostEditsString = (char*) sp->post_edits.value;
	else
		*PostEditsString = (char *) NULL;

   /* ID statement */
	if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
		KeyDATA->Position = sp->dsr_indata.dsr.VL_unit_id.positions[0];
		IOUtil_copy_varname(KeyDATA->Name, sp->dsr_indata.dsr.col_names[KeyDATA->Position]);
		KeyDATA->Size = DSR_get_column_length(&sp->dsr_indata.dsr, sp->dsr_indata.dsr.VL_unit_id.positions[0]); //sp->dsr_indata.dsr.unit_id_size;
		KeyDATA->Value = STC_AllocateMemory (KeyDATA->Size+1);
	}

	/* DATAEXCLVAR statement: no mandatory */
	if (VL_is_specified(&sp->dsr_indata.VL_data_excl)) {
		(*ExclVarFlag) = 1;
		ExclVarDATA->Position = sp->dsr_indata.VL_data_excl.positions[0];
		IOUtil_copy_varname(ExclVarDATA->Name, sp->dsr_indata.VL_data_excl.names[0]);
		ExclVarDATA->Size = DSR_get_column_length(&sp->dsr_indata.dsr, sp->dsr_indata.VL_data_excl.positions[0]); //sp->dsr_indata.data_excl_var_size;
		ExclVarDATA->Value = STC_AllocateMemory (ExclVarDATA->Size+1);
	}
	else
		(*ExclVarFlag) = 0;

	/* RANDNUM statement: no mandatory */
	if (VL_is_specified(&sp->dsr_indata.VL_rand_num)) {
		(*RandNumFlag) = 1;
		RandNumDATA->Position = sp->dsr_indata.VL_rand_num.positions[0];
		IOUtil_copy_varname(RandNumDATA->Name, sp->dsr_indata.VL_rand_num.names[0]);
		RandNumDATA->Size = DSR_get_column_length(&sp->dsr_indata.dsr, sp->dsr_indata.VL_rand_num.positions[0]);
		RandNumDATA->Value = STC_AllocateMemory (RandNumDATA->Size+1);
	}
	else
		(*RandNumFlag) = 0;

	/* Must Match Fields no mandatory */
	if (!VL_is_specified(&sp->dsr_indata.VL_must_match)) {
		*NumberOfMustMatchVars = 0;
		*MustMatchVariablesList = NULL;
	}
	else {
		int * l;
		int i;

		/* make a copy of the must match var because we will change it later */
		*NumberOfMustMatchVars = sp->dsr_indata.VL_must_match.count;
		l = sp->dsr_indata.VL_must_match.positions;

		*MustMatchVariablesList = STC_AllocateMemory (
			*NumberOfMustMatchVars * sizeof **MustMatchVariablesList);

		for (i = 0; i < *NumberOfMustMatchVars; i++) {
			(*MustMatchVariablesList)[i] = l[i];
		}
	}

	/* Minimum Acceptable Number of Donors: default value. */
	/* (an INT in the grammar)                             */
	if (sp->min_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MinimumNumberOfDonors = DEFAULT_MIN_DONORS;
	else {
		DoubleMinimumNumberOfDonors = (double) sp->min_donors.value;
		if ((EIM_DBL_GT (DoubleMinimumNumberOfDonors, EIM_MIN_INT)) &&
			(EIM_DBL_LT (DoubleMinimumNumberOfDonors, EIM_MAX_INT)))
			*MinimumNumberOfDonors = (int) DoubleMinimumNumberOfDonors;
		else if (EIM_DBL_LT (DoubleMinimumNumberOfDonors, 0.0))
			*MinimumNumberOfDonors = EIM_MIN_INT;
		else if (EIM_DBL_GT (DoubleMinimumNumberOfDonors, 0.0))
			*MinimumNumberOfDonors = EIM_MAX_INT;
	}

	/* Acceptable Percentage of Donors: default value. */
	/* (an INT or NBRE in the grammar)                 */
	if (sp->percent_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
		*MinimumPercentOfDonors = DEFAULT_PCENT_DONORS;
	else {
		*MinimumPercentOfDonors = sp->percent_donors.value;
		if (EIM_DBL_LT (*MinimumPercentOfDonors, EIM_MIN_INT))
			*MinimumPercentOfDonors = EIM_MIN_INT;
		else if (EIM_DBL_GT (*MinimumPercentOfDonors, EIM_MAX_INT))
			*MinimumPercentOfDonors = EIM_MAX_INT;
	}

	/* Number of Donors to Find: mandatory. */
	/* (an INT in the grammar)              */
	if (sp->n.meta.is_specified == IOSV_SPECIFIED) {
		DoubleNumberOfDonorsToFind = (double) sp->n.value;
		if ((EIM_DBL_GT (DoubleNumberOfDonorsToFind, EIM_MIN_INT)) &&
			(EIM_DBL_LT (DoubleNumberOfDonorsToFind, EIM_MAX_INT)))
			*NumberOfDonorsToFind = (int) DoubleNumberOfDonorsToFind;
		else if (EIM_DBL_LT (DoubleNumberOfDonorsToFind, 0.0))
			*NumberOfDonorsToFind= EIM_MIN_INT;
		else if (EIM_DBL_GT (DoubleNumberOfDonorsToFind, 0.0))
			*NumberOfDonorsToFind= EIM_MAX_INT;
	}

	/* Eligible donors : Any or Original */
	if (sp->eligdon.meta.is_specified == IOSV_NOT_SPECIFIED)
		EligDonorString[0]= '\0';
	else {
		size_t str_len = strlen(sp->eligdon.value);
		strncpy (EligDonorString, sp->eligdon.value, str_len);
		EligDonorString[str_len] = '\0';
	}

	/* Random Selection option.
	   - not specified (set to zero):
		 no random selection will be performed.
	   - specified (set to 1):
		 random selection will be performed. */
	*RandomSelection = sp->random.meta.is_specified == IOSV_SPECIFIED && sp->random.value == IOB_TRUE ? 1 : 0;

	/* Write matching fields to `outmatching_fields` when requested. */
	*WriteMatchingFields = sp->dsw_outmatching_fields.is_requested == IOB_TRUE ? 1 : 0;

	/* Get the positivity option set by the user */
	if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
		*PositivityOptionSet = EIPE_REJECT_NEGATIVE_DEFAULT;
	else if (sp->accept_negative.value == IOB_FALSE)
		*PositivityOptionSet = EIPE_REJECT_NEGATIVE;
	else if (sp->accept_negative.value == IOB_TRUE)
		*PositivityOptionSet = EIPE_ACCEPT_NEGATIVE;

	/* Get the logging verbosity level set by the user */
	if (sp->display_level.meta.is_specified == IOSV_NOT_SPECIFIED) {
		*LoggingVerbosityLevel = 0;
	} else {
		*LoggingVerbosityLevel = sp->display_level.value;
	}

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

	if (sp->n_limit.meta.is_specified == IOSV_NOT_SPECIFIED)
		*NLimit = -1;
	else
		*NLimit = sp->n_limit.value;

	if (sp->mrl.meta.is_specified == IOSV_NOT_SPECIFIED)
		*Mrl = -1.0;
	else
		*Mrl = sp->mrl.value;
}

/**********************************************************
 Increment "Total counter" with "Counter for one BY group"
***********************************************************/
static void IncrementTotalCounter (
	T_INFO * TotalInfo, /* Total counter */
	T_INFO * Info) /* Counter for one BY group */
{
	TotalInfo->ObsInByGroup += Info->ObsInByGroup;
	TotalInfo->DonorsReachedDonorLimit += Info->DonorsReachedDonorLimit;
	TotalInfo->WithMatchingFields += Info->WithMatchingFields;
	TotalInfo->WithMatchingFieldsImputed += Info->WithMatchingFieldsImputed;
	TotalInfo->WithMatchingFieldsNotImputed +=
		Info->WithMatchingFieldsNotImputed;
	TotalInfo->WithoutMatchingFields += Info->WithoutMatchingFields;
	TotalInfo->WithoutMatchingFieldsImputed +=
		Info->WithoutMatchingFieldsImputed;
	TotalInfo->WithoutMatchingFieldsNotImputed +=
		Info->WithoutMatchingFieldsNotImputed;
	TotalInfo->WithInsufficientDonors += Info->WithInsufficientDonors;
	TotalInfo->MissingKey += Info->MissingKey;
	TotalInfo->MissingData += Info->MissingData;
	TotalInfo->NegativeData += Info->NegativeData;
	TotalInfo->MissingStatus += Info->MissingStatus;
	TotalInfo->Mixed += Info->Mixed;
	TotalInfo->ExcludedDonors += Info->ExcludedDonors;
	TotalInfo->NoEligibleDonors += Info->NoEligibleDonors;
}

/*********************************************
 Initialize counters of structure T_INFO
*********************************************/
static void InitCounter (
	T_INFO * Info)
{
	Info->ObsInByGroup = 0;
	Info->ValidObsInByGroup = 0;
	Info->DonorsReachedDonorLimit = 0;
	Info->WithMatchingFields = 0;
	Info->WithMatchingFieldsImputed = 0;
	Info->WithMatchingFieldsNotImputed = 0;
	Info->WithoutMatchingFields = 0;
	Info->WithoutMatchingFieldsImputed = 0;
	Info->WithoutMatchingFieldsNotImputed = 0;
	Info->WithInsufficientDonors = 0;
	Info->MissingKey = 0;
	Info->MissingData = 0;
	Info->MissingStatus = 0;
	Info->Mixed = 0;
	Info->ExcludedDonors = 0;
	Info->NoEligibleDonors = 0;
	Info->NegativeData = 0;
}

/*******************************************
 Identify if a respondent is a recipient.
 Return: 0->donor or  1-> recipient
*******************************************/
static int IsRecipient (
	int EditsNumberOfFields,
	EIT_STATUS_OF_FIELDS * StatusFlag)
{
	int i;

	for (i = 0; i < EditsNumberOfFields; i++)
		if (StatusFlag[i] == FIELDFTI)
			return 1;

	return 0;
}

/*******************************************
 Identify if an edit variable read for one
 respondent is among MUSTMATCH variable.
*******************************************/
static int IsUserMatchingField (
	int OneEditVariablePosition,
	int * MustMatchVariableList,
	int NumberOfMustMatchVars)
{
	int i;

	for (i = 0; i < NumberOfMustMatchVars; i++) {
		if (OneEditVariablePosition == MustMatchVariableList[i])
			return 1;
	}
	return 0;
}

/**************************************************
 Function: LpFreeEnv

 Purpose:  Frees the ressources used by the LP
		   package (initialized by LPI_InitLpEnv)
***************************************************/
static void LpFreeEnv (void)
{
	char ilperrmsg[1024];
	if (LPI_FreeLpEnv (ilperrmsg) != LPI_SUCCESS)
		/* printing a generic message is enough */
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgFunctionFailure, "LPI_FreeLpEnv()");
}

/*******************************************
 Print counters for observations dropped
 in DATA data set for one BY Group.
*******************************************/
static void PrintInfoCounters (
	char * KeyName,
	char * ExclVarNameInData,
	T_INFO * Info)
{
	/*if (Info->MissingKey || Info->MissingData || Info->NegativeData ||
		Info->Mixed || Info->ExcludedDonors || Info->NoEligibleDonors)
		IO_PRINT_LINE (""); */

	if (Info->MissingKey > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameInDataSet,
			Info->MissingKey, DSN_INDATA, KeyName);

	if (Info->MissingData > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingValuesInEditsMustmatchNotFTI,
			Info->MissingData);

	if (Info->NegativeData > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedNegativeValuesInEditsMustmatchNotFTI,
			Info->NegativeData);

	if (Info->Mixed > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMixedObs,
			Info->Mixed);

	if (Info->ExcludedDonors > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgNumberDroppedExcludedDonors,
			Info->ExcludedDonors, ExclVarNameInData);

	if (Info->NoEligibleDonors > 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgNumberDroppedNonEligibleDonors,
			Info->NoEligibleDonors);

	IO_PRINT_LINE ("");
}

/******************************************************
 Print the values of the parameters to the LOG
 (for "seed", get/print it and start the generator...)
******************************************************/
static void  PrintParms (
	SP_donorimp* sp,
	int MinimumNumberOfDonors,
	double MinimumPercentOfDonors,
	int NumberOfDonorsToFind,
	char * EligDonorString,
	int RandomSelection,
	char * KeyName,
	int ExclVarFlag,
	int RandNumFlag,
	char * ExclVarName,
	char * RandNumName,
	char * EditsString,
	char * PostEditsString,
	EIPT_POSITIVITY_OPTION PositivityOptionSet,
	int LoggingVerbosityLevel,
	int NLimit,
	double Mrl,
	int ByGroupLoggingLevel)
{
	double DoubleSeed;
	int RandSeed;

	///* All data sets name */
	SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);
	SUtil_PrintInputDataSetInfo(&sp->dsr_instatus.dsr);

	SUtil_PrintOutputDataSetInfo(&sp->dsw_outdata);
	SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
	SUtil_PrintOutputDataSetInfo(&sp->dsw_outdonormap);
	SUtil_PrintOutputDataSetInfo(&sp->dsw_outmatching_fields);

	/* Edits */
	if (EditsString != NULL)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, EditsString);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);

	/* Post imputation edits */
	if (PostEditsString != NULL)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_POST_EDITS, PostEditsString);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_POST_EDITS);

	/* Minimum acceptable number of donors */
	if ((MinimumNumberOfDonors > EIM_MIN_INT) &&
		(MinimumNumberOfDonors < EIM_MAX_INT)) {
		if (sp->min_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_MIN_DONORS,
				MinimumNumberOfDonors);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_MIN_DONORS,
				MinimumNumberOfDonors);
	}
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE "%s = **********", BPN_MIN_DONORS);

	/* Minimum percentage of donors */
	if ((EIM_DBL_GT (MinimumPercentOfDonors, EIM_MIN_INT)) &&
		(EIM_DBL_LT (MinimumPercentOfDonors, EIM_MAX_INT))) {
		if (sp->percent_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgPcentdonorsEqualDoublePcentSymbolDefault,
				NB_DECIMAL, MinimumPercentOfDonors);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgPcentdonorsEqualDoublePcentSymbol,
				NB_DECIMAL, MinimumPercentOfDonors);
	}
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE "%s = **********", BPN_PERCENT_DONORS);

	/* Maximum number of donors to try: mandatory */
	if (sp->n.meta.is_specified == IOSV_SPECIFIED) {
		if ((NumberOfDonorsToFind > EIM_MIN_INT) &&
			(NumberOfDonorsToFind < EIM_MAX_INT))
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_N, NumberOfDonorsToFind);
		else
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE "%s = **********", BPN_N);
	}
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_N);

	/* NLIMIT: optional */
	if (sp->n_limit.meta.is_specified == IOSV_SPECIFIED)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_N_LIMIT, NLimit);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_N_LIMIT);

	/* MRL: optional */
	if (sp->mrl.meta.is_specified == IOSV_SPECIFIED) {
		char s[101];
		sprintf (s, "%g", Mrl);//no %g modifier in IO_PRINT_LINE()
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_MRL, s);
	}
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_MRL);

	/* Eligible donors */
	if (strlen (EligDonorString) == 0)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, BPN_ELIGDON, ELIGDON_STRING_DEFAULT);
	else {
		UTIL_StrUpper (EligDonorString, EligDonorString);
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_ELIGDON, EligDonorString);
	}

	/* Random selection for recipients without matching fields */
	if (RandomSelection == 1)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE BPN_RANDOM);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgNoRandomSearch, BPN_RANDOM);

	/* Seed: get/print it and start the generator number.
	   Take time() if none is given or a value < 1 or out of range;
	   print also a warning if the given value is negative, 0 or out of range.*/
	if (sp->seed.meta.is_specified == IOSV_NOT_SPECIFIED) {
		RandSeed = (int) time (NULL);
		/*Ideally, it would be preferable to not display this Note SEED = .... (value chosen by the system) 
		  when RANDNUMVAR is specified but SEED is not, as it can be confusing for the user.*/
		if (RandNumFlag != 1) {
		   IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
         }
	}
	else {
		DoubleSeed = (double) sp->seed.value;
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
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgSeedZeroGeneratedNumberTaken);
			else if (EIM_DBL_LT (DoubleSeed, 0.0))
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgSeedNegativeGeneratedNumberTaken);
		}
	}
	UTIL_SRand (RandSeed);
	UTIL_Rand (); /* kick starting the generator. do not remove. */

	/* Positivity Option */
	if (PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
	else if (PositivityOptionSet == EIPE_REJECT_NEGATIVE)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
	else if (PositivityOptionSet == EIPE_ACCEPT_NEGATIVE)
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

	IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE BPN_DISPLAY_LEVEL " = %d", LoggingVerbosityLevel);

	/* Key variable name */
	if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id))
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, KeyName);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

	/* "Exclude variable" name */
	if (ExclVarFlag == 1)
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_DATA_EXCL_VAR, ExclVarName);
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_DATA_EXCL_VAR);
	/* "Randnum variable" name */
	if (RandNumFlag == 1) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_RAND_NUM_VAR, RandNumName);
		/*when both the SEED and RANDNUMVAR parameters have been specified*/
	    if (sp->seed.meta.is_specified == IOSV_SPECIFIED) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgRanndumvarSeedSpecified);
         }
     }
	else
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_RAND_NUM_VAR);

	/* List of user matching fields ie MUSTMATCH - no mandatory */
	DSR_indata* dsr = &sp->dsr_indata;
    SUtil_PrintStatementVars (&sp->must_match.meta, &dsr->dsr, &dsr->VL_must_match, VARSTMT_LEX);

	/* List of BY-variables - no mandatory */
    SUtil_PrintStatementVars (&sp->by.meta, &dsr->dsr, &dsr->dsr.VL_by_var, BPN_BY);


	IO_PRINT_LINE ("");

}

/********************************************************
 Print the imputation statistics to the LOG window.
********************************************************/
static void PrintStatistics (
	int NumberOfRespondents,
	int NumberOfRecipients,
	T_INFO * Info,
	int RandomSelection)
{
	int NumberOfDonors;

#define ONETAB   4
#define TWOTABS  8

/* PRINT_COUNT_NOPERCENT() is used to print the count of observations.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT_NOPERCENT(Msg, LengthPlaceHolders, Count) \
IO_PRINT_LINE (Msg, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

/* PRINT_SUBTOTAL_NOPERCENT() is used to print subtotal of observations,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is ONETAB or TWOTABS */
#define PRINT_SUBTOTAL_NOPERCENT(Msg, LengthPlaceHolders, LengthHeadingSpace, Count) \
IO_PRINT_LINE (Msg, \
LengthHeadingSpace, MsgPadding,\
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - PRINT_PRECISION),\
"............................................................", \
PRINT_PRECISION, Count);

	NumberOfDonors = NumberOfRespondents - NumberOfRecipients;

	PRINT_COUNT_NOPERCENT (MsgNumberObservations,
		MsgNumberObservations_LPH, Info->ObsInByGroup);

	PRINT_COUNT_NOPERCENT (MsgNumberObservationsDropped,
		MsgNumberObservationsDropped_LPH,
		Info->MissingKey + Info->MissingData + Info->NegativeData +
		Info->Mixed + Info->ExcludedDonors + Info->NoEligibleDonors);

	if (Info->MissingKey + Info->MissingData + Info->NegativeData +
		Info->Mixed + Info->ExcludedDonors + Info->NoEligibleDonors > 0) {

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedMissingKey,
			MsgNumberObsDroppedMissingKey_LPH, ONETAB,
			Info->MissingKey);

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedMissingData,
			MsgNumberObsDroppedMissingData_LPH, ONETAB,
			Info->MissingData);

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedNegativeData,
			MsgNumberObsDroppedNegativeData_LPH, ONETAB,
			Info->NegativeData);

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedMixed,
			MsgNumberObsDroppedMixed_LPH, ONETAB,
			Info->Mixed);

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedExcludedDonors,
			MsgNumberObsDroppedExcludedDonors_LPH, ONETAB,
			Info->ExcludedDonors);

		PRINT_SUBTOTAL_NOPERCENT (MsgNumberObsDroppedNonEligibleDonors,
			MsgNumberObsDroppedNonEligibleDonors_LPH, ONETAB,
			Info->NoEligibleDonors);
	}

/* Used when printing count and percent */
#define PRINT_LENGTH_FLOAT    6
#define PRINT_LENGTH_INTERSPACES  5  /* blanks between count and percent*/
#define PRINT_LENGTH_SYMBOL   1      /* for symbol % */
#define PRINT_NUMBER_DECIMAL  2

/* PRINT_COUNT_PERCENT() is used to print the count of obs and percent.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT_PERCENT(Msg, LengthPlaceHolders, Count, Percent)  \
IO_PRINT_LINE (Msg, \
(int) (PRINT_STATISTICS_WIDTH + PRINT_LENGTH_FLOAT + PRINT_LENGTH_SYMBOL - \
strlen (Msg) + LengthPlaceHolders - PRINT_PRECISION - \
PRINT_LENGTH_FLOAT + 2 - PRINT_LENGTH_SYMBOL), \
"............................................................", \
PRINT_PRECISION, Count, \
PRINT_LENGTH_INTERSPACES - 2, MsgPadding, \
PRINT_LENGTH_FLOAT, PRINT_NUMBER_DECIMAL, Percent);

/* PRINT_SUBTOTAL_PERCENT() is used to print subtotal of obs and percent,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is ONETAB or TWOTABS */
#define PRINT_SUBTOTAL_PERCENT(Msg, LengthPlaceHolders, LengthHeadingSpace, Count, Percent) \
IO_PRINT_LINE (Msg, \
LengthHeadingSpace, MsgPadding,\
(int) (PRINT_STATISTICS_WIDTH + PRINT_LENGTH_FLOAT + PRINT_LENGTH_SYMBOL - \
strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - \
PRINT_PRECISION - PRINT_LENGTH_FLOAT + 2 - PRINT_LENGTH_SYMBOL), \
"............................................................", \
PRINT_PRECISION, Count, \
PRINT_LENGTH_INTERSPACES - 2, MsgPadding, \
PRINT_LENGTH_FLOAT, PRINT_NUMBER_DECIMAL, Percent);

	IO_PRINT_LINE ("");
	PRINT_COUNT_PERCENT (MsgNumberPercentValidObs,
		MsgNumberPercentValidObs_LPH,
		NumberOfRespondents, 100.0);

	if (NumberOfRespondents <= 0)
		return; /* skip remaining */

	PRINT_COUNT_PERCENT (MsgNumberPercentDonors,
		MsgNumberPercentDonors_LPH,
		NumberOfDonors,
		100.0 * NumberOfDonors / NumberOfRespondents);

	if (NumberOfDonors > 0)
		PRINT_COUNT_PERCENT (MsgNumberPercentReachedDonorLimit,
			MsgNumberPercentReachedDonorLimit_LPH,
			Info->DonorsReachedDonorLimit,
			100.0 * Info->DonorsReachedDonorLimit / NumberOfDonors);

	PRINT_COUNT_PERCENT (MsgNumberPercentRecipients,
		MsgNumberPercentRecipients_LPH,
		NumberOfRecipients,
		100.0 * NumberOfRecipients / NumberOfRespondents);

	if (NumberOfRecipients <= 0)
		return; /* skip remaining */

	if (NumberOfRecipients == Info->WithInsufficientDonors) {
		/* no imputation performed */
		PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsWithoutEnoughDonors,
			MsgNumberPercentRecipientsWithoutEnoughDonors_LPH, ONETAB,
			Info->WithInsufficientDonors,
			100.0 * Info->WithInsufficientDonors / NumberOfRespondents);
	}
	else {
		/* imputation performed */
		PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsWithMatchingFields,
			MsgNumberPercentRecipientsWithMatchingFields_LPH, ONETAB,
			Info->WithMatchingFields,
			100.0 * Info->WithMatchingFields / NumberOfRespondents);

		PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsImputed,
			MsgNumberPercentRecipientsImputed_LPH, TWOTABS,
			Info->WithMatchingFieldsImputed,
			100.0 * Info->WithMatchingFieldsImputed / NumberOfRespondents);

		PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsNotImputed,
			MsgNumberPercentRecipientsNotImputed_LPH, TWOTABS,
			Info->WithMatchingFieldsNotImputed,
			100.0 * Info->WithMatchingFieldsNotImputed / NumberOfRespondents);

		PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsWithoutMatchingFields,
			MsgNumberPercentRecipientsWithoutMatchingFields_LPH, ONETAB,
			Info->WithoutMatchingFields,
			100.0 * Info->WithoutMatchingFields / NumberOfRespondents);

		if (RandomSelection == 1) {
			PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsImputed,
				MsgNumberPercentRecipientsImputed_LPH, TWOTABS,
				Info->WithoutMatchingFieldsImputed,
				100.0 * Info->WithoutMatchingFieldsImputed / NumberOfRespondents);

			PRINT_SUBTOTAL_PERCENT (MsgNumberPercentRecipientsNotImputed,
				MsgNumberPercentRecipientsNotImputed_LPH, TWOTABS,
				Info->WithoutMatchingFieldsNotImputed,
				100.0 * Info->WithoutMatchingFieldsNotImputed / NumberOfRespondents);
		}

		/* only for "total" of all by-groups statistics */
		if (Info->WithInsufficientDonors > 0) {
			PRINT_SUBTOTAL_PERCENT(MsgNumberPercentRecipientsWithoutEnoughDonors,
				MsgNumberPercentRecipientsWithoutEnoughDonors_LPH, ONETAB,
				Info->WithInsufficientDonors,
				100.0 * Info->WithInsufficientDonors / NumberOfRespondents);
		}
	} /* imputation performed */
	IO_PRINT("\n");
}
/*******************************************************
 Read all data for one "by group" and fill "DataTable"
 structure.
 ("InStatus" data set is read for each "by group")

 Return: 0 when no more data in DATA data set
		 1 when no more data in the "by group"
*******************************************************/
static PROC_RETURN_CODE ReadByGroup (
	SP_donorimp* sp,
	EIT_FIELDNAMES * AllFields,
	T_CHAR_VARIABLE * KeyDATA,
	int ExclVarFlag,
	int RandNumFlag,
	T_CHAR_VARIABLE * ExclVarDATA,
	T_CHAR_VARIABLE * RandNumDATA,
	double * InVarValue,
	T_CHAR_VARIABLE * KeyINSTATUS,
	T_CHAR_VARIABLE * FieldidINSTATUS,
	T_CHAR_VARIABLE * StatusINSTATUS,
	EIT_DATATABLE * DataTable,
	T_INFO * Info,
	EIT_BOOLEAN InStatusHasAllByVariables,
	tSList   * randkey_list,
	tADTList * randnum_list)
{
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


	PROC_RETURN_CODE rc_ReadData = ReadData (sp, AllFields,
		KeyDATA, ExclVarFlag, RandNumFlag, ExclVarDATA, RandNumDATA, InVarValue, DataTable, Info, randkey_list, randnum_list);
	if (rc_ReadData != PRC_SUCCESS_NO_MORE_DATA) {
		return rc_ReadData;
	}

	PROC_RETURN_CODE rc_ReadStatus = ReadStatus (sp, AllFields, KeyINSTATUS, FieldidINSTATUS,
		StatusINSTATUS, DataTable, Info, InStatusHasAllByVariables);
	if (
		   rc_ReadStatus != PRC_SUCCESS					// no matching BY group
		&& rc_ReadStatus != PRC_SUCCESS_NO_MORE_DATA 	// finished reading BY group
		) {
		return rc_ReadStatus;
	}

	return PRC_SUCCESS;
}

/*****************************************************
 Read DATA data set.
*****************************************************/
static PROC_RETURN_CODE ReadData (
	SP_donorimp* sp,
	EIT_FIELDNAMES * AllFields, /* Edits + MustMatch outside Edits */
	T_CHAR_VARIABLE * KeyDATA,
	int ExclVarFlag,
	int RandNumFlag,
	T_CHAR_VARIABLE * ExclVarDATA,
	T_CHAR_VARIABLE * RandNumDATA,
	double * InVarValue,
	EIT_DATATABLE * DataTable,
	T_INFO * Info,
	tSList   * randkey_list,
	tADTList * randnum_list)
{
	EIT_DATA * Data;
	int i;
	EIT_DATATABLELOOKUP_RETURNCODE rcLookup;
	int insertion_point;
	int key_already_in_randkey_list;
	int string_insertion_rc = 0;

	IO_DATASET_RC rc_next_rec;

	while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {

		Info->ObsInByGroup++;

		IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_indata.dsr);
		if (rc_get_rec != IORC_SUCCESS) {
			return PRC_FAIL_GET_REC;
		}

		SUtil_NullTerminate (KeyDATA->Value, KeyDATA->Size);

		if (ExclVarFlag)
			SUtil_NullTerminate (ExclVarDATA->Value, ExclVarDATA->Size);

		if (KeyDATA->Value[0] == '\0') {
			Info->MissingKey++;
		}
		else {
			if (RandNumFlag) {
				insertion_point = SList_FindInsertionPointBinary(randkey_list, KeyDATA->Value);
				if (insertion_point < -1) {
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandkeylistNotFound);
					return PRC_FAIL_DONORIMP_RANDNUMVAR;
				}
				key_already_in_randkey_list = ((randkey_list->l                                          != NULL) &&
				                               (randkey_list->ne                                         >  0   ) &&
				                               (insertion_point                                          >  -1  ) &&
				                               (strcmp(KeyDATA->Value, randkey_list->l[insertion_point]) == 0   )
				                              );
				if (key_already_in_randkey_list) {
					/*-"KeyDATA->Value" is already in "randkey_list" (at index "insertion_point"): */
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDuplicateKey, KeyDATA->Value);
				} else {
					/*-"KeyDATA->Value" isn't already in "randkey_list" (and the position at which it should be inserted is */
					/* IMMEDIATELY FOLLOWING the existing element at index "insertion_point"):                              */
					string_insertion_rc = SList_InsertStringAfter(randkey_list, insertion_point, KeyDATA->Value);
				}
				if (string_insertion_rc != eSListSucceed) {
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarKeyInsertFailed);
					return PRC_FAIL_DONORIMP_RANDNUMVAR;
				}
				if (IOUtil_is_missing(*(double *)RandNumDATA->Value)) {
                    IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandumvarInvalidValues);
					return PRC_FAIL_DONORIMP_RANDNUMVAR;
			    }
				else if (*(double *)RandNumDATA->Value < 0.0) {
                   IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorImputationLessThanZero, *(double *)RandNumDATA->Value);
                   return PRC_FAIL_DONORIMP_RANDNUMVAR;
                }
                else if (*(double *)RandNumDATA->Value > 1.0) {
                   IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorImputationGreaterThanOne, *(double *)RandNumDATA->Value);
                   return PRC_FAIL_DONORIMP_RANDNUMVAR;
                }
				if (key_already_in_randkey_list) {
					/*-"KeyDATA->Value" was already in "randkey_list" (at index "insertion_point"), so */
					/* replace the corresponding element of "randnum_list" with "RandNumDATA->Value":  */
					/*-"memcpy()" takes 3 arguments: (char * <the starting destination address>, char * <the starting source address>, int <the number of bytes to copy>) */
					/*-"INDEX_LIST(l,index)" is a macro defined in "adtlist.c" which returns the address of the "index"th element of the tADTList instance "l" */
					memcpy ((char *) INDEX_LIST (randnum_list, insertion_point), ((char *)&(RandNumDATA->Value)), randnum_list->itemSize);
				} else {
					/*-"KeyDATA->Value" wasn't already in "randkey_list" (and the position at which it was inserted is */
					/* IMMEDIATELY FOLLOWING the existing element at index "insertion_point"), so insert at the        */
					/* corresponding position of "randnum_list" the value "RandNumDATA->Value":                        */
					randnum_list = ADTList_Insert(randnum_list, (void *)RandNumDATA->Value, (insertion_point + 1));
				}
			}
			rcLookup = EI_DataTableLookup (DataTable, KeyDATA->Value,
				EIE_DATATABLELOOKUPTYPE_CREATE, &Data);

			switch (rcLookup) {
			case EIE_DATATABLELOOKUP_NOTFOUND:
				/* Cool... continuons */

				Info->ValidObsInByGroup++;

				if (ExclVarFlag && (strcmp (ExclVarDATA->Value,
						RESPONDENT_TO_EXCLUDE) == 0))
					Data->ExcludedObs[EIE_PERIOD_CURRENT] = EIE_EXCLUDED;
				else
					Data->ExcludedObs[EIE_PERIOD_CURRENT] = EIE_INCLUDED;

				Data->DataRec[EIE_PERIOD_CURRENT] = STC_AllocateMemory (
					sizeof *Data->DataRec[EIE_PERIOD_CURRENT]);

				EI_DataRecAllocate (AllFields->FieldName,
					AllFields->NumberofFields,
					Data->DataRec[EIE_PERIOD_CURRENT]);
				/* check all "in var" and "must match" for MISSING values
					Note that AllFields->NumberOfFields = #Must_match + #from_edits
					See DefineScatterReadData(...), where these pointers and positions are setup */
				DSR_generic* dsr;
				DS_varlist* vl;
				// in-var
				dsr = &sp->dsr_indata.dsr;
				vl = &dsr->VL_in_var;
				for (i = 0; i < AllFields->NumberofFields; i++)
					if (IOUtil_is_missing (InVarValue[i])){
						Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] = EIM_MISSING_VALUE;
					} else {
						Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] = InVarValue[i];
					}

				/* set datarec flag to ok or missing */
				EI_DataRecSetMissingFlag (Data->DataRec[EIE_PERIOD_CURRENT]);
				break;

			case EIE_DATATABLELOOKUP_FOUND:
				IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgKeyNameWithDuplicateValueInDataSet SAS_NEWLINE,
					KeyDATA->Name, KeyDATA->Value, DSN_INDATA);
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

/*****************************************************
 Read INSTATUS data set.
*****************************************************/
static PROC_RETURN_CODE ReadStatus (
	SP_donorimp* sp,
	EIT_FIELDNAMES * AllFields,
	T_CHAR_VARIABLE * KeyINSTATUS,
	T_CHAR_VARIABLE * FieldidINSTATUS,
	T_CHAR_VARIABLE * StatusINSTATUS,
	EIT_DATATABLE * DataTable,
	T_INFO * Info,
	EIT_BOOLEAN InStatusHasAllByVariables)
{
	EIT_DATA * Data;
	int i;
	EIT_DATATABLELOOKUP_RETURNCODE rcLookup;

	IO_DATASET_RC rc_next_rec;

	/* synchronize STATUS */
	if (InStatusHasAllByVariables) {
		/* STATUS has all the by variables. synchronize STATUS. */
		IO_DATASET_RC rc_sync_by = DSR_cursor_sync_by(&sp->dsr_indata.dsr, &sp->dsr_instatus.dsr);
		if (rc_sync_by == DSRC_DIFFERENT_GROUPS) {
			/* no matching by variables in status data set */
			/*return; */ /* do not read status */
			return PRC_SUCCESS;
		}
		else if (rc_sync_by != DSRC_SUCCESS) {
			// arriving here likely indicates a bug in the code
			return PRC_FAIL_SYNC_GENERIC;
		}
		// else if == DSRC_SUCCESS... continue

	}
	else {
		/* STATUS does not have all the by variables.
		   rewind STATUS to read it all. */
		if (DSRC_SUCCESS != DSR_cursor_rewind(&sp->dsr_instatus.dsr, IOCA_REWIND_DATASET)) {
			return PRC_FAIL_SYNC_GENERIC;
		}
	}

	while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_instatus.dsr)) == DSRC_NEXT_REC_SUCCESS) {

		IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_instatus.dsr);
		if (rc_get_rec != IORC_SUCCESS) {
			return PRC_FAIL_GET_REC;
		}

		SUtil_NullTerminate (KeyINSTATUS->Value, KeyINSTATUS->Size);
		SUtil_NullTerminate (FieldidINSTATUS->Value, FieldidINSTATUS->Size);
		SUtil_NullTerminate (StatusINSTATUS->Value, StatusINSTATUS->Size);

		/* remove leading blanks from fieldid */
		UTIL_DropBlanks (FieldidINSTATUS->Value);
		UTIL_StrUpper (FieldidINSTATUS->Value, FieldidINSTATUS->Value);

		if (KeyINSTATUS->Value[0] == '\0' ||
				FieldidINSTATUS->Value[0] == '\0') {
			Info->MissingStatus++;
		}
		else {
			rcLookup = EI_DataTableLookup (DataTable, KeyINSTATUS->Value,
				EIE_DATATABLELOOKUPTYPE_FIND, &Data);

			switch (rcLookup) {
			case EIE_DATATABLELOOKUP_NOTFOUND:
				/* variable not in data table */
				break;

			case EIE_DATATABLELOOKUP_FOUND:
				/* HIST could be NULL, but not current */
				if (Data->DataRec[EIE_PERIOD_CURRENT] != NULL) {
					for (i = 0; i < AllFields->NumberofFields; i++) {
						/* compare uppercase name */
						if (strcmp (AllFields->FieldName[i], FieldidINSTATUS->Value) == 0) {

							/* the only status of concern are: FTI, FTE, Ianything (but not IDE) */
							if (strcmp (StatusINSTATUS->Value, EIM_STATUS_FTI) == 0)
								Data->DataRec[EIE_PERIOD_CURRENT]->StatusFlag[i] = FIELDFTI;
							else if (strcmp (StatusINSTATUS->Value, EIM_STATUS_FTE) == 0)
								Data->DataRec[EIE_PERIOD_CURRENT]->StatusFlag[i] = FIELDFTE;
							else if (strcmp (StatusINSTATUS->Value, EIM_STATUS_IDE) == 0)
								/*
								NOTHING TO DO, IDE var are considered good
								but we need this because it is an exception
								to the next if.
								*/
								;
							else if (StatusINSTATUS->Value[0] == 'I')
								Data->DataRec[EIE_PERIOD_CURRENT]->StatusFlag[i] = FIELDIMPUTED;

							/* skip remaining variables */
							i = AllFields->NumberofFields;
						}
					} /*for*/
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

/*****************************************************
 Remove DATA from "DataTable" where:
 1. some missing data fields are not flagged as FTI.
 2. some negative data fields are not flagged as FTI
	and REJECTNEGATIVE option is YES.
 2. a mixed record.
 3. a potential donor flagged to be EXCLUDED with
	DATAEXCLVAR or non eligible via ELIGDON.
 Set counters.
*****************************************************/
static void RemoveSomeData (
	EIT_DATATABLE * DataTable,
	int EditsNumberOfFields,
	int ExclVarFlag,
	int EligDonorFlag,
	T_INFO * Info,
	EIPT_POSITIVITY_OPTION PositivityOptionSet)
{
	EIT_DATA * Data;
	EIT_DATA * DataNext;
	EIT_DATAREC * DataRec;
	int i;
	int isRecipient;
	EIT_DATA * PreviousData = NULL;
	EIT_BOOLEAN ToRemove;

	for (i = 0; i < DataTable->Size; i++) {
		for (Data = DataTable->Data[i]; Data != NULL; Data = DataNext) {

			ToRemove = EIE_FALSE;
			DataRec = Data->DataRec[EIE_PERIOD_CURRENT];

			/* Check if some missing data fields  */
			/* are not flagged as FTI.            */
			ToRemove = DataUnflaggedMissing (DataRec);
			if (ToRemove == EIE_TRUE)
				Info->MissingData++;

			else if (((PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT) ||
					(PositivityOptionSet == EIPE_REJECT_NEGATIVE)) &&
					(DataUnflaggedNegative (DataRec) == EIE_TRUE)) {
				/*this block is for positivity option*/
				ToRemove = EIE_TRUE;
				Info->NegativeData++;
			}

			else {
				if (DataRec->NumberofFields > EditsNumberOfFields) {
					/* Check if it's a mixed record. */
					ToRemove = DataMixed (DataRec, EditsNumberOfFields);
					if (ToRemove == EIE_TRUE)
						Info->Mixed++;
				}

				if (ToRemove == EIE_FALSE) {
					/* Check if it's a donor to remove. */
					isRecipient = IsRecipient (EditsNumberOfFields, DataRec->StatusFlag);
					if ((!isRecipient) &&(ExclVarFlag || EligDonorFlag)) {
						if (ExclVarFlag) {
							/* Value 1 for "ExclVarFlag" means only
								that DATA data set has a DATAEXCLVAR.
								The observation could be to exclude or not. */
							if (Data->ExcludedObs[EIE_PERIOD_CURRENT] == EIE_EXCLUDED) {
								ToRemove = EIE_TRUE;
								Info->ExcludedDonors++;
							}
						}
						if ((ToRemove == EIE_FALSE) && EligDonorFlag) {
							/* Value 1 for "EligDonorFlag" means "ORIGINAL"
								ie fields can't be flagged FIELDIMPUTED. */
							ToRemove = DataNonEligible (DataRec);
							if (ToRemove == EIE_TRUE)
								Info->NoEligibleDonors++;
						}
					}
				}
			}

			/* save the Next pointer now, after the free it is too late */
			DataNext = Data->Next;

			/* Remove DATA from "DataTable" */
			if (ToRemove == EIE_TRUE) {
				if (Data == DataTable->Data[i])
					DataTable->Data[i] = Data->Next;
				else
					PreviousData->Next = Data->Next;

				EI_DataTableFreeItem (Data);
				DataTable->NumberOfData--;
				Info->ValidObsInByGroup--;
			}
			else
				PreviousData = Data;
		} /* for (Data=DataTable->Data[i];...;...;) */
	} /* for (i = 0;...; i++) */
}

/******************************************************
 Validate parameter ELIGDON.
 Valid cases: ORIGINAL or O
			  ANY or A
 (If not set, ORIGINAL will be taken)
******************************************************/
static EIT_RETURNCODE ValidateEligDonParameter (
	char * EligDonorString,
	int  * EligDonorFlag)
{
	char aLetterEligDon = ' ';
	char TempEligDon[ELIGDON_MAX_LENGTH+1];

	if (strlen (EligDonorString) == 0) {
		*EligDonorFlag = ELIGDON_FLAG_ORIGINAL;
		return EIE_SUCCEED;
	}

	strcpy (TempEligDon, EligDonorString);
	if (strlen (TempEligDon) == 1)
		aLetterEligDon = TempEligDon[0];

	if (!strcmp (TempEligDon, ELIGDON_STRING_ORIGINAL) ||
		(aLetterEligDon == ELIGDON_LETTER_ORIGINAL)) {
		*EligDonorFlag = ELIGDON_FLAG_ORIGINAL;
		return EIE_SUCCEED;
	}

	if (!strcmp (TempEligDon, ELIGDON_STRING_ANY) ||
		(aLetterEligDon == ELIGDON_LETTER_ANY)) {
		*EligDonorFlag = ELIGDON_FLAG_ANY;
		return EIE_SUCCEED;
	}

	/* Other cases: invalid ELIGDON */
	IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidEligdon);
	return EIE_FAIL;
}

/********************************************
 Validate Procedure Parameters.
********************************************/
static EIT_RETURNCODE ValidateParms (
	SP_donorimp* sp,
	char * EditsString,
	char * PostEditsString,
	int MinimumNumberOfDonors,
	double MinimumPercentOfDonors,
	int NumberOfDonorsToFind,
	char * EligDonorString,
	int * EligDonorFlag,
	int NumberOfMustMatchVars,
	int * MustMatchVariablesList,
	EIPT_POSITIVITY_OPTION PositivityOptionSet,
	int LoggingVerbosityLevel,
	int NLimit,
	double Mrl)
{
	EIT_RETURNCODE crc = EIE_SUCCEED; /* cumulative return code */
	EIT_RETURNCODE rc;

	/* DATA is "lookup" data set with a default data set */
	/* (this test is not required ...)                   */
	if (sp->dsr_indata.dsr.is_specified == IOSV_NOT_SPECIFIED) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, sp->dsr_indata.dsr.dataset_name);
		crc = EIE_FAIL;
	}

	/* INSTATUS data set is mandatory and not NULL */
	if (sp->dsr_instatus.dsr.is_specified == IOSV_NOT_SPECIFIED) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, sp->dsr_instatus.dsr.dataset_name);
		crc = EIE_FAIL;
	}

	/* Edits (string given) - mandatory          */
	/* (with GetParms(), EditsString value is NULL */
	/* if not given or a wrong type and mode)   */
	if (EditsString == NULL) {
		if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_EDITS);
			crc = EIE_FAIL;
		}
		else {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmGenericProblem, BPN_EDITS);
			crc = EIE_FAIL;
		}
	}

	/* Post Edits (string given) - no mandatory      */
	/* (with GetParms(), PostEditsString value is NULL */
	/* if not given or a wrong type and mode)       */
	if (PostEditsString == NULL) {
		if (sp->post_edits.meta.is_specified == IOSV_NOT_SPECIFIED)
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgEditsTakenAsPostEdits);
		else {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmGenericProblem, BPN_POST_EDITS);
			crc = EIE_FAIL;
		}
	}

	/* Minimum Acceptable Number of Donors: default value */
	if ((MinimumNumberOfDonors == EIM_MIN_INT) ||
		(MinimumNumberOfDonors == EIM_MAX_INT)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmOutOfRange, BPN_MIN_DONORS);
		crc = EIE_FAIL;
	}
	else if (MinimumNumberOfDonors <= 0) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmIntegerGE1, BPN_MIN_DONORS);
		crc = EIE_FAIL;
	}

	/* Percent Acceptable of Donors: default value */
	if ((EIM_DBL_EQ (MinimumPercentOfDonors, EIM_MIN_INT)) ||
		(EIM_DBL_EQ (MinimumPercentOfDonors, EIM_MAX_INT))) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmOutOfRange, BPN_PERCENT_DONORS);
		crc = EIE_FAIL;
	}
	else if ((EIM_DBL_LT (MinimumPercentOfDonors, MIN_PCENT_DONORS)) ||
			(EIM_DBL_GE (MinimumPercentOfDonors, MAX_PCENT_DONORS))) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidPcentdonors);
		crc = EIE_FAIL;
	}

	/* Number of Donors to Find - mandatory */
	if (sp->n.meta.is_specified == IOSV_NOT_SPECIFIED) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_N);
		crc = EIE_FAIL;
	}
	else if ((NumberOfDonorsToFind == EIM_MIN_INT) ||
			(NumberOfDonorsToFind == EIM_MAX_INT)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmOutOfRange, BPN_N);
		crc = EIE_FAIL;
	}
	else if (NumberOfDonorsToFind <= 0) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmIntegerGE1, BPN_N);
		crc = EIE_FAIL;
	}

	if (sp->n_limit.meta.is_specified == IOSV_SPECIFIED) {
		if (NLimit < 1) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidNLimit);
			crc = EIE_FAIL;
		}
	}

	if (sp->mrl.meta.is_specified == IOSV_SPECIFIED) {
		if (Mrl <= 0.0) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidMrl);
			crc = EIE_FAIL;
		}
	}

	/* Eligible Donor: ANY or ORIGINAL */
	*EligDonorFlag = ELIGDON_FLAG_ORIGINAL; /* Default value */
	rc = ValidateEligDonParameter (EligDonorString, EligDonorFlag);
	if (rc != EIE_SUCCEED)
		crc = EIE_FAIL;

    if ((LoggingVerbosityLevel != 0) &&
        (LoggingVerbosityLevel != 1) &&
        (LoggingVerbosityLevel != 2) &&
        (LoggingVerbosityLevel != 3) &&
        (LoggingVerbosityLevel != 4) &&
        (LoggingVerbosityLevel != 5)
       ) {
        IO_PRINT_LINE ("Error: The value specified for " BPN_DISPLAY_LEVEL " is not one of 0, 1, 2, 3, 4, or 5: " BPN_DISPLAY_LEVEL " = " SAS_MESSAGE_PREFIX_NOTE ".");
    }

	/* ID statement - mandatory */
	if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
		IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_UNIT_ID);
		crc = EIE_FAIL;
	}

	/* Check if variables are repeated in MUSTMATCH */
	/* (MUSTMATCH is no mandatory)                  */
	if (NumberOfMustMatchVars) {
		if (SUtil_AreDuplicateInListPosition (NumberOfMustMatchVars, MustMatchVariablesList)) {
			IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmWithDuplicateVariable, VARSTMT_LEX);
			crc = EIE_FAIL;
		}
	}

	return crc;
}

/*****************************************************
 Write new observation.
*****************************************************/
static PROC_RETURN_CODE WriteResults (
	SP_donorimp* sp,
	T_WRITEINFO * UserData,
	EIT_DATAREC * RecipientData,
	char * RecipientKey,
	char * DonorKey,
	int NumberOfAttempts,
	int DonorLimit,
	char ** AllFieldsNameOriginal)
{
	int i;

	for (i = 0; i < UserData->EditsNumberOfFields; i++)
		UserData->OutVarValue[i] = RecipientData->FieldValue[i];
	SUtil_CopyPad (UserData->RecipientKey, RecipientKey, UserData->LengthKeyData);

	if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdata)) {
		return PRC_FAIL_WRITE_DATA;
	}

	/* write out the status IDN for fields */
	/* that have been imputed.             */
	SUtil_CopyPad (UserData->RespondentKey, RecipientKey,
		UserData->LengthKeyData);
	/* Write name of variables to impute in `outstatus` and `outmatching_fields` datasets */
	for (i = UserData->EditsNumberOfFields-1; i >= 0; i--) {
		if (RecipientData->StatusFlag[i] == FIELDFTI) {
			SUtil_CopyPad (UserData->OutFieldid, AllFieldsNameOriginal[i], LEGACY_MAXNAME);
			SUtil_CopyPad (UserData->OutStatus, EIM_STATUS_IDN, EIM_STATUS_SIZE);
			// copy value from `outdata` dataset
			UserData->FieldValue = UserData->OutVarValue[i];

			if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
				return PRC_FAIL_WRITE_DATA;
			}

			if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outmatching_fields)) {
				return PRC_FAIL_WRITE_DATA;
			}
		}
	}

	/* Write name of matching variables in `outmatching_fields` dataset */
	/* if MATCHFIELDSTAT option was set */
	if (UserData->MatchingFieldsSelection) {
		for (i = (RecipientData->NumberofFields)-1; i >= 0; i--) {
			if (RecipientData->StatusFlag[i] == FIELDMFS ||
					RecipientData->StatusFlag[i] == FIELDMFU ||
					RecipientData->StatusFlag[i] == FIELDMFB) {
				SUtil_CopyPad (UserData->OutFieldid, AllFieldsNameOriginal[i], LEGACY_MAXNAME);
				if (RecipientData->StatusFlag[i] == FIELDMFS)
					SUtil_CopyPad (UserData->OutStatus, EIM_STATUS_MFS, EIM_STATUS_SIZE);
				else if (RecipientData->StatusFlag[i] == FIELDMFU)
					SUtil_CopyPad (UserData->OutStatus, EIM_STATUS_MFU, EIM_STATUS_SIZE);
				else
					SUtil_CopyPad (UserData->OutStatus, EIM_STATUS_MFB, EIM_STATUS_SIZE);

				if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outmatching_fields)) {
					return PRC_FAIL_WRITE_DATA;
				}
			}
		}
	}

	/* write out the recipient and donor keys */
	SUtil_CopyPad (UserData->DonorKey, DonorKey, UserData->LengthKeyData);
	UserData->NumberOfAttempts = (double) NumberOfAttempts;
	if (DonorLimit == INT_MAX)
		UserData->DonorLimit = IOUtil_missing_value('.');
	else
		UserData->DonorLimit = (double) DonorLimit;

	if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdonormap)) {
		return PRC_FAIL_WRITE_DATA;
	}

	return PRC_SUCCESS;
}

/*********************************************************************
prints time.
help to analyse performance.
*********************************************************************/
static void ShowTime (
	char * Message)
{
#ifdef _DEBUG
	UTIL_ShowTime (Message);
#endif
}
