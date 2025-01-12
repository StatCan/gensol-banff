/***********************************************************************/
/* NAME:         DETERMIN                                              */
/* PRODUCT:      SAS/TOOLKIT                                           */
/* LANGUAGE:     C                                                     */
/* TYPE:         PROCEDURE                                             */
/* PURPOSE:      Implementing the deterministic imputation method in   */
/*               the form of a SAS Procedure.                          */
/*               This method supplies valid values to fields to impute */
/*               in the case where only one possible imputed value for */
/*               them will permit the record to pass the set of edits. */
/*               (accept negative values data)                         */
/***********************************************************************/
#define MAINPROC 1
#define SASPROC  1

#include <stdlib.h>
#include <string.h>

#include "determin_JIO.h"

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Deterministic.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"
#include "EI_EditsConsistency.h"
#include "EI_EditsRedundancy.h"
#include "EI_Message.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "slist.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageBanffAPI.h"

#define  STATUS_NBVARS     3
#define  VAR_NOT_FOUND    -1

/* number of digit to print in report before IO_PRINT_LINE will start to use
scientific notation */
#define PRINT_PRECISION    7
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

    EIT_DATAREC * DataRec;

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

typedef struct {
    int ObsInByGroup;      /* # of obs in DATA data set                       */
    int ValidObsInByGroup; /* # of valid obs in DATA data set                 */
    int MissingKeyDATA;    /* # of missing key in DATA data set               */
    int MissingValues;     /* # of missing variable non FTI in DATA           */
    int NegativeValues;    /* # of negative when negative value are rejected  */
    int MissingStatus;     /* # of missing key/fieldid in INSTATUS            */
    int ValidObsWithoutFTI;/* # of valid obs in DATA not to be imputed        */
    int ValidObsWithFTI;   /* # of valid obs in DATA to be imputed            */
    int ValidObsImputed;   /* # of valid obs in DATA imputed                  */
} T_DETERMIN_COUNTER;

static EIT_RETURNCODE CheckCharVarInStatus (SP_determin* sp, char *, T_CHAR_VARIABLE *);
static EIT_RETURNCODE CheckDataKeyOrder (char *, char *);
static EIT_RETURNCODE CheckStatusKeyOrder (char *, char *);
static PROC_RETURN_CODE DefineGatherWriteData (SP_determin* sp, tDataDataSet *, tDataDataSet *, int *);
static PROC_RETURN_CODE DefineGatherWriteStatus (SP_determin* sp, tStatusDataSet *, tDataDataSet *);
static void DefineKeyDATA (SP_determin* sp, tDataDataSet *);
static PROC_RETURN_CODE DefineScatterReadData (SP_determin* sp, tDataDataSet *, int *);
static void DefineScatterReadStatus (SP_determin* sp, tStatusDataSet *);
static EIT_RETURNCODE DefineVarsINSTATUS (SP_determin* sp, tStatusDataSet *);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_determin* sp, int, int *, int);
static tSList * FindOriginalNames (SP_determin* sp, char **, int);
static EIT_RETURNCODE FindVariablesPosition (SP_determin* sp, char **, int, int *);
static void GetParms (SP_determin* sp, char **, EIPT_POSITIVITY_OPTION *, int *);
static EIT_BOOLEAN HasFTI (EIT_DATAREC *);
static EIT_BOOLEAN HasInvalidData (EIT_DATAREC *, T_DETERMIN_COUNTER *);
static void InitCounter (T_DETERMIN_COUNTER *);
static void LpFreeEnv (void);
static PROC_RETURN_CODE MatchStatus (SP_determin* sp, tDataDataSet *, tStatusDataSet *,
    T_DETERMIN_COUNTER *);
static void PrintParms (SP_determin* sp, char *, EIPT_POSITIVITY_OPTION, int ByGroupLoggingLevel);
static void PrintStatistics (T_DETERMIN_COUNTER *, char *);
static PROC_RETURN_CODE ProcessByGroups (SP_determin* sp, tDataDataSet *, tStatusDataSet *,
    tDataDataSet *, tStatusDataSet *, EIT_EDITS *, tSList *,
    EIPT_POSITIVITY_OPTION, int);
static PROC_RETURN_CODE ReadStatus (SP_determin* sp, tStatusDataSet *,
    T_DETERMIN_COUNTER *);
static void ResetKeys (tDataDataSet *, tStatusDataSet *);
static void SetStatus (EIT_DATAREC *, char *);
static EIT_RETURNCODE ValidateParms (SP_determin* sp, char *, EIPT_POSITIVITY_OPTION);
static PROC_RETURN_CODE WriteResults (SP_determin* sp, tDataDataSet *, tStatusDataSet *, tSList *);



/*********************************************************************
*********************************************************************/
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int determin(
    T_in_parm in_parms,

    T_in_ds in_ds_indata,
    T_in_ds in_ds_instatus,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus
)
{
	EIT_EDITS Edits								= { 0 };
    int EditsNumberofEquations;
	char * EditsString							= NULL;
    int * EditVariablesList				= NULL;
	tDataDataSet InData							= { 0 };
	tStatusDataSet InStatus						= { 0 };
    char *lpversion								= 0;
	char *lpname								= 0;
    tSList * OriginalVarNamesInEdits			= NULL;
	tDataDataSet OutData						= { 0 };
	tStatusDataSet OutStatus					= { 0 };
    EIPT_POSITIVITY_OPTION PositivityOption		= 0;
	int ByGroupLoggingLevel;

	EIT_EDITSCONSISTENCY_RETURNCODE rcConsistency	= { 0 };
    EIT_EDITSREDUNDANCY_RETURNCODE rcRedundancy		= { 0 };
    EIT_RETURNCODE rcLocal = { 0 };

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

	SP_determin sp = { 0 };
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
        out_arr_outstatus
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

    /***************************************/
    /* Get procedure parameters except ID  */
    /***************************************/
    GetParms (&sp, &EditsString, &PositivityOption, &ByGroupLoggingLevel);
    PrintParms (&sp, EditsString, PositivityOption, ByGroupLoggingLevel);

    /***************************************/
    /* Validate procedure parameters.      */
    /***************************************/
    rcLocal = ValidateParms (&sp, EditsString, PositivityOption);
    if (rcLocal == EIE_FAIL) {
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
    }

    /* Initializes the LP environment */
    if (LPI_InitLpEnv (&lpname, &lpversion) != LPI_SUCCESS) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInitLPFail SAS_NEWLINE, "");
		proc_exit_code = BRC_FAIL_LPI_INIT;
		goto error_cleanup;
    }

    /****************************************/
    /* Process EDITS                        */
    /****************************************/
    rcLocal = EI_EditsParse (EditsString, &Edits);
    if (rcLocal != EIE_SUCCEED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_EDITS);
        EI_PrintMessages ();
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_EDITS_PARSE;
		goto error_cleanup;
    }
    EI_PrintMessages (); /* print warnings of EI_EditsParse */

    if (PositivityOption == EIPE_REJECT_NEGATIVE ||
        PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT) {
        rcLocal = EI_EditsAddPositivity (&Edits);
        EI_PrintMessages ();
        if (rcLocal == EIE_FAIL) {
            LpFreeEnv();
			proc_exit_code = BRC_FAIL_EDITS_OTHER;
			goto error_cleanup;
        }
    }

    IO_PRINT_LINE (MsgHeaderEdits);
    if (PositivityOption != EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE (MsgPositivityEditsAdded);
    SUtil_PrintEdits (&Edits);

    rcConsistency = EI_EditsConsistency (&Edits);
    EI_PrintMessages ();
    if (rcConsistency != EIE_EDITSCONSISTENCY_CONSISTENT) {
        if (rcConsistency == EIE_EDITSCONSISTENCY_INCONSISTENT)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInconsistentEdits SAS_NEWLINE);
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_EDITS_CONSISTENCY;
		goto error_cleanup;
    }

    EditsNumberofEquations = Edits.NumberofEquations;

    rcRedundancy = EI_EditsRedundancy (&Edits);
    EI_PrintMessages ();
    if (rcRedundancy != EIE_EDITSREDUNDANCY_SUCCESS) {
        EI_PrintMessages ();
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_EDITS_REDUNDANCY;
		goto error_cleanup;
    }

    if (EditsNumberofEquations == Edits.NumberofEquations)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgReducedEditsEquivalent SAS_NEWLINE);
    else {
        IO_PRINT_LINE (MsgHeaderReducedEdits);
        SUtil_PrintEdits (&Edits);
    }

    /***************************************/
    /* Get info on ID parameter.           */
    /* Fill structure InData.              */
    /***************************************/
    DefineKeyDATA (&sp, &InData);
    InData.DataRec = STC_AllocateMemory (sizeof *InData.DataRec);
    EI_DataRecAllocate (Edits.FieldName, Edits.NumberofColumns-1,
        InData.DataRec);

    /* List of position in DATA of the variables in the edits */
    EditVariablesList = STC_AllocateMemory (
        (Edits.NumberofColumns-1) * sizeof *EditVariablesList);
    rcLocal = FindVariablesPosition (&sp, Edits.FieldName,
        Edits.NumberofColumns-1, EditVariablesList);
    if (rcLocal == EIE_FAIL) {
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
		goto error_cleanup;
    }

    /*************************************************************/
    /* Checks and gets info on mandatory variables in INSTATUS   */
    /*************************************************************/
    InStatus.Key = STC_AllocateMemory (sizeof *InStatus.Key);
    IOUtil_copy_varname (InStatus.Key->Name, InData.Key->Name);
    InStatus.FieldId = STC_AllocateMemory (sizeof *InStatus.FieldId);
    InStatus.Status = STC_AllocateMemory (sizeof *InStatus.Status);
    rcLocal = DefineVarsINSTATUS (&sp, &InStatus);
    if (rcLocal == EIE_FAIL) {
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_SETUP_DATASET_IN;
		goto error_cleanup;
    }

    /*********************************************************/
    /* Checks exclusivity between ID, BY and EDITS variables */
    /*********************************************************/
    rcLocal = ExclusivityBetweenLists (&sp, InData.Key->Position, EditVariablesList,
        Edits.NumberofColumns-1);
    if (rcLocal == EIE_FAIL) {
        LpFreeEnv();
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
    }

    /****************************************/
    /* INDATASET: Define scatter read.      */
    /****************************************/
    if (PRC_SUCCESS != DefineScatterReadData(&sp, &InData, EditVariablesList)) {
        proc_exit_code = BRC_FAIL_READ_GENERIC;
        goto error_cleanup;
    }

    /****************************************/
    /* INSTATUSSET : Define scatter read.   */
    /****************************************/
    DefineScatterReadStatus (&sp, &InStatus);

    /****************************************/
    /* OUTDATASET: Define gather write.     */
    /****************************************/
    OutData.Key = InData.Key;
    OutData.DataRec = InData.DataRec;
    if (PRC_SUCCESS != DefineGatherWriteData(&sp, &InData, &OutData, EditVariablesList)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    /****************************************/
    /* OUTSTATUSSET: Define gather write.   */
    /****************************************/
    OutStatus.Key = InData.Key;
    OutStatus.FieldId = STC_AllocateMemory (sizeof *InStatus.FieldId);
    OutStatus.FieldId->Value = STC_AllocateMemory (LEGACY_MAXNAME+1);
    OutStatus.Status = STC_AllocateMemory (sizeof *InStatus.Status);
    OutStatus.Status->Value = STC_AllocateMemory (EIM_STATUS_SIZE+1);
    SUtil_CopyPad (OutStatus.Status->Value, EIM_STATUS_IDE,
        EIM_STATUS_SIZE);
    if (PRC_SUCCESS != DefineGatherWriteStatus (&sp, &OutStatus, &InData)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    OriginalVarNamesInEdits = FindOriginalNames (&sp, 
        Edits.FieldName, Edits.NumberofColumns-1);

    /* Exit if no observation in DATA or in INSTATUS */
    // The following funcition deleted, unused prior to conversion anyway//ExitIfNoData (InData.DataSet, InStatus.DataSet);

    /* check if INSTATUS data set has all the by variables */
    rcLocal = SUtil_AreAllByVariablesInDataSet(&sp.dsr_indata.dsr, &sp.dsr_instatus.dsr);
	EI_PrintMessages ();
    if (rcLocal == EIE_SUCCEED) {
        /* all BY variables were found in INSTATUS data set */
        InStatus.HasAllByVariables = EIE_TRUE;

        /* build BY list for status data set */
        // the following SAS statement need not be replaced //SAS_XBYLIST (InData.DataSet, InStatus.DataSet);
    }
    else {
        /* One or more BY variable was not found in INSTATUS data set */
        InStatus.HasAllByVariables = EIE_FALSE;

        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFasterPerformanceIfByVarsInDataSet SAS_NEWLINE,
            DSN_INSTATUS);
    }

	TIME_CPU_STOPDIFF(load_init);
	TIME_WALL_STOPDIFF(load_init);

	TIME_WALL_START(processing);
	TIME_CPU_START(processing);

    /*****************************************************/
    /* DETERMINISTIC IMPUTATION                          */
    /* bulk of processing performed in ProcessByGroups() */
    /* rc is checked at the end of the proc              */
    /*****************************************************/
    PROC_RETURN_CODE rc_processing = ProcessByGroups (&sp, &InData, &InStatus, &OutData, &OutStatus, &Edits,
        OriginalVarNamesInEdits, PositivityOption, ByGroupLoggingLevel);

    proc_exit_code = convert_processing_rc(rc_processing);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }

	TIME_CPU_STOPDIFF(processing);
	TIME_WALL_STOPDIFF(processing);

    mem_usage("Before SP_wrap");
    if (PRC_SUCCESS != SP_wrap(&sp)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    /* return address of each output dataset */
    mem_usage("After SP_wrap");

    /****************/
    /* Free memory. */
    /****************/
	error_cleanup:
	normal_cleanup:
    
	TIME_WALL_BEGIN(cleanup);
	TIME_CPU_BEGIN(cleanup);
    
	if ((Edits.FieldName != NULL) &&
		(Edits.EditId != NULL) &&
		(Edits.Coefficient != NULL)
		) {
		EI_EditsFree (&Edits);
		Edits.FieldName = NULL;
		Edits.EditId = NULL;
		Edits.Coefficient = NULL;
	}

	if (EditVariablesList != NULL) {
		STC_FreeMemory (EditVariablesList);
		EditVariablesList = NULL;
	}

	if (OriginalVarNamesInEdits != NULL) {
		SList_Free (OriginalVarNamesInEdits);
		OriginalVarNamesInEdits = NULL;
	}

	

	if (InData.Key != NULL) {
		if (InData.Key->Value != NULL) {
			STC_FreeMemory(InData.Key->Value);
			InData.Key->Value = NULL;
		}
		STC_FreeMemory (InData.Key);
		InData.Key = NULL;
	}

	if (InData.PreviousKey != NULL) {
		STC_FreeMemory (InData.PreviousKey);
		InData.PreviousKey = NULL;
	}

	if (InData.DataRec != NULL) {
		EI_DataRecFree (InData.DataRec);
		STC_FreeMemory (InData.DataRec);
		InData.DataRec = NULL;
	}
	
	

	if (InStatus.Key != NULL) {
		if (InStatus.Key->Value != NULL) {
			STC_FreeMemory(InStatus.Key->Value);
			InStatus.Key->Value = NULL;
		}
		STC_FreeMemory (InStatus.Key);
		InStatus.Key = NULL;
	}

	if (InStatus.PreviousKey != NULL) {
		STC_FreeMemory (InStatus.PreviousKey);
		InStatus.PreviousKey = NULL;
	}

	

	if (InStatus.FieldId != NULL) {
		if (InStatus.FieldId->Value != NULL) {
			STC_FreeMemory(InStatus.FieldId->Value);
			InStatus.FieldId->Value = NULL;
		}
		STC_FreeMemory (InStatus.FieldId);
		InStatus.FieldId = NULL;
	}

	

	if (InStatus.Status != NULL) {
		if (InStatus.Status->Value != NULL) {
			STC_FreeMemory(InStatus.Status->Value);
			InStatus.Status->Value = NULL;
		}
		STC_FreeMemory (InStatus.Status);
		InStatus.Status = NULL;
	}

    /* nothing to free in OutData */
	
	if (OutStatus.FieldId != NULL) {
		if (OutStatus.FieldId->Value != NULL) {
			STC_FreeMemory(OutStatus.FieldId->Value);
			OutStatus.FieldId->Value = NULL;
		}
		STC_FreeMemory(OutStatus.FieldId);
		OutStatus.FieldId = NULL;
	}
	
	if (OutStatus.Status != NULL) {
		if (OutStatus.Status->Value != NULL) {
			STC_FreeMemory(OutStatus.Status->Value);
			OutStatus.Status->Value = NULL;
		}
		STC_FreeMemory(OutStatus.Status);
		OutStatus.Status = NULL;
	}

    LpFreeEnv();

	if (EI_mMessageList_Message_is_NULL() != 1){
		EI_FreeMessageList();
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

    return proc_exit_code;
}

/*****************************************************/
/* Function: CheckCharVarInStatus                    */
/*                                                   */
/* Purpose:- Checks that a VariableName exists in    */
/*           INSTATUS and it's a character variable. */
/*         - Gets position and name.                 */
/*****************************************************/
static EIT_RETURNCODE CheckCharVarInStatus (
    SP_determin* sp,
    char * VariableName,
    T_CHAR_VARIABLE * Variable)
{
    EIT_RETURNCODE rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_instatus.dsr;

    IOUtil_copy_varname(Variable->Name, VariableName);
    IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &VariableName, &Variable->Position);
    if (rc_io == IORC_VARLIST_NOT_FOUND) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgCharacterVarNotInDataSet,
            Variable->Name, dsr->dataset_name);
        rc = EIE_FAIL;
    }
    else {
        if (IOVT_CHAR != DSR_get_col_type(dsr, Variable->Position)) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNotCharacterInDataSet,
                Variable->Name, dsr->dataset_name);
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
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted SAS_NEWLINE,
            DSN_INDATA, Key, PreviousKey);
        return EIE_FAIL;
    }
    else if (rc == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysDuplicate SAS_NEWLINE,
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
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetWithTwoKeysNotSorted SAS_NEWLINE,
            DSN_INSTATUS, Key, PreviousKey);
        return EIE_FAIL;
    }

    strcpy (PreviousKey, Key);

    return EIE_SUCCEED;
}
/***********************************************************/
/* Function: DefineGatherWriteData                         */
/*                                                         */
/* Purpose : Define gather write for OUTDATASET.           */
/***********************************************************/
static PROC_RETURN_CODE DefineGatherWriteData (
    SP_determin* sp,
    tDataDataSet * InData, /* for BY variables */
    tDataDataSet * OutData,
    int * EditVariablesList)
{
    DSW_generic* dsw = &sp->dsw_outdata;
    int var_added = 0;

    if (IORC_SUCCESS != DSW_allocate(dsw, OutData->DataRec->NumberofFields + 1)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* For key (variable without change) */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, InData->Key->Name, OutData->Key->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* For variables that change. */
    for (int i = OutData->DataRec->NumberofFields - 1; i >= 0; i--) {
        const char* var_name = sp->dsr_indata.dsr.col_names[EditVariablesList[i]];
        if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, (char*)var_name, &OutData->DataRec->FieldValue[i], IOVT_NUM)) {
            return PRC_FAIL_WRITE_DATA;
        }
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
    SP_determin* sp,
    tStatusDataSet * OutStatus,
    tDataDataSet * InData)
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
/* Purpose:  Fills Key with ID characteristics  */
/*           and allocate memory for key value.      */
/* Note: ID has been verified by the grammar.        */
/*****************************************************/
static void DefineKeyDATA (
    SP_determin* sp,
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
    SP_determin* sp,
    tDataDataSet * InData,
    int * List)
{
    DSR_generic* dsr = &sp->dsr_indata.dsr;
    // key var
    dsr->VL_unit_id.ptrs[0] = InData->Key->Value;

    // other variables
    if (IORC_SUCCESS != VL_init_from_position_list(&dsr->VL_in_var, dsr, VL_NAME_in_var, IOVT_NUM, InData->DataRec->NumberofFields, List)) {
        return PRC_FAIL_UNHANDLED;
    }

    for (int i = 0; i < InData->DataRec->NumberofFields; i++) {
        dsr->VL_in_var.ptrs[i] = &InData->DataRec->FieldValue[i];
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
    SP_determin* sp,
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
    SP_determin* sp,
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
         STC_FreeMemory (InStatus->FieldId->Value);
         STC_FreeMemory (InStatus->Status->Value);
		 InStatus->Key->Value = NULL;
		 InStatus->FieldId->Value = NULL;
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
    SP_determin* sp,
    int KeyPosition,
    int * EditVariablesList,
    int EditsNumberofFields)
{
    int * ByList;
    int i;
    int j;
    int NumberOfByVariables;
    EIT_RETURNCODE rc = EIE_SUCCEED;

    ByList = (int*)sp->dsr_indata.dsr.VL_by_var.positions;
    NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;

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
            if (ByList[i] == EditVariablesList[j]) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_EDITS, BPN_BY);
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
    SP_determin* sp,
    char ** VariableName,
    int NumberOfVariables)
{
    int i;
    tSList * OriginalVarNamesInEdits;

    SList_New (&OriginalVarNamesInEdits);

    for (i = 0; i < NumberOfVariables; i++) {
        SList_Add (VariableName[i], OriginalVarNamesInEdits);
        /* It can't fail, existence has already been tested */
        SUtil_GetOriginalName (&sp->dsr_indata.dsr, OriginalVarNamesInEdits->l[i]);
    }

    return OriginalVarNamesInEdits;
}
/***********************************************************/
/* Function: FindVariablesPosition                         */
/*                                                         */
/* Purpose : Build a list containing the numbers of the    */
/*           variables corresponding to the EDITS fields   */
/*           that can be found on the data set pointed by  */
/*           InData.                                       */
/***********************************************************/
static EIT_RETURNCODE FindVariablesPosition (
    SP_determin* sp,
    char ** FieldName,
    int NumberofFields,
    int * EditVariablesList)
{
    int i;
    EIT_RETURNCODE rc;

    rc = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    for (i = 0; i < NumberofFields; i++) {
        IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &FieldName[i], &EditVariablesList[i]);
        if (rc_io == IORC_VARLIST_NOT_FOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotInDataSet,
                FieldName[i], dsr->dataset_name);
            rc = EIE_FAIL;
        }
        else {
            if (IOVT_NUM != DSR_get_col_type(dsr, EditVariablesList[i])) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotNumericInDataSet,
                    FieldName[i], dsr->dataset_name);
                rc = EIE_FAIL;
            }
        }
    }
    return rc;
}
/***********************************************************/
/* Function: GetParms except ID                            */
/***********************************************************/
static void GetParms (
    SP_determin* sp,
    char ** EditsString,
    EIPT_POSITIVITY_OPTION * PositivityOption,
	int * ByGroupLoggingLevel)
{
    /* Edits */
    if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED)
        *EditsString = NULL;
    else
        *EditsString = sp->edits.value;

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
    ){
		*ByGroupLoggingLevel = 1;
	}
	else {
		*ByGroupLoggingLevel = 0;
	}
}
/**************************************************************/
/* Function: HasFTI                                           */
/*                                                            */
/* Purpose: Checks if obs has at least one variable FTI       */
/* Returns :                                                  */
/*  EIE_TRUE  (yes, at least one field with a FTI status)     */
/*  EIE_FALSE (no)                                            */
/**************************************************************/
static EIT_BOOLEAN HasFTI (
    EIT_DATAREC * DataRec)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++)
         if (DataRec->StatusFlag[i] == FIELDFTI)
             return EIE_TRUE;

    return EIE_FALSE;
}
/**************************************************************/
/* Function: HasInvalidData                                   */
/*                                                            */
/* Purpose: Checks if obs has at least one variable FIELDMISS */
/*          or FIELDNEG.                                      */
/*          Call this function AFTER the INSTATUS was read.   */
/* Returns :                                                  */
/*  EIE_TRUE  (yes, at least one field is invalid)            */
/*  EIE_FALSE (no)                                            */
/**************************************************************/
static EIT_BOOLEAN HasInvalidData (
    EIT_DATAREC * DataRec,
    T_DETERMIN_COUNTER * Counter)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++) {
        if (DataRec->StatusFlag[i] == FIELDMISS) {
            Counter->MissingValues++;
            return EIE_TRUE;
        }
        else if (DataRec->StatusFlag[i] == FIELDNEG) {
            Counter->NegativeValues++;
            return EIE_TRUE;
        }
    }

    return EIE_FALSE;
}
/**************************************************/
/* Function: InitCounter                          */
/*                                                */
/* Purpose:  Initialize components of "Counter"   */
/*           structure                            */
/**************************************************/
static void InitCounter (
    T_DETERMIN_COUNTER * Counter)
{
    Counter->ObsInByGroup = 0;
    Counter->ValidObsInByGroup = 0;
    Counter->MissingKeyDATA = 0;
    Counter->MissingValues = 0;
    Counter->NegativeValues = 0;
    Counter->MissingStatus = 0;
    Counter->ValidObsWithoutFTI = 0;
    Counter->ValidObsWithFTI = 0;
    Counter->ValidObsImputed = 0;
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
    if (LPI_FreeLpEnv(ilperrmsg) != LPI_SUCCESS)
        /* printing a generic message is enough */
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgFunctionFailure, "LPI_FreeLpEnv()");
}
/*************************************************************
Reads all INSTATUS obs belonging to the current DATA key.
Set status having FTI status for the key processed.

This function assumes that InStatus contains valid data.
*************************************************************/
static PROC_RETURN_CODE MatchStatus (
    SP_determin* sp,
    tDataDataSet * InData,
    tStatusDataSet * InStatus,
    T_DETERMIN_COUNTER * Counter)
{
    int l = -1;
    PROC_RETURN_CODE rc = PRC_SUCCESS;

    while (rc == PRC_SUCCESS) {
        l = strcmp (InData->Key->Value, InStatus->Key->Value);

        if (l == 0) {
            /* this instatus key is equal to the indata key */
            SetStatus (InData->DataRec, InStatus->FieldId->Value);
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

    return rc; /* error RC or PRC_SUCCESS_NO_MORE_DATA */
}
/***************************/
/* Function: PrintParms    */
/***************************/
static void PrintParms (
    SP_determin* sp,
    char * EditsString,
    EIPT_POSITIVITY_OPTION PositivityOption,
	int ByGroupLoggingLevel)
{
    /* Print the input and output files name */
    SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);
    SUtil_PrintInputDataSetInfo(&sp->dsr_instatus.dsr);

    SUtil_PrintOutputDataSetInfo(&sp->dsw_outdata);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);

    if (EditsString == NULL)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, EditsString);

    /* ID (the grammar has verified that ID */
    /*     exists in DATA data set)         */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, sp->dsr_indata.dsr.VL_unit_id.names[0]);
    }
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

    if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
         IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOption == EIPE_REJECT_NEGATIVE)
         IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
         IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

	if (ByGroupLoggingLevel == 1) {
		IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE BPN_NO_BY_STATS);
	}

    /* List of BY-variables - not mandatory */
    SUtil_PrintStatementVars(&sp->by.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_by_var, BPN_BY);

    IO_PRINT_LINE ("");
}
/**********************************************/
/* Function: PrintStatistics                  */
/*                                            */
/* Purpose: Prints statistics ("by group")    */
/* (warning will be print if no valid data)   */
/**********************************************/
static void PrintStatistics (
    T_DETERMIN_COUNTER * Counter,
    char * KeyName)
{
/* PRINT_COUNT_DATASET() is used to print the count of obs
   with the name of the data set.
   (LengthPlaceHolders: in Msg printed) */
#define PRINT_COUNT_DATASET(Msg, LengthPlaceHolders, DataSetName, Count) \
IO_PRINT_LINE (Msg, DataSetName, \
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - strlen (DataSetName) - \
PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

#define  ONETAB   4
#define  TWOTABS  8

/* PRINT_SUBTOTAL_NODATASET() is used to print subtotal of obs
   without the name of the data set, adding some heading spaces
   before the message.
   (LengthPlaceHolders: in Msg printed) */
#define PRINT_SUBTOTAL_NODATASET(Msg, LengthPlaceHolders, LengthHeadingSpace, \
Count) \
IO_PRINT_LINE (Msg, \
LengthHeadingSpace, \
MsgPadding,\
(int) (PRINT_STATISTICS_WIDTH - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace - \
PRINT_PRECISION), \
"............................................................", \
PRINT_PRECISION, Count);

    IO_PRINT_LINE ("");

    PRINT_COUNT_DATASET (MsgNumberObs, MsgNumberObs_LPH, DSN_INDATA ,
        Counter->ObsInByGroup);

    if (Counter->ValidObsInByGroup > 0) {

        PRINT_COUNT_DATASET (MsgNumberValidObs,
            MsgNumberValidObs_LPH, DSN_INDATA ,
            Counter->ValidObsInByGroup);

        PRINT_SUBTOTAL_NODATASET (MsgNumberValidObsWithoutFTI,
            MsgNumberValidObsWithoutFTI_LPH, ONETAB,
            Counter->ValidObsWithoutFTI);

        PRINT_SUBTOTAL_NODATASET (MsgNumberValidObsWithFTI,
            MsgNumberValidObsWithFTI_LPH, ONETAB,
            Counter->ValidObsWithFTI);

        PRINT_SUBTOTAL_NODATASET (MsgNumberValidObsImputed,
            MsgNumberValidObsImputed_LPH, TWOTABS,
            Counter->ValidObsImputed);

        PRINT_SUBTOTAL_NODATASET (MsgNumberValidObsNotImputed,
            MsgNumberValidObsNotImputed_LPH, TWOTABS,
            Counter->ValidObsWithFTI - Counter->ValidObsImputed);
    }

    if ((Counter->MissingKeyDATA > 0) || (Counter->MissingValues > 0) ||
        (Counter->NegativeValues > 0))
       IO_PRINT_LINE ("");

    if (Counter->MissingKeyDATA > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameInDataSet,
            Counter->MissingKeyDATA, DSN_INDATA , KeyName);

    if (Counter->MissingValues > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2,
                Counter->MissingValues, DSN_INDATA , DSN_INSTATUS);

    if (Counter->NegativeValues > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING
            MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2,
                Counter->NegativeValues, DSN_INDATA , DSN_INSTATUS);

    IO_PRINT_LINE ("");
}
/*********************************************************************
Reads DATA data set.
matches status
calls deterministic
*********************************************************************/
static PROC_RETURN_CODE ProcessByGroups (
    SP_determin* sp,
    tDataDataSet * InData,
    tStatusDataSet * InStatus,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    EIT_EDITS * Edits,
    tSList * OriginalVarNamesInEdits,
    EIPT_POSITIVITY_OPTION PositivityOption,
	int ByGroupLoggingLevel)
{
    T_DETERMIN_COUNTER Counter;
    EIT_BOOLEAN hasFTI;
    EIT_BOOLEAN hasInvalidData;
    int i;
    EIT_BOOLEAN MustReadStatus;
    int rc;

	InitCounter (&Counter);

    IO_DATASET_RC rc_next_by = DSRC_SUCCESS;
    while ((rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr)) == DSRC_NEXT_BY_SUCCESS) {

        /* synchronize INSTATUS
           if it cannot be synchronize we will NOT read it later */
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

        InitCounter (&Counter);
        ResetKeys (InData, InStatus);

        IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;
        while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {

            Counter.ObsInByGroup++;

            IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_indata.dsr);
            if (rc_get_rec != IORC_SUCCESS) {
                return PRC_FAIL_GET_REC;
            }

            SUtil_NullTerminate (InData->Key->Value, InData->Key->Size);

            if (InData->Key->Value[0] == '\0') {
                Counter.MissingKeyDATA++;
            }
            else {
                rc = CheckDataKeyOrder (InData->PreviousKey,
                    InData->Key->Value);
                if (rc == EIE_FAIL)
                    return PRC_FAIL_REC_NOT_SORTED;

                /* replace SAS missing with Banff missing value. */
                /* the API doesn't know anything about SAS missing value. */
                for (i = 0; i < InData->DataRec->NumberofFields; i++) {
                    if (IOUtil_is_missing(InData->DataRec->FieldValue[i])) {
                        InData->DataRec->FieldValue[i] = EIM_MISSING_VALUE;
                    }
                }

                if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
                    /* set status to FIELDOK or FIELDMISS */
                    EI_DataRecSetMissingFlag (InData->DataRec);
                else
                    /* set status to FIELDOK, FIELDMISS or FIELDNEG */
                    EI_DataRecSetFlag (InData->DataRec);

                if (MustReadStatus == EIE_TRUE) {
                    PROC_RETURN_CODE rc_match_status = MatchStatus (sp, InData, InStatus, &Counter);
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

                if (DEBUG)
                    EI_DataRecPrint (InData->DataRec);

                hasInvalidData = HasInvalidData (InData->DataRec, &Counter);
                if (!hasInvalidData) {
                    Counter.ValidObsInByGroup++;

                    hasFTI = HasFTI (InData->DataRec);
                    if (!hasFTI)
                        Counter.ValidObsWithoutFTI++;
                    else {
                        PROC_RETURN_CODE rc_write = PRC_FAIL_UNHANDLED;
                        Counter.ValidObsWithFTI++;
                        rc = EI_Deterministic (Edits, InData->DataRec);
                        EI_PrintMessages ();
                        switch (rc) {
                        case EIE_DETERMINISTIC_IMPUTED:
                            Counter.ValidObsImputed++;
                            rc_write = WriteResults (sp, OutData, OutStatus,
                                OriginalVarNamesInEdits);
                            if (rc_write != PRC_SUCCESS) {
                                return rc_write;
                            }

                            break;

                        case EIE_DETERMINISTIC_NOTIMPUTED:
                            if (DEBUG)
                                EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s not imputed\n", InData->Key->Value);
                            break;

                        case EIE_DETERMINISTIC_FAIL:
                        default:
                            return PRC_FAIL_DETERMINISTIC;
                        }
                    }
                }
            }
        }

        // check for errors
        if (rc_next_rec == DSRC_NO_MORE_REC_IN_BY) { // no error
            if (ByGroupLoggingLevel != 1) {
                PrintStatistics(&Counter, InData->Key->Name);
                DSR_cursor_print_by_message(&sp->dsr_indata.dsr, MSG_PREFIX_NOTE MsgHeaderForByGroupAbove_SAS_FREE, 1);
            }
        }
        else {
            return PRC_FAIL_ADVANCE_REC;
        }
    }

    // check for errors
    if(rc_next_by == DSRC_NO_MORE_REC_IN_DS){ // no error
        /* Status data set is read for each "by group"
        Print statistics on it one time only. */
        if (Counter.MissingStatus > 0) {
            IO_PRINT_LINE (SAS_NEWLINE SAS_MESSAGE_PREFIX_WARNING
                MsgNumberDroppedMissingKeyNameOrFieldidInDataSet SAS_NEWLINE,
                    Counter.MissingStatus, DSN_INSTATUS, InData->Key->Name);
        }

        EI_PrintMessages ();
        return PRC_SUCCESS_NO_MORE_DATA;
    }
    else if (rc_next_by == DSRC_BY_NOT_SORTED) {
        return PRC_FAIL_BY_NOT_SORTED;
    }
    else {
        return PRC_FAIL_ADVANCE_BY;
    }
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
static PROC_RETURN_CODE ReadStatus (
    SP_determin* sp,
    tStatusDataSet * InStatus,
    T_DETERMIN_COUNTER * Counter)
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

            if (strcmp (InStatus->Status->Value, EIM_STATUS_FTI) == 0) {
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
/*********************************************************************
reset key values so we can check key ordering from one by group to the next
*********************************************************************/
static void ResetKeys (
    tDataDataSet * InData,
    tStatusDataSet * InStatus)
{
    InData->Key->Value[0] = '\0';
    InData->PreviousKey[0] = '\0';
    InStatus->Key->Value[0] = '\0';
    InStatus->PreviousKey[0] = '\0';
}
/*********************************************************************
set status of variable to FTI, if variable is in the edits
*********************************************************************/
static void SetStatus (
    EIT_DATAREC * DataRec,
    char * FieldName)
{
    int Index;

    Index = UTIL_Binary_Reverse (FieldName, DataRec->FieldName,
        DataRec->NumberofFields);
    if (Index != -1)
        DataRec->StatusFlag[Index] = FIELDFTI;
}
/*****************************************************/
/* Function: ValidateParms                           */
/*                                                   */
/* Purpose: Validate procedure parameters.           */
/*****************************************************/
static EIT_RETURNCODE ValidateParms (
    SP_determin* sp,
    char * EditsString,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    EIT_RETURNCODE rc;

    rc = EIE_SUCCEED;

    /* INDATASET is "lookup" data set with a default data set */
    /* (this test is not required ...)                        */
    if (sp->dsr_indata.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INDATA );
        rc = EIE_FAIL;
    }

    if (sp->dsr_instatus.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INSTATUS);
        rc = EIE_FAIL;
    }

    if (EditsString == NULL || !strcmp (EditsString, "")) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_EDITS);
        rc = EIE_FAIL;
    }

    if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgStatementMandatory, BPN_UNIT_ID);
        rc = EIE_FAIL;
    }

    return rc;
}
/***********************************************************/
/* Function: WriteResults                                  */
/*                                                         */
/* Purpose : - Copy new imputed values for variables in    */
/*             edits to the vector used in gather write    */
/*             for OUTDATASET.                             */
/*           - Execute gather write for OUTDATASET and     */
/*             OUTSTATUSSET.                               */
/***********************************************************/
static PROC_RETURN_CODE WriteResults (
    SP_determin* sp,
    tDataDataSet * OutData,
    tStatusDataSet * OutStatus,
    tSList * OriginalVarNamesInEdits)
{
    int i;

    /* OutData->Key and OutStatus->Key is the same variable.
       So we only have to pad one of them. */
    SUtil_CopyPad (OutData->Key->Value, OutData->Key->Value,
        OutData->Key->Size);

    /* Get information on values for variables in the edits */
    for (i = 0; i < OutData->DataRec->NumberofFields; i++) {
        if (OutData->DataRec->StatusFlag[i] == FIELDIDE) {
            SUtil_CopyPad (OutStatus->FieldId->Value,
                OriginalVarNamesInEdits->l[i], LEGACY_MAXNAME);
            // copy value from `outdata` dataset
            OutStatus->FieldValue = OutData->DataRec->FieldValue[i];

            if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                return PRC_FAIL_WRITE_DATA;
            }
        }
        else if (OutData->DataRec->FieldValue[i] == EIM_MISSING_VALUE)
            OutData->DataRec->FieldValue[i] = IOUtil_missing_value('.');
    } /* for i */

    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdata)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}
