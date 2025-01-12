/*-------------------------------------------------------------------*/
/* NAME:         PRORATE                                             */
/* PRODUCT:      SAS/TOOLKIT                                         */
/* LANGUAGE:     C                                                   */
/* TYPE:         PROCEDURE                                           */
/* PURPOSE:      Applying the prorating edit rules to the related    */
/*               values and raking them with rounding.               */
/*               The raking will balance a summation by distributing */
/*               the differential with the expected total across the */
/*               summation components based on a specific weight     */
/*               associated with each of the components which might  */
/*               be changeable.                                      */
/*-------------------------------------------------------------------*/
#define MAINPROC       1
#define SASPROC        1

#include <assert.h>

#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prorate_JIO.h"

#include "EI_Common.h"
#include "EI_Message.h"
#include "EI_Prorating.h"
#include "EI_ProratingEdits.h"
#include "EIP_Common.h"
#include "STC_Memory.h"

#include "slist.h"
#include "util.h"

#include "MessageBanffAPI.h"
/* end constant for dataset, parameters and options */

#define MAX_LENGTH_METHOD               8
#define MAX_LENGTH_MODIFIER             8
#define REJECT_NBVARS                   5
#define REJECT_NAME_ERROR               "NAME_ERROR"
#define REJECT_TOTAL_NAME               "TOTAL_NAME"
#define REJECT_RATIO_ERROR              "RATIO_ERROR"
#define STATUS_NBVARS                   3
#define VAR_NOT_FOUND                   -1

#define ERROR_NAME_DECIMAL_ERROR        "DECIMAL ERROR"
#define ERROR_NAME_K_GT_PLUS_ONE        "SCALING VALUE K GREATER THAN +1"
#define ERROR_NAME_K_LT_MINUS_ONE       "SCALING VALUE K LOWER THAN -1"
#define ERROR_NAME_NEGATIVE_DATA_VALUE  "NEGATIVE VALUE IN DATA"
#define ERROR_NAME_NOTHING_TO_PRORATE   "NOTHING TO PRORATE"
#define ERROR_NAME_OUT_OF_BOUNDS        "OUT OF BOUNDS"
#define ERROR_NAME_SUM_NULL             "SUM OF PRORATABLE COLUMNS IS 0"

/* number of digit to print in report before IO_PRINT_LINE will start to use
scientific notation */
#define PRINT_PRECISION 7
#define PRINT_STATISTICS_WIDTH PRINT_STATISTICS_WIDTH_DEFAULT

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

enum T_READSTATUS_RETURNCODE {
    E_READSTATUS_FAIL = -1, /* key not in order */
    E_READSTATUS_SUCCEED,
    E_READSTATUS_EOF /* EOF or End of by group */
};
typedef enum T_READSTATUS_RETURNCODE T_READSTATUS_RETURNCODE;

typedef struct {
    char Name[LEGACY_MAXNAME+1];
    int Size;
    int Position;
    char * Value;
} T_CHAR_VARIABLE;

struct tDataDataSet {
    T_CHAR_VARIABLE * Key;

    int NumberOfVariables;
    char ** VariableName;
    double * VariableValue;
    int * VariablePosition;
    EIT_STATUS_OF_FIELDS * StatusFlag;

    char * PreviousKey; /* to check order */
};
typedef struct tDataDataSet tDataDataSet;

struct tStatusDataSet {
    T_CHAR_VARIABLE * Key;
    T_CHAR_VARIABLE * FieldId;
    T_CHAR_VARIABLE * Status;
    double FieldValue;

    EIT_BOOLEAN HasAllByVariables;

    char * PreviousKey; /* to check order */
};
typedef struct tStatusDataSet tStatusDataSet;

struct tRejectDataSet {
    T_CHAR_VARIABLE * Key;
    T_CHAR_VARIABLE * NameError;
    T_CHAR_VARIABLE * TotalName;
    T_CHAR_VARIABLE * FieldId;
    double Ratio;
};
typedef struct tRejectDataSet tRejectDataSet;

typedef struct {
    int ObsInByGroup;
    int ValidObsInByGroup;
    int MissingKeyDATA;    /* # of missing key in DATA data set               */
    int NegativeValues;    /* # of negative when negative value are rejected  */
    int MissingStatus;     /* # of missing key/fieldid in INSTATUS            */
} T_PRORATE_COUNTER;

static tSList * BuildListName (EIT_PRORATING_GROUP *);
static EIT_RETURNCODE CheckCharVarInStatus (SP_prorate* sp, char *, T_CHAR_VARIABLE *);
static EIT_RETURNCODE CheckDataKeyOrder (char *, char *);
static EIT_RETURNCODE CheckStatusKeyOrder (char *, char *);
static char ConvertProratingFlagNumberToLetter (EIT_PRORATING_MODIFIER);
static void CopyInDataToGroupData (tDataDataSet *, EIT_GROUP_DATA *);
static PROC_RETURN_CODE DefineGatherWriteData (SP_prorate* sp, tDataDataSet *, tDataDataSet *);
static PROC_RETURN_CODE DefineGatherWriteReject (SP_prorate* sp, tDataDataSet *, tRejectDataSet *);
static PROC_RETURN_CODE DefineGatherWriteStatus (SP_prorate* sp, tDataDataSet *, tStatusDataSet *);
static void DefineKeyDATA (SP_prorate* sp, tDataDataSet *);
static PROC_RETURN_CODE DefineScatterReadData (SP_prorate* sp, tDataDataSet *);
static void DefineScatterReadStatus (SP_prorate* sp, tStatusDataSet *);
static EIT_RETURNCODE DefineVarsINSTATUS (SP_prorate* sp, tStatusDataSet *);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_prorate* sp, int, int *, int);
static tSList * FindOriginalNames (SP_prorate* sp, char **, int);
static EIT_RETURNCODE FindVariablesPosition (SP_prorate* sp, char **, int, int *);
static EIT_RETURNCODE GetEdits (SP_prorate* sp, char **, int *);
static void GetParms (SP_prorate* sp, int *, double *, double *, char *,
    char *, EIPT_POSITIVITY_OPTION *, int *);
static EIT_BOOLEAN HasNegativeData (EIT_GROUP_DATA *, int *, int *);
static void InDataPrint (tDataDataSet *);
static void InitCounter (T_PRORATE_COUNTER *);
static PROC_RETURN_CODE MatchStatus (SP_prorate* sp, tDataDataSet *, tStatusDataSet *,
    T_PRORATE_COUNTER *);
static void PrintLogEditsGroups (EIT_PRORATING_GROUP *);
static void PrintParms (SP_prorate* sp, int, double, double, char *, char *,
    int, char *, EIPT_POSITIVITY_OPTION, int ByGroupLoggingLevel);
static void PrintStatistics (T_PRORATE_COUNTER *, char *);
static PROC_RETURN_CODE ProcessByGroups (SP_prorate* sp, tDataDataSet *, tStatusDataSet *,
    tDataDataSet *, tStatusDataSet *, tRejectDataSet *, EIT_PRORATING_GROUP *,
    EIT_PRORATING_METHOD, int, double, double, EIPT_POSITIVITY_OPTION, int);
static PROC_RETURN_CODE ReadStatus (SP_prorate* sp, tStatusDataSet *,
    T_PRORATE_COUNTER *);
static void ResetKeys (SP_prorate* sp, tDataDataSet *, tStatusDataSet *);
static void SetEditsModifierNotSet (EIT_PRORATING_GROUP *,
    EIT_PRORATING_MODIFIER);
static void SetStatus (tDataDataSet *, char *);
static EIT_RETURNCODE ValidateModifierParameter (char *,
    EIT_PRORATING_MODIFIER *);
static EIT_RETURNCODE ValidatePrecisionParameters (int, double, double, char *,
    EIT_PRORATING_METHOD *, EIPT_POSITIVITY_OPTION);
static EIT_BOOLEAN VerifyModifierAllAlwaysOrNever (EIT_PRORATING_GROUP *,
    EIT_PRORATING_MODIFIER);
static EIT_BOOLEAN VerifyModifierAllNever (EIT_PRORATING_GROUP *,
    EIT_PRORATING_MODIFIER);
static PROC_RETURN_CODE WriteReject (SP_prorate* sp, EIT_RAKING_RETURNCODE, EIT_GROUP_DATA *,
    tDataDataSet *, tRejectDataSet *, tSList *, int, int, double);
static PROC_RETURN_CODE WriteResults (SP_prorate* sp, EIT_GROUP_DATA *, tDataDataSet *, tStatusDataSet *,
    tSList *);

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int prorate(
    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_instatus,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outreject,
    T_out_ds out_arr_outreject
)
{
    EIT_PRORATING_MODIFIER DefaultModifier; /* Parameter */
    EIT_PRORATING_GROUP Edits = { 0 }; /* Structure for Edits filled by Parser */
    char * EditsString; /* Parameter */
    int FlagVerifyEdits; /* Option: VERIFYEDITS */
    tDataDataSet InData = { 0 };
    tStatusDataSet InStatus = { 0 };
    tSList * ListVarNameInEdits = { 0 }; /* List of variables name in the edits */
    double LowerBound; /* Parameter */
    EIT_PRORATING_METHOD Method;
    char MethodString[MAX_LENGTH_METHOD+1]; /* Parameter  */
    char ModifierString[MAX_LENGTH_MODIFIER+1];
    int NbDecimal; /* Parameter */
    tDataDataSet OutData = { 0 };
    tRejectDataSet OutReject = { 0 };
    tStatusDataSet OutStatus = { 0 };
    EIPT_POSITIVITY_OPTION PositivityOption;
	int ByGroupLoggingLevel;
    EIT_RETURNCODE rc = { 0 };
    int rcParser;
    double UpperBound; /* Parameter */

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

    /* SI : System Info */
    SUtil_PrintSystemInfo (SI_SYSTEMNAME, SI_SYSTEMVERSION, BANFF_PROC_NAME,
        SI_PROCVERSION, SI_EMAIL, NULL);

	SP_prorate sp = { 0 };
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
        out_sch_outreject,
        out_arr_outreject
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

    /* Get EDITS and VERIFYEDITS */
    rc = GetEdits (&sp, &EditsString, &FlagVerifyEdits);
	if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_EDITS_OTHER;
        goto error_cleanup;
	}
    /* Get Parameters other than EDITS and ID */
    GetParms (&sp, &NbDecimal,
        &LowerBound, &UpperBound, ModifierString, MethodString,
        &PositivityOption, &ByGroupLoggingLevel);

    /* Prints the parameters to the logfile */
    PrintParms (&sp, NbDecimal,
        LowerBound, UpperBound, ModifierString, MethodString, FlagVerifyEdits,
        EditsString, PositivityOption, ByGroupLoggingLevel);

    /* Process EDITS
       - parse/print them in log
       - validate variables with input data file
       - stop the procedure if VERIFYEDITS.  */
    rcParser = EI_ProratingEditsParse (EditsString, &Edits);
    if (rcParser != EIE_PRORATING_EDITS_PARSE_SUCCEED) {
        EI_PrintMessages ();
        proc_exit_code = BRC_FAIL_EDITS_PARSE;
        goto error_cleanup;
    }

    /* Validation: MODIFIER */
    rc = ValidateModifierParameter (ModifierString, &DefaultModifier);
	if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
	}

    /* After this, the DefaultModifier is useless */
    SetEditsModifierNotSet (&Edits, DefaultModifier);
    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgHeaderProrateEdits MsgHeaderEditsModifier);
    PrintLogEditsGroups (&Edits);

    /* Validation
       A. First list with all variables name in the edits (no duplicate). */
    ListVarNameInEdits = BuildListName (&Edits);
    InData.VariableName = ListVarNameInEdits->l;
    InData.NumberOfVariables = SList_NumEntries (ListVarNameInEdits);

    /* B. Second list with their position in the input data set
          Validate: - variable exists,
                    - numeric variable. */
    InData.VariablePosition = STC_AllocateMemory (
        InData.NumberOfVariables * sizeof *InData.VariablePosition);
    rc = FindVariablesPosition (&sp, InData.VariableName,
        InData.NumberOfVariables, InData.VariablePosition);
	if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
        goto error_cleanup;
	}

    if (FlagVerifyEdits) {
        proc_exit_code = BRC_SUCCESS;
        goto normal_cleanup;
    }

    /* Validation: ID statement */
    if (!VL_is_specified(&sp.dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_UNIT_ID);
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
    }

    /* Validation of Precision parameters: Method, Decimal,
       lower and upper bounds, positivity option */
    rc = ValidatePrecisionParameters (NbDecimal, LowerBound, UpperBound,
        MethodString, &Method, PositivityOption);
	if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
	}

    /* Validation: STOP if all modifier are "NEVER"  */
    if (VerifyModifierAllNever (&Edits, DefaultModifier)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgNoProratingAllModifiersAreString,
            EIM_PRORATING_MODIFIER_NEVER_STRING);
        EI_ProratingEditsFree (&Edits);
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
    }

    /* Fill structure KeyData.   */
    DefineKeyDATA (&sp, &InData);
    InData.VariableValue = STC_AllocateMemory (
        InData.NumberOfVariables * sizeof *InData.VariableValue);
    InData.StatusFlag = STC_AllocateMemory (
        InData.NumberOfVariables * sizeof *InData.StatusFlag);
    if (PRC_SUCCESS != DefineScatterReadData (&sp, &InData)) {
        proc_exit_code = BRC_FAIL_READ_GENERIC;
        goto error_cleanup;
    }

    /* Checks exclusivity between ID, BY and EDITS variables */
    rc = ExclusivityBetweenLists (&sp, InData.Key->Position, InData.VariablePosition,
        InData.NumberOfVariables);
	if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
	}

    /* OUTDATASET: Define gather write. */
    OutData.Key = InData.Key;
    OutData.NumberOfVariables = InData.NumberOfVariables;
    OutData.VariableName = InData.VariableName;
    OutData.VariableValue = InData.VariableValue;
    OutData.StatusFlag = InData.StatusFlag;
    if (PRC_SUCCESS != DefineGatherWriteData (&sp, &InData, &OutData)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    /* About InStatusFile:
      If InStatusFile parameter is specified and
      at least one "modifier" in the EDITS is "IMPUTED" or "ORIGINAL"
      (ie other than all "ALWAYS" or a combination of "ALWAYS" with "NEVER")
      - Check/get info on mandatory variables in INSTATUSSET
      - Define scatter read for INSTATUSSET.
    */
    if (VerifyModifierAllAlwaysOrNever (&Edits, DefaultModifier)) {
       if (sp.dsr_instatus.dsr.is_specified == IOSV_SPECIFIED){
          IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgDataSetInstatusIgnored);
          /* dataset was specified but won't be used, so pretend it wasn't specified from here forward */
          sp.dsr_instatus.dsr.is_specified = IOSV_NOT_SPECIFIED;
       }
    }
    else {
       if (sp.dsr_instatus.dsr.is_specified == IOSV_NOT_SPECIFIED) {
          IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetInstatusMandatory);
          proc_exit_code = BRC_FAIL_MISSING_DATASET;
          goto error_cleanup;
       }
       else {
          /* Checks and gets info on mandatory variables in INSTATUS */
          InStatus.Key = STC_AllocateMemory (sizeof *InStatus.Key);
          IOUtil_copy_varname(InStatus.Key->Name, InData.Key->Name);
          InStatus.FieldId = STC_AllocateMemory (sizeof *InStatus.FieldId);
          InStatus.Status = STC_AllocateMemory (sizeof *InStatus.Status);
          rc = DefineVarsINSTATUS (&sp, &InStatus);
		  if (rc == EIE_FAIL) {
              proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
              goto error_cleanup;
		  }

          /* INSTATUSSET : Define scatter read. */
          DefineScatterReadStatus (&sp, &InStatus);
       }
    }

    /* OUTSTATUSSET: Define gather write. */
    OutStatus.Key = InData.Key;
    OutStatus.FieldId = STC_AllocateMemory (sizeof *InStatus.FieldId);
    OutStatus.FieldId->Value = STC_AllocateMemory (LEGACY_MAXNAME+1);
    OutStatus.Status = STC_AllocateMemory (sizeof *InStatus.Status);
    OutStatus.Status->Value = STC_AllocateMemory (EIM_STATUS_SIZE+1);
    SUtil_CopyPad (OutStatus.Status->Value, EIM_STATUS_IPR, EIM_STATUS_SIZE);
    if (PRC_SUCCESS != DefineGatherWriteStatus (&sp, &InData, &OutStatus)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    /* OUTREJECTSET: Define gather write */
    OutReject.Key = InData.Key;
    OutReject.NameError = STC_AllocateMemory (sizeof *OutReject.NameError);
    OutReject.NameError->Value = STC_AllocateMemory (LEGACY_MAXNAME+1);
    OutReject.TotalName = STC_AllocateMemory (sizeof *OutReject.TotalName);
    OutReject.TotalName->Value = STC_AllocateMemory (LEGACY_MAXNAME+1);
    OutReject.FieldId = STC_AllocateMemory (sizeof *OutReject.FieldId);
    OutReject.FieldId->Value = STC_AllocateMemory (LEGACY_MAXNAME+1);
    if (PRC_SUCCESS != DefineGatherWriteReject (&sp, &InData, &OutReject)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    /* Exit if no observation in DATA */
    // actually, function `ExitIfNoData (InData.DataSet)` was deleted, wasn't used in Banff 2.08.002 or likely recent older versions;

    /* check if INSTATUS data set has all the by variables */
    if (sp.dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
        rc = SUtil_AreAllByVariablesInDataSet(&sp.dsr_indata.dsr, &sp.dsr_instatus.dsr);
		EI_PrintMessages ();
		if (rc == EIE_SUCCEED) {
            /* BY variables were found in INSTATUS data set */
            InStatus.HasAllByVariables = EIE_TRUE;

            /* build BY list for status data set */
            // the following SAS statement need not be replaced //SAS_XBYLIST (InData.DataSet, InStatus.DataSet);
        }
        else {
            /* One or more BY variable was not found in INSTATUS data set */
            InStatus.HasAllByVariables = EIE_FALSE;

            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet "\n",
                DSN_INSTATUS);
        }
    }
	TIME_CPU_STOPDIFF(load_init);
	TIME_WALL_STOPDIFF(load_init);

	TIME_WALL_START(processing);
	TIME_CPU_START(processing);

    /********************************************************/
    /* PRORATE IMPUTATION                                   */
    /* bulk of processing performed in ProcessDataGroups () */
    /* rc is checked at the end of the proc                 */
    /********************************************************/
    PROC_RETURN_CODE rc_processing = ProcessByGroups (&sp, &InData, &InStatus, &OutData, &OutStatus, &OutReject,
        &Edits, Method, NbDecimal, LowerBound, UpperBound, PositivityOption, ByGroupLoggingLevel);

    proc_exit_code = convert_processing_rc(rc_processing);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }
	TIME_CPU_STOPDIFF(processing);
	TIME_WALL_STOPDIFF(processing);

    IO_PRINT("\n");

    mem_usage("Before SP_wrap");
    if (PRC_SUCCESS != SP_wrap(&sp)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    mem_usage("After SP_wrap");
    
    normal_cleanup: // indeed, no different than `error_cleanup`, but sometimes we go here without error occurring
    error_cleanup:
	TIME_WALL_BEGIN(cleanup);
	TIME_CPU_BEGIN(cleanup);
    /*********************/
    /* Free memory.      */
    /*********************/
    EI_ProratingEditsFree (&Edits);
    if (NULL != InData.Key) {
        if (NULL != InData.Key->Value)
            STC_FreeMemory(InData.Key->Value);
        STC_FreeMemory(InData.Key);
        InData.Key = NULL;
    }
    if (NULL != InData.PreviousKey){
        STC_FreeMemory (InData.PreviousKey);
        InData.PreviousKey = NULL;
    }
    if (NULL != InData.VariablePosition) {
        STC_FreeMemory (InData.VariablePosition);
        InData.VariablePosition = NULL;
    }
    if (NULL != InData.VariableValue) {
        STC_FreeMemory (InData.VariableValue);
        InData.VariableValue = NULL;
    }
    if (NULL != InData.StatusFlag) {
        STC_FreeMemory (InData.StatusFlag);
        InData.StatusFlag = NULL;
    }
    if (NULL != ListVarNameInEdits) {
        SList_Free(ListVarNameInEdits);
        ListVarNameInEdits = NULL;
    }

    if (sp.dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
        if (NULL != InStatus.Key) {
            if(NULL != InStatus.Key->Value)
                STC_FreeMemory(InStatus.Key->Value);
            STC_FreeMemory(InStatus.Key);
            InStatus.Key = NULL;
        }
        if (NULL != InStatus.PreviousKey) {
            STC_FreeMemory (InStatus.PreviousKey);
            InStatus.PreviousKey = NULL;
        }
        if (NULL != InStatus.FieldId) {
            if (NULL != InStatus.FieldId->Value)
                STC_FreeMemory(InStatus.FieldId->Value);
            STC_FreeMemory(InStatus.FieldId);
            InStatus.FieldId = NULL;
        }
        if (NULL != InStatus.Status) {
            if (NULL != InStatus.Status->Value)
                STC_FreeMemory(InStatus.Status->Value);
            STC_FreeMemory(InStatus.Status);
            InStatus.Status = NULL;
        }
    }

    /* DO NOT free `OutData`, all of its members are just pointers to `InData` members*/

    /* DO NOT free `OutStatus.key`, it is a pointer to `InData.key` */
    if (NULL != OutStatus.FieldId) {
        if (NULL != OutStatus.FieldId->Value)
            STC_FreeMemory(OutStatus.FieldId->Value);
        STC_FreeMemory(OutStatus.FieldId);
        OutStatus.FieldId = NULL;
    }
    if (NULL != OutStatus.Status) {
        if (NULL != OutStatus.Status->Value)
            STC_FreeMemory(OutStatus.Status->Value);
        STC_FreeMemory(OutStatus.Status);
        OutStatus.Status = NULL;
    }

    /* DO NOT free `OutReject.key`, it is a pointer to `InData.key` */
    if (NULL != OutReject.NameError) {
        if (NULL != OutReject.NameError->Value)
            STC_FreeMemory(OutReject.NameError->Value);
        STC_FreeMemory(OutReject.NameError);
        OutReject.NameError = NULL;
    }
    if (NULL != OutReject.TotalName) {
        if (NULL != OutReject.TotalName->Value)
            STC_FreeMemory(OutReject.TotalName->Value);
        STC_FreeMemory(OutReject.TotalName);
        OutReject.TotalName = NULL;
    }
    if (NULL != OutReject.FieldId) {
        if (NULL != OutReject.FieldId->Value)
            STC_FreeMemory(OutReject.FieldId->Value);
        STC_FreeMemory(OutReject.FieldId);
        OutReject.FieldId = NULL;
    }

    EI_FreeMessageList ();

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

    return proc_exit_code;
}
/***********************************************************/
/* Function: BuildListName                                 */
/*                                                         */
/* Purpose : Build a list with variable names in the edits.*/
/***********************************************************/
static tSList * BuildListName (
    EIT_PRORATING_GROUP * Edits)
{
    int i;
    int j;
    tSList * ListVarNameInEdits;

    SList_New (&ListVarNameInEdits);
    for (i = 0; i < Edits->NumberOfEdits; i++) {
        SList_AddNoDup (Edits->Edit[i].TotalName, ListVarNameInEdits);

        for (j = 0; j < Edits->Edit[i].NumberOfFields; j++) {
            SList_AddNoDup (Edits->Edit[i].FieldName[j], ListVarNameInEdits);
        }
    }

    /* important to do binary search later */
    SList_Sort (ListVarNameInEdits, eSListSortDescending);

    return ListVarNameInEdits;
}
/*****************************************************/
/* Function: CheckCharVarInStatus                    */
/*                                                   */
/* Purpose:- Checks that a VariableName exists in    */
/*           INSTATUS and it's a character variable. */
/*         - Gets position and name.                 */
/*****************************************************/
static EIT_RETURNCODE CheckCharVarInStatus (
    SP_prorate* sp,
    char * VariableName,
    T_CHAR_VARIABLE * Variable)
{
    EIT_RETURNCODE rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_instatus.dsr;

    IOUtil_copy_varname(Variable->Name, VariableName);
    IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &VariableName, &Variable->Position);
    if (rc_io == IORC_VARLIST_NOT_FOUND) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet, Variable->Name,
            dsr->dataset_name);
        rc = EIE_FAIL;
    }
    else {
        if (IOVT_CHAR != DSR_get_col_type(dsr, Variable->Position)) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet, Variable->Name,
                dsr->dataset_name);
            rc = EIE_FAIL;
        }
    }

    return rc;
}
/*********************************************************************
Check the order of the keys in the DATA data set.
*********************************************************************/
static EIT_RETURNCODE CheckDataKeyOrder (
    char * PreviousKey,
    char * Key)
{
    int rc;

    rc = strcmp (PreviousKey, Key);
    if (rc > 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted "\n",
            DSN_INDATA, Key, PreviousKey);
        return EIE_FAIL;
    }
    else if (rc == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysDuplicate "\n",
            DSN_INDATA, Key, PreviousKey);
        return EIE_FAIL;
    }

    strcpy (PreviousKey, Key);

    return EIE_SUCCEED;
}
/*********************************************************************
Check the order of the keys in the INSTATUS data set.
*********************************************************************/
static EIT_RETURNCODE CheckStatusKeyOrder (
    char * PreviousKey,
    char * Key)
{
    int rc;

    rc = strcmp (PreviousKey, Key);
    if (rc > 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted "\n",
            DSN_INSTATUS, Key, PreviousKey);
        return EIE_FAIL;
    }

    strcpy (PreviousKey, Key);

    return EIE_SUCCEED;
}
/***********************************************************/
/* Function: ConvertProratingFlagNumberToLetter            */
/*                                                         */
/* Purpose : Convert the Prorating Flag                    */
/*           (number to one letter).                       */
/***********************************************************/
static char ConvertProratingFlagNumberToLetter (
    EIT_PRORATING_MODIFIER  ProratingFlagNumber)
{
    char ModifierLetter;

    if (ProratingFlagNumber == EIE_PRORATING_MODIFIER_ALWAYS)
        ModifierLetter = EIM_PRORATING_MODIFIER_ALWAYS_LETTER;
    else if (ProratingFlagNumber == EIE_PRORATING_MODIFIER_IMPUTED)
        ModifierLetter = EIM_PRORATING_MODIFIER_IMPUTED_LETTER;
    else if (ProratingFlagNumber == EIE_PRORATING_MODIFIER_NEVER)
        ModifierLetter = EIM_PRORATING_MODIFIER_NEVER_LETTER;
    else if (ProratingFlagNumber == EIE_PRORATING_MODIFIER_ORIGINAL)
        ModifierLetter = EIM_PRORATING_MODIFIER_ORIGINAL_LETTER;
    else
        ModifierLetter = EIM_PRORATING_MODIFIER_NOTSET_LETTER;

    return ModifierLetter;
}
/***********************************************************/
/* Function: CopyInDataToGroupData                         */
/*                                                         */
/* Purpose : Get contents from one record and add values   */
/*           for corresponding variables in the data edits */
/*           structure.                                    */
/***********************************************************/
static void CopyInDataToGroupData (
    tDataDataSet * InData,
    EIT_GROUP_DATA * GroupData)
{
    EIT_EDIT_DATA * Edit;
    int i;
    int Index;
    int j;

    /* Get information on values for variables in the edits */
    for (i = 0; i < GroupData->NumberOfEdits; i++) {

        Edit = &GroupData->Edit[i];

        for (j = 0; j < Edit->NumberOfFields; j++) {

            Index = UTIL_Binary_Reverse (Edit->FieldName[j],
                InData->VariableName, InData->NumberOfVariables);

            Edit->FieldValue[j] = InData->VariableValue[Index];

            switch (Edit->ProratingFlag[j]) {
            case EIE_PRORATING_MODIFIER_NEVER:
                Edit->ProratingFlag[j] = EIE_RAKING_FLAG_NO_PRORATABLE;
                break;

            case EIE_PRORATING_MODIFIER_ALWAYS:
                Edit->ProratingFlag[j] = EIE_RAKING_FLAG_PRORATABLE;
                break;

            case EIE_PRORATING_MODIFIER_ORIGINAL:
                if (InData->StatusFlag[Index] == FIELDIMPUTED)
                    /* The field has been imputed */
                    Edit->ProratingFlag[j] = EIE_RAKING_FLAG_NO_PRORATABLE;
                else
                    /* The field has its original value */
                    Edit->ProratingFlag[j] = EIE_RAKING_FLAG_PRORATABLE;
                break;

            case EIE_PRORATING_MODIFIER_IMPUTED:
                if (InData->StatusFlag[Index] == FIELDIMPUTED)
                    /* The field has been imputed */
                    Edit->ProratingFlag[j] = EIE_RAKING_FLAG_PRORATABLE;
                else
                    /* The field has its original value */
                    Edit->ProratingFlag[j] = EIE_RAKING_FLAG_NO_PRORATABLE;
                break;
            } /* switch */
        } /* for j : all FieldName except TotalName*/

        /* For TotalName */
        Index = UTIL_Binary_Reverse (Edit->TotalName,
            InData->VariableName, InData->NumberOfVariables);

        Edit->TotalValue = InData->VariableValue[Index];
    } /* for i */
}
/***********************************************************/
/* Function: DefineGatherWriteData                         */
/*                                                         */
/* Purpose : Define gather write for OUTDATASET.           */
/***********************************************************/
static PROC_RETURN_CODE DefineGatherWriteData (
    SP_prorate* sp,
    tDataDataSet * InData,
    tDataDataSet * OutData)
{
    DSW_generic* dsw = &sp->dsw_outdata;
    int var_added = 0;

    if (IORC_SUCCESS != DSW_allocate(dsw, OutData->NumberOfVariables + 1)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* For key (variable without change) */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, InData->Key->Name, OutData->Key->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* For variables that change. */
    for (int i = OutData->NumberOfVariables-1; i >= 0; i--) {
        const char* var_name = sp->dsr_indata.dsr.col_names[InData->VariablePosition[i]];
            if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, (char*) var_name, &OutData->VariableValue[i], IOVT_NUM)) {
                return PRC_FAIL_WRITE_DATA;
            }
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/***********************************************************/
/* Function: DefineGatherWriteReject                       */
/*                                                         */
/* Purpose : Define gather write for OUTREJECTSET.         */
/***********************************************************/
static PROC_RETURN_CODE DefineGatherWriteReject (
    SP_prorate* sp,
    tDataDataSet * InData,
    tRejectDataSet * OutReject)
{
    DSW_generic* dsw = &sp->dsw_outreject;
    int var_added = 0;

    if (IORC_SUCCESS != DSW_allocate(dsw, REJECT_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* key var */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, InData->Key->Name, OutReject->Key->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* other vars */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, REJECT_NAME_ERROR, OutReject->NameError->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, REJECT_TOTAL_NAME, OutReject->TotalName->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, SUTIL_FIELDSTAT_FIELDID, OutReject->FieldId->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, REJECT_RATIO_ERROR, &OutReject->Ratio, IOVT_NUM)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/***********************************************************/
/* Function: DefineGatherWriteStatus                       */
/*                                                         */
/* Purpose : Define gather write for OUTSTATUSSET.         */
/***********************************************************/
static PROC_RETURN_CODE DefineGatherWriteStatus (
    SP_prorate* sp,
    tDataDataSet * InData,
    tStatusDataSet * OutStatus)
{
    return banff_setup_dsw_outstatus(
        &sp->dsw_outstatus,
        InData->Key->Name,
        (void*)OutStatus->Key->Value,
        (void*)OutStatus->FieldId->Value,
        (void*)OutStatus->Status->Value,
        (void*)&OutStatus->FieldValue
    );

}
/*****************************************************/
/* Function: DefineKeyDATA                           */
/*                                                   */
/* Purpose:  Fills KeyDATA  with ID characteristics  */
/*           and allocate memory for key value.      */
/* Note: ID has been verified by the grammar.        */
/*****************************************************/
static void DefineKeyDATA (
    SP_prorate* sp,
    tDataDataSet * InData)
{
    InData->Key = STC_AllocateMemory (sizeof *InData->Key);
    InData->Key->Position = sp->dsr_indata.dsr.VL_unit_id.positions[0];
    IOUtil_copy_varname(InData->Key->Name, sp->dsr_indata.dsr.col_names[InData->Key->Position]);
    InData->Key->Size = DSR_get_column_length(&sp->dsr_indata.dsr, InData->Key->Position);
    InData->Key->Value = STC_AllocateMemory (InData->Key->Size+1);
    InData->PreviousKey = STC_AllocateMemory (InData->Key->Size+1);
}
/***********************************************************/
/* Function: DefineScatterReadData                         */
/*                                                         */
/* Purpose : Define scatter read for INDATASET.            */
/***********************************************************/
static PROC_RETURN_CODE DefineScatterReadData (
    SP_prorate* sp,
    tDataDataSet * InData)
{
    DSR_generic* dsr = &sp->dsr_indata.dsr;

    // key var
    dsr->VL_unit_id.ptrs[0] = InData->Key->Value;

    // other variables
    if (IORC_SUCCESS != VL_init_from_position_list(&dsr->VL_in_var, dsr, VL_NAME_in_var, IOVT_NUM, InData->NumberOfVariables, InData->VariablePosition)) {
        return PRC_FAIL_UNHANDLED;
    }

    for (int i = 0; i < InData->NumberOfVariables; i++) {
        dsr->VL_in_var.ptrs[i] = &InData->VariableValue[i];
    }

    return PRC_SUCCESS;
}
/***********************************************************/
/* Function: DefineScatterReadStatus                       */
/*                                                         */
/* Purpose : Define scatter read for INSTATUSSET.          */
/*                                                         */
/* (Executes after function "DefineVarsINSTATUS",          */
/*  so we got information about length and position        */
/*  of variables.)                                         */
/***********************************************************/
static void DefineScatterReadStatus (
    SP_prorate* sp,
    tStatusDataSet * InStatus)
{
    banff_setup_dsr_instatus(
        &sp->dsr_instatus,
        (void*)InStatus->Key->Value,
        (void*)InStatus->FieldId->Value,
        (void*)InStatus->Status->Value
    );
}
/*****************************************************/
/* Function: DefineVarsINSTATUS                      */
/*                                                   */
/* Purpose:  Check/fill structures for KeyINSTATUS,  */
/*           FieldIdINSTATUS and StatusINSTATUS      */
/*           with their characteristics and          */
/*           allocates memory for their value.       */
/*****************************************************/
static EIT_RETURNCODE DefineVarsINSTATUS (
    SP_prorate* sp,
    tStatusDataSet * InStatus)
{
    int rc = EIE_SUCCEED;

    InStatus->Key->Value = NULL;
    InStatus->FieldId->Value = NULL;
    InStatus->Status->Value = NULL;

    /* Var: "key" (idem to KeyDATA.Name) */
    if (CheckCharVarInStatus (sp, InStatus->Key->Name,
            InStatus->Key) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->Key->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->Key->Position);
        InStatus->Key->Value = STC_AllocateMemory (InStatus->Key->Size+1);
        InStatus->PreviousKey = STC_AllocateMemory (InStatus->Key->Size+1);
    }

    /* Var: FIELDID */
    if (CheckCharVarInStatus (sp, SUTIL_FIELDSTAT_FIELDID,
            InStatus->FieldId) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->FieldId->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->FieldId->Position);
        InStatus->FieldId->Value = STC_AllocateMemory (
            InStatus->FieldId->Size+1);
    }

    /* Var: STATUS */
    if (CheckCharVarInStatus (sp, SUTIL_FIELDSTAT_STATUS,
            InStatus->Status) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->Status->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->Status->Position);
        InStatus->Status->Value = STC_AllocateMemory (InStatus->Status->Size+1);
    }

    /* A variable is not in INSTATUS or  */
    /* wrong type: free memory.          */
    if (rc == EIE_FAIL) {
       STC_FreeMemory (InStatus->Key->Value);
       InStatus->Key->Value = NULL;

       STC_FreeMemory (InStatus->FieldId->Value);
       InStatus->FieldId->Value = NULL;

       STC_FreeMemory (InStatus->Status->Value);
       InStatus->Status->Value = NULL;
    }

    return rc;
}
/***********************************************************/
/* Function: ExclusivityBetweenLists                       */
/*                                                         */
/* Purpose : Checks  exclusivity between lists:            */
/*           - BY and ID variables position                */
/*           - BY and EDITS variables position             */
/*(not between ID and EDITS because their type is          */
/* different and verified by the grammar or by the parser) */
/***********************************************************/
static EIT_RETURNCODE ExclusivityBetweenLists (
    SP_prorate* sp,
    int KeyPosition,
    int * VariablePosition,
    int NumberOfVariables)
{
    int * ByPosition;
    int i;
    int j;
    int NumberOfByVariables;
    EIT_RETURNCODE rc = EIE_SUCCEED;

    ByPosition = (int*)sp->dsr_indata.dsr.VL_by_var.positions;
    NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;

    /* BY and ID variables */
    for (i = 0; i < NumberOfByVariables; i++) {
         if (ByPosition[i] == KeyPosition) {
             IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, BPN_UNIT_ID, BPN_BY);
             rc = EIE_FAIL;
             break;
         }
    }

    /* BY and EDITS variables */
    for (i = 0; i < NumberOfByVariables; i++) {
        for (j = 0; j < NumberOfVariables; j++) {
            if (ByPosition[i] == VariablePosition[j]) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarInTwoStatementsExclusive, BPN_EDITS,
                    BPN_BY);
                return EIE_FAIL;
            }
        }
    }

    return rc;
}
/*********************************************************************
Create a list of original names of the variables
in the Edits as they appear in the INDATA data set
*********************************************************************/
static tSList * FindOriginalNames (
    SP_prorate* sp, 
    char ** VariableName,
    int NumberOfVariables)
{
    int i;
    tSList * ListVarNameInEditsOriginal;

    SList_New (&ListVarNameInEditsOriginal);

    for (i = 0; i < NumberOfVariables; i++) {
        SList_AddNoDup (VariableName[i], ListVarNameInEditsOriginal);
        /* It can't fail, existence has already been tested */
        SUtil_GetOriginalName (&sp->dsr_indata.dsr, ListVarNameInEditsOriginal->l[i]);
    }

    return ListVarNameInEditsOriginal;
}
/***********************************************************/
/* Function: FindVariablesPosition                         */
/*                                                         */
/* Purpose : Build an array with variable numbers in       */
/*           INDATASET corresponding to variables name in  */
/*           edits                                         */
/*           (verify the variable type is numeric)         */
/***********************************************************/
static EIT_RETURNCODE FindVariablesPosition (
    SP_prorate* sp,
    char ** VariableName,
    int NumberOfVariables,
    int * VariablePosition)
{
    int i;
    EIT_RETURNCODE rc;

    rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    for (i = 0; i < NumberOfVariables; i++) {
        IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &VariableName[i], &VariablePosition[i]);

        if (rc_io == IORC_VARLIST_NOT_FOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotInDataSet,
                VariableName[i], dsr->dataset_name);
            rc = EIE_FAIL;
        }
        else {
            if (IOVT_NUM != DSR_get_col_type(dsr, VariablePosition[i])) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotNumericInDataSet,
                    VariableName[i], dsr->dataset_name);
                rc = EIE_FAIL;
            }
        }
    }
    return rc;
}
/***********************************************************/
/* Function: GetEdits                                      */
/*                                                         */
/* Purpose : Get EDITSPARM and VERIFYOPTION.               */
/***********************************************************/
static EIT_RETURNCODE GetEdits (
    SP_prorate* sp,
    char ** Edits,
    int * FlagVerifyEdits)
{
    EIT_RETURNCODE rc;

    rc = EIE_SUCCEED;

    /* EDITS Param */
    if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR BPN_EDITS " is mandatory.");
        rc = EIE_FAIL;
    }
    else
        *Edits = (char *) sp->edits.value;

    /* FlagVerifyEdits:          */
    /*   0 : no verify option    */
    /*   1 : with verify option  */
    *FlagVerifyEdits = sp->verify_edits.meta.is_specified == IOSV_SPECIFIED && sp->verify_edits.value == IOB_TRUE ? 1 : 0;

    return rc;
}
/***********************************************************/
/* Function: GetParms                                      */
/*                                                         */
/* Purpose : Get Parameters other than EDITSPARM or        */
/*           VERIFYOPTION.                                 */
/***********************************************************/
static void GetParms (
    SP_prorate* sp,
    int * NbDecimal,
    double * LowerBound,
    double * UpperBound,
    char * ModifierString,
    char * MethodString,
    EIPT_POSITIVITY_OPTION * PositivityOption,
	int * ByGroupLoggingLevel)
{
    /* Other parameters */
//case DECIMALPARM:
    if (sp->decimal.meta.is_specified == IOSV_NOT_SPECIFIED)
        *NbDecimal = EIM_PRORATING_LOWER_DECIMAL;
    else
        *NbDecimal = sp->decimal.value;

//case LOWBOUNDPARM:
    if (sp->lower_bound.meta.is_specified == IOSV_NOT_SPECIFIED)
        *LowerBound = EIM_PRORATING_DEFAULT_LOWERBOUND;
    else
        *LowerBound = sp->lower_bound.value;

//case UPBOUNDPARM:
    if (sp->upper_bound.meta.is_specified == IOSV_NOT_SPECIFIED)
        *UpperBound = DBL_MAX;
    else
        *UpperBound = sp->upper_bound.value;

//case MODIFIERPARM:
    if (sp->modifier.meta.is_specified == IOSV_NOT_SPECIFIED) {
        /* not set, use default */
        strcpy (ModifierString, EIM_PRORATING_MODIFIER_ALWAYS_STRING);
    }
    else {
        strncpy (ModifierString, (char *) sp->modifier.value,
            strlen(sp->modifier.value));
        ModifierString[strlen(sp->modifier.value)] = '\0';
    }

//case METHODPARM:
    if (sp->method.meta.is_specified == IOSV_NOT_SPECIFIED) {
        /* not set, use default */
        strcpy (MethodString, EIM_PRORATING_METHOD_BASIC_STRING);
    }
    else {
        strncpy (MethodString, (char *) sp->method.value,
            strlen(sp->method.value));
        MethodString[strlen(sp->method.value)] = '\0';
    }

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
}
/**************************************************************/
/* Function: HasNegativeData                                  */
/*                                                            */
/* Purpose: Checks if at least one variable is negative.      */
/* Will set TotalIndex and FieldIndex as side effect.         */
/* Returns :                                                  */
/*  EIE_TRUE  (yes, at least one variable is negative)        */
/*  EIE_FALSE (no)                                            */
/**************************************************************/
static EIT_BOOLEAN HasNegativeData (
    EIT_GROUP_DATA * GroupData,
    int * TotalIndex,
    int * FieldIndex)
{
    int i;
    int j;

    for (i = 0; i < GroupData->NumberOfEdits; i++) {
        for (j = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
            if (GroupData->Edit[i].FieldValue[j] < 0.0) {
                *TotalIndex = i;
                *FieldIndex = j;
                return EIE_TRUE;
            }
        }
    }

    return EIE_FALSE;
}
/*********************************************************************
Print InData.
Convenience function.
In the context of prorate only 3 status are relevant.
*********************************************************************/
static void InDataPrint (
    tDataDataSet * InData)
{
#define CASEFLAG(X) case X: EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "(" #X ") "); break;
    int i;

    for (i = 0; i < InData->NumberOfVariables; i++) {
        if (InData->VariableValue[i] != EIM_MISSING_VALUE)
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s=%.1f", InData->VariableName[i],
                InData->VariableValue[i]);
        else
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s=MISSING", InData->VariableName[i]);

        switch (InData->StatusFlag[i]) {
        CASEFLAG (FIELDOK)
        CASEFLAG (FIELDMISS)
        CASEFLAG (FIELDIMPUTED) /* by any methods */
        default:
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "(UNKNOWN)\n");
            exit (-1);
        }
    }
}
/**************************************************/
/* Function: InitCounter                          */
/*                                                */
/* Purpose:  Initialize components of "Counter"   */
/*           structure                            */
/**************************************************/
static void InitCounter (
    T_PRORATE_COUNTER * Counter)
{
    Counter->ObsInByGroup = 0;
    Counter->ValidObsInByGroup = 0;
    Counter->MissingKeyDATA = 0;
    Counter->NegativeValues = 0;
    Counter->MissingStatus = 0;
}

/*************************************************************
Reads all INSTATUS obs belonging to the current DATA key.
Set status having IMPUTED (but not IDE) status for the key processed.

This function assumes that InStatus contains valid data.
*************************************************************/
static PROC_RETURN_CODE MatchStatus (
    SP_prorate* sp,
    tDataDataSet * InData,
    tStatusDataSet * InStatus,
    T_PRORATE_COUNTER * Counter)
{
    int l = -1;
    PROC_RETURN_CODE rc = PRC_SUCCESS;
    while (rc == PRC_SUCCESS) {
        l = strcmp (InData->Key->Value, InStatus->Key->Value);

        if (l == 0) {
            /* this instatus key is equal to the indata key */
            SetStatus (InData, InStatus->FieldId->Value);
        }
        else if (l < 0) {
            /* this instatus key is greater than the indata key */
            return PRC_SUCCESS; // NOTE: there may be more data, just not for the current `InData->Key`
        }

        /* at this point either the indata key and instatus key are equal and
        we want to continue to read until we find all matching obs or,
        instatus key is lower and we must read until we find matching obs */

        rc = ReadStatus (sp, InStatus, Counter);
    }

    return rc; /* PRC_SUCCESS_NO_MORE_DATA or some error RC */
}
/***********************************************************/
/* Function: PrintLogEditsGroups                     */
/*                                                         */
/* Purpose : Print in the log window edits groups          */
/*           generated by the parser using edits given by  */
/*           the user                                      */
/***********************************************************/
static void PrintLogEditsGroups (
    EIT_PRORATING_GROUP * Edits)
{
    int i;
    int j;

    for (i = 0; i < Edits->NumberOfEdits; i++) {
        if (Edits->Edit[i].Level == 0)
            IO_PRINT_LINE ("\n" MsgHeaderGroup);
        IO_PRINT_LINE (MsgEditLevelEqualInteger, Edits->Edit[i].Level);
        for (j = 0; j < Edits->Edit[i].NumberOfFields - 1; j++) {
            IO_PRINT_LINE ("%.4f %s:%c + ",
                Edits->Edit[i].Weight[j],
                Edits->Edit[i].FieldName[j],
                ConvertProratingFlagNumberToLetter (
                    Edits->Edit[i].ProratingFlag[j]));
        }
        IO_PRINT_LINE ("%.4f %s:%c = %s\n",
            Edits->Edit[i].Weight[j],
            Edits->Edit[i].FieldName[j],
            ConvertProratingFlagNumberToLetter (
                Edits->Edit[i].ProratingFlag[j]),
            Edits->Edit[i].TotalName);
    }
}
/***********************************************************/
/* Function: PrintParms                                    */
/*                                                         */
/* Purpose : Print the values of the parameters to the LOG */
/***********************************************************/
static void PrintParms (
    SP_prorate* sp,
    int NbDecimal,
    double LowerBound,
    double UpperBound,
    char * ModifierString,
    char * MethodString,
    int FlagVerifyEdits,
    char * Edits,
    EIPT_POSITIVITY_OPTION PositivityOption,
	int ByGroupLoggingLevel)

{
	char s[101];

    /* All data sets name */
    SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_instatus.dsr);

    SUtil_PrintOutputDataSetInfo(&sp->dsw_outdata);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outreject);

    /* Edits */
    if (Edits != NULL)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, Edits);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);

    /* Method string */
    if (sp->method.meta.is_specified == IOSV_NOT_SPECIFIED)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, BPN_METHOD, MethodString);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_METHOD, MethodString);

    /*Number of decimals */
    if (sp->decimal.meta.is_specified == IOSV_NOT_SPECIFIED)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_DECIMAL, NbDecimal);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_DECIMAL, NbDecimal);

    /*Lower bound */
	sprintf (s, "%.*g", 6, LowerBound);//no %g modifier in IO_PRINT_LINE()
    if (sp->lower_bound.meta.is_specified == IOSV_NOT_SPECIFIED)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, BPN_LOWER_BOUND, s); /* default=0 */
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_LOWER_BOUND, s);

    /*Upper bound */
    if (UpperBound == DBL_MAX)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified " " MsgNotConstrainedByUpperBound, BPN_LOWER_BOUND);
    else {
		sprintf (s, "%.*g", 6, UpperBound);//no %g modifier in IO_PRINT_LINE()
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_LOWER_BOUND, s);
    }

    /* Modifier string */
    if (sp->modifier.meta.is_specified == IOSV_NOT_SPECIFIED)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, BPN_MODIFIER, ModifierString);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_MODIFIER, ModifierString);

    /* VeryfyEdits option*/
    if (FlagVerifyEdits == 1)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE BPN_VERIFY_EDITS);

    if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOption == EIPE_REJECT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

    /* ID (the grammar has verified that ID */
    /*     exists in DATA data set)         */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, sp->dsr_indata.dsr.VL_unit_id.names[0]);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

    /* List of BY-variables - not mandatory */
    SUtil_PrintStatementVars(&sp->by.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_by_var, BPN_BY);

    IO_PRINT_LINE ("");
}
/**********************************************/
/* Function: PrintStatistics                  */
/*                                            */
/* Purpose: Prints statistics ("by group")    */
/**********************************************/
static void PrintStatistics (
    T_PRORATE_COUNTER * Counter,
    char * KeyName)
{
/* PRINT_COUNT_DATASET() is used to print the count of obs
   with the name of the data set */
#define PRINT_COUNT_DATASET(Msg, LengthPlaceHolders, DataSetName, Count) \
IO_PRINT_LINE (Msg, DataSetName, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - strlen (DataSetName) - PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

    IO_PRINT_LINE ("");

    PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH,
        DSN_INDATA, Counter->ObsInByGroup);

    PRINT_COUNT_DATASET (MsgNumberValidObs, MsgNumberValidObs_LPH,
        DSN_INDATA, Counter->ValidObsInByGroup);

    if ((Counter->MissingKeyDATA > 0) || (Counter->NegativeValues > 0)) {
       IO_PRINT_LINE ("");

       if (Counter->MissingKeyDATA > 0)
           IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameInDataSet,
               Counter->MissingKeyDATA, DSN_INDATA, KeyName);

       if (Counter->NegativeValues > 0)
           IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
               MsgNumberObsDroppedNegativeValueAndPositivityInconsistent,
                   Counter->NegativeValues);

       IO_PRINT_LINE ("");
    }

    if (Counter->ValidObsInByGroup == 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNoValidObsByGroup "\n");

}
/*********************************************************************
Reads DATA data set.
matches status
calls deterministic
*********************************************************************/
static PROC_RETURN_CODE ProcessByGroups (
    SP_prorate* sp,
    tDataDataSet * InData,
    tStatusDataSet * InStatus,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tRejectDataSet * OutReject,
    EIT_PRORATING_GROUP * Edits,
    EIT_PRORATING_METHOD Method,
    int NbDecimal,
    double LowerBound,
    double UpperBound,
    EIPT_POSITIVITY_OPTION PositivityOption,
	int ByGroupLoggingLevel)
{
    T_PRORATE_COUNTER Counter;
    EIT_GROUP_DATA GroupData;  /* Structure for Edits used for raking   */
    EIT_BOOLEAN hasNegativeData;
    int i;
    int IndexFieldOutOfBound = { 0 };
    tSList * ListVarNameInEditsOriginal;
    EIT_BOOLEAN MustReadStatus = EIE_FALSE;
    int NumberEditFail = { 0 };      /* No edit with prorate error */
    int rc;
    EIT_RAKING_RETURNCODE rcRaking; /* Return code for EI_Prorating */
    double ValueRatioOutOfBound = { 0 }; /* For Output rejected data : value of first
                                    ratio OutOfBound */
    PROC_RETURN_CODE rc_function = PRC_FAIL_UNHANDLED;
    PROC_RETURN_CODE rc_write = PRC_FAIL_UNHANDLED;

    /* Find variable original names */
    ListVarNameInEditsOriginal = FindOriginalNames (sp,
        InData->VariableName, InData->NumberOfVariables);

    /* Initialize DataGroup structure with Edits parsed */
    EI_ProratingInitDataGroup (Edits, &GroupData);

	InitCounter (&Counter);

    IO_RETURN_CODE rc_next_by = IORC_SUCCESS;

    while ((rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr)) == DSRC_NEXT_BY_SUCCESS) {

        /* synchronize INSTATUS, if present
            if it cannot be synchronize we will NOT read it later */
        if (sp->dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
            if (InStatus->HasAllByVariables) {
                /* INSTATUS has all the by variables. synchronize INSTATUS. */
                IO_DATASET_RC rc_sync_by = DSR_cursor_sync_by(&sp->dsr_indata.dsr, &sp->dsr_instatus.dsr);
                if (rc_sync_by == DSRC_SUCCESS) {
                    MustReadStatus = EIE_TRUE;
                } 
                else if (rc_sync_by == DSRC_DIFFERENT_GROUPS) {
                    MustReadStatus = EIE_FALSE;
                }
                else {
                    // arriving here likely indicates a bug in the code
                    rc_function = PRC_FAIL_SYNC_GENERIC;
                    goto error_cleanup;
                }

            }
            else {
                /* INSTATUS does not have all the by variables.
                    rewind INSTATUS to read it all. */
                MustReadStatus = EIE_TRUE;
                if (DSRC_SUCCESS != DSR_cursor_rewind(&sp->dsr_instatus.dsr, IOCA_REWIND_DATASET)) {
                    return PRC_FAIL_SYNC_GENERIC;
                }
            }
        }

        InitCounter(&Counter);
        ResetKeys(sp, InData, InStatus);

        IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;
        while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {

            Counter.ObsInByGroup++;

            IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_indata.dsr);
            if (rc_get_rec != IORC_SUCCESS) {
                rc_function = PRC_FAIL_GET_REC;
                goto error_cleanup;
            }

            SUtil_NullTerminate (InData->Key->Value, InData->Key->Size);

            if (InData->Key->Value[0] == '\0') {
                Counter.MissingKeyDATA++;
            }
            else {
                rc = CheckDataKeyOrder (InData->PreviousKey,
                    InData->Key->Value);
                if (rc == EIE_FAIL){
                    rc_function = PRC_FAIL_REC_NOT_SORTED;
                    goto error_cleanup;
                }

                /* replace SAS missing with Banff missing value. */
                /* the API doesn't know anything about SAS missing value. */
                for (i = 0; i < InData->NumberOfVariables; i++) {
                    if (IOUtil_is_missing (InData->VariableValue[i])) {
                        InData->VariableValue[i] = 0;
                        InData->StatusFlag[i] = FIELDMISS;
                    }
                    else
                        InData->StatusFlag[i] = FIELDOK;
                }

                if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s\n", InData->Key->Value);

                if (sp->dsr_instatus.dsr.is_specified == IOSV_SPECIFIED && MustReadStatus == EIE_TRUE) {
                    PROC_RETURN_CODE rc_match_status = MatchStatus (sp, InData, InStatus, &Counter);
                    switch (rc_match_status) {
                    case PRC_SUCCESS:
                        break;
                    case PRC_SUCCESS_NO_MORE_DATA:
                        MustReadStatus = EIE_FALSE;
                        break;
                    default: // error
                        rc_function = rc_match_status;
                        goto error_cleanup;
                    }
                }

                if (DEBUG) InDataPrint (InData);

                Counter.ValidObsInByGroup++;

                /* Fill EIT_GROUP_DATA structure with value of InData */
                CopyInDataToGroupData (InData, &GroupData);

                if (DEBUG) EI_ProratingDataGroupPrint (&GroupData);

                /* check if there is negative data and REJECTNEGATIVE is YES.
                   If it is the case, the record will not be prorated. */

                rcRaking = EIE_RAKING_SUCCEED;
                if (PositivityOption != EIPE_ACCEPT_NEGATIVE) {
                    hasNegativeData = HasNegativeData (&GroupData,
                        &NumberEditFail, &IndexFieldOutOfBound);
                    if (hasNegativeData == EIE_TRUE)  {
                        rcRaking = EIE_RAKING_FAIL_NEGATIVE_VALUE;
                        Counter.NegativeValues++;
                        Counter.ValidObsInByGroup--;
                    }
                }

                /* Process all edits. The appropriate prorating method is
                   called based on the METHOD= option set by the user. */
                if (rcRaking != EIE_RAKING_FAIL_NEGATIVE_VALUE) {
                    if (Method == EIE_PRORATING_METHOD_BASIC) {
                        rcRaking = EI_ProratingBasic (&GroupData, NbDecimal,
                            LowerBound, UpperBound, &NumberEditFail,
                            &IndexFieldOutOfBound, &ValueRatioOutOfBound);
                    }
                    else {
                        rcRaking = EI_ProratingScaling (&GroupData, NbDecimal,
                            LowerBound, UpperBound, &NumberEditFail,
                            &IndexFieldOutOfBound, &ValueRatioOutOfBound);
                    }
                }

                switch (rcRaking) {
                case EIE_RAKING_SUCCEED:
                    rc_write = WriteResults (sp, &GroupData, OutData, OutStatus,
                        ListVarNameInEditsOriginal);
                    if (rc_write != PRC_SUCCESS) {
                        rc_function = rc_write;
                        goto error_cleanup;
                    }
                    break;

                case EIE_RAKING_FAIL:
                    EI_PrintMessages ();

                    rc_function = PRC_FAIL_PRORATE_RANKING;
                    goto error_cleanup;
                    break;

                case EIE_RAKING_SUCCEED_NOTHING_TO_PRORATE:
                    break;

                case EIE_RAKING_FAIL_DECIMAL_ERROR:
                case EIE_RAKING_FAIL_K_GT_PLUS_ONE:
                case EIE_RAKING_FAIL_K_LT_MINUS_ONE:
                case EIE_RAKING_FAIL_NEGATIVE_VALUE:
                case EIE_RAKING_FAIL_NOTHING_TO_PRORATE:
                case EIE_RAKING_FAIL_OUT_OF_BOUNDS:
                case EIE_RAKING_FAIL_SUM_NULL:
                    rc_write = WriteReject (sp, rcRaking, &GroupData, OutData, OutReject,
                        ListVarNameInEditsOriginal, NumberEditFail,
                        IndexFieldOutOfBound, ValueRatioOutOfBound);
                    if (rc_write != PRC_SUCCESS) {
                        rc_function = rc_write;
                        goto error_cleanup;
                    }
                    break;

                default:
                    break;
                }

                /* Reset ProratingFlag with original information */
                EI_ProratingResetProratingFlag (Edits, &GroupData);
            }
        }

        // check for errors
        if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
            if (ByGroupLoggingLevel != 1) {
                PrintStatistics(&Counter, InData->Key->Name);
                DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
            }
        }
        else{
            rc_function = PRC_FAIL_ADVANCE_REC;
            goto error_cleanup;
        }
    }

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        rc_function = PRC_SUCCESS_NO_MORE_DATA;
        IO_PRINT_LINE("");
        /* Status data set is read for each "by group"
           Print statistics on it one time only. */
        if (Counter.MissingStatus > 0) {
            IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING
                MsgNumberDroppedMissingKeyNameOrFieldidInDataSet "\n",
                Counter.MissingStatus, DSN_INSTATUS, InData->Key->Name);
        }
    }
    else if (rc_next_by == DSRC_BY_NOT_SORTED) {
        rc_function = PRC_FAIL_BY_NOT_SORTED;
        goto error_cleanup;
    }
    else {
        rc_function = PRC_FAIL_ADVANCE_BY;
        goto error_cleanup;
    }

error_cleanup:
    EI_ProratingFreeDataGroup (&GroupData);
    SList_Free (ListVarNameInEditsOriginal);
    EI_PrintMessages ();

    return rc_function;
}
/*********************************************************************
Read Status data set until a valid obs is found
a valid obs is:
    key is not empty,
    fieldid is not empty,
    status is "I" something but not "IDE"
Exit if the data set is not sorted.
Count observations with missing key or missing fieldid.

Returns E_READSTATUS_SUCCEED if a valid obs is found
        E_READSTATUS_EOF if eof is reached
        E_READSTATUS_FAIL if data set not ordered by key
*********************************************************************/
static PROC_RETURN_CODE ReadStatus (
    SP_prorate* sp,
    tStatusDataSet * InStatus,
    T_PRORATE_COUNTER * Counter)
{
    EIT_RETURNCODE rc;

    IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;

    while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_instatus.dsr)) == DSRC_NEXT_REC_SUCCESS) {
        IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_instatus.dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }

        SUtil_NullTerminate (InStatus->Key->Value, InStatus->Key->Size);
        SUtil_NullTerminate (InStatus->FieldId->Value,
            InStatus->FieldId->Size);
        SUtil_NullTerminate (InStatus->Status->Value, InStatus->Status->Size);

        /* remove leading blanks from fieldid and uppercase it */
        UTIL_DropBlanks (InStatus->FieldId->Value);
        UTIL_StrUpper (InStatus->FieldId->Value, InStatus->FieldId->Value);

        if (InStatus->Key->Value[0] == '\0' || /* missing key */
                InStatus->FieldId->Value[0] == '\0') { /* missing fieldid */
            Counter->MissingStatus++;
        }
        else {
            rc = CheckStatusKeyOrder (InStatus->PreviousKey,
                InStatus->Key->Value);
            if (rc == EIE_FAIL)
                return PRC_FAIL_REC_NOT_SORTED; /* not in order */

            if (InStatus->Status->Value[0] == 'I' &&
                strcmp (InStatus->Status->Value, EIM_STATUS_IDE) != 0) {
                /* status is relevant */
                return PRC_SUCCESS;
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
reset key values so we can check key ordering from one by group to the next
*********************************************************************/
static void ResetKeys (
    SP_prorate* sp,
    tDataDataSet * InData,
    tStatusDataSet * InStatus)
{
    InData->Key->Value[0] = '\0';
    InData->PreviousKey[0] = '\0';
    if (sp->dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
        InStatus->Key->Value[0] = '\0';
        InStatus->PreviousKey[0] = '\0';
    }
}
/*********************************************************************
replace unset modifier with the default modifier
*********************************************************************/
static void SetEditsModifierNotSet (
    EIT_PRORATING_GROUP * Edits,
    EIT_PRORATING_MODIFIER DefaultModifier)
{
    EIT_PRORATING_EDIT * Edit;
    int i;
    int j;

    for (i = 0; i < Edits->NumberOfEdits; i++) {
        Edit = &Edits->Edit[i];
        for (j = 0; j < Edit->NumberOfFields; j++) {
            if (Edit->ProratingFlag[j] == EIE_PRORATING_MODIFIER_NOTSET) {
                Edit->ProratingFlag[j] = DefaultModifier;
            }
        }
    }
}
/*********************************************************************
set status of variable to IMPUTED, if variable is in the edits
*********************************************************************/
static void SetStatus (
    tDataDataSet * InData,
    char * VariableName)
{
    int Index;

    Index = UTIL_Binary_Reverse (VariableName, InData->VariableName,
        InData->NumberOfVariables);
    if (Index != -1)
        InData->StatusFlag[Index] = FIELDIMPUTED;
}
/***********************************************************/
/* Function: ValidateModifierParameter                     */
/*                                                         */
/* Purpose : Validate MODIFIERPARM.                        */
/*           Valid cases: ALWAYS   or A                    */
/*                        IMPUTED  or I                    */
/*                        ORIGINAL or O                    */
/*                        NEVER    or N                    */
/*           (If not set, ALWAYS will be taken)            */
/***********************************************************/
static EIT_RETURNCODE ValidateModifierParameter (
    char * ModifierString,
    EIT_PRORATING_MODIFIER * DefaultModifier)
{
    /* Note: At this point ModifierString already contains (obtained in
             GetParms) the default modifier or the one given by the user */
    char aLetterModifier = ' ';
    char TempModifier[MAX_LENGTH_MODIFIER+1];

    strcpy (TempModifier, ModifierString);
    if (strlen (TempModifier) == 1)
        aLetterModifier = tolower (*TempModifier);

    if (!strcmp (TempModifier, EIM_PRORATING_MODIFIER_ALWAYS_STRING)||
        (aLetterModifier == EIM_PRORATING_MODIFIER_ALWAYS_LETTER)) {
        *DefaultModifier = EIE_PRORATING_MODIFIER_ALWAYS;
        return EIE_SUCCEED;
    }

    if (!strcmp (TempModifier, EIM_PRORATING_MODIFIER_ORIGINAL_STRING)||
        (aLetterModifier == EIM_PRORATING_MODIFIER_ORIGINAL_LETTER)) {
        *DefaultModifier = EIE_PRORATING_MODIFIER_ORIGINAL;
        return EIE_SUCCEED;
    }

    if (!strcmp (TempModifier, EIM_PRORATING_MODIFIER_IMPUTED_STRING)||
        (aLetterModifier == EIM_PRORATING_MODIFIER_IMPUTED_LETTER)) {
        *DefaultModifier = EIE_PRORATING_MODIFIER_IMPUTED;
        return EIE_SUCCEED;
    }

    if (!strcmp (TempModifier, EIM_PRORATING_MODIFIER_NEVER_STRING)||
        (aLetterModifier == EIM_PRORATING_MODIFIER_NEVER_LETTER)) {
        *DefaultModifier = EIE_PRORATING_MODIFIER_NEVER;
        return EIE_SUCCEED;
    }

    /* Other cases */
    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidModifierStringPlusValidKeywords,
        TempModifier,
        toupper (EIM_PRORATING_MODIFIER_ALWAYS_LETTER),
        EIM_PRORATING_MODIFIER_ALWAYS_STRING,
        toupper (EIM_PRORATING_MODIFIER_IMPUTED_LETTER),
        EIM_PRORATING_MODIFIER_IMPUTED_STRING,
        toupper (EIM_PRORATING_MODIFIER_NEVER_LETTER),
        EIM_PRORATING_MODIFIER_NEVER_STRING,
        toupper (EIM_PRORATING_MODIFIER_ORIGINAL_LETTER),
        EIM_PRORATING_MODIFIER_ORIGINAL_STRING);
    return EIE_FAIL;
}
/***********************************************************/
/* Function: ValidatePrecisionParameters                   */
/*                                                         */
/* Purpose :Validate precision parameters that are:        */
/*     Number of decimals, Lower bound, Upper bound,       */
/*    method, positivity options                           */
/***********************************************************/
static EIT_RETURNCODE ValidatePrecisionParameters (
    int NbDecimal,
    double LowerBound,
    double UpperBound,
    char * MethodString,
    EIT_PRORATING_METHOD * Method,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    char aLetterMethod = ' ';
    int FlagValidMethod = 1;
    char TempMethod[MAX_LENGTH_METHOD+1];
    EIT_RETURNCODE rc = EIE_SUCCEED;

    /* Note: return "rc" only at the end of the function.
             Modif. made Jan 2004, D.Desrosiers */

    /* Note: At this point MethodString already contains (obtained in
             GetParms) the default method or the one given by the user */
    strcpy (TempMethod, MethodString);
    if (strlen (TempMethod) == 1)
        aLetterMethod = tolower (*TempMethod);

    if (!strcmp (TempMethod, EIM_PRORATING_METHOD_BASIC_STRING)||
        (aLetterMethod == EIM_PRORATING_METHOD_BASIC_LETTER))
        *Method = EIE_PRORATING_METHOD_BASIC;
    else if (!strcmp (TempMethod, EIM_PRORATING_METHOD_SCALING_STRING)||
         (aLetterMethod == EIM_PRORATING_METHOD_SCALING_LETTER))
        *Method = EIE_PRORATING_METHOD_SCALING;
    else {
        /* Other cases: error if method is not B/Basic or S/Scaling */
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInvalidMethodStringPlusValidKeywords "\n",
            TempMethod,
            toupper (EIM_PRORATING_METHOD_BASIC_LETTER),
            EIM_PRORATING_METHOD_BASIC_STRING,
            toupper (EIM_PRORATING_METHOD_SCALING_LETTER),
            EIM_PRORATING_METHOD_SCALING_STRING);
        FlagValidMethod = 0;
        rc = EIE_FAIL;
    }

    if ((NbDecimal < EIM_PRORATING_LOWER_DECIMAL) ||
        (NbDecimal > EIM_PRORATING_UPPER_DECIMAL)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDecimalMustBeIntegerBetweenTwoValues,
            EIM_PRORATING_LOWER_DECIMAL, EIM_PRORATING_UPPER_DECIMAL);
        rc = EIE_FAIL;
    }

    if (LowerBound >= UpperBound) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgUpperBoundGTLowerBound);
        rc = EIE_FAIL;
    }

    if (PositivityOption == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        rc = EIE_FAIL;
    }

    /*Note: At this point, "FlagValidMethod" is true only if approach
            is B/Basic or S/Scaling. Jan 2004, D.Desrosiers */
    if (*Method == EIE_PRORATING_METHOD_BASIC) {
         if (((PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT) ||
              (PositivityOption == EIPE_REJECT_NEGATIVE)) &&
             EIM_DBL_LT (LowerBound, 0.0)) {
             IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgBasicMethodLowerBoundAndPositivityInconsistent);
             rc = EIE_FAIL;
         }

    }
    else if (FlagValidMethod) {
        /* Now make sure the lower and upper bounds are positive number, as
           required by the New Prorating Procedure for Banff Using a Scaling
           Approch.Revised April 17, 2003
        */
        if (LowerBound < EIM_PRORATING_LOWER_LOWERBOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgScalingMethodLowerBoundGEDouble, EIM_PRORATING_LOWER_LOWERBOUND);
            rc = EIE_FAIL;
        }
        if (UpperBound < EIM_PRORATING_LOWER_UPPERBOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgScalingMethodUpperBoundGEDouble, EIM_PRORATING_LOWER_UPPERBOUND);
            rc = EIE_FAIL;
        }
    }

    return rc;
}
/***********************************************************/
/* Function: VerifyModifierAllAlwaysOrNever                */
/*                                                         */
/* Purpose : Test if all modifier in Edits are             */
/*           EIE_PRORATING_MODIFIER_ALWAYS or              */
/*           EIE_PRORATING_MODIFIER_NEVER                  */
/*           (if a modifier is not set, take the global    */
/*           modifier parameter).                          */
/* Note:                                                   */
/* - before calling this function, we must call            */
/*   "VerifyModifierAllNever" to verify if all modifiers   */
/*   are NEVER (nothing to process in that case).          */
/* - call after "VerifyModifierAllAlwaysOrNever" to verify */
/*   if we have to read INSTATUSSET                        */
/*   --> don't read it if all modifiers are ALWAYS or      */
/*       a combination of ALWAYS and NEVER only.           */
/*   --> read it if at least one modifier is ORIGINAL or   */
/*       IMPUTED.                                          */
/***********************************************************/
static EIT_BOOLEAN VerifyModifierAllAlwaysOrNever (
    EIT_PRORATING_GROUP * Edits,
    EIT_PRORATING_MODIFIER DefaultModifier)
{
    /* EIE_TRUE:  All modifiers are ALWAYS or NEVER
       EIE_FALSE: At least one modifier is IMPUTED or ORIGINAL */
    int i;
    int j;

    for (i = 0; i < Edits->NumberOfEdits; i++) {
        for (j = 0; j < Edits->Edit[i].NumberOfFields; j++) {
            switch (Edits->Edit[i].ProratingFlag[j]) {
            case EIE_PRORATING_MODIFIER_ALWAYS:
            case EIE_PRORATING_MODIFIER_NEVER:
                break;

            case EIE_PRORATING_MODIFIER_NOTSET:
                if (DefaultModifier == EIE_PRORATING_MODIFIER_ALWAYS ||
                    DefaultModifier == EIE_PRORATING_MODIFIER_NEVER)
                    /* NOTHING TO DO */;
                else
                    return EIE_FALSE;
                break;

            case EIE_PRORATING_MODIFIER_IMPUTED:
            case EIE_PRORATING_MODIFIER_ORIGINAL:
            default:
                return EIE_FALSE;
            } /* switch */
        } /* For j */
    } /* For i */

    return EIE_TRUE;
}
/***********************************************************/
/* Function: VerifyModifierAllNever                        */
/*                                                         */
/* Purpose : Test if all modifier in Edits are             */
/*           EIE_PRORATING_MODIFIER_NEVER                  */
/*           (if a modifier is not set, take the global    */
/*           modifier parameter).                          */
/***********************************************************/
static EIT_BOOLEAN VerifyModifierAllNever (
    EIT_PRORATING_GROUP * Edits,
    EIT_PRORATING_MODIFIER DefaultModifier)
{
    /* EIE_TRUE:  All modifiers are NEVER
       EIE_FALSE: At least one modifier is not NEVER */
    int i;
    int j;

    for (i = 0; i < Edits->NumberOfEdits; i++) {
        for (j = 0; j < Edits->Edit[i].NumberOfFields; j++) {
            switch (Edits->Edit[i].ProratingFlag[j]) {
            case EIE_PRORATING_MODIFIER_NEVER:
                break;

            case EIE_PRORATING_MODIFIER_NOTSET:
                if (DefaultModifier != EIE_PRORATING_MODIFIER_NEVER)
                    return EIE_FALSE;
                break;

            case EIE_PRORATING_MODIFIER_ALWAYS:
            case EIE_PRORATING_MODIFIER_IMPUTED:
            case EIE_PRORATING_MODIFIER_ORIGINAL:
            default:
                return EIE_FALSE;
            } /* switch */
        } /* For j */
    } /* For i */

    return EIE_TRUE;
}
/***********************************************************/
/* Function: WriteReject                                   */
/*                                                         */
/* Purpose : Execute gather write for OUTREJECTSET.        */
/***********************************************************/
static PROC_RETURN_CODE WriteReject (
    SP_prorate* sp,
    EIT_RAKING_RETURNCODE rcRaking,
    EIT_GROUP_DATA * GroupData,
    tDataDataSet * OutData,
    tRejectDataSet * OutReject,
    tSList * ListVarNameInEditsOriginal,
    int NumberEditFail,
    int IndexFieldOutOfBound,
    double ValueRatioOutOfBound)
{
    int FieldIndex;
    int TotalIndex;

    SUtil_Pad (OutData->Key->Value, OutData->Key->Size);

    memset (OutReject->NameError->Value, ' ', LEGACY_MAXNAME);
    memset (OutReject->TotalName->Value, ' ', LEGACY_MAXNAME);
    memset (OutReject->FieldId->Value, ' ', LEGACY_MAXNAME);
    OutReject->Ratio = IOUtil_missing_value('_');

    /* find index of total */
    TotalIndex = UTIL_Binary_Reverse (GroupData->Edit[NumberEditFail].TotalName,
        OutData->VariableName, OutData->NumberOfVariables);


    switch (rcRaking) {
    case EIE_RAKING_FAIL_NOTHING_TO_PRORATE:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_NOTHING_TO_PRORATE, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_K_GT_PLUS_ONE:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_K_GT_PLUS_ONE, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_K_LT_MINUS_ONE:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_K_LT_MINUS_ONE, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_NEGATIVE_VALUE:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_NEGATIVE_DATA_VALUE, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_DECIMAL_ERROR:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_DECIMAL_ERROR, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_SUM_NULL:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_SUM_NULL, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    case EIE_RAKING_FAIL_OUT_OF_BOUNDS:
        SUtil_CopyPad (OutReject->NameError->Value,
            ERROR_NAME_OUT_OF_BOUNDS, LEGACY_MAXNAME);
        SUtil_CopyPad (OutReject->TotalName->Value,
            ListVarNameInEditsOriginal->l[TotalIndex], LEGACY_MAXNAME);
        break;

    default:
        break;
    }

    if (rcRaking == EIE_RAKING_FAIL_OUT_OF_BOUNDS) {
        FieldIndex = UTIL_Binary_Reverse (
            GroupData->Edit[NumberEditFail].FieldName[IndexFieldOutOfBound],
            OutData->VariableName, OutData->NumberOfVariables);

        SUtil_CopyPad (OutReject->FieldId->Value,
            ListVarNameInEditsOriginal->l[FieldIndex], LEGACY_MAXNAME);
        OutReject->Ratio = ValueRatioOutOfBound;
    }
    else if (rcRaking == EIE_RAKING_FAIL_NEGATIVE_VALUE) {
        FieldIndex = UTIL_Binary_Reverse (
            GroupData->Edit[NumberEditFail].FieldName[IndexFieldOutOfBound],
            OutData->VariableName, OutData->NumberOfVariables);

        SUtil_CopyPad (OutReject->FieldId->Value,
            ListVarNameInEditsOriginal->l[FieldIndex], LEGACY_MAXNAME);
    }
    else if (rcRaking == EIE_RAKING_FAIL_K_GT_PLUS_ONE ||
        rcRaking == EIE_RAKING_FAIL_K_LT_MINUS_ONE) {
        SUtil_CopyPad (OutReject->FieldId->Value, "_", LEGACY_MAXNAME);
    }
    else { // added during conversion from SAS. SAS must have automatically trimmed.  
        SUtil_CopyPad(OutReject->FieldId->Value, " ", LEGACY_MAXNAME);
    }

    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outreject)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
/***********************************************************/
/* Function: WriteResults                                  */
/*                                                         */
/* Purpose : - Copy new prorated values for variables in   */
/*             edits to the vector used in gather write    */
/*             for OUTDATASET.                             */
/*             (if value was missing, keep the original    */
/*             missing value).                             */
/*           - Execute gather write for OUTDATASET and     */
/*             OUTSTATUSSET.                               */
/***********************************************************/
static PROC_RETURN_CODE WriteResults (
    SP_prorate* sp,
    EIT_GROUP_DATA * GroupData,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tSList * ListVarNameInEditsOriginal)
{
    int i;
    int j;
    int Index;

    /*EI_ProratingDataGroupPrint (GroupData);*/

    SUtil_Pad (OutData->Key->Value, OutData->Key->Size);

    /* Get information on values for variables in the edits */
    for (i = 0; i < GroupData->NumberOfEdits; i++) {
        for (j = 0; j < GroupData->Edit[i].NumberOfFields; j++) {

            Index = UTIL_Binary_Reverse (GroupData->Edit[i].FieldName[j],
                OutData->VariableName, OutData->NumberOfVariables);

            if (OutData->StatusFlag[Index] == FIELDMISS)
                OutData->VariableValue[Index] = IOUtil_missing_value('.');
            else
                OutData->VariableValue[Index] =
                    GroupData->Edit[i].FieldValue[j];

            if (GroupData->Edit[i].ProratingFlag[j] ==
                    EIE_RAKING_FLAG_PRORATED) {
                SUtil_CopyPad (OutStatus->FieldId->Value,
                    ListVarNameInEditsOriginal->l[Index], LEGACY_MAXNAME);
                // copy value from `outdata` dataset
                OutStatus->FieldValue = OutData->VariableValue[Index];

                if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                    return PRC_FAIL_WRITE_DATA;
                }
            }
        } /* for j : all FieldName except TotalName*/

        /* For TotalName in Edit[i] */

        Index = UTIL_Binary_Reverse (GroupData->Edit[i].TotalName,
            OutData->VariableName, OutData->NumberOfVariables);

        if (OutData->StatusFlag[Index] == FIELDMISS)
            OutData->VariableValue[Index] = IOUtil_missing_value('.');
        else
            OutData->VariableValue[Index] = GroupData->Edit[i].TotalValue;
    } /* for i */

    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdata)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
