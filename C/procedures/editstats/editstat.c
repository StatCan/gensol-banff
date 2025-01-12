/* EDITSTATS                                        */
/* Apply an edit group to a data set                */
/* - put info in tables to summarize the results    */
/* - output the same results in output data sets    */


/* these must be define before including <uwproc.h> */
#define MAINPROC 1
#define SASPROC  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editstat_JIO.h"

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"
#include "EI_EditsConsistency.h"
#include "EI_EditsRedundancy.h"
#include "EI_Message.h"
#include "EI_Statistics.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageBanffAPI.h"

/* related to output data sets */
#define OUTEDITAPPLIC_NBVARS        6
#define OUTEDITSTATUS_NBVARS        4
#define OUTGLOBALSTATUS_NBVARS      4
#define OUTKEDITSSTATUS_NBVARS      4
#define OUTREDUCEDEDITS_NBVARS      2
#define OUTREDUCEDEDITS_MAXSIZE     32000 /* 32K: max size for one cell */
#define OUTVARSROLE_NBVARS          5

#define OUT_EDITAPPLIC_FAILED    "EDIT_APPLIC_FAILED"
#define OUT_EDITAPPLIC_INVOLVED  "EDITS_INVOLVED"
#define OUT_EDITAPPLIC_MISSED    "EDIT_APPLIC_MISSED"
#define OUT_EDITAPPLIC_NOTINVOLVED  "EDIT_APPLIC_NOTINVOLVED"
#define OUT_EDITAPPLIC_PASSED    "EDIT_APPLIC_PASSED"
#define OUT_EDITID      "EDITID"
#define OUT_EDIT_EQUATION  "EDIT_EQUATION"
#define OUT_FIELDID     SUTIL_FIELDSTAT_FIELDID
#define OUT_KEDITS      "K_EDITS"
#define OUT_OBS_FAILED  "OBS_FAILED"
#define OUT_OBS_MISSED  "OBS_MISSED"
#define OUT_OBS_NOTAPPLICABLE "OBS_NOT_APPLICABLE"
#define OUT_OBS_PASSED  "OBS_PASSED"
#define OUT_TOTAL       "OBS_TOTAL"  /* For OutGlobalStatus data set */


struct tVariableChar {
    char Name[LEGACY_MAXNAME+1];
    int Size;
    char * Value;
};
typedef struct tVariableChar tVariableChar;

struct tVariableNumeric {
    char Name[LEGACY_MAXNAME+1];
    int Size;
    double Value;
};
typedef struct tVariableNumeric tVariableNumeric;

struct tInData {
    EIT_DATAREC Data;
};
typedef struct tInData tInData;

struct tOutEditApplic {
    tVariableChar FieldId;
    tVariableNumeric EditApplicPassed;
    tVariableNumeric EditApplicMissed;
    tVariableNumeric EditApplicFailed;
    tVariableNumeric EditApplicNotInvolved;
    tVariableNumeric EditsInvolved;
};
typedef struct tOutEditApplic tOutEditApplic;

struct tOutEditStatus {
    tVariableNumeric EditId;
    tVariableNumeric ObsPassed;
    tVariableNumeric ObsMissed;
    tVariableNumeric ObsFailed;
};
typedef struct tOutEditStatus tOutEditStatus;

struct tOutGlobalStatus {
    tVariableNumeric ObsPassed;
    tVariableNumeric ObsMissed;
    tVariableNumeric ObsFailed;
    tVariableNumeric TotalObs;
};
typedef struct tOutGlobalStatus tOutGlobalStatus;

struct tOutKEditsStatus {
    tVariableNumeric KEdits;
    tVariableNumeric ObsPassed;
    tVariableNumeric ObsMissed;
    tVariableNumeric ObsFailed;
};
typedef struct tOutKEditsStatus tOutKEditsStatus;

struct tOutReducedEdits {
    tVariableNumeric EditId;
    tVariableChar    EditEquation;
};
typedef struct tOutReducedEdits tOutReducedEdits;

struct tOutVarsRole {
    tVariableChar FieldId;
    tVariableNumeric ObsPassed;
    tVariableNumeric ObsMissed;
    tVariableNumeric ObsFailed;
    tVariableNumeric ObsNotApplicable;
};
typedef struct tOutVarsRole tOutVarsRole;

/* for formatting one edit (to write in output "OutReducedEdits" */
static int mOneEditSize;
static int mTotalLength;


/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};


static PROC_RETURN_CODE DefineGatherWriteOutEditApplic (DSW_generic* dsw, tOutEditApplic *);
static PROC_RETURN_CODE DefineGatherWriteOutEditStatus (DSW_generic* dsw, tOutEditStatus *);
static PROC_RETURN_CODE DefineGatherWriteOutGlobalStatus (DSW_generic* dsw, tOutGlobalStatus *);
static PROC_RETURN_CODE DefineGatherWriteOutKEditsStatus (DSW_generic* dsw, tOutKEditsStatus *);
static PROC_RETURN_CODE DefineGatherWriteOutReducedEdits (DSW_generic* dsw, tOutReducedEdits *);
static PROC_RETURN_CODE DefineGatherWriteOutVarsRole (DSW_generic* dsw, tOutVarsRole *);
static PROC_RETURN_CODE DefineScatterRead (SP_editstat* sp, tInData *, int *);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_editstat* sp, int, int*);
static EIT_RETURNCODE FindVariablesPosition (SP_editstat* sp, char **, int, int *);
static void FormatOneEditForDataSet (double *, EIT_FIELDNAMES *, char *, char *);
static void GetParms (SP_editstat* sp, char **, EIPT_POSITIVITY_OPTION *);
static void LpFreeEnv (void);
static void PrintParms (SP_editstat* sp, char *, EIPT_POSITIVITY_OPTION);
static PROC_RETURN_CODE ProcessData (SP_editstat* sp, EIT_EDITS *, tInData *, tOutReducedEdits *,
    tOutEditStatus *, tOutKEditsStatus *, tOutGlobalStatus *, tOutEditApplic *,
    tOutVarsRole */*, EIPT_POSITIVITY_OPTION*/);
static EIT_RETURNCODE ValidateOutputsNotAllNULL (SP_editstat* sp);
static EIT_RETURNCODE ValidateParms (SP_editstat* sp, char *, EIT_EDITS *, EIPT_POSITIVITY_OPTION);
static PROC_RETURN_CODE VariableNumericDefineForOutput (DSW_generic* dsw, int destination_index, tVariableNumeric *);
static EIT_RETURNCODE SetUpOutDataSets (SP_editstat* sp, tOutReducedEdits *, int, tOutEditStatus *,
    tOutKEditsStatus *, tOutGlobalStatus *, tOutEditApplic *, tOutVarsRole *);
static void UnsetOutDataSets (SP_editstat* sp, tOutReducedEdits *, tOutEditApplic *, tOutVarsRole *);
static PROC_RETURN_CODE VariableCharDefineForOutput (DSW_generic* dsw, int destination_index, tVariableChar *);
static void VariableCharSet (tVariableChar *, char *);
static void VariableNumericSet (tVariableNumeric *, double);
static PROC_RETURN_CODE WriteOutReducedEdits (SP_editstat* sp, tOutReducedEdits *, EIT_EDITS *);
static PROC_RETURN_CODE WriteStatisticsOutDataSets (SP_editstat* sp, tOutEditStatus *, tOutKEditsStatus *,
    tOutGlobalStatus *, tOutEditApplic *, tOutVarsRole *, EIT_STATISTICS *,
    EIT_EDITS *);


/*********************************************************************
*********************************************************************/
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int editstat(
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
)
{
    EIT_EDITS Edits		 = { 0 };
	char * EditsString	= NULL;
    tInData InData		= { 0 };
    char *lpname		= NULL;
    char *lpversion		= NULL;
    tOutEditApplic OutEditApplic	= { 0 }; /* idem to Table 2.1 */
    tOutEditStatus OutEditStatus	= { 0 }; /* idem to Table 1.1 */
    tOutGlobalStatus OutGlobalStatus = { 0 }; /* idem to Table 1.3 */
    tOutKEditsStatus OutKEditsStatus = { 0 }; /* idem to Table 1.2 */
    tOutReducedEdits OutReducedEdits = { 0 }; /* idem to list of reduced edits
                                         (helpful to 'EditId' column
                                         in 'OutEditStatus' data set)*/
    tOutVarsRole OutVarsRole = { 0 };         /* idem to Table 2.2 */
    EIPT_POSITIVITY_OPTION PositivityOption = { 0 };
    int rc;
    int * VariablesPosition = { 0 };

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
    SUtil_PrintSystemInfo(SI_SYSTEMNAME, SI_SYSTEMVERSION, BANFF_PROC_NAME,
        SI_PROCVERSION, SI_EMAIL, NULL);

    SP_editstat sp = { 0 };
    TIME_WALL_START(load_init);
    TIME_CPU_START(load_init);
    mem_usage("before SP_init");

    IO_RETURN_CODE rc_sp_init = SP_init(
        &sp,
        in_parms,
        in_ds_indata,
        out_sch_outedit_applic,
        out_arr_outedit_applic,
        out_sch_outedit_status,
        out_arr_outedit_status,
        out_sch_outglobal_status,
        out_arr_outglobal_status,
        out_sch_outk_edits_status,
        out_arr_outk_edits_status,
        out_sch_outedits_reduced,
        out_arr_outedits_reduced,
        out_sch_outvars_role,
        out_arr_outvars_role
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

    GetParms (&sp, &EditsString, &PositivityOption);

    PrintParms (&sp, EditsString, PositivityOption);

    /* Initializes the LP environment */
    if (LPI_InitLpEnv (&lpname, &lpversion) != LPI_SUCCESS) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInitLPFail SAS_NEWLINE, "");
        proc_exit_code = BRC_FAIL_LPI_INIT;
		goto error_cleanup;
    }

    /* Validates parameters
       (identify also output data sets requested for statistics) */
    rc = ValidateParms (&sp, EditsString, &Edits, PositivityOption);
    if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
    }

    /* Finds variable number of edits fields in input data set */
    VariablesPosition = STC_AllocateMemory (
        (Edits.NumberofColumns-1) * sizeof *VariablesPosition);
    rc = FindVariablesPosition (&sp, Edits.FieldName,
        Edits.NumberofColumns-1, VariablesPosition);
    if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_EDITS_OTHER;
		goto error_cleanup;
    }

    /* Verify the exclusivity of variables between EDITS and BY */
    if (sp.dsr_indata.dsr.VL_by_var.count > 0) {
        rc = ExclusivityBetweenLists (&sp, Edits.NumberofColumns-1,
            VariablesPosition);
        if (rc == EIE_FAIL) {
            proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
			goto error_cleanup;
        }
    }

    /* Validation: STOP if all output data sets are _NULL_  */
    rc = EIE_SUCCEED;
    rc = ValidateOutputsNotAllNULL (&sp);
    if (rc == EIE_FAIL) {
        // this isn't an error, but there's nothing to do so we go to "error_cleanup"
        proc_exit_code = BRC_SUCCESS;
		goto error_cleanup;
    }

    EI_DataRecAllocate (Edits.FieldName, Edits.NumberofColumns-1, &InData.Data);

    if (PRC_SUCCESS != DefineScatterRead (&sp, &InData, VariablesPosition)) {
        proc_exit_code = BRC_FAIL_READ_GENERIC;
        goto error_cleanup;
    }

    /* Set up output data sets */
    rc = SetUpOutDataSets (&sp, &OutReducedEdits, Edits.NumberofColumns, &OutEditStatus,
        &OutKEditsStatus, &OutGlobalStatus, &OutEditApplic, &OutVarsRole);
    if (rc == EIE_FAIL) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
		goto error_cleanup;
    }

    TIME_CPU_STOPDIFF(load_init);
    TIME_WALL_STOPDIFF(load_init);

    TIME_WALL_START(processing);
    TIME_CPU_START(processing);

    /* Bulk of processing performed in ProcessData () */
    PROC_RETURN_CODE rc_processing = ProcessData (&sp, &Edits, &InData, &OutReducedEdits, &OutEditStatus,
        &OutKEditsStatus, &OutGlobalStatus, &OutEditApplic, &OutVarsRole);

    TIME_CPU_STOPDIFF(processing);
    TIME_WALL_STOPDIFF(processing);

    proc_exit_code = convert_processing_rc(rc_processing);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }

    mem_usage("Before SP_wrap");
    if (PRC_SUCCESS != SP_wrap(&sp)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }
    mem_usage("After SP_wrap");
error_cleanup:	
normal_cleanup:

    TIME_WALL_BEGIN(cleanup);
    TIME_CPU_BEGIN(cleanup);

    /* Unset output data sets */
    UnsetOutDataSets (&sp, &OutReducedEdits, &OutEditApplic, &OutVarsRole);

    if (VariablesPosition != NULL) {
        STC_FreeMemory(VariablesPosition);
        VariablesPosition = NULL;
    }
    // no null check: `.Data` statically allocated
    EI_DataRecFree (&InData.Data);

    if ((Edits.FieldName != NULL) &&
        (Edits.EditId != NULL) &&
        (Edits.Coefficient != NULL)
        ) {
        EI_EditsFree (&Edits);
        Edits.FieldName = NULL;
        Edits.EditId = NULL;
        Edits.Coefficient = NULL;
    }

    LpFreeEnv();
	if ((EI_mMessageList_Message_is_NULL() != 1)
		) {
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


/*
Define gather write for output OUTEDITAPPLIC= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutEditApplic (
    DSW_generic* dsw,
    tOutEditApplic * OutEditApplic)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTEDITAPPLIC_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutEditApplic->FieldId.Name, OUT_FIELDID);
    OutEditApplic->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutEditApplic->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditApplic->EditApplicPassed.Name, OUT_EDITAPPLIC_PASSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutEditApplic->EditApplicPassed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditApplic->EditApplicMissed.Name, OUT_EDITAPPLIC_MISSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutEditApplic->EditApplicMissed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditApplic->EditApplicFailed.Name, OUT_EDITAPPLIC_FAILED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutEditApplic->EditApplicFailed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditApplic->EditApplicNotInvolved.Name,
        OUT_EDITAPPLIC_NOTINVOLVED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutEditApplic->EditApplicNotInvolved)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditApplic->EditsInvolved.Name, OUT_EDITAPPLIC_INVOLVED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutEditApplic->EditsInvolved)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for output OUTEDITSTATUS= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutEditStatus (
    DSW_generic* dsw,
    tOutEditStatus * OutEditStatus)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTEDITSTATUS_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutEditStatus->EditId.Name, OUT_EDITID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++,
        &OutEditStatus->EditId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditStatus->ObsPassed.Name, OUT_OBS_PASSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++,
        &OutEditStatus->ObsPassed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditStatus->ObsMissed.Name, OUT_OBS_MISSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++,
        &OutEditStatus->ObsMissed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutEditStatus->ObsFailed.Name, OUT_OBS_FAILED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++,
        &OutEditStatus->ObsFailed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for output OUTGLOBALSTATUS= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutGlobalStatus (
    DSW_generic* dsw,
    tOutGlobalStatus * OutGlobalStatus)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTGLOBALSTATUS_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutGlobalStatus->ObsPassed.Name, OUT_OBS_PASSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutGlobalStatus->ObsPassed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutGlobalStatus->ObsMissed.Name, OUT_OBS_MISSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutGlobalStatus->ObsMissed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutGlobalStatus->ObsFailed.Name, OUT_OBS_FAILED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutGlobalStatus->ObsFailed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutGlobalStatus->TotalObs.Name, OUT_TOTAL);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutGlobalStatus->TotalObs)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for output OUTKEDITSSTATUS= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutKEditsStatus (
    DSW_generic* dsw,
    tOutKEditsStatus * OutKEditsStatus)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTKEDITSSTATUS_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutKEditsStatus->KEdits.Name, OUT_KEDITS);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutKEditsStatus->KEdits)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutKEditsStatus->ObsPassed.Name, OUT_OBS_PASSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutKEditsStatus->ObsPassed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutKEditsStatus->ObsMissed.Name, OUT_OBS_MISSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutKEditsStatus->ObsMissed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutKEditsStatus->ObsFailed.Name, OUT_OBS_FAILED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutKEditsStatus->ObsFailed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for output OUTREDUCEDEDITS= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutReducedEdits (
    DSW_generic* dsw,
    tOutReducedEdits * OutReducedEdits)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTREDUCEDEDITS_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutReducedEdits->EditId.Name, OUT_EDITID);
    if (PRC_SUCCESS != VariableNumericDefineForOutput (dsw, var_added++,
        &OutReducedEdits->EditId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutReducedEdits->EditEquation.Name, OUT_EDIT_EQUATION);
    OutReducedEdits->EditEquation.Size = mOneEditSize - 1;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++,
        &OutReducedEdits->EditEquation)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define gather write for output OUTVARSROLE= data set
*/
static PROC_RETURN_CODE DefineGatherWriteOutVarsRole (
    DSW_generic* dsw,
    tOutVarsRole * OutVarsRole)
{
    if (IORC_SUCCESS != DSW_allocate(dsw, OUTVARSROLE_NBVARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    IOUtil_copy_varname(OutVarsRole->FieldId.Name, OUT_FIELDID);
    OutVarsRole->FieldId.Size = LEGACY_MAXNAME;
    if (PRC_SUCCESS != VariableCharDefineForOutput (dsw, var_added++, &OutVarsRole->FieldId)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutVarsRole->ObsPassed.Name, OUT_OBS_PASSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutVarsRole->ObsPassed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutVarsRole->ObsMissed.Name, OUT_OBS_MISSED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutVarsRole->ObsMissed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutVarsRole->ObsFailed.Name, OUT_OBS_FAILED);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutVarsRole->ObsFailed)) {
        return PRC_FAIL_WRITE_DATA;
    }

    IOUtil_copy_varname(OutVarsRole->ObsNotApplicable.Name, OUT_OBS_NOTAPPLICABLE);
    if (PRC_SUCCESS != VariableNumericDefineForOutput(dsw, var_added++,
        &OutVarsRole->ObsNotApplicable)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_start_appending(dsw)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
Define scatter read
*/
static PROC_RETURN_CODE DefineScatterRead (
    SP_editstat* sp,
    tInData * InData,
    int * List)
{
    DSR_generic* dsr = &sp->dsr_indata.dsr;
    int i;

    // no key var...

    if (IORC_SUCCESS != VL_init_from_position_list(&dsr->VL_in_var, dsr, VL_NAME_in_var, IOVT_NUM, InData->Data.NumberofFields, List)) {
        return PRC_FAIL_UNHANDLED;
    }

    /* Define var. for scatter read */
    for (i = 0; i < InData->Data.NumberofFields; i++) {
        dsr->VL_in_var.ptrs[i] = (InData->Data.FieldValue + i);
    }

    return PRC_SUCCESS;
}

/*
Verify that the same variable do not appear in more then one place:
- EDITS (numeric variables) vs BY (numeric or character variables)
Called only if BY statement is specified.
*/
static EIT_RETURNCODE ExclusivityBetweenLists (
    SP_editstat* sp,
    int NumberofFields,
    int * VariablesPosition)
{
    int * ByList;
    int i;
    int j;
    int NumberOfByVariables;
    EIT_RETURNCODE rcLocal;

    rcLocal = EIE_SUCCEED;

    ByList = (int *)sp->dsr_indata.dsr.VL_by_var.positions;
    NumberOfByVariables = sp->dsr_indata.dsr.VL_by_var.count;

    for (i = 0; i < NumberOfByVariables; i++)
        for (j = 0; j < NumberofFields; j++)
            if (ByList[i] == VariablesPosition[j]) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgVarNameInTwoStatementsExclusive,
                    sp->dsr_indata.dsr.VL_by_var.names[i],
                        BPN_EDITS, BPN_BY);
                rcLocal = EIE_FAIL;
            }

    return rcLocal;
}

/*
 Build a list containing the position of the variables corresponding
 to the EDITS fields that can be found on the data set pointed to by
 InDataSet.
*/
static EIT_RETURNCODE FindVariablesPosition (
    SP_editstat* sp,
    char ** FieldName,
    int NumberofFields,
    int * VariablesPosition)
{
    int i;
    EIT_RETURNCODE rcLocal;
    long VariableType;

    rcLocal = EIE_SUCCEED;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    for (i = 0; i < NumberofFields; i++) {
        IO_RETURN_CODE rc_io = DSR_get_pos_from_names(dsr, 1, &FieldName[i], &VariablesPosition[i]);
        if (rc_io == IORC_VARLIST_NOT_FOUND) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotInDataSet,
                FieldName[i], dsr->dataset_name);
            rcLocal = EIE_FAIL;
        }
        else {
            VariableType = DSR_get_col_type(dsr, VariablesPosition[i]);
            if (VariableType != IOVT_NUM) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditsVarNotNumericInDataSet,
                    FieldName[i], dsr->dataset_name);
                rcLocal = EIE_FAIL;
            }
        }
    }

    return rcLocal;
}

/*
Reads procedure parameters.
Parameters not set are either set to their default value or mark as not set.
*/
static void GetParms (
    SP_editstat* sp,
    char ** EditsString,
    EIPT_POSITIVITY_OPTION * PositivityOption)
{
    if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED)
        *EditsString = NULL;
    else
        *EditsString = (char *) sp->edits.value;

    /* Get the positivity option set by the user */
    if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
        *PositivityOption = EIPE_REJECT_NEGATIVE_DEFAULT;
    else if (sp->accept_negative.value == IOB_FALSE)
        *PositivityOption = EIPE_REJECT_NEGATIVE;
    else if (sp->accept_negative.value == IOB_TRUE)
        *PositivityOption = EIPE_ACCEPT_NEGATIVE;
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

/*
Prints parameters
*/
static void PrintParms (
    SP_editstat* sp,
    char * EditsString,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    /* input data set */
    SUtil_PrintInputDataSetInfo (&sp->dsr_indata.dsr);

    /* output data sets */
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outedits_reduced);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outedit_status);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outk_edits_status);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outglobal_status);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outedit_applic);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outvars_role);

    if (EditsString == NULL)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, EditsString);

    if (PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOption == EIPE_REJECT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOption == EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

    /* List of BY-variables - no mandatory */
    SUtil_PrintStatementVars (&sp->by.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_by_var, BPN_BY);

    IO_PRINT_LINE ("");
}

/*
*/
static PROC_RETURN_CODE ProcessData (
    SP_editstat* sp,
    EIT_EDITS * Edits,
    tInData * InData,
    tOutReducedEdits * OutReducedEdits,
    tOutEditStatus * OutEditStatus,
    tOutKEditsStatus * OutKEditsStatus,
    tOutGlobalStatus * OutGlobalStatus,
    tOutEditApplic * OutEditApplic,
    tOutVarsRole * OutVarsRole/*,
    EIPT_POSITIVITY_OPTION PositivityOption*/)
{
    int i;
    EIT_STATISTICS Statistics;

    EI_StatisticsAllocate (&Statistics, Edits);

    /* write out OUTREDUCEDEDITS= data set */
    if (sp->dsw_outedits_reduced.is_requested == IOB_TRUE){
        PROC_RETURN_CODE rc_write = WriteOutReducedEdits (sp, OutReducedEdits, Edits);
        if (rc_write != PRC_SUCCESS) {
            return rc_write;
        }
    }

    /* Position at beginning of BY group
       (every iteration process one "by group")
    */

    PROC_RETURN_CODE rc_prc = PRC_FAIL_UNHANDLED;
    IO_DATASET_RC rc_next_by = DSRC_SUCCESS;
    while ((rc_next_by = DSR_cursor_next_by(&sp->dsr_indata.dsr)) == DSRC_NEXT_BY_SUCCESS) {
        EI_StatisticsInitialize (&Statistics);

        IO_DATASET_RC rc_next_rec = DSRC_SUCCESS;
        while ((rc_next_rec = DSR_cursor_next_rec(&sp->dsr_indata.dsr)) == DSRC_NEXT_REC_SUCCESS) {
            /* read an obs */

            IO_RETURN_CODE rc_get_rec = DSR_rec_get(&sp->dsr_indata.dsr);
            if (rc_get_rec != IORC_SUCCESS) {
                rc_prc = PRC_FAIL_GET_REC;
                goto error_cleanup;
            }


            /* replace SAS missing with Banff missing value. */
            /* the API doesn't know anything about SAS missing value. */
            for (i = 0; i < InData->Data.NumberofFields; i++) {
                if (IOUtil_is_missing(InData->Data.FieldValue[i]))
                    InData->Data.FieldValue[i] = EIM_MISSING_VALUE;
            }

            /* set datarec flag to ok or missing */
            EI_DataRecSetMissingFlag (&InData->Data);
            if (DEBUG) EI_DataRecPrint (&InData->Data);

            EI_StatisticsApplyEdits (Edits, &InData->Data, &Statistics);

            EI_StatisticsUpdateTables (&Statistics, Edits);
        }

        // check for errors
        if (rc_next_rec != DSRC_NO_MORE_REC_IN_BY) { // error
            rc_prc = PRC_FAIL_ADVANCE_REC;
            goto error_cleanup;
        }

        /* write out statistics on the "by group" in output data sets */
        if ((sp->dsw_outedit_status.is_requested == IOB_TRUE) ||
            (sp->dsw_outk_edits_status.is_requested == IOB_TRUE) ||
            (sp->dsw_outglobal_status.is_requested == IOB_TRUE) ||
            (sp->dsw_outedit_applic.is_requested == IOB_TRUE) ||
            (sp->dsw_outvars_role.is_requested == IOB_TRUE)) {
           PROC_RETURN_CODE rc_write = WriteStatisticsOutDataSets (sp, OutEditStatus, OutKEditsStatus,
               OutGlobalStatus, OutEditApplic, OutVarsRole,
               &Statistics, Edits);
           if (rc_write != PRC_SUCCESS) {
               return rc_write;
           }
        }
    }

    // check for errors
    if (rc_next_by == DSRC_NO_MORE_REC_IN_DS) { // no error
        rc_prc = PRC_SUCCESS_NO_MORE_DATA;
    }
    else if (rc_next_by == DSRC_BY_NOT_SORTED) {
        rc_prc = PRC_FAIL_BY_NOT_SORTED;
    }
    else {
        rc_prc = PRC_FAIL_ADVANCE_BY;
    }

    /* Useless... print only in the LOG
                  put information in output data sets.
    EI_StatisticsPrintDefinition (SUtil_PrintfToOutput);
    */
    error_cleanup:
    EI_StatisticsFree (&Statistics);
    SUtil_PrintfFree (); // no point? Not being used here?  See verifyedits for example of actual usage

    return rc_prc;
}

/*
Sets up optional structure for output data sets
*/
static EIT_RETURNCODE SetUpOutDataSets (
    SP_editstat* sp,
    tOutReducedEdits * OutReducedEdits,
    int NumberofColumns, /* including constant term in Edits */
    tOutEditStatus * OutEditStatus,
    tOutKEditsStatus * OutKEditsStatus,
    tOutGlobalStatus * OutGlobalStatus,
    tOutEditApplic * OutEditApplic,
    tOutVarsRole * OutVarsRole)
{
    if (sp->dsw_outedits_reduced.is_requested == IOB_TRUE) {
        /* calculate space needed for character variable EDIT
           - related to function "FormatOneEditForDataSet()" */
        mOneEditSize = NumberofColumns * (LEGACY_MAXNAME + 20) + 1000 + 1;
        if (mOneEditSize > OUTREDUCEDEDITS_MAXSIZE) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgEditSizeExceededMax, "OutReducedEdits");
            return EIE_FAIL;
        }
        else {
            if (PRC_SUCCESS != DefineGatherWriteOutReducedEdits(&sp->dsw_outedits_reduced, OutReducedEdits)) {
                return EIE_FAIL;
            }

        }
    }

    if (sp->dsw_outedit_status.is_requested == IOB_TRUE) {
        if (PRC_SUCCESS != DefineGatherWriteOutEditStatus (&sp->dsw_outedit_status, OutEditStatus)) {
            return EIE_FAIL;
        }
    }
    if (sp->dsw_outk_edits_status.is_requested == IOB_TRUE){
        if (PRC_SUCCESS != DefineGatherWriteOutKEditsStatus (&sp->dsw_outk_edits_status, OutKEditsStatus)) {
            return EIE_FAIL;
        }
    }
    if (sp->dsw_outglobal_status.is_requested == IOB_TRUE){
        if (PRC_SUCCESS != DefineGatherWriteOutGlobalStatus (&sp->dsw_outglobal_status, OutGlobalStatus)) {
            return EIE_FAIL;
        }
    }
    if (sp->dsw_outedit_applic.is_requested == IOB_TRUE){
        if (PRC_SUCCESS != DefineGatherWriteOutEditApplic (&sp->dsw_outedit_applic, OutEditApplic)) {
            return EIE_FAIL;
        }
    }
    if (sp->dsw_outvars_role.is_requested == IOB_TRUE){
        if (PRC_SUCCESS != DefineGatherWriteOutVarsRole (&sp->dsw_outvars_role, OutVarsRole)) {
            return EIE_FAIL;
        }
    }

    return EIE_SUCCEED;

}

/*
Free memory allocated for output data sets
*/
static void UnsetOutDataSets (
    SP_editstat* sp,
    tOutReducedEdits * OutReducedEdits,
    tOutEditApplic * OutEditApplic,
    tOutVarsRole * OutVarsRole)
{
    if (sp->dsw_outedits_reduced.is_requested == IOB_TRUE) {
        if (OutReducedEdits->EditEquation.Value != NULL) {
            STC_FreeMemory(OutReducedEdits->EditEquation.Value);
            OutReducedEdits->EditEquation.Value = NULL;
        }
    }

    if (sp->dsw_outedit_status.is_requested == IOB_TRUE) {
        if (OutEditApplic->FieldId.Value != NULL) {
            STC_FreeMemory(OutEditApplic->FieldId.Value);
            OutEditApplic->FieldId.Value = NULL;
        }
    }

    if (sp->dsw_outvars_role.is_requested == IOB_TRUE) {
        if (OutVarsRole->FieldId.Value != NULL) {
            STC_FreeMemory(OutVarsRole->FieldId.Value);
            OutVarsRole->FieldId.Value = NULL;
        }
    }
}

/*
Validate output data sets are not all _NULL_
*/
static EIT_RETURNCODE ValidateOutputsNotAllNULL (
    SP_editstat* sp)
{
    EIT_RETURNCODE rc;

    rc = EIE_SUCCEED;

    if ((sp->dsw_outedits_reduced.is_requested == IOB_FALSE) &&
        (sp->dsw_outedit_status.is_requested == IOB_FALSE)   &&
        (sp->dsw_outk_edits_status.is_requested == IOB_FALSE) &&
        (sp->dsw_outglobal_status.is_requested == IOB_FALSE) &&
        (sp->dsw_outedit_applic.is_requested == IOB_FALSE)   &&
        (sp->dsw_outvars_role.is_requested == IOB_FALSE)) {
           IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgAllOutDataSetsNULL);
           rc = EIE_FAIL;
    }
    return rc;
}

/*
Validate Parms.
NOTE: Validate all parms before quitting.
*/
static EIT_RETURNCODE ValidateParms (
    SP_editstat* sp,
    char * EditsString,
    EIT_EDITS * Edits,
    EIPT_POSITIVITY_OPTION PositivityOption)
{
    EIT_RETURNCODE crc; /* cumulative return code */
    int EditsNumberofEquations;
    int rc;

    crc = EIE_SUCCEED;

    if (sp->dsr_indata.dsr.is_specified == IOSV_NOT_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INDATA);
        crc = EIE_FAIL;
    }

    if (EditsString == NULL || !strcmp (EditsString, "")) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_EDITS);
        crc = EIE_FAIL;
    }

    if (PositivityOption == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        crc = EIE_FAIL;
    }

    if (crc == EIE_FAIL) return EIE_FAIL;

    rc = EI_EditsParse (EditsString, Edits);
    if (rc != EIE_SUCCEED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_EDITS);
        EI_PrintMessages ();
        return EIE_FAIL;
    }

    if (PositivityOption == EIPE_REJECT_NEGATIVE ||
        PositivityOption == EIPE_REJECT_NEGATIVE_DEFAULT) {
        rc = EI_EditsAddPositivity (Edits);
        if (rc == EIE_FAIL) {
            EI_PrintMessages ();
            EI_EditsFree (Edits);
            return EIE_FAIL;
        }
    }

    IO_PRINT_LINE (MsgHeaderEdits);
    if (PositivityOption != EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE (MsgPositivityEditsAdded);
    SUtil_PrintEdits (Edits);

    rc = EI_EditsConsistency (Edits);
    if (rc != EIE_EDITSCONSISTENCY_CONSISTENT) {
        if (rc == EIE_EDITSCONSISTENCY_INCONSISTENT)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInconsistentEdits SAS_NEWLINE);
        EI_PrintMessages (); /* Inconsistent or Fail*/
        EI_EditsFree (Edits);
        return EIE_FAIL;
    }

    EditsNumberofEquations = Edits->NumberofEquations;

    if (EI_EditsRedundancy (Edits) != EIE_EDITSREDUNDANCY_SUCCESS) {
        EI_PrintMessages ();
        EI_EditsFree (Edits);
        return EIE_FAIL;
    }

    if (EditsNumberofEquations == Edits->NumberofEquations)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgReducedEditsEquivalent SAS_NEWLINE);
    else {
        IO_PRINT_LINE (MsgHeaderReducedEdits);
        SUtil_PrintEdits (Edits);
    }

    return EIE_SUCCEED;
}

/*
Define char variable for output
Allocates the space needed.
*/
static PROC_RETURN_CODE VariableCharDefineForOutput (
    DSW_generic* dsw,
    int destination_index,
    tVariableChar * Variable)
{
    Variable->Value = STC_AllocateMemory (Variable->Size+1);
    if (IORC_SUCCESS != DSW_define_column(dsw, destination_index, Variable->Name, Variable->Value, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    return PRC_SUCCESS;
}

/*
set a tVariableChar, value must be null terminated.
*/
static void VariableCharSet (
    tVariableChar * Variable,
    char * Value)
{
    SUtil_CopyPad (Variable->Value, Value, Variable->Size);
}

/*
Define numeric variable for output
*/
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

/*
set a tVariableNumeric, takes care of missing values
*/
static void VariableNumericSet (
    tVariableNumeric * Variable,
    double Value)
{
    if (Value == EIM_MISSING_VALUE)
        Variable->Value = IOUtil_missing_value('.');
    else
        Variable->Value = Value;
}

/*
*/
static PROC_RETURN_CODE WriteOutReducedEdits (
    SP_editstat* sp,
    tOutReducedEdits * OutReducedEdits,
    EIT_EDITS * Edits)
{
    EIT_FIELDNAMES Field;
    int i;
    size_t l;
    char * OneEditString;

    Field.FieldName = Edits->FieldName;
    Field.NumberofFields = Edits->NumberofColumns-1;

    l = mOneEditSize - 1;
    OneEditString = STC_AllocateMemory(l);

    // error handling: since we need to free an allocation we have `rc_func`, which 
    // is success unless a non-success value is assigned
    // after error, minimal processing will occur before freeing and returning an error code
    PROC_RETURN_CODE rc_func = PRC_SUCCESS;

    /* write out the inequalities */
    for (i = 0; i < Edits->NumberofInequalities; i++) {
        VariableNumericSet (&OutReducedEdits->EditId, Edits->EditId[i]);

        memset (OneEditString, ' ', l);
        FormatOneEditForDataSet (Edits->Coefficient[i], &Field, "<=",
            OneEditString);

        VariableCharSet (&OutReducedEdits->EditEquation, OneEditString);

        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outedits_reduced)){
            rc_func = PRC_FAIL_WRITE_DATA;
        }
    }

    /* write out the equalities  */
    for (i = Edits->NumberofInequalities; i < Edits->NumberofEquations; i++) {
        VariableNumericSet (&OutReducedEdits->EditId, Edits->EditId[i]);

        memset (OneEditString, ' ', l);
        FormatOneEditForDataSet (Edits->Coefficient[i], &Field, " =",
            OneEditString);

        VariableCharSet (&OutReducedEdits->EditEquation, OneEditString);

        if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outedits_reduced)) {
            rc_func = PRC_FAIL_WRITE_DATA;
        }
    }

    STC_FreeMemory (OneEditString);

    return rc_func;
}

/*
*/
static PROC_RETURN_CODE WriteStatisticsOutDataSets (
    SP_editstat* sp,
    tOutEditStatus * OutEditStatus,
    tOutKEditsStatus * OutKEditsStatus,
    tOutGlobalStatus * OutGlobalStatus,
    tOutEditApplic * OutEditApplic,
    tOutVarsRole * OutVarsRole,
    EIT_STATISTICS * Statistics,
    EIT_EDITS * Edits)
{
    int i;
    int j;
    int NonZero; /* Number of edits using variable.
                    For column 'EditsInvolved' in
                    'OutEditApplic' data set. */

    /* write out OUTEDITSTATUS= data set
       (idem to Table 1.1) */
    if (sp->dsw_outedit_status.is_requested == IOB_TRUE) {
       for (i = 0; i < Statistics->NumberOfEquations; i++) {
           VariableNumericSet (&OutEditStatus->EditId, Edits->EditId[i]);
           VariableNumericSet (&OutEditStatus->ObsPassed,
               Statistics->Table11[STATISTICS_PASS][i]);
           VariableNumericSet (&OutEditStatus->ObsMissed,
               Statistics->Table11[STATISTICS_MISSING][i]);
           VariableNumericSet (&OutEditStatus->ObsFailed,
               Statistics->Table11[STATISTICS_FAIL][i]);
           
           if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outedit_status)) {
               return PRC_FAIL_WRITE_DATA;
           }
       }
    }

    /* write out OUTKEDITSSTATUS= data set
       (idem to Table 1.2) */
    if (sp->dsw_outk_edits_status.is_requested == IOB_TRUE) {
       for (i = 0; i < Statistics->NumberOfEquations+1; i++) {
           VariableNumericSet (&OutKEditsStatus->KEdits, i);
           VariableNumericSet (&OutKEditsStatus->ObsPassed,
               Statistics->Table12[STATISTICS_PASS][i]);
           VariableNumericSet (&OutKEditsStatus->ObsMissed,
               Statistics->Table12[STATISTICS_MISSING][i]);
           VariableNumericSet (&OutKEditsStatus->ObsFailed,
               Statistics->Table12[STATISTICS_FAIL][i]);

           if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outk_edits_status)) {
               return PRC_FAIL_WRITE_DATA;
           }
       }
    }

    /* write out OUTGLOBALSTATUS= data set
       (idem to Table 1.3) */
    if (sp->dsw_outglobal_status.is_requested == IOB_TRUE) {
       VariableNumericSet (&OutGlobalStatus->ObsPassed,
           Statistics->Table13[STATISTICS_PASS]);
       VariableNumericSet (&OutGlobalStatus->ObsMissed,
           Statistics->Table13[STATISTICS_MISSING]);
       VariableNumericSet (&OutGlobalStatus->ObsFailed,
           Statistics->Table13[STATISTICS_FAIL]);
       VariableNumericSet (&OutGlobalStatus->TotalObs,
           Statistics->Table13[STATISTICS_PASS] +
               Statistics->Table13[STATISTICS_MISSING] +
               Statistics->Table13[STATISTICS_FAIL]);

       if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outglobal_status)) {
           return PRC_FAIL_WRITE_DATA;
       }
    }

    /* write out OUTEDITAPPLIC= data set
       (idem to Table 2.1) */
    if (sp->dsw_outedit_applic.is_requested == IOB_TRUE) {
       for (i = Statistics->NumberOfColumns - 2; i >= 0; i--) {
           NonZero = 0;
           for (j=0; j < Statistics->NumberOfEquations; j++)
               if (Edits->Coefficient[j][i] != 0.0)
                   NonZero++;

           VariableCharSet (&OutEditApplic->FieldId,
               Edits->FieldName[i]);
           VariableNumericSet (&OutEditApplic->EditApplicPassed,
               Statistics->Table21[STATISTICS_PASS][i]);
           VariableNumericSet (&OutEditApplic->EditApplicMissed,
               Statistics->Table21[STATISTICS_MISSING][i]);
           VariableNumericSet (&OutEditApplic->EditApplicFailed,
               Statistics->Table21[STATISTICS_FAIL][i]);
           VariableNumericSet (&OutEditApplic->EditApplicNotInvolved,
               Statistics->Table21[STATISTICS_NOTAPPLICABLE][i]);
           VariableNumericSet (&OutEditApplic->EditsInvolved,
               NonZero);

           if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outedit_applic)) {
               return PRC_FAIL_WRITE_DATA;
           }
       }
    }

    /* write out OUTVARSROLE= data set
       (idem to Table 2.2) */
    if (sp->dsw_outvars_role.is_requested == IOB_TRUE) {
       for (i = Statistics->NumberOfColumns - 2; i >= 0; i--) {

           VariableCharSet (&OutVarsRole->FieldId,
               Edits->FieldName[i]);
           VariableNumericSet (&OutVarsRole->ObsPassed,
               Statistics->Table22[STATISTICS_PASS][i]);
           VariableNumericSet (&OutVarsRole->ObsMissed,
               Statistics->Table22[STATISTICS_MISSING][i]);
           VariableNumericSet (&OutVarsRole->ObsFailed,
               Statistics->Table22[STATISTICS_FAIL][i]);
           VariableNumericSet (&OutVarsRole->ObsNotApplicable,
               Statistics->Table22[STATISTICS_NOTAPPLICABLE][i]);

           if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outvars_role)) {
               return PRC_FAIL_WRITE_DATA;
           }
       }
    }

    return PRC_SUCCESS;
}

/*
to format one edit for output data set "OutReducedEdits"
*/
static void FormatOneEditForDataSet (
    double * Coefficient,   /* Coefficients of the edit (including constant) */
    EIT_FIELDNAMES * Field, /* Array of variable names */
    char * Relation,        /* Relation of the edit    */
    char * OneEditString)
{
    char Constant[1001]; /* Coefficient of the constant      */
    char Coeff[1001];    /* Coefficient of a variable        */
    char * Coeffptr;
    int FirstElement;    /* Indicator of first element       */
    int i;

    mTotalLength = 0;
    FirstElement = 1;

    mTotalLength += sprintf(OneEditString+mTotalLength, "\0");

    /* Convert the coefficient of the constant  */
    /* from a double to a character string      */
    if (TOOEXTREME (Coefficient[Field->NumberofFields]))
        sprintf (Constant, "%e", Coefficient[Field->NumberofFields]);
    else {
        sprintf (Constant, "%.5f", Coefficient[Field->NumberofFields]);
        UTIL_DropZeros (Constant);
    }

    /* Get coefficient of each variable  */
    for (i = Field->NumberofFields-1; i >= 0; i--) {
        if (Coefficient[i] != 0.0) {
            /* Convert the coefficient of the variable */
            /* from a double to a character string     */
            if (TOOEXTREME (Coefficient[i]))
                sprintf (Coeff, "%+e", Coefficient[i]);
            else {
                sprintf (Coeff, "%+.5f", Coefficient[i]);
                UTIL_DropZeros (Coeff);
            }
            Coeffptr = Coeff;

            if (FirstElement != 0) {
                /* First element: print the minus sign but not the plus sign */
                if (Coeffptr[0] == '-')
                    mTotalLength += sprintf(OneEditString+mTotalLength, "- ");
                Coeffptr++;
                FirstElement = 0;
            }
            else {
                /* Not first element: print the minus sign or plus sign     */
                if (Coeffptr[0] == '-')
                    mTotalLength += sprintf(OneEditString+mTotalLength, "- ");
                else
                    mTotalLength += sprintf(OneEditString+mTotalLength, "+ ");
                Coeffptr++;
            }

            if (Coefficient[i] != 1.0 && Coefficient[i] != -1.0) {
                /* Do not print a 1 coefficient    */
                mTotalLength += sprintf(OneEditString+mTotalLength, "%s ", Coeffptr);
                mTotalLength += sprintf(OneEditString+mTotalLength, "* ");
            }

            /* Print variable name */
            mTotalLength += sprintf(OneEditString+mTotalLength, "%s ",
                Field->FieldName[i]);
        }
    }

    /* Print relation and constant term*/
    mTotalLength += sprintf(OneEditString+mTotalLength, "%2.2s", Relation);
    mTotalLength += sprintf(OneEditString+mTotalLength, " %s ", Constant);

}
