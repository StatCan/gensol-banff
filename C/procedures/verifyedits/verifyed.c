/* NAME:         VERIFYEDITS                      */
/* PURPOSE:      Verifies edits for consistency,  */
/*               prints implied edits and         */
/*               prints extremal points.          */

/* these must be define before including <uwproc.h> */
#define MAINPROC 1
#define SASPROC  1

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "verifyed_JIO.h"

#include "EI_Check.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_EditsAddPositivity.h"
#include "EI_EditsConsistency.h"
#include "EI_EditsRedundancy.h"
#include "EI_Extremal.h"
#include "EI_Imply.h"
#include "EI_Message.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageBanffAPI.h"

static void GetParms (SP_verifyed* sp, char **, int *, int *, EIPT_POSITIVITY_OPTION *);
static void LpFreeEnv (void);
static void PrintParms (SP_verifyed* sp, char *, int *, int *, EIPT_POSITIVITY_OPTION);
static EIT_RETURNCODE ValidateParms (char *, EIT_EDITS *, int *,
    EIPT_POSITIVITY_OPTION);

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

EXPORTED_FUNCTION int verifyed(T_in_parm in_parms)
{
    EIT_EDITS Edits = { 0 };
    char * EditsString;
    EIT_CHECK_RETURNCODE EI_Check_rc;
    int ExtremalMaximumCardinality;
    char *lpversion = NULL;
    char *lpname = NULL;
    int NumberOfImpliedEdits;
    EIPT_POSITIVITY_OPTION PositivityOptionSet;
    int rc;

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

    SP_verifyed sp = { 0 };
    TIME_WALL_START(load_init);
    TIME_CPU_START(load_init);
    mem_usage("before SP_init");
    IO_RETURN_CODE rc_sp_init = SP_init(&sp, in_parms);
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

    /* Initializes the LP environment */
    if (LPI_InitLpEnv (&lpname, &lpversion) != LPI_SUCCESS) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgInitLPFail SAS_NEWLINE, "");
        proc_exit_code = BRC_FAIL_LPI_INIT;
        goto error_cleanup;
    }

    GetParms (&sp, &EditsString, &NumberOfImpliedEdits, &ExtremalMaximumCardinality,
        &PositivityOptionSet);

    PrintParms (&sp, EditsString, &NumberOfImpliedEdits, &ExtremalMaximumCardinality,
        PositivityOptionSet);

    rc = ValidateParms (EditsString, &Edits, &ExtremalMaximumCardinality,  // no need to pass `sp`, it won't be used
        PositivityOptionSet);
    if (rc == EIE_FAIL) {
        LpFreeEnv();
        proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
        goto error_cleanup;
    }

    SUtil_PrintfAllocate (); /* mandatory before "SUtil_PrintfToLog" */


    IO_PRINT_LINE (MsgHeaderEdits);
    if ((PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT) ||
        (PositivityOptionSet == EIPE_REJECT_NEGATIVE)) {

        EI_EditsAddPositivity (&Edits);
        IO_PRINT_LINE (MsgPositivityEditsAdded);
    }
    IO_PRINT_LINE ("");

    SUtil_PrintEdits (&Edits);

    TIME_CPU_STOPDIFF(load_init);
    TIME_WALL_STOPDIFF(load_init);

    TIME_WALL_START(processing);
    TIME_CPU_START(processing);

    rc = EI_EditsConsistency (&Edits);

    switch (rc) {
    case EIE_EDITSCONSISTENCY_CONSISTENT:
        EI_Check_rc = EI_Check (&Edits, SUtil_PrintfToLog);
        if (EI_Check_rc == EIE_CHECK_SUCCEED) {

            if (NumberOfImpliedEdits > 0 || ExtremalMaximumCardinality > 0)
                EI_EditsRedundancy (&Edits);

            if (NumberOfImpliedEdits > 0)
                EI_Imply (&Edits, NumberOfImpliedEdits, SUtil_PrintfToLog);

            if (ExtremalMaximumCardinality > 0)
                EI_Extremal (&Edits, ExtremalMaximumCardinality,
                    SUtil_PrintfToLog);

            proc_exit_code = BRC_SUCCESS;
        }
        else
            proc_exit_code = BRC_FAIL_EDITS_OTHER;
        break;

    case EIE_EDITSCONSISTENCY_INCONSISTENT:
        /* edits are inconsistent. call EI_Check (). */
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgInconsistentEdits);
        EI_Check (&Edits, SUtil_PrintfToLog);

        proc_exit_code = BRC_FAIL_EDITS_CONSISTENCY;
        break;
    case EIE_EDITSCONSISTENCY_FAIL:
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgFunctionFailure, "EI_EditsConsistency()");
        proc_exit_code = BRC_FAIL_UNHANDLED;
        break;
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

error_cleanup:
    TIME_WALL_BEGIN(cleanup);
    TIME_CPU_BEGIN(cleanup);

    SUtil_PrintfFree ();

    if ((Edits.FieldName != NULL) &&
        (Edits.EditId != NULL) &&
        (Edits.Coefficient != NULL)
        ) {
        EI_EditsFree(&Edits);
        Edits.FieldName = NULL;
        Edits.EditId = NULL;
        Edits.Coefficient = NULL;
    }


    if ((LPI_glpkpbtab_is_NULL() != 1)
        ) {
        LpFreeEnv();
    }
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
reads procedure parameters.
parameters not set will be set to their default value "after" printing.
*/
static void GetParms (
    SP_verifyed* sp,
    char ** EditsString,
    int * NumberOfImpliedEdits,
    int * ExtremalMaximumCardinality,
    EIPT_POSITIVITY_OPTION * PositivityOptionSet)
{
    /* EDITS */
    if (sp->edits.meta.is_specified == IOSV_NOT_SPECIFIED)
        *EditsString = "";
    else
        *EditsString = sp->edits.value;

    /* IMPLY: Number Of Implied Edits */
    if (sp->imply.meta.is_specified == IOSV_SPECIFIED)
        *NumberOfImpliedEdits = (int) sp->imply.value;

    /* EXTREMAL: Maximum cardinality */
    if (sp->extremal.meta.is_specified == IOSV_SPECIFIED)
        *ExtremalMaximumCardinality = (int) sp->extremal.value;

    /* Get the positivity option set by the user */
    if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
        *PositivityOptionSet = EIPE_REJECT_NEGATIVE_DEFAULT;
    else if (sp->accept_negative.value == IOB_FALSE)
        *PositivityOptionSet = EIPE_REJECT_NEGATIVE;
    else if (sp->accept_negative.value == IOB_TRUE)
        *PositivityOptionSet = EIPE_ACCEPT_NEGATIVE;
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
If "imply" or "extremal" aren't fixed or are negative,
fix them to the default value ie 0.
*/
static void PrintParms (
    SP_verifyed* sp,
    char * EditsString,
    int * NumberOfImpliedEdits,
    int * ExtremalMaximumCardinality,
    EIPT_POSITIVITY_OPTION PositivityOptionSet)
{
    int TempExtremalMaximumCardinality = 0;
    int TempNumberOfImpliedEdits = 0;

    /* EDITS */
    if (strcmp (EditsString, "") == 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_EDITS);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualLongString, BPN_EDITS, EditsString);

    /* IMPLY: Number Of Implied Edits */
    if (sp->imply.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *NumberOfImpliedEdits = 0;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_IMPLY, 0);
    }
    else if (*NumberOfImpliedEdits >= 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_IMPLY, *NumberOfImpliedEdits);
    else {
        /* if it's fixed, fix it to something valid */
        TempNumberOfImpliedEdits = -1; /* original value was negatif */
        *NumberOfImpliedEdits = 0;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_IMPLY, 0);
    }

    /* EXTREMAL: Maximum cardinality */
    if (sp->extremal.meta.is_specified == IOSV_NOT_SPECIFIED) {
        *ExtremalMaximumCardinality = 0;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_EXTREMAL, 0);
    }
    else if (*ExtremalMaximumCardinality >= 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_EXTREMAL,
            *ExtremalMaximumCardinality);
    else {
        /* if it's fixed, fix it to something valid */
        TempExtremalMaximumCardinality = -1; /* original value was negatif */
        *ExtremalMaximumCardinality = 0;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_EXTREMAL, 0);
    }

    /* Positivity option */
    if (PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegativeDefault);
    else if (PositivityOptionSet == EIPE_REJECT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgRejectNegative);
    else if (PositivityOptionSet == EIPE_ACCEPT_NEGATIVE)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE MsgAcceptNegative);

    if (TempNumberOfImpliedEdits < 0)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgParmInvalidNegativeValueReplacedByDefault,
            BPN_IMPLY);
    if (TempExtremalMaximumCardinality < 0)
        IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgParmInvalidNegativeValueReplacedByDefault,
            BPN_EXTREMAL);

    IO_PRINT_LINE ("");
}

/*
Validate Parms.
NOTE: Validate all parms before quitting.
*/
static EIT_RETURNCODE ValidateParms (
    char * EditsString,
    EIT_EDITS * Edits,
    int * ExtremalMaximumCardinality,
    EIPT_POSITIVITY_OPTION PositivityOptionSet)
{
    EIT_RETURNCODE crc; /* cumulative return code */
    EIT_RETURNCODE rc;

    crc = EIE_SUCCEED;

    if (strcmp (EditsString, "") == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_EDITS);
        crc = EIE_FAIL;
    }
    else {
        rc = EI_EditsParse (EditsString, Edits);
        if (rc != EIE_SUCCEED) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmInvalid, BPN_EDITS);
            EI_PrintMessages ();
            crc = EIE_FAIL;
        }
        else
            EI_PrintMessages ();
    }

    if (PositivityOptionSet == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        crc = EIE_FAIL;
    }

    if (PositivityOptionSet == EIPE_ACCEPT_NEGATIVE &&
            *ExtremalMaximumCardinality > 0) {
        *ExtremalMaximumCardinality = 0;
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNoExtremalPrinted SAS_NEWLINE);
    }
    return crc;
}
