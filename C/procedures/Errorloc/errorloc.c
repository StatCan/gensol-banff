/* NAME:         ERRORLOC                           */
/* PURPOSE:      Find variables in error.           */

/* these must be define before including <uwproc.h> */
#define MAINPROC 1
#define SASPROC  1

#ifdef BSMD_TRANSPORT
#include <signal.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorloc_JIO.h"

#include "EI_Chernikova.h"
#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"
#include "EI_EditsApplyChern.h"
#include "EI_EditsBounds.h"
#include "EI_EditsConsistency.h"
#include "EI_EditsRedundancy.h"
#include "EI_ErrorLocalization.h"
#include "EI_Message.h"
#include "EI_Weights.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "slist.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageBanffAPI.h"

#define REJECT_ERROR_CARDINALITY "CARDINALITY EXCEEDED"
#define REJECT_ERROR_SPACE "SPACE EXCEEDED"
#define REJECT_ERROR_TIME  "TIME EXCEEDED"
#define REJECT_NAME_ERROR  "NAME_ERROR"
#define REJECT_NBVARS   2
#define STATUS_NBVARS   3
#define DEFAULT_TIMEPEROBSPARM 20

/* number of digit to print in report before IO_PRINT will start to use
scientific notation */
#define PRINT_PRECISION    7
#define PRINT_STATISTICS_WIDTH PRINT_STATISTICS_WIDTH_DEFAULT + 20

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

struct CounterStructure {
    int Obs;
    int MissingKey;
    int ValidObs;
    int MissingStatus;     /* # of missing key/fieldid in INSTATUS            */
    int PassedEdits;
    int FailedEdits;
    int FailedEditsSolutionNotFound;
    int FailedEditsSolutionNotFoundCardinality;
    int FailedEditsSolutionNotFoundSpace;
    int FailedEditsSolutionNotFoundTime;
    int FailedEditsSolutionFound;
    int FailedEditsSolutionFoundVar;
};
typedef struct CounterStructure tCounter;

typedef struct {
    char Name[LEGACY_MAXNAME + 1];
    int Size;
    int Position;
    char* Value;
} T_CHAR_VARIABLE;

struct tStatusDataSet {
    T_CHAR_VARIABLE* Key;
    T_CHAR_VARIABLE* FieldId;
    T_CHAR_VARIABLE* Status;
    double FieldValue;

    EIT_BOOLEAN HasAllByVariables;

    char* PreviousKey; /* to check order */
};
typedef struct tStatusDataSet tStatusDataSet;

struct InDataStructure {
    char * Key;
    int KeyLength;
    EIT_DATAREC DataRec;
    int RandNumFlag;
    double RandomNumber;
};
typedef struct InDataStructure tInData;

struct OutRejectStructure {
    char *Key;
    int KeyLength;
    char NameError[LEGACY_MAXNAME+1];
};
typedef struct OutRejectStructure tOutReject;

struct OutStatusStructure {
    char *Key;
    int KeyLength;
    char FieldId[LEGACY_MAXNAME+1];
    char Status[EIM_STATUS_SIZE+1];
    double FieldValue;
};
typedef struct OutStatusStructure tOutStatus;

static EIT_RETURNCODE CheckCharVarInStatus(SP_errorloc* sp, char* VariableName, T_CHAR_VARIABLE* Variable);
static void CheckDataValidity (SP_errorloc* sp, EIT_DATAREC *, EIT_WEIGHTS *,
    EIPT_POSITIVITY_OPTION, double *, double *, EIT_BOOLEAN *, double *);
static EIT_RETURNCODE CheckKeyOrder (char *, char *);
static EIT_RETURNCODE CheckStatusKeyOrder(char* PreviousKey, char* Key);
static PROC_RETURN_CODE DefineGatherWriteReject (SP_errorloc* sp, tOutReject *, char *);
static PROC_RETURN_CODE DefineGatherWriteStatus (SP_errorloc* sp, tOutStatus *, char *);
static IO_RETURN_CODE DefineScatterRead (SP_errorloc* sp, tInData *, int *);
static void DefineScatterReadStatus(SP_errorloc* sp, tStatusDataSet* InStatus);
static EIT_RETURNCODE DefineVarsINSTATUS(SP_errorloc* sp, tStatusDataSet* InStatus);
static int DoChernikova (SP_errorloc* sp, tInData *, tOutStatus *, tOutReject *, EIT_EDITS *,
    EIT_WEIGHTS *, double, double, EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *,
    tCounter *, int, tSList* OriginalVarNamesInEdits);
static EIT_RETURNCODE DoErrorLocalization (SP_errorloc* sp, tInData *, tOutStatus *,
    tOutReject *, EIT_EDITS *, EIT_WEIGHTS *, double, double,
    EIPT_POSITIVITY_OPTION, int, double *, double *, EIT_CHERNIKOVA *,
    EIT_CHERNIKOVA_COMPANION  *, tCounter *, tSList* OriginalVarNamesInEdits);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_errorloc* sp, int, int *, int);
static void ExitIfNoData (SP_errorloc* sp);
static tSList* FindOriginalNames(SP_errorloc* sp, char** VariableName, int NumberOfVariables);
static EIT_RETURNCODE FindVariablesPosition (SP_errorloc* sp, char **, int, int *);
static int GetBoundsVariables (EIT_EDITS *, double[], double[]);
static void GetParms (SP_errorloc* sp, tInData *, char **, char **,
    double *, double *, EIPT_POSITIVITY_OPTION *, int *, int *);
static void InitCounter (tCounter *);
static void LpFreeEnv (void);
static PROC_RETURN_CODE MatchStatus(SP_errorloc* sp, tInData* InData, tStatusDataSet* InStatus, tCounter* Counter);
static void PrintParms (SP_errorloc* sp,char *, char *,
    double, double, EIPT_POSITIVITY_OPTION, int, int ByGroupLoggingLevel);
static void PrintStatistics (tCounter *);
static void PrintStatus (tOutStatus *);
static PROC_RETURN_CODE ProcessByGroups (SP_errorloc* sp, tInData *, tStatusDataSet*, tOutStatus *, tOutReject *,
    EIT_EDITS *, EIT_WEIGHTS *, double, double, EIPT_POSITIVITY_OPTION, tSList*, int, int);
static PROC_RETURN_CODE ReadStatus(SP_errorloc* sp, tStatusDataSet* InStatus, tCounter* Counter);
static void SetFieldValueOutsideLimit (EIT_DATAREC *, double *, double *);
static void SetStatus(EIT_DATAREC* DataRec, char* FieldName);
static void UpdateDataForEditsApplyChern (int, int, EIT_DATAREC *,
    EIT_WEIGHTS *, double *, double *, EIT_BOOLEAN *, double *);
static EIT_RETURNCODE ValidateParms (SP_errorloc* sp, char *, char *, double, double *,
    EIPT_POSITIVITY_OPTION, int, EIT_EDITS *, EIT_WEIGHTS *);


#ifdef BSMD_TRANSPORT

/* point to the current key. Should be set right after an observation is read. */
static char * mKey;

/* print the current key. */
/* this function should be set to handle floating point exception interrupt. */
void FpSignalHandler(
    int sig)
{
	IO_PRINT_LINE ("Floating point exception." SAS_NEWLINE);
	IO_PRINT_LINE ("Id = <%s>." SAS_NEWLINE, mKey);
	IO_PRINT_LINE ("");
	// this function used to call `exit()`
    // if it's needed again, have it return a code and the caller can decide what to do
}
#endif


#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int errorloc(
    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_instatus,

    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outreject,
    T_out_ds out_arr_outreject
){
    double Cardinality;
	EIT_EDITS Edits = { 0 };
    char * EditsString = { 0 };
    int i;
    tInData InData = { 0 };
    tStatusDataSet InStatus = { 0 };
    tSList* OriginalVarNamesInEdits = NULL;
    char KeyName[LEGACY_MAXNAME+1];
    int * KeyPosition;
    char *lpversion;
    char *lpname;
	int * VariablesPosition = { 0 };
	tOutReject OutReject = { 0 };
	tOutStatus OutStatus = { 0 };
    EIPT_POSITIVITY_OPTION PositivityOption;
    int LoggingVerbosityLevel;
	int ByGroupLoggingLevel;
    int rc;
    EIT_RETURNCODE rcLocal = { 0 };
    double TimePerObs;
	EIT_WEIGHTS Weights = { 0 };
    char * WeightsString = { 0 };

	int sr_for_data_initialized   = 0;
	int gw_for_status_initialized = 0;
	int gw_for_reject_initialized = 0;

	BANFF_RETURN_CODE proc_exit_code = BRC_SUCCESS;

#ifdef BSMD_TRANSPORT

#ifdef WIN32
	IO_PRINT_LINE ("avant _control87" SAS_NEWLINE);
    /* Unmask all floating-point exceptions. */
    //_controlfp (0, _MCW_EM);
	IO_PRINT_LINE ("apres _control87" SAS_NEWLINE);
#endif
	IO_PRINT_LINE ("avant signal" SAS_NEWLINE);
    /* if a floating point exception is triggered, call FpSignalHandler() */
    if (signal (SIGFPE, (void (*)(int)) FpSignalHandler) == SIG_ERR ) {
		/* couln't set interrupt signal handling */
		IO_PRINT_LINE ("Couldn't set SIGFPE." SAS_NEWLINE);
		proc_exit_code = BRC_FAIL_UNHANDLED;
		goto error_cleanup;
    }
	{
		double d = 1e3 * 1e3;
		IO_PRINT_LINE ("d=%f" SAS_NEWLINE, d);
		d = 1e30 * 1e30;
		IO_PRINT_LINE ("d=%f" SAS_NEWLINE, d);
		//d = 1e300 * 1e300;
		//IO_PRINT_LINE ("d=%f" SAS_NEWLINE, d);
		//proc_exit_code = BRC_FAIL_UNHANDLED;
		//goto error_cleanup;
	}

    IO_PRINT_LINE ("apres signal" SAS_NEWLINE);
#endif

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

    SP_errorloc sp = { 0 };
    TIME_WALL_START(load_init);
    TIME_CPU_START(load_init);
    mem_usage("before SP_init");
    IO_RETURN_CODE rc_sp_init = SP_init(
        &sp,
        in_parms,
        in_ds_indata,
        in_ds_instatus,
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

    IO_RETURN_CODE rc_validation = SPG_validate((SP_generic*) &sp);
    if (rc_validation != IORC_SUCCESS) {
        proc_exit_code = BRC_FAIL_VALIDATION_NEW;
        goto error_cleanup;
    }

    /* Get procedure parameters except "seed". */
    /* (it'll be get later, inside PrintParms) */
    GetParms (&sp, &InData, &EditsString, &WeightsString,
        &Cardinality, &TimePerObs, &PositivityOption, &LoggingVerbosityLevel, &ByGroupLoggingLevel);

    /* For "seed": get/print it and start the generator number */
    PrintParms (&sp, EditsString, WeightsString,
        Cardinality, TimePerObs, PositivityOption, LoggingVerbosityLevel, ByGroupLoggingLevel);

    /* Initializes the LP environment */
    if (LPI_InitLpEnv (&lpname, &lpversion) != LPI_SUCCESS) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInitLPFail  SAS_NEWLINE, "");
        proc_exit_code = BRC_FAIL_LPI_INIT;
        goto error_cleanup;
    }

    rc = ValidateParms (&sp, EditsString, WeightsString, Cardinality, &TimePerObs,
        PositivityOption, LoggingVerbosityLevel, &Edits, &Weights);
    if (rc == EIE_FAIL) {
        /* LpFreeEnv (); */
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
    }

    /* Find variable position in sas data set of edits variables */
    VariablesPosition = STC_AllocateMemory (
        (Edits.NumberofColumns-1) * sizeof *VariablesPosition);
    rc = FindVariablesPosition (&sp, Edits.FieldName,
        Edits.NumberofColumns-1, VariablesPosition);
    if (rc == EIE_FAIL) {
        /* LpFreeEnv (); */
        proc_exit_code = BRC_FAIL_EDITS_OTHER;
        goto error_cleanup;
    }
    /* Find key position in sas data set */
    KeyPosition = (int *) &(sp.dsr_indata.dsr.VL_unit_id.positions[0]);

    /* Check exclusivity between ID, BY and EDITS variables */
    rc = ExclusivityBetweenLists (&sp, *KeyPosition, VariablesPosition,
        Edits.NumberofColumns-1);
    if (rc == EIE_FAIL) {
        /* LpFreeEnv (); */
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
    }
    /* Get the real case of the variable name from the data set */
    OriginalVarNamesInEdits = FindOriginalNames(&sp,
        Edits.FieldName, Edits.NumberofColumns - 1);

    /* Allocate the key for "ScatterRead" and "GatherWrite" */
    InData.KeyLength = DSR_get_column_length(&sp.dsr_indata.dsr, sp.dsr_indata.dsr.VL_unit_id.positions[0]);
    InData.Key = STC_AllocateMemory (InData.KeyLength+1);
    OutStatus.Key = STC_AllocateMemory (InData.KeyLength+1);
    OutReject.Key = STC_AllocateMemory (InData.KeyLength+1);
    OutStatus.KeyLength = InData.KeyLength;
    OutReject.KeyLength = InData.KeyLength;
    IOUtil_copy_varname(KeyName, sp.dsr_indata.dsr.col_names[KeyPosition[0]]);

    if (sp.dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
        /*************************************************************/
        /* Checks and gets info on mandatory variables in INSTATUS   */
        /*************************************************************/
        InStatus.Key = STC_AllocateMemory(sizeof * InStatus.Key);
        IOUtil_copy_varname(InStatus.Key->Name, KeyName);
        InStatus.FieldId = STC_AllocateMemory(sizeof * InStatus.FieldId);
        InStatus.Status = STC_AllocateMemory(sizeof * InStatus.Status);
        rcLocal = DefineVarsINSTATUS(&sp, &InStatus);
        if (rcLocal == EIE_FAIL) {
            LpFreeEnv();
            proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
            goto error_cleanup;
        }

        /****************************************/
        /* INSTATUSSET : Define scatter read.   */
        /****************************************/
        DefineScatterReadStatus(&sp, &InStatus);
    }

    EI_DataRecAllocate (Edits.FieldName, Edits.NumberofColumns-1,
        &InData.DataRec);
    IO_RETURN_CODE rc_define_scatter_read = DefineScatterRead (&sp, &InData, VariablesPosition);
    if (IORC_SUCCESS != rc_define_scatter_read) {
        proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
        goto error_cleanup;
    }
    sr_for_data_initialized = 1;

    if (PRC_SUCCESS != DefineGatherWriteStatus(&sp, &OutStatus, KeyName)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    gw_for_status_initialized   = 1;

    if (PRC_SUCCESS != DefineGatherWriteReject (&sp, &OutReject, KeyName)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    gw_for_reject_initialized   = 1;

    /* Quit if DATA has no observation */
    //ExitIfNoData (InData.DataSet); //do not call ExitIfNoData. It makes SAS stop in batch mode.

    /* check if INSTATUS data set has all the by variables */
    if (sp.dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
        rcLocal = SUtil_AreAllByVariablesInDataSet(&sp.dsr_indata.dsr, &sp.dsr_instatus.dsr);
        EI_PrintMessages();
        if (rcLocal == EIE_SUCCEED) {
            /* all BY variables were found in INSTATUS data set */
            InStatus.HasAllByVariables = EIE_TRUE;

            /* build BY list for status data set */
            // the following SAS statement need not be replaced //SAS_XBYLIST (InData.DataSet, InStatus.DataSet);
        }
        else {
            /* One or more BY variable was not found in INSTATUS data set */
            InStatus.HasAllByVariables = EIE_FALSE;

            IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet SAS_NEWLINE,
                DSN_INSTATUS);
        }
    }
    TIME_CPU_STOPDIFF(load_init);
    TIME_WALL_STOPDIFF(load_init);

    TIME_WALL_START(processing);
    TIME_CPU_START(processing);

    /* bulk of processing performed in ProcessDataGroups () */
    PROC_RETURN_CODE rc_processing = ProcessByGroups (&sp, &InData, &InStatus, &OutStatus, &OutReject, &Edits, &Weights,
        Cardinality, TimePerObs, PositivityOption, OriginalVarNamesInEdits, LoggingVerbosityLevel, ByGroupLoggingLevel);

    TIME_CPU_STOPDIFF(processing);
    TIME_WALL_STOPDIFF(processing);

    mem_usage("Before SP_wrap");
    /* CONSIDERATION: should `ProcessByGroups`' RC be handled before SP_wrap?
        - Not until there's more meaningful RC being returned */
    if (PRC_SUCCESS != SP_wrap(&sp)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    mem_usage("After SP_wrap");

    TIME_WALL_BEGIN(cleanup);
    TIME_CPU_BEGIN(cleanup);

    proc_exit_code = convert_processing_rc(rc_processing);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }

    goto normal_cleanup;

error_cleanup:

normal_cleanup:
    if ((VariablesPosition                != NULL)
    ) {
        STC_FreeMemory (VariablesPosition);
        VariablesPosition = NULL;
    }
    if ((Edits.FieldName                  != NULL) &&
        (Edits.EditId                     != NULL) &&
        (Edits.Coefficient                != NULL)
    ) {
        EI_EditsFree   (&Edits);
        Edits.FieldName   = NULL;
        Edits.EditId      = NULL;
        Edits.Coefficient = NULL;
    }
    if ((Weights.Value                    != NULL)
    ) {
        EI_WeightsFree (&Weights);
        Weights.Value = NULL;
    }
    if ((InData.Key                       != NULL)
    ) {
        STC_FreeMemory (InData.Key);
        InData.Key = NULL;
    }
    if (((&InData.DataRec)                != NULL) &&
        ((&InData.DataRec)->FieldValue    != NULL) &&
        ((&InData.DataRec)->StatusFlag    != NULL)
    ) {
        EI_DataRecFree (&InData.DataRec);
        /*-this structure has a pointer "FieldName" to a list of strings, */
        /* but it shares that list of strings with the "Edits" structure  */
        /* which is treated as the owner of the list of strings, so the   */
        /* list of strings isn't freed by "EI_DataRecFree()"              */
        (&InData.DataRec)->FieldName  = NULL; /* <- points to the same memory as Edits.FieldName */
        (&InData.DataRec)->FieldValue = NULL;
        (&InData.DataRec)->StatusFlag = NULL;
    }

    if (InStatus.Key != NULL) {
        if (InStatus.Key->Value != NULL) {
            STC_FreeMemory(InStatus.Key->Value);
            InStatus.Key->Value = NULL;
        }
        STC_FreeMemory(InStatus.Key);
        InStatus.Key = NULL;
    }

    if (InStatus.PreviousKey != NULL) {
        STC_FreeMemory(InStatus.PreviousKey);
        InStatus.PreviousKey = NULL;
    }

    if (InStatus.FieldId != NULL) {
        if (InStatus.FieldId->Value != NULL) {
            STC_FreeMemory(InStatus.FieldId->Value);
            InStatus.FieldId->Value = NULL;
        }
        STC_FreeMemory(InStatus.FieldId);
        InStatus.FieldId = NULL;
    }

    if (InStatus.Status != NULL) {
        if (InStatus.Status->Value != NULL) {
            STC_FreeMemory(InStatus.Status->Value);
            InStatus.Status->Value = NULL;
        }
        STC_FreeMemory(InStatus.Status);
        InStatus.Status = NULL;
    }

    if (OriginalVarNamesInEdits != NULL) {
        SList_Free(OriginalVarNamesInEdits);
        OriginalVarNamesInEdits = NULL;
    }

    if ((OutStatus.Key                    != NULL)
    ) {
        STC_FreeMemory (OutStatus.Key);
        OutStatus.Key = NULL;
    }
    if ((OutReject.Key                    != NULL)
    ) {
        STC_FreeMemory (OutReject.Key);
        OutReject.Key = NULL;
    }

    /* Terminate Gather Write */
    if ((gw_for_status_initialized        != 0   )
    ) {
        gw_for_status_initialized = 0;
    }
    if ((gw_for_reject_initialized        != 0   )
    ) {
        gw_for_reject_initialized = 0;
    }
    /* Terminate Scatter Read */
    if ((sr_for_data_initialized          != 0   )
    ) {
        sr_for_data_initialized = 0;
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
    SPG_free((SP_generic*) &sp);
    mem_usage("After SPG_free");

    /* TIME MEASUREMENT */
    TIME_CPU_STOPDIFF(cleanup);
    TIME_WALL_STOPDIFF(cleanup);
    TIME_CPU_STOPDIFF(main);
    TIME_WALL_STOPDIFF(main);

    deinit_runtime_env();

    return (proc_exit_code);
}

/*****************************************************/
/* Function: CheckCharVarInStatus                    */
/*                                                   */
/* Purpose:- Checks that a VariableName exists in    */
/*           INSTATUS and it's a character variable. */
/*         - Gets position and name.                 */
/*****************************************************/
static EIT_RETURNCODE CheckCharVarInStatus(
    SP_errorloc* sp,
    char* VariableName,
    T_CHAR_VARIABLE* Variable)
{
    EIT_RETURNCODE rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_instatus.dsr;

    IOUtil_copy_varname(Variable->Name, VariableName);
    IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &VariableName, &Variable->Position);
    if (rc_io == IORC_VARLIST_NOT_FOUND) {
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet,
            Variable->Name, dsr->dataset_name);
        rc = EIE_FAIL;
    }
    else {
        if (IOVT_CHAR != DSR_get_col_type(dsr, Variable->Position)) {
            IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet,
                Variable->Name, dsr->dataset_name);
            rc = EIE_FAIL;
        }
    }

    return rc;
}

/* set datarec flag */
static void CheckDataValidity (
    SP_errorloc* sp,
    EIT_DATAREC * DataRec,
    EIT_WEIGHTS * Weights,
    EIPT_POSITIVITY_OPTION PositivityOption,
    double * Ubound,
    double * Lbound,
    EIT_BOOLEAN * InvalidData,
    double * CardinalityOfInvalidData)
{
    int i;

    *InvalidData = EIE_FALSE;
    *CardinalityOfInvalidData = 0.0;

    for (i = 0; i < DataRec->NumberofFields; i++) {

        /* Errorloc Instatus Implementation
            When a field is marked as FTI we set it to MISSING, and subsequently
            let the procedure handle the data as-if it were originally missing from indata.  
            This emulates how the SAS based Banff Processor behaved when an instatus file was
            available for an errorloc step.  */
        if (DataRec->StatusFlag[i] == FIELDFTI) {
            DataRec->FieldValue[i] = IOUtil_missing_value('.');
        }

        if (IOUtil_is_missing (DataRec->FieldValue[i])) {
            /* set datarec flag to missing */
            UpdateDataForEditsApplyChern (i, FIELDMISS, DataRec, Weights,
                Ubound, Lbound, InvalidData, CardinalityOfInvalidData);
        }
        else if (EIM_DBL_LT (DataRec->FieldValue[i], 0.0) &&
            PositivityOption != EIPE_ACCEPT_NEGATIVE) {
            /* Flag negative values as FIELDNEG if Positivity Option given by
            the user is REJECTNEGATIVE (by default or explicitely).
            FIELDNEG will become FTI*/
            UpdateDataForEditsApplyChern (i, FIELDNEG, DataRec, Weights,
                Ubound, Lbound, InvalidData, CardinalityOfInvalidData);
        }
        else if ((EIM_DBL_LT (DataRec->FieldValue[i], Lbound[i]) &&
                Lbound[i] != EIM_NOBOUND) ||
            (EIM_DBL_GT (DataRec->FieldValue[i], Ubound[i]) &&
                (Ubound[i] != EIM_NOBOUND))) {
            /* If a variable is outside of its bounds, set it to its lower
            bound. I set to FIELDFTI because I couldn't think of anything
            better. The important thing is that it is NOT set to FIELDOK.*/
            UpdateDataForEditsApplyChern (i, FIELDFTI, DataRec, Weights,
                Ubound, Lbound, InvalidData, CardinalityOfInvalidData);
        }
        else {
            /* set datarec flag to ok */
            DataRec->StatusFlag[i] = FIELDOK;
        }
    }
}

/*
Check the order of the keys in the data set.
*/
static EIT_RETURNCODE CheckKeyOrder (
    char * PreviousKey,
    char * Key)
{
    int rc;

    rc = strcmp (PreviousKey, Key);
    if (rc > 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted  SAS_NEWLINE,
            DSN_INDATA, Key, PreviousKey);
        return EIE_FAIL;
    }
    else if (rc == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysDuplicate  SAS_NEWLINE,
            DSN_INDATA, Key, PreviousKey);
        return EIE_FAIL;
    }

    strcpy (PreviousKey, Key);

    return EIE_SUCCEED;
}

/*********************************************************************
Check the order of the keys in the INSTATUS data set.
*********************************************************************/
static EIT_RETURNCODE CheckStatusKeyOrder(
    char* PreviousKey,
    char* Key)
{
    int rc;

    rc = strcmp(PreviousKey, Key);
    if (rc > 0) {
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted SAS_NEWLINE,
            DSN_INSTATUS, Key, PreviousKey);
        return EIE_FAIL;
    }

    strcpy(PreviousKey, Key);

    return EIE_SUCCEED;
}

/*
Define gather write for reject data set, it contains 2 variables:
The key and NAME_ERROR.
- The key is a character variable of the same length as the variable
  specified by the ID statement.
- NAME_ERROR is a character variable of LEGACY_MAXNAME length.
BY variables are added if there is a BY statement.
*/
static PROC_RETURN_CODE DefineGatherWriteReject (
    SP_errorloc* sp,
    tOutReject * OutReject,
    char * KeyNameInData)
{
    DSW_generic* dsw = &sp->dsw_outreject;
    
    if (IORC_SUCCESS != DSW_allocate(dsw, REJECT_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* For key */
    // copy name from input column name
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, KeyNameInData, (void*)OutReject->Key, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* For NAME_ERROR */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, REJECT_NAME_ERROR, (void*)OutReject->NameError, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for status data set, it contains 3 variables:
The key, FIELDID and STATUS.
- The key is a character variable of the same length as the variable
  specified by the ID statement.
- FIELDID is a character variable of the maximum length allowed for a variable
  name in SAS. we use LEGACY_MAXNAME to specify it.
- STATUS a character variable of EIM_STATUS_SIZE length.
BY variables are added if there is a BY statement.
*/
static PROC_RETURN_CODE DefineGatherWriteStatus (
    SP_errorloc* sp,
    tOutStatus * OutStatus,
    char * KeyNameInData)
{
    /* since we always write the same status (FTI) we may as well set it here */
    SUtil_CopyPad(OutStatus->Status, EIM_STATUS_FTI, EIM_STATUS_SIZE);

    return banff_setup_dsw_outstatus(
        &sp->dsw_outstatus,
        KeyNameInData,
        (void*)OutStatus->Key,
        (void*)OutStatus->FieldId,
        (void*)OutStatus->Status,
        (void*)&OutStatus->FieldValue
    );
}

/*
Define scatter read for DATA
*/
static IO_RETURN_CODE DefineScatterRead (
    SP_errorloc* sp,
    tInData * InData,
    int * List)
{
    DSR_indata* dsr = &sp->dsr_indata;


    /* For key */
    dsr->dsr.VL_unit_id.ptrs[0] = InData->Key;

    if (InData->RandNumFlag != 0) {
        /* For random number */
        dsr->VL_rand_num.ptrs[0] = &(InData->RandomNumber);
    }

    /* manually configure the in var varlist
        - Note that names need not be specified */
    IO_RETURN_CODE rc_vl_init = VL_init_from_position_list(&dsr->dsr.VL_in_var, &dsr->dsr, VL_NAME_in_var, IOVT_NUM, InData->DataRec.NumberofFields, List);
    if (IORC_SUCCESS != rc_vl_init) {
        return rc_vl_init;
    }
    for (int i = 0; i < dsr->dsr.VL_in_var.count; i++) {
        /* Define var. for scatter read */
        dsr->dsr.VL_in_var.ptrs[i] = (InData->DataRec.FieldValue + i);
    }

    return IORC_SUCCESS;
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
static void DefineScatterReadStatus(
    SP_errorloc* sp,
    tStatusDataSet* InStatus)
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
static EIT_RETURNCODE DefineVarsINSTATUS(
    SP_errorloc* sp,
    tStatusDataSet* InStatus)
{
    int rc = EIE_SUCCEED;

    InStatus->Key->Value = NULL;
    InStatus->FieldId->Value = NULL;
    InStatus->Status->Value = NULL;

    /* Var: "key" (idem to KeyDATA.Name) */
    if (CheckCharVarInStatus(sp, InStatus->Key->Name,
        InStatus->Key) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->Key->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->Key->Position);
        InStatus->Key->Value = STC_AllocateMemory(InStatus->Key->Size + 1);
        InStatus->PreviousKey = STC_AllocateMemory(InStatus->Key->Size + 1);
    }

    /* Var: FIELDID */
    if (CheckCharVarInStatus(sp, SUTIL_FIELDSTAT_FIELDID,
        InStatus->FieldId) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->FieldId->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->FieldId->Position);
        InStatus->FieldId->Value = STC_AllocateMemory(
            InStatus->FieldId->Size + 1);
    }

    /* Var: STATUS */
    if (CheckCharVarInStatus(sp, SUTIL_FIELDSTAT_STATUS,
        InStatus->Status) == EIE_FAIL)
        rc = EIE_FAIL;
    else {
        InStatus->Status->Size = DSR_get_column_length(&sp->dsr_instatus.dsr, InStatus->Status->Position);
        InStatus->Status->Value = STC_AllocateMemory(InStatus->Status->Size + 1);
    }

    /* A variable is not in INSTATUS or  */
    /* wrong type: free memory.          */
    if (rc == EIE_FAIL) {
        STC_FreeMemory(InStatus->Key->Value);
        STC_FreeMemory(InStatus->FieldId->Value);
        STC_FreeMemory(InStatus->Status->Value);
        InStatus->Key->Value = NULL;
        InStatus->FieldId->Value = NULL;
        InStatus->Status->Value = NULL;
    }

    return rc;
}

/*
Calls EI_ErrorLocalization ()
*/
static int DoChernikova (
    SP_errorloc* sp,
    tInData * InData,
    tOutStatus * OutStatus,
    tOutReject * OutReject,
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights,
    double Cardinality,
    double TimePerObs,
    EIT_CHERNIKOVA * Chernikova,
    EIT_CHERNIKOVA_COMPANION * ChernikovaCompanion,
    tCounter * Counter,
    int LoggingVerbosityLevel,
    tSList* OriginalVarNamesInEdits)
{
    int i;
    long rc;
    short* VariableIndexList = { 0 }; /*Initialize pointer to NULL*/
    int function_rc;
    
    function_rc = 0;

    rc = EI_ErrorLocalization (Edits, Weights, &InData->DataRec,
        Cardinality, TimePerObs, Chernikova, ChernikovaCompanion);
    EI_PrintMessages ();

    switch (rc) {
    case EIE_ERRORLOCALIZATION_SUCCEED:
        Counter->FailedEditsSolutionFound++;
        if (DEBUG) EI_ChernikovaPrintSolutions (Chernikova, &InData->DataRec);
        if (LoggingVerbosityLevel > 0) {
            IO_PRINT_LINE("Choosing solution for record with key \"%s\".", InData->Key);
        }
		VariableIndexList = STC_AllocateMemory(
			sizeof * VariableIndexList * InData->DataRec.NumberofFields);
        if (0 != EI_ChernikovaChooseSolution (Chernikova, InData->DataRec.NumberofFields,
                VariableIndexList, InData->RandNumFlag, InData->RandomNumber, LoggingVerbosityLevel)
        ) {
            function_rc = -1;
            goto error_cleanup;
        }
        SUtil_CopyPad (OutStatus->Key, InData->Key, OutStatus->KeyLength);
        for (i = 0; VariableIndexList[i] != -1; i++) {
            Counter->FailedEditsSolutionFoundVar++;
            SUtil_CopyPad (OutStatus->FieldId,
                OriginalVarNamesInEdits->l[VariableIndexList[i]], LEGACY_MAXNAME);
            // copy value from `indata` dataset
            IO_RETURN_CODE rc_io = DSR_read_num(
                &sp->dsr_indata.dsr,
                sp->dsr_indata.dsr.rec_cursor.cur_record_index,
                sp->dsr_indata.dsr.VL_in_var.positions[VariableIndexList[i]],
                &OutStatus->FieldValue);
            if (rc_io != IORC_SUCCESS) {
                function_rc = -1;
                goto error_cleanup;
            }
            if (DEBUG) PrintStatus (OutStatus);

            if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                function_rc = -1;
                goto error_cleanup;
            }
        }
        break;

    case EIE_ERRORLOCALIZATION_CARDINALITY_EXCEEDED:
        Counter->FailedEditsSolutionNotFound++;
        Counter->FailedEditsSolutionNotFoundCardinality++;
        SUtil_CopyPad (OutReject->Key, InData->Key, OutReject->KeyLength);
        SUtil_CopyPad (OutReject->NameError, REJECT_ERROR_CARDINALITY,
            LEGACY_MAXNAME);

        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outreject)) {
            function_rc = -1;
            goto error_cleanup;
        }
        break;

    case EIE_ERRORLOCALIZATION_SPACE_EXCEEDED:
        Counter->FailedEditsSolutionNotFound++;
        Counter->FailedEditsSolutionNotFoundSpace++;
        SUtil_CopyPad (OutReject->Key, InData->Key, OutReject->KeyLength);
        SUtil_CopyPad (OutReject->NameError, REJECT_ERROR_SPACE,LEGACY_MAXNAME);

        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outreject)) {
            function_rc = -1;
            goto error_cleanup;
        }
        break;

    case EIE_ERRORLOCALIZATION_TIME_EXCEEDED:
        Counter->FailedEditsSolutionNotFound++;
        Counter->FailedEditsSolutionNotFoundTime++;
        SUtil_CopyPad (OutReject->Key, InData->Key, OutReject->KeyLength);
        SUtil_CopyPad (OutReject->NameError, REJECT_ERROR_TIME, LEGACY_MAXNAME);

        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outreject)) {
            function_rc = -1;
            goto error_cleanup;
        }
        break;
    }
    goto normal_cleanup;

error_cleanup:

normal_cleanup:
	if (VariableIndexList != NULL) {												//free VariableIndexList
		STC_FreeMemory(VariableIndexList);
		VariableIndexList = NULL;
	}
    return function_rc;
}

/*
*/
static EIT_RETURNCODE DoErrorLocalization (
    SP_errorloc* sp,
    tInData * InData,
    tOutStatus * OutStatus,
    tOutReject * OutReject,
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights,
    double Cardinality,
    double TimePerObs,
    EIPT_POSITIVITY_OPTION PositivityOption,
    int LoggingVerbosityLevel,
    double * Ubound,
    double * Lbound,
    EIT_CHERNIKOVA * Chernikova,
    EIT_CHERNIKOVA_COMPANION  * ChernikovaCompanion,
    tCounter * Counter,
    tSList* OriginalVarNamesInEdits)
{
    double CardinalityOfInvalidData;
    EIT_DATAREC * DataRec;
    EIT_BOOLEAN InvalidData;
    int j;
    EIT_EDITSAPPLYCHERN_RETURNCODE rc;

    DataRec = &InData->DataRec;

    CheckDataValidity (sp, DataRec, Weights, PositivityOption, Ubound, Lbound,
        &InvalidData, &CardinalityOfInvalidData);

    rc = EI_EditsApplyChern (InvalidData, Edits, DataRec);
    switch (rc) {
    case EIE_EDITSAPPLYCHERN_SUCCEED:
        Counter->PassedEdits++;
        break;

    case EIE_EDITSAPPLYCHERN_FAILDOCHERN:
        Counter->FailedEdits++;
        if (InvalidData)
            SetFieldValueOutsideLimit (DataRec, Ubound, Lbound);

        if (0 != DoChernikova (sp, InData, OutStatus, OutReject, Edits, Weights,
            Cardinality, TimePerObs, Chernikova, ChernikovaCompanion, Counter, LoggingVerbosityLevel, OriginalVarNamesInEdits)
        ) {
            return EIE_FAIL;
        }
        break;

    case EIE_EDITSAPPLYCHERN_FAILNOCHERN:
        Counter->FailedEdits++;
        if (CardinalityOfInvalidData > Cardinality) {
            Counter->FailedEditsSolutionNotFound++;
            Counter->FailedEditsSolutionNotFoundCardinality++;
            SUtil_CopyPad (OutReject->Key, InData->Key, OutReject->KeyLength);
            SUtil_CopyPad (OutReject->NameError, REJECT_ERROR_CARDINALITY,
                LEGACY_MAXNAME);

            if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outreject)) {
                return EIE_FAIL;
            }
        }
        else {
            Counter->FailedEditsSolutionFound++;
            SUtil_CopyPad (OutStatus->Key, InData->Key, OutStatus->KeyLength);
            for (j = 0; j < DataRec->NumberofFields; j++) {
                if (DataRec->StatusFlag[j] != FIELDOK) {
                    Counter->FailedEditsSolutionFoundVar++;
                    SUtil_CopyPad (OutStatus->FieldId,
                        OriginalVarNamesInEdits->l[j], LEGACY_MAXNAME);
                    // copy value from `indata` dataset
                    IO_RETURN_CODE rc_io = DSR_read_num(
                        &sp->dsr_indata.dsr,
                        sp->dsr_indata.dsr.rec_cursor.cur_record_index,
                        sp->dsr_indata.dsr.VL_in_var.positions[j],
                        &OutStatus->FieldValue);
                    if (rc_io != IORC_SUCCESS) {
                        return EIE_FAIL;
                    }

                    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                        return EIE_FAIL;
                    }
                }
            }
        }
        break;

    case EIE_EDITSAPPLYCHERN_FAIL:
    default:
        return EIE_FAIL;
    } /* switch */

    return EIE_SUCCEED;
}

/*
Checks  exclusivity between lists:
- BY and ID variables position
- BY and EDITS variables position
(not between ID and EDITS because their type is
different and verified by the grammar or by the parser)
*/
static EIT_RETURNCODE ExclusivityBetweenLists (
    SP_errorloc* sp,
    int  KeyPosition,
    int * VariablesPosition,
    int EditsNumberofFields)
{
    int* ByList;
    int i;
    int j;
    int NumberOfByVariables;
    int rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    ByList = dsr->VL_by_var.positions;
    NumberOfByVariables = dsr->VL_by_var.count;

    /* BY and ID variables */
    for (i = 0; i < NumberOfByVariables; i++) {
         if (ByList[i] == KeyPosition) {
             IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_UNIT_ID, BPN_BY);
             rc = EIE_FAIL;
             break;
         }
    }

    /* BY and EDITS variables */
    for (i = 0; i < NumberOfByVariables; i++) {
        for (j = 0; j < EditsNumberofFields; j++) {
            if (ByList[i] == VariablesPosition[j]) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_EDITS, BPN_BY);
                rc = EIE_FAIL;
                j = EditsNumberofFields;
                i = NumberOfByVariables;
            }
        }
    }

    return rc;
}

/*
Exit if no observation in DATA.
XO_ANY can't return -1 because the key and the edits variables exist
*/
static void ExitIfNoData (
   SP_errorloc* sp)
{

    if(sp->dsr_indata.dsr.num_records == 0){
        EI_FreeMessageList ();
        LpFreeEnv ();
        IO_PRINT_LINE ( SAS_NEWLINE SAS_MESSAGE_PREFIX_WARNING MsgNoObservationsInDataSet  SAS_NEWLINE, DSN_INDATA);
        return; // definitely don't call `exit()`, just have main return 
    }
}

/*********************************************************************
Create a list of original names of the variables
in the Edits as they appear in the INDATA data set
*********************************************************************/
static tSList* FindOriginalNames(
    SP_errorloc* sp,
    char** VariableName,
    int NumberOfVariables)
{
    int i;
    tSList* OriginalVarNamesInEdits;

    SList_New(&OriginalVarNamesInEdits);

    for (i = 0; i < NumberOfVariables; i++) {
        SList_Add(VariableName[i], OriginalVarNamesInEdits);
        /* It can't fail, existence has already been tested */
        SUtil_GetOriginalName(&sp->dsr_indata.dsr, OriginalVarNamesInEdits->l[i]);
    }

    return OriginalVarNamesInEdits;
}

/*
Build a list containing the position of the variables corresponding
to the EDITS fields that can be found on the data set pointed to by
InDataSet.
*/
static EIT_RETURNCODE FindVariablesPosition (
    SP_errorloc* sp,
    char ** FieldName,
    int NumberofFields,
    int* VariablesPosition)
{
    int i;
    EIT_RETURNCODE rcLocal;
    long VariableType;

    rcLocal = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    DSR_get_pos_from_names(dsr, NumberofFields, FieldName, (int*)VariablesPosition);
    for (i = 0; i < NumberofFields; i++) {
        if (VariablesPosition[i] == IOSV_VAR_MISSING) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotInDataSet  SAS_NEWLINE,
                FieldName[i], dsr->dataset_name);
            rcLocal = EIE_FAIL;
        }
        else {
            VariableType = DSR_get_col_type(dsr, VariablesPosition[i]);
            if (VariableType != SUTIL_VARIABLE_TYPE_NUMERIC) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotNumericInDataSet,
                    FieldName[i], dsr->dataset_name);
                rcLocal = EIE_FAIL;
            }
        }
    }

    return rcLocal;
}

/*
Get the upper and lower bounds of the variables in Edits.
*/
static int GetBoundsVariables (
    EIT_EDITS * Edits,
    double Ubound[],
    double Lbound[])
{
    int i;
    int rc;

    rc = 0;

    if (EI_EditsBounds (Edits, Ubound, Lbound)) {
        for (i = Edits->NumberofColumns-2; i >= 0; i--) {
            if ((Lbound[i] == Ubound[i]) && (Lbound[i] == EIM_NOBOUND)) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameUnbounded, Edits->FieldName[i]);
                /* LpFreeEnv (); */
                rc = -1;
                goto error_cleanup;
            }
        }
    }
    else {
        /* Case: can't make the edits the lp constraints */
        EI_PrintMessages ();
        /* LpFreeEnv (); */
        rc = -1;
        goto error_cleanup;
    }
    goto normal_cleanup;

error_cleanup:

normal_cleanup:
    return rc;
}

/*
Reads procedure parameters (excepted "seed").
Parameters not set are either set to their default value or mark as not set.
*/
static void GetParms (
    SP_errorloc* sp,
    tInData * InData,
    char ** EditsString,
    char ** WeightsString,
    double * Cardinality,
    double * TimePerObs,
    EIPT_POSITIVITY_OPTION * PositivityOption,
    int * LoggingVerbosityLevel,
	int * ByGroupLoggingLevel)
{

    if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *EditsString = (char*) NULL;
    } else {
        *EditsString = (char*) sp->edits.value;
    }

    if (sp->weights.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *WeightsString = (char*) NULL;
    } else {
        *WeightsString = (char*) sp->weights.value;
    }

    if (sp->cardinality.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *Cardinality = DBL_MAX;
    } else {
        *Cardinality = sp->cardinality.value;
    }

    if (sp->time_per_obs.meta.is_specified == IOSV_NOT_SPECIFIED) {// if not specified or if provided value of 0
        *TimePerObs = DEFAULT_TIMEPEROBSPARM;
    } else {
        *TimePerObs = sp->time_per_obs.value;
    }

    /* Get the positivity option set by the user */
    if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
        *PositivityOption = EIPE_REJECT_NEGATIVE_DEFAULT;
    else if (sp->accept_negative.value == IOB_FALSE)
        *PositivityOption = EIPE_REJECT_NEGATIVE;
    else if (sp->accept_negative.value == IOB_TRUE)
        *PositivityOption = EIPE_ACCEPT_NEGATIVE;

    if (sp->display_level.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *LoggingVerbosityLevel = 0;
    } else {
        *LoggingVerbosityLevel = (int) sp->display_level.value;
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

    if (VL_is_specified(&sp->dsr_indata.VL_rand_num)) {
        InData->RandNumFlag = 1;
    }
    else {
        InData->RandNumFlag = 0;
    }
}

/*
Initialize components of Counter structure
*/
static void InitCounter (
    tCounter * Counter)
{
    Counter->Obs = 0;
    Counter->MissingKey = 0;
    Counter->MissingStatus = 0;
    Counter->PassedEdits = 0;
    Counter->FailedEdits = 0;
    Counter->FailedEditsSolutionFound = 0;
    Counter->FailedEditsSolutionFoundVar = 0;
    Counter->FailedEditsSolutionNotFound = 0;
    Counter->FailedEditsSolutionNotFoundCardinality = 0;
    Counter->FailedEditsSolutionNotFoundSpace = 0;
    Counter->FailedEditsSolutionNotFoundTime = 0;
}

/**************************************************/
/* Function: LpFreeEnv                            */
/*                                                */
/* Purpose:  Frees the ressources used by the LP  */
/*    package (initialized by LPI_InitLpEnv)      */
/**************************************************/
static void LpFreeEnv (void)
{
    char ilperrmsg[1024];
    if (LPI_FreeLpEnv (ilperrmsg) != LPI_SUCCESS)
        /* printing a generic message is enough */
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgFunctionFailure, "LPI_FreeLpEnv()");
}

/*************************************************************
Reads all INSTATUS obs belonging to the current DATA key.
Set status having FTI status for the key processed.

This function assumes that InStatus contains valid data.
*************************************************************/
static PROC_RETURN_CODE MatchStatus(
    SP_errorloc* sp,
    tInData* InData,
    tStatusDataSet* InStatus,
    tCounter* Counter)
{
    int l = -1;
    PROC_RETURN_CODE rc = PRC_SUCCESS;

    while (rc == PRC_SUCCESS) {
        l = strcmp(InData->Key, InStatus->Key->Value);

        if (l == 0) {
            /* this instatus key is equal to the indata key */
            SetStatus(&InData->DataRec, InStatus->FieldId->Value);
        }
        else if (l < 0) {
            /* this instatus key is greater than the indata key */
            return PRC_SUCCESS; // NOTE: there may be more data, just not for the current `InData->Key`
        }

        /* at this point either the indata key and instatus key are equal and
        we want to continue to read until we find all matching obs or,
        instatus key is lower and we must read until we find matching obs */

        rc = ReadStatus(sp, InStatus, Counter);
    }

    return rc; /* error RC or PRC_SUCCESS_NO_MORE_DATA */
}

/*
Prints parameters
(for "seed", get/print it and start the generator number)
*/
static void PrintParms (
    SP_errorloc* sp,
    char * EditsString,
    char * WeightsString,
    double Cardinality,
    double TimePerObs,
    EIPT_POSITIVITY_OPTION PositivityOption,
    int LoggingVerbosityLevel,
	int ByGroupLoggingLevel)
{
    double DoubleSeed;
    int RandSeed;
	char s[101];

    SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);

    SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outreject);

    if (EditsString == NULL || strcmp (EditsString, "") == 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, EditsString);

    if (WeightsString == NULL || strcmp (WeightsString, "") == 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified " " MsgWeightVarDefault, BPN_WEIGHTS);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_WEIGHTS, WeightsString);

    if (Cardinality == DBL_MAX)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified " " MsgNotConstrainedByCardinality, BPN_CARDINALITY);
    else {
		sprintf (s, "%.*g", 6, Cardinality);//no %g modifier in SAS_XPSLOG()
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_CARDINALITY, s);
	}

	sprintf (s, "%.*g", 6, TimePerObs);
    if (sp->time_per_obs.meta.is_specified == IOSV_NOT_SPECIFIED) // if not provided, or provided value of 0
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString MsgDefault, BPN_TIME_PER_OBS, s);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_TIME_PER_OBS, s);

    /* Seed: get/print it and start the generator number.
       Take time() if none is given or a value < 1 or out of range;
       print also a warning if the given value is negative, 0 or out of range.*/
    if (sp->seed.meta.is_specified == IOSV_NOT_SPECIFIED) {
        RandSeed = (int) time (NULL);
		/*Ideally, it would be preferable to not display this Note SEED = .... (value chosen by the system) 
		  when RANDNUMVAR is specified but SEED is not, as it can be confusing for the user.*/
        if (!VL_is_specified(&sp->dsr_indata.VL_rand_num)) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
         }
    }
    else {
        DoubleSeed = (double) sp->seed.value;
        if (DoubleSeed >= 1.0 && DoubleSeed <= EIM_MAX_INT ) {
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

    if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOption == EIPE_REJECT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE BPN_DISPLAY_LEVEL " = %d", LoggingVerbosityLevel);

    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, sp->dsr_indata.dsr.VL_unit_id.names[0]);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

    if (VL_is_specified(&sp->dsr_indata.VL_rand_num)) {
         IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_RAND_NUM_VAR, sp->dsr_indata.VL_rand_num.names[0]);
		/*when both the SEED and RANDNUMVAR parameters have been specified*/
	    if (sp->seed.meta.is_specified == IOSV_SPECIFIED) {
             IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgRanndumvarSeedSpecified);
         }
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_RAND_NUM_VAR);

    /* List of BY-variables - not mandatory */

    DSR_generic* dsr = &sp->dsr_indata.dsr;
    SUtil_PrintStatementVars (&sp->by.meta, dsr, &dsr->VL_by_var, BPN_BY);

    IO_PRINT_LINE ("");
}

/*
Prints statistics for one "by group"
*/
static void PrintStatistics (
    tCounter * Counter)
{

#define  ONETAB   4
#define  TWOTABS  8

/* Used when printing count and percent */
#define PRINT_LENGTH_FLOAT    6
#define PRINT_LENGTH_INTERSPACES  5 /* nb of blanks between count and percent*/
#define PRINT_LENGTH_SYMBOL   1     /* for symbol % */
#define PRINT_NUMBER_DECIMAL  2

/* PRINT_COUNT_PERCENT() is used to print the count of obs and percent.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT_PERCENT(Msg, LengthPlaceHolders, Count, Percent)  \
IO_PRINT_LINE (Msg, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - PRINT_PRECISION - \
PRINT_LENGTH_FLOAT - PRINT_LENGTH_INTERSPACES + 2 - PRINT_LENGTH_SYMBOL), \
"............................................................", \
PRINT_PRECISION, Count, \
PRINT_LENGTH_INTERSPACES - 2, MsgPadding, \
PRINT_LENGTH_FLOAT, PRINT_NUMBER_DECIMAL, Percent);

/* PRINT_SUBTOTAL_PERCENT() is used to print subtotal of obs and percent,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is ONETAB or TWOTABS */
#define PRINT_SUBTOTAL_PERCENT(Msg, LengthPlaceHolders, LengthHeadingSpace, \
Count, Percent) \
IO_PRINT_LINE (Msg, \
LengthHeadingSpace, MsgPadding,\
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - \
PRINT_PRECISION - PRINT_LENGTH_FLOAT - PRINT_LENGTH_INTERSPACES + 2 - \
PRINT_LENGTH_SYMBOL), \
"............................................................", \
PRINT_PRECISION, Count, \
PRINT_LENGTH_INTERSPACES - 2, "                                ", \
PRINT_LENGTH_FLOAT, PRINT_NUMBER_DECIMAL, Percent);

/* PRINT_SUBTOTAL_NOPERCENT() is used to print subtotal of observations,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is ONETAB or TWOTABS
   (PRINT_LENGTH_INTERSPACES, PRINT_LENGTH_FLOAT and  PRINT_LENGTH_SYMBOL
    are used to calculate length of string "...." and align the
    statistics. */
#define PRINT_SUBTOTAL_NOPERCENT(Msg, LengthPlaceHolders, LengthHeadingSpace, \
Count) \
IO_PRINT_LINE (Msg, \
LengthHeadingSpace, \
MsgPadding,\
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - \
PRINT_PRECISION- PRINT_LENGTH_FLOAT - PRINT_LENGTH_INTERSPACES - \
PRINT_LENGTH_SYMBOL), \
"............................................................", \
PRINT_PRECISION, Count);


    IO_PRINT_LINE ("");
    PRINT_COUNT_PERCENT (MsgNumberPercentObs, MsgNumberPercentObs_LPH,
        Counter->Obs, 100.0);

    if (Counter->Obs <= 0) {
        /* Don't print anything else; skip this section.*/
        /* if Counter->Obs == 0, you will get an error. */
        IO_PRINT_LINE ("");
        return;
    }

    if (Counter->MissingKey > 0) {
        PRINT_COUNT_PERCENT (MsgNumberPercentObsDroppedMissingKey,
            MsgNumberPercentObsDroppedMissingKey_LPH,
            Counter->MissingKey,
            (double) 100 * Counter->MissingKey / Counter->Obs);
    }

    if (Counter->Obs - Counter->MissingKey == 0) {
        /* skip remaining */
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNoValidDataInGroup);
        IO_PRINT_LINE ("");
        return;
    }

    PRINT_COUNT_PERCENT (MsgNumberPercentObsPassedEdits,
        MsgNumberPercentObsPassedEdits_LPH,
        Counter->PassedEdits,
        (double) 100 * Counter->PassedEdits / Counter->Obs);

    PRINT_COUNT_PERCENT (MsgNumberPercentObsFailedEdits,
        MsgNumberPercentObsFailedEdits_LPH,
        Counter->FailedEdits,
        (double) 100 * Counter->FailedEdits / Counter->Obs);

    PRINT_SUBTOTAL_PERCENT (MsgNumberPercentObsWithVarsToImpute,
        MsgNumberPercentObsWithVarsToImpute_LPH, ONETAB,
        Counter->FailedEditsSolutionFound,
        (double) 100 * Counter->FailedEditsSolutionFound / Counter->Obs);

    PRINT_SUBTOTAL_NOPERCENT (MsgNumberVarsToImpute,
        MsgNumberVarsToImpute_LPH, TWOTABS,
        Counter->FailedEditsSolutionFoundVar);

    if (Counter->FailedEditsSolutionNotFound > 0) {
        PRINT_SUBTOTAL_PERCENT (MsgNumberPercentObsNoSolution,
            MsgNumberPercentObsNoSolution_LPH, ONETAB,
            Counter->FailedEditsSolutionNotFound,
            (double) 100 * Counter->FailedEditsSolutionNotFound / Counter->Obs);

        if (Counter->FailedEditsSolutionNotFoundCardinality > 0) {
            PRINT_SUBTOTAL_PERCENT (MsgNumberPercentObsNoSolutionCardinality,
                MsgNumberPercentObsNoSolutionCardinality_LPH, TWOTABS,
                Counter->FailedEditsSolutionNotFoundCardinality,
                (double) 100 * Counter->FailedEditsSolutionNotFoundCardinality /
                    Counter->Obs);
        }

        if (Counter->FailedEditsSolutionNotFoundSpace > 0) {
            PRINT_SUBTOTAL_PERCENT (MsgNumberPercentObsNoSolutionSpace,
                MsgNumberPercentObsNoSolutionSpace_LPH, TWOTABS,
                Counter->FailedEditsSolutionNotFoundSpace,
                (double) 100 * Counter->FailedEditsSolutionNotFoundSpace /
                    Counter->Obs);
        }

        if (Counter->FailedEditsSolutionNotFoundTime > 0) {
            PRINT_SUBTOTAL_PERCENT (MsgNumberPercentObsNoSolutionTime,
                MsgNumberPercentObsNoSolutionTime_LPH, TWOTABS,
                Counter->FailedEditsSolutionNotFoundTime,
                (double) 100 * Counter->FailedEditsSolutionNotFoundTime /
                    Counter->Obs);
        }
    }

    IO_PRINT_LINE ("");
}

/*
For debug: Print info for an an observation in OUTSTATUS
*/
static void PrintStatus (
    tOutStatus * OutStatus)
{
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "123456789012345678901234567890123\n");
    UTIL_PrintNNTString (OutStatus->Key, OutStatus->KeyLength);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "|\n");
    UTIL_PrintNNTString (OutStatus->FieldId, LEGACY_MAXNAME);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "|\n");
    UTIL_PrintNNTString (OutStatus->Status, EIM_STATUS_SIZE);
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "|\n");
}

/*
Reads DATA data set.
return 0 when end of DATA data set is reached.
*/
static PROC_RETURN_CODE ProcessByGroups (
    SP_errorloc* sp,
    tInData * InData,
    tStatusDataSet* InStatus,
    tOutStatus * OutStatus,
    tOutReject * OutReject,
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights,
    double Cardinality,
    double TimePerObs,
    EIPT_POSITIVITY_OPTION PositivityOption,
    tSList* OriginalVarNamesInEdits,
    int LoggingVerbosityLevel,
	int ByGroupLoggingLevel)
{
    tCounter Counter;
    EIT_BOOLEAN MustReadStatus;
    EIT_CHERNIKOVA Chernikova;
    EIT_CHERNIKOVA_COMPANION  ChernikovaCompanion;
    double * Lbound = NULL;
    char * PreviousKey = NULL;
    PROC_RETURN_CODE rc_function = PRC_FAIL_UNHANDLED;
    double * Ubound = NULL;

    Chernikova.column            = NULL;
    Chernikova.preinit           = NULL;
    ChernikovaCompanion.pivolist = NULL;
    ChernikovaCompanion.rowtype  = NULL;
    ChernikovaCompanion.w        = NULL;
    ChernikovaCompanion.I0       = NULL;
    ChernikovaCompanion.I0s      = NULL;
    ChernikovaCompanion.I0st     = NULL;
    ChernikovaCompanion.I0t      = NULL; /* <- points to the same memory as "ChernikovaCompanion.I0" */

    PreviousKey = STC_AllocateMemory (InData->KeyLength+1);

    /* Get the upper and lower bounds of the variables in Edits */
    Ubound = STC_AllocateMemory ((Edits->NumberofColumns-1) * sizeof *Ubound);
    Lbound = STC_AllocateMemory ((Edits->NumberofColumns-1) * sizeof *Lbound);
    if (0 != GetBoundsVariables (Edits, Ubound, Lbound)) {
        rc_function = PRC_FAIL_ERRORLOC_BOUNDS;
        goto error_cleanup;
    }

    /* Allocate memory for structures */
    EI_ChernikovaAllocate (Edits, Weights, &Chernikova, &ChernikovaCompanion);

    IO_DATASET_RC rc_next_by = DSRC_SUCCESS;

    while ((rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr)) == DSRC_NEXT_BY_SUCCESS) {

        /* synchronize INSTATUS, if specified */
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
                    return PRC_FAIL_SYNC_GENERIC;
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

        InitCounter (&Counter);
        /* reset keys */
        PreviousKey[0] = '\0';
        if (sp->dsr_instatus.dsr.is_specified == IOSV_SPECIFIED) {
            InStatus->Key->Value[0] = '\0';
            InStatus->PreviousKey[0] = '\0';
        }

        IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;
        while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {

            Counter.Obs++;

            IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_indata.dsr);
            if (rc_get_rec != IORC_SUCCESS) {
                rc_function = PRC_FAIL_GET_REC;
                goto error_cleanup;
            }

            SUtil_NullTerminate (InData->Key, InData->KeyLength);

            if (IOUtil_is_missing(InData->RandomNumber)) {
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandumvarInvalidValues);
				rc_function = PRC_FAIL_ERRORLOC_RANDOM_NUM;
				goto error_cleanup;
			}
			else if (InData->RandomNumber < 0.0) {
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarErrorlocLessThanZero_main, InData->RandomNumber);
                rc_function = PRC_FAIL_ERRORLOC_RANDOM_NUM;
                goto error_cleanup;
            }
            else if (InData->RandomNumber > 1.0) {
                IO_PRINT_LINE(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarErrorlocGreaterThanOne_main, InData->RandomNumber);
                rc_function = PRC_FAIL_ERRORLOC_RANDOM_NUM;
                goto error_cleanup;
            }

#ifdef BSMD_TRANSPORT
			/* set the pointer used in FpSignalHandler() in case of floating point exception */
			mKey = InData->Key;
#endif

            /* reset status flags */
            for (int i = 0; i < InData->DataRec.NumberofFields; i++) {
                InData->DataRec.StatusFlag[i] = FIELDOK;
            }

            if (InData->Key[0] == '\0') {
                Counter.MissingKey++;
            }
            else {
                EIT_RETURNCODE rc_ei = CheckKeyOrder (PreviousKey, InData->Key);
                if (rc_ei == EIE_FAIL) {
                    rc_function = PRC_FAIL_REC_NOT_SORTED;
                    goto error_cleanup;
                }

                Counter.ValidObs++;

                if (sp->dsr_instatus.dsr.is_specified == IOSV_SPECIFIED && MustReadStatus == EIE_TRUE) {
                    PROC_RETURN_CODE rc_match_status = MatchStatus(sp, InData, InStatus, &Counter);
                    switch (rc_match_status) {
                    case PRC_SUCCESS:
                        break;
                    case PRC_SUCCESS_NO_MORE_DATA:
                        MustReadStatus = EIE_FALSE;
                        break;
                    default:
                        return rc_match_status;
                    }
                }

                rc_ei = DoErrorLocalization (sp, InData, OutStatus, OutReject, Edits,
                    Weights, Cardinality, TimePerObs, PositivityOption, LoggingVerbosityLevel, Ubound,
                    Lbound, &Chernikova, &ChernikovaCompanion, &Counter, OriginalVarNamesInEdits);
                if (rc_ei == EIE_FAIL) {
                    rc_function = PRC_FAIL_ERRORLOC_DO_LOCALIZATION;
                    goto error_cleanup;
                }
            }
        }

        // check for errors
        if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
            /* Print statistics on "by group data" read */
            /* (warning will be print if no valid data) */
            if (ByGroupLoggingLevel != 1) {
                PrintStatistics(&Counter);
                DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
            }
        }
        else{
            rc_function = PRC_FAIL_ADVANCE_REC;
            goto error_cleanup;
        }
    }
    EI_PrintMessages ();

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        rc_function = PRC_SUCCESS_NO_MORE_DATA;
    }
    else if (rc_next_by == DSRC_BY_NOT_SORTED){
        rc_function = PRC_FAIL_BY_NOT_SORTED;
        goto error_cleanup;
    }
    else  {
        rc_function = PRC_FAIL_ADVANCE_BY;
        goto error_cleanup;
    }

    goto normal_cleanup;

error_cleanup: // yea, it's the same as `normal_cleanup`... 
normal_cleanup:
    if ((PreviousKey                  != NULL)
    ) {
        STC_FreeMemory (PreviousKey);
        PreviousKey = NULL;
    }
    if ((Lbound                       != NULL)
    ) {
        STC_FreeMemory (Lbound);
        Lbound = NULL;
    }
    if ((Ubound                       != NULL)
    ) {
        STC_FreeMemory (Ubound);
        Ubound = NULL;
    }
    if ((Chernikova.column            != NULL) &&
        (Chernikova.preinit           != NULL) &&
        (ChernikovaCompanion.pivolist != NULL) &&
        (ChernikovaCompanion.rowtype  != NULL) &&
        (ChernikovaCompanion.w        != NULL) &&
        (ChernikovaCompanion.I0       != NULL) &&
        (ChernikovaCompanion.I0s      != NULL) &&
        (ChernikovaCompanion.I0st     != NULL) &&
        (ChernikovaCompanion.I0t      != NULL)
    ) {
        EI_ChernikovaFree (&Chernikova, &ChernikovaCompanion);
        Chernikova.column            = NULL;
        Chernikova.preinit           = NULL;
        ChernikovaCompanion.pivolist = NULL;
        ChernikovaCompanion.rowtype  = NULL;
        ChernikovaCompanion.w        = NULL;
        ChernikovaCompanion.I0       = NULL;
        ChernikovaCompanion.I0s      = NULL;
        ChernikovaCompanion.I0st     = NULL;
        /*-this structure has a pointer "I0t" to a list of ints, but it shares */
        /* that list of ints with the "I0" member of the same structure, which */
        /* is treated as the owner of the list of ints, so the list of ints is */
        /* only freed once (through "I0") by "EI_ChernikovaFree()"             */
        ChernikovaCompanion.I0t      = NULL; /* <- points to the same memory as "ChernikovaCompanion.I0" */
    }

    return rc_function;
}

/*********************************************************************
Read Status data set until a valid obs is found
a valid obs is:
    key is not empty,
    fieldid is not empty,
    status is "FTI"
Exit if the data set is not sorted.
Count observations with missing key or missing fieldid.

Returns E_READSTATUS_SUCCEED if a valid obs is found
        E_READSTATUS_EOF if eof is reached
        E_READSTATUS_FAIL if data set not ordered by key
*********************************************************************/
static PROC_RETURN_CODE ReadStatus(
    SP_errorloc* sp,
    tStatusDataSet* InStatus,
    tCounter* Counter)
{
    EIT_RETURNCODE rc;
    IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;

    while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_instatus.dsr)) == DSRC_NEXT_REC_SUCCESS) {
        IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_instatus.dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }

        SUtil_NullTerminate(InStatus->Key->Value, InStatus->Key->Size);
        SUtil_NullTerminate(InStatus->FieldId->Value,
            InStatus->FieldId->Size);
        SUtil_NullTerminate(InStatus->Status->Value, InStatus->Status->Size);

        /* remove leading blanks from fieldid and uppercase it */
        UTIL_DropBlanks(InStatus->FieldId->Value);
        UTIL_StrUpper(InStatus->FieldId->Value, InStatus->FieldId->Value);

        if (InStatus->Key->Value[0] == '\0' || /* missing key */
            InStatus->FieldId->Value[0] == '\0') { /* missing fieldid */
            Counter->MissingStatus++;
        }
        else {
            rc = CheckStatusKeyOrder(InStatus->PreviousKey,
                InStatus->Key->Value);
            if (rc == EIE_FAIL)
                return PRC_FAIL_REC_NOT_SORTED; /* not in order */

            if (strcmp(InStatus->Status->Value, EIM_STATUS_FTI) == 0) {
                /* status is relevant */
                return PRC_SUCCESS;
            }
        }
    }

    // check for errors
    if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
        // NOTE: the caller must be able to distinguish between `RCP_SUCCESS` and this
        //       as this case is handled differently
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else {
        return PRC_FAIL_ADVANCE_REC;
    }
}

/*
Those are IMPORTANT lines. The value must lie outside the acceptable limit.
According to ANSI, the smallest difference between 2 doubles is <= 10e-9
(== DBL_EPSILON), so I used '1'.  This will only cause a problem when
Ubound > DBL_MAX - 1 or Lbound < DBL_MIN + 1.
*/
static void SetFieldValueOutsideLimit (
    EIT_DATAREC * DataRec,
    double * Ubound,
    double * Lbound)
{
    int j;

    for (j = 0; j < DataRec->NumberofFields; j++) {
        if (DataRec->StatusFlag[j] != FIELDOK) {
            if (Lbound[j] != EIM_NOBOUND)
                DataRec->FieldValue[j] = Lbound[j] - 1;
            else
                DataRec->FieldValue[j] = Ubound[j] + 1;
        }
    }
}

/*********************************************************************
set status of variable to FTI, if variable is in the edits
*********************************************************************/
static void SetStatus(
    EIT_DATAREC* DataRec,
    char* FieldName)
{
    int Index;

    Index = UTIL_Binary_Reverse(FieldName, DataRec->FieldName,
        DataRec->NumberofFields);
    if (Index != -1)
        DataRec->StatusFlag[Index] = FIELDFTI;
}

/*
Set field value to its acceptable limit
*/
static void UpdateDataForEditsApplyChern (
    int Index,
    int Status,
    EIT_DATAREC * DataRec,
    EIT_WEIGHTS * Weights,
    double * Ubound,
    double * Lbound,
    EIT_BOOLEAN * InvalidData,
    double * CardinalityOfInvalidData)
{
    DataRec->StatusFlag[Index] = Status;
    if (Lbound[Index] != EIM_NOBOUND)
        DataRec->FieldValue[Index] = Lbound[Index];
    else
        DataRec->FieldValue[Index] = Ubound[Index];
    *InvalidData = EIE_TRUE;
    *CardinalityOfInvalidData += Weights->Value[Index];
}
/*
Validate Parms.
NOTE: Validate all parms before quitting.
*/
static EIT_RETURNCODE ValidateParms (
    SP_errorloc* sp,
    char * EditsString,
    char * WeightsString,
    double Cardinality,
    double * TimePerObs,
    EIPT_POSITIVITY_OPTION PositivityOption,
    int LoggingVerbosityLevel,
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights)
{
    int crc = EIE_SUCCEED; /* cumulative return code */
    int EditsNumberofEquations;
    int rc;

    if (*TimePerObs < EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW) {
        *TimePerObs = EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgTimePerObsAdjustedToMinValue  SAS_NEWLINE,
            EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW);
    }

    /* add the smallest period of the cpu so that what the user requested is
    the minimum they will get */
    *TimePerObs += EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW;

    if (EditsString == NULL || strcmp (EditsString, "") == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_EDITS);
        crc = EIE_FAIL;
    }
    else if (PositivityOption == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        crc = EIE_FAIL;
    }
    else {
        rc = EI_EditsParse (EditsString, Edits);
        if (rc != EIE_SUCCEED) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_EDITS);
            EI_PrintMessages ();
            crc = EIE_FAIL;
        }
        else {
            EI_PrintMessages (); /* print warnings of EI_EditsParse */

            IO_PRINT_LINE (MsgHeaderEdits);
            if ((PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT) ||
                (PositivityOption == EIPE_REJECT_NEGATIVE)) {
                if (EIE_SUCCEED == EI_EditsAddPositivity (Edits))
                    IO_PRINT_LINE (MsgPositivityEditsAdded);
                else {
                    IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgAddingPositivityEditsFail);
                    crc = EIE_FAIL;
                }
            }

            if (crc != EIE_FAIL) {
                SUtil_PrintEdits (Edits);
                rc = EI_EditsConsistency (Edits);
                if (rc != EIE_EDITSCONSISTENCY_CONSISTENT) {
                    if (rc == EIE_EDITSCONSISTENCY_INCONSISTENT)
                        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInconsistentEdits  SAS_NEWLINE);
                    EI_PrintMessages (); /* Inconsistent or Fail */
                    crc = EIE_FAIL;
                }
                else {

                    EditsNumberofEquations = Edits->NumberofEquations;

                    if (EI_EditsRedundancy (Edits)!=EIE_EDITSREDUNDANCY_SUCCESS)
                        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgRedundantEditsFail);
                    if (EditsNumberofEquations == Edits->NumberofEquations)
                        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgReducedEditsEquivalent  SAS_NEWLINE);
                    else {
                      IO_PRINT_LINE (MsgHeaderReducedEdits);
                      SUtil_PrintEdits (Edits);
                    }
                    EI_WeightsAllocate (Edits, Weights);

                    if (WeightsString != NULL && strcmp (WeightsString, "") != 0) {
                        rc = EI_WeightsParse (WeightsString, Weights);
                        if (rc != 0) {
                            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_WEIGHTS);
                            EI_WeightsFree (Weights);
							Weights->Value = NULL;
                            EI_EditsFree (Edits);
							Edits->FieldName = NULL;
							Edits->EditId = NULL;
							Edits->Coefficient = NULL;
                            crc = EIE_FAIL;
                        }
                        EI_PrintMessages ();
                    }
                } /* rc == EIE_EDITSCONSISTENCY_CONSISTENT */
            } /* succeed to add positivity edits if it was necessary */
        } /* EI_EditsParse: succeed */
    } /* Edits are specified and PositivityOption is valid */

    if ((LoggingVerbosityLevel != 0) &&
        (LoggingVerbosityLevel != 1) &&
        (LoggingVerbosityLevel != 2) &&
        (LoggingVerbosityLevel != 3) &&
        (LoggingVerbosityLevel != 4) &&
        (LoggingVerbosityLevel != 5)
       ) {
        IO_PRINT_LINE ("Error: The value specified for " BPN_DISPLAY_LEVEL " is not one of 0, 1, 2, 3, 4, or 5: " BPN_DISPLAY_LEVEL " = " SAS_MESSAGE_PREFIX_NOTE ".");// not sure why this print statement originally ended with: , LoggingVerbosityLevel);
    }

    if (Cardinality <= 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_CARDINALITY);
        crc = EIE_FAIL;
    }

    if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_UNIT_ID);
        crc = EIE_FAIL;
    }

    return crc;
}
