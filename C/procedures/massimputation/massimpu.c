/*************************************************************************/
/* NAME:         MASSIMPUTATION                                          */
/* DESCRIPTION:  This function implements in the form of a SAS Procedure */
/*               the donor imputation method without searching for       */
/*               System matching fields, when a set of variables must    */
/*               all be imputed.                                         */
/*               Accept optionnally negative values data.                */
/*                                                                       */
/*               1. The DATA data set must have the following fields:    */
/*                  - "key name" ie ID variable                          */
/*                  - all the variables to impute ie MUSTIMPUTE list     */
/*                    --> a recipient must have missing values for       */
/*                        all these variables                            */
/*                  - all the "user matching fields" ie MUSTMATCH list   */
/*               2. No EDITS, no POSTEDITS                               */
/*                  (first nearest donor found is kept)                  */
/*                                                                       */
/* PRODUCT:      SAS/TOOLKIT                                             */
/* TYPE:         PROCEDURE                                               */
/*************************************************************************/
#define MAINPROC 1
#define SASPROC  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "massimpu_JIO.h"

#include "Donor.h"
#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Donor.h"
#include "EI_Edits.h"
#include "EI_Message.h"
#include "EI_SortRespondents.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"

/* to do with OUTSTATUS dataset */
#define NB_VARS_INSTATUS 3

/* To do with DONORMAP out dataset  */
#define DONORMAP_NB_VARS     4
#define DONORMAP_DONOR       "DONOR"
#define DONORMAP_DONORLIMIT  "DONORLIMIT"
#define DONORMAP_NB_ATTEMPTS "NUMBER_OF_ATTEMPTS"
#define DONORMAP_RECIPIENT   "RECIPIENT"

#define DEFAULT_MIN_DONORS   30
#define DEFAULT_PCENT_DONORS 30.0
#define MAX_PCENT_DONORS    100.0
#define MIN_PCENT_DONORS      1.0

#define NB_DECIMAL  2

#define VARIMPUSTMT_LEX  BPN_MUST_IMPUTE
#define VARMATCHSTMT_LEX BPN_MUST_MATCH

/* number of digit to print in report before IO_PRINT_LINE will start to use
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
    int MissingKey;       /* Counter to keep track of respondents */
                          /* with missing value for the key       */
    int MissingData;      /* Counter to keep track of potential   */
                          /* donor among respondents where at     */
                          /* least one field among MUSTIMPUTE     */
                          /* list is missing.                     */
    int NegativeData;     /* Counter to keep track of respondents */
                          /* with negative data value among       */
                          /* MUSTIMPUTE or MUSTMATCH lists.       */
    int Mixed;            /* Counter to keep track of potential   */
                          /* donor among respondents where at     */
                          /* least one field among MUSTMATCH      */
                          /* list is missing (not yet count among */
                          /* MissingData)                         */
} T_INFO;

/* Structure to hold info needed by the write function.*/
/* (Don't need information for status data set)                 */
 typedef struct {
    char * DonorKey;
    double NumberOfAttempts;
    double DonorLimit;
    char OutFieldid[LEGACY_MAXNAME + 1];
    char OutStatus[EIM_STATUS_SIZE + 1];
    double FieldValue;
    double * OutVarValue;
    char * RecipientKey;
    char * RespondentKey;
    int LengthKeyData;
    int RulesNumberOfFields; /* Number of fields in MUSTIMPUTE list  */
} T_WRITEINFO;


static void AdjustStatusRemoveSomeData (EIT_DATATABLE *, EIPT_POSITIVITY_OPTION,
    int, int, T_INFO *);
static EIT_DATATABLE * AllocateDataTable (SP_massimpu* sp);
static void AllocateDataVariables (char **, int, double **, int, int);
static void AllocateWriteVariables (char  **, double **, int, int);
static void ConcatenateAllFieldsNames (char **, int, char **, int,
     EIT_FIELDNAMES *);
static void CopyAllRespondentsToAllRecipients (EIT_ALL_RESPONDENTS *,
    EIT_ALL_RECIPIENTS *, int);
static void CopyDataTableToAllRespondents (EIT_DATATABLE *,
    EIT_ALL_RESPONDENTS *);
static EIT_BOOLEAN DataNegative (EIT_DATAREC *);
static PROC_RETURN_CODE DefineGatherWriteData (SP_massimpu* sp, T_CHAR_VARIABLE *, char *,
     char **, int, double *);
static PROC_RETURN_CODE DefineGatherWriteDonorMap (SP_massimpu* sp, char *, char *, double *, double *);
static PROC_RETURN_CODE DefineGatherWriteStatus(DSW_generic* dsw, T_CHAR_VARIABLE, T_WRITEINFO* UserData);
static void DefineScatterReadData (SP_massimpu* sp, int, int, T_CHAR_VARIABLE *, double *);
static PROC_RETURN_CODE DoImputation (SP_massimpu* sp, EIT_ALL_RESPONDENTS *, EIT_ALL_RECIPIENTS *,
    EIT_TRANSFORMED *, EIT_ALL_DONORS *, EIT_FIELDNAMES *, int, int, int,
    EIT_KDTREE *, T_INFO *, T_WRITEINFO *,
    char** AllFieldsNameOriginal);
static EIT_RETURNCODE ExclusivityBetweenLists (SP_massimpu* sp, int*, int, int*, int);
static void FillMatchingFieldsStruct (char **, int, EIT_FIELDNAMES *);
static EIT_READCALLBACK_RETURNCODE GetMatchingFieldData (char *, int *,
    char **, double *);
static void GetNamesOfFieldsInStatementList (DS_varlist* vl, EIT_FIELDNAMES *, int);
static void GetParms (SP_massimpu* sp, int *, int**,
    int *, int**, int *, double *, int *, T_CHAR_VARIABLE *,
    EIPT_POSITIVITY_OPTION *, int *, double *, int *);
static void IncrementTotalCounter (T_INFO *, T_INFO *);
static void InitCounter (T_INFO *);
static int IsRecipient (int, EIT_STATUS_OF_FIELDS *);
static void PrintInfoCounters (char *, T_INFO);
static void PrintParms (SP_massimpu* sp, int, double, int,
     char *, EIPT_POSITIVITY_OPTION, int, double, int ByGroupLoggingLevel);
static void PrintStatistics (int, int, T_INFO *, int);
static PROC_RETURN_CODE ReadByGroup (SP_massimpu* sp, int, char **, T_CHAR_VARIABLE *, double *,
    EIT_DATATABLE *, T_INFO *);
static PROC_RETURN_CODE ReadData (SP_massimpu* sp, int, char **, T_CHAR_VARIABLE *, double *,
    EIT_DATATABLE *, T_INFO *);
static EIT_RETURNCODE ValidateParms (SP_massimpu* sp, int, double, int,
    int*, int, int*, int, EIPT_POSITIVITY_OPTION, int, double);
static PROC_RETURN_CODE WriteResults (SP_massimpu* sp, T_WRITEINFO *, EIT_DATAREC *, char *, char *, int, int,
    char** AllFieldsNameOriginal);

/* These variables are used by callback functions. */
/* This is why they are global.                    */
static EIT_ALL_RESPONDENTS mAllRespondents;

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

EXPORTED_FUNCTION int massimpu(
    T_in_parm in_parms,

    T_in_ds in_ds_indata,

    T_out_ds out_sch_outdata,
    T_out_ds out_arr_outdata,
    T_out_ds out_sch_outstatus,
    T_out_ds out_arr_outstatus,
    T_out_ds out_sch_outdonormap,
    T_out_ds out_arr_outdonormap
)
{
	EIT_ALL_DONORS AllDonors			= { 0 };
    EIT_FIELDNAMES AllFields			= { 0 };
    char** AllFieldsNameOriginal = NULL;	/* Used by "WriteResults" (all the   */
                                    /* process is done with "uppercase") */
    EIT_ALL_RECIPIENTS AllRecipients	= { 0 };
    EIT_TRANSFORMED AllTransformed		= { 0 };
    EIT_DATATABLE * DataTable			= { 0 };/* To hold "all "data" for one "by group" */
												/* and copy them in "mAllRespondents"     */
												/* before processing. Overhead but:       */
												/* - no need to sorted datasets before    */
												/*   calling this procedure               */
												/* - no need to change the API's          */
    int EnoughDonors					= { 0 };
    int i								= { 0 };
    T_INFO Info							= { 0 };
    double * InVarValue					= { 0 }; /* Values read from input data set */
    EIT_KDTREE * KDTree					= { 0 };
    EIT_KDTREE_RETURNCODE KDTREErcode	= { 0 };
    T_CHAR_VARIABLE KeyDATA				= { 0 }; /* For "key" in DATA */
    EIT_FIELDNAMES MatchingFields		= { 0 };
    int MinimumNumberOfDonors			= { 0 };
    double MinimumPercentOfDonors		= { 0 };
	double Mrl							= { 0 };
    EIT_FIELDNAMES MustImputeFields		= { 0 };
    int* MustImputeVariablesList	= { 0 }; /* Array of numbers representing   */
												 /* the position of the must impute */
												 /* variables on the input data set */
    int* MustMatchVariablesList	= { 0 };/* Array of numbers representing   */
											    /* the position of the must match  */
											    /* variables on the input data set */
	int NLimit							= { 0 };
    int NumberOfMustImputeVars			= { 0 };
    int NumberOfMustMatchVars			= { 0 };
    EIPT_POSITIVITY_OPTION PositivityOptionSet	= { 0 };
    int RandomSelection					= { 0 };
	int ByGroupLoggingLevel;
    /* type not used but kept if ...
       EIT_SORTRESPONDENTS_QUICKSORT_RETURNCODE rcSortRespondents; */
    T_INFO TotalInfo					= { 0 };
    int TotalRecipients					= { 0 };
    int TotalRespondents				= { 0 };
    EIT_TRF_RETURNCODE TRFrcode			= { 0 };
    T_WRITEINFO UserData				= { 0 };
    EIT_FIELDNAMES UserMatchingFields	= { 0 };


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

    SP_massimpu sp = { 0 };
    TIME_WALL_START(load_init);
    TIME_CPU_START(load_init);
    mem_usage("before SP_init");
    IO_RETURN_CODE rc_sp_init = SP_init(
        &sp,
        in_parms,
        in_ds_indata,
        out_sch_outdata,
        out_arr_outdata,
        out_sch_outstatus,
        out_arr_outstatus,
        out_sch_outdonormap,
        out_arr_outdonormap
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
    GetParms (&sp, &NumberOfMustImputeVars,
        &MustImputeVariablesList, &NumberOfMustMatchVars,
        &MustMatchVariablesList, &MinimumNumberOfDonors,
        &MinimumPercentOfDonors, &RandomSelection, &KeyDATA,
        &PositivityOptionSet, &NLimit, &Mrl, &ByGroupLoggingLevel);

    /*********************************************/
    /* Get fields name on MUSTIMPUTE (mandatory) */
    /* and on MUSTMATCH (no mandatory).          */
    /*********************************************/
    GetNamesOfFieldsInStatementList (&sp.dsr_indata.VL_must_impute, &MustImputeFields,
        NumberOfMustImputeVars);

    GetNamesOfFieldsInStatementList (&sp.dsr_indata.VL_must_match, &UserMatchingFields,
        NumberOfMustMatchVars);

    /*************************************************************/
    /* Print parameters to LOG                                   */
    /* (for "seed": get/print it and start the generator number) */
    /*************************************************************/
    PrintParms (&sp, 
        MinimumNumberOfDonors, MinimumPercentOfDonors, RandomSelection, KeyDATA.Name,
        PositivityOptionSet, NLimit, Mrl, ByGroupLoggingLevel);

    /***************************/
    /* Validate parameters     */
    /***************************/
    EIT_RETURNCODE rcLocal = ValidateParms (&sp, MinimumNumberOfDonors,
        MinimumPercentOfDonors, NumberOfMustImputeVars, MustImputeVariablesList,
        NumberOfMustMatchVars, MustMatchVariablesList, RandomSelection,
        PositivityOptionSet, NLimit, Mrl);
    if (rcLocal == EIE_FAIL) {
        EI_PrintMessages ();
		proc_exit_code = BRC_FAIL_VALIDATION_LEGACY;
		goto error_cleanup;
    }

    /************************************************************/
    /* Get the total number of fields involved in this run      */
    /* which is the total number of must impute fields, when    */
    /* doing "massive imputation".                              */
    /* (in Proc DONOR, it's the total number of fields involved */
    /* in the PostEdits plus the total number of user defined   */
    /* matching fields that are not in the PostEdits)           */
    /************************************************************/
    UserData.RulesNumberOfFields = NumberOfMustImputeVars;

    ConcatenateAllFieldsNames (MustImputeFields.FieldName,
        NumberOfMustImputeVars, UserMatchingFields.FieldName,
        NumberOfMustMatchVars, &AllFields);

    /* Get "original" names of all fields: for "WriteResult" */
    if (AllFields.NumberofFields > 0) {
        AllFieldsNameOriginal = STC_AllocateMemory(
            AllFields.NumberofFields * sizeof * AllFieldsNameOriginal);
        if (AllFieldsNameOriginal == NULL) {
            EI_PrintMessages();
            /* LpFreeEnv (); */
            proc_exit_code = BRC_FAIL_ALLOCATE_MEMORY;
            goto error_cleanup;
        }
    }
    for (i = 0; i < AllFields.NumberofFields; i++) {
        AllFieldsNameOriginal[i] = STC_StrDup(AllFields.FieldName[i]);
        SUtil_GetOriginalName(&sp.dsr_indata.dsr, AllFieldsNameOriginal[i]);
    }

    /******************************************/
    /* Define scatter read for INDATAFILE     */
    /******************************************/

    /* Get the length of the key variable identified by the ID statement */
    /* This is the length of the column and not the length of the name   */
    /* of the column.                                                    */
    UserData.LengthKeyData = KeyDATA.Size;
    AllocateDataVariables (&UserData.RespondentKey, KeyDATA.Size, &InVarValue,
        NumberOfMustImputeVars, NumberOfMustMatchVars);

    DefineScatterReadData (&sp, NumberOfMustImputeVars, NumberOfMustMatchVars,
        &KeyDATA, InVarValue);

    /*******************************************************/
    /* Define gather write for writing updated respondents */
    /*******************************************************/
    AllocateWriteVariables (&UserData.RecipientKey, &UserData.OutVarValue,
        NumberOfMustImputeVars, KeyDATA.Size);

    if (PRC_SUCCESS != DefineGatherWriteData (&sp, &KeyDATA, UserData.RecipientKey, MustImputeFields.FieldName,
        NumberOfMustImputeVars, UserData.OutVarValue)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
        goto error_cleanup;
    }

    if (PRC_SUCCESS != DefineGatherWriteStatus(&sp.dsw_outstatus, KeyDATA, &UserData)) {
        proc_exit_code = BRC_FAIL_WRITE_GENERIC;
            goto error_cleanup;
    }

    UserData.DonorKey = STC_AllocateMemory (KeyDATA.Size+1);

    if (PRC_SUCCESS != DefineGatherWriteDonorMap (&sp, UserData.RecipientKey, UserData.DonorKey,
        &UserData.NumberOfAttempts, &UserData.DonorLimit)) {
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

    /******************************************/
    /* PROCESS: read and keep valid data      */
    /*          impute and write output data  */
    /******************************************/
    PROC_RETURN_CODE rc_processing = ReadByGroup (&sp, AllFields.NumberofFields,
        AllFields.FieldName, &KeyDATA, InVarValue, DataTable, &Info);

    /* Every iteration process one BY group */
    while (rc_processing == PRC_SUCCESS) {

        /* Adjust status and remove DATA with negative values if it's
           the option specified and "bad" donor (at least one missing value
           among MUSTIMPUTE/MUSTMATCH vars)
        */
        AdjustStatusRemoveSomeData (DataTable, PositivityOptionSet,
            NumberOfMustImputeVars, AllFields.NumberofFields, &Info);
		if (ByGroupLoggingLevel != 1) IO_PRINT_LINE ("");
		if (ByGroupLoggingLevel != 1 && (Info.MissingKey || Info.MissingData || Info.NegativeData || Info.Mixed))
            PrintInfoCounters (KeyDATA.Name, Info);

        if (Info.ValidObsInByGroup == 0) {
			if (ByGroupLoggingLevel != 1) {
				IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNoValidObs);
				PrintStatistics(mAllRespondents.NumberofRespondents,
					AllRecipients.NumberofRecipients, &Info, RandomSelection);
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
            EI_SortRespondents_Shellsort (&mAllRespondents);

            /* ... keep code for "Quicksort" */
            /*
            rcSortRespondents = EI_SortRespondents_Quicksort (&mAllRespondents);
            if (rcSortRespondents == EIE_SORTRESPONDENTS_QUICKSORT_FAIL ||
                rcSortRespondents == EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK){
                EI_PrintMessages ();
                if(rcSortRespondents==EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK)
                   IO_PRINT_LINE(SAS_MESSAGE_PREFIX_NOTE "Technical problem. Call your administrator.");
                proc_exit_code = BRC_FAIL_PROCESSING_GENERIC;
            }
            */

            /* Identify all recipients in the current BY Group */
            CopyAllRespondentsToAllRecipients (&mAllRespondents, &AllRecipients,
                NumberOfMustImputeVars);

            /* If any recipients, verify if enough donors.*/
            /* Otherwise, skip the current By group.      */
            if (!AllRecipients.NumberofRecipients) {
                /* SKIP the current BY Group */
				if (ByGroupLoggingLevel != 1) {
					IO_PRINT_LINE(SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNoRecipient);
					PrintStatistics(mAllRespondents.NumberofRespondents,
						AllRecipients.NumberofRecipients, &Info, RandomSelection);
				}
                IncrementTotalCounter (&TotalInfo, &Info);
                TotalRecipients += AllRecipients.NumberofRecipients;
                TotalRespondents += mAllRespondents.NumberofRespondents;
            }
            else { /* some "Recipients" */
                EnoughDonors = EI_EnoughDonors (mAllRespondents.NumberofRespondents, AllRecipients.NumberofRecipients,
					MinimumNumberOfDonors, MinimumPercentOfDonors);

                if (!EnoughDonors) {
                    /* SKIP the current BY Group */
					if (ByGroupLoggingLevel != 1) IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNoImputationDoneNotEnoughDonors);
                    Info.WithInsufficientDonors = AllRecipients.NumberofRecipients;
					if (ByGroupLoggingLevel != 1) PrintStatistics (mAllRespondents.NumberofRespondents,
                        AllRecipients.NumberofRecipients,&Info,RandomSelection);
                    IncrementTotalCounter (&TotalInfo, &Info);
                    TotalRecipients += AllRecipients.NumberofRecipients;
                    TotalRespondents += mAllRespondents.NumberofRespondents;
                }
                else { /* enough "Donors" --> PROCESS */

					int DonorLimit;

                    FillMatchingFieldsStruct (UserMatchingFields.FieldName,
                        UserMatchingFields.NumberofFields, &MatchingFields);

                    /* - Transform data      */
                    /* - Build KDtree        */
                    /* - Do imputation       */
                    TRFrcode = EI_Transform (&MatchingFields,
                        GetMatchingFieldData, &AllTransformed);
                    if (TRFrcode == EIE_TRANSFORM_FAIL) {
                        EI_PrintMessages ();
						proc_exit_code = BRC_EIE_TRANSFORM_FAIL;
						goto error_cleanup;
                    }

                    KDTREErcode = EI_BuildKDTree (&AllDonors, &KDTree,
                        &AllTransformed, &MatchingFields, &mAllRespondents);
                    if (KDTREErcode == EIE_KDTREE_FAIL) {
                        EI_PrintMessages ();
						proc_exit_code = BRC_EIE_KDTREE_FAIL;
						goto error_cleanup;
                    }

                    /* DEBUG
                    EI_PrintKDTree (KDTree, 0);
					SUtil_PrintMessages ();
                    ENDDEBUG */

					DonorLimit = EI_CalculateDonorLimit (NLimit, Mrl,
						mAllRespondents.NumberofRespondents-AllDonors.NumberOfDonors, AllDonors.NumberOfDonors);

                    PROC_RETURN_CODE rc_do_imputation = DoImputation (&sp, &mAllRespondents, &AllRecipients, &AllTransformed,
						&AllDonors, &MatchingFields, NumberOfMustImputeVars, RandomSelection,
						DonorLimit, KDTree, &Info, &UserData, AllFieldsNameOriginal);
                    if (rc_do_imputation != PRC_SUCCESS) {
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

						PrintStatistics(mAllRespondents.NumberofRespondents,
							AllRecipients.NumberofRecipients, &Info,
							RandomSelection);
					}
                    IncrementTotalCounter (&TotalInfo, &Info);
                    TotalRecipients += AllRecipients.NumberofRecipients;
                    TotalRespondents += mAllRespondents.NumberofRespondents;

                    EI_DestructKDTree (KDTree);
                    EI_FreeDonors (&AllDonors);
                    EI_FreeTransformed (&AllTransformed);

                    for (i = 0; i < MatchingFields.NumberofFields; i++)
                         STC_FreeMemory (MatchingFields.FieldName[i]);
                    STC_FreeMemory (MatchingFields.FieldName);
                } /* enough Donors */
            } /* some recipients */
        } /* some "valid observations" */

        InitCounter (&Info);
        AllRecipients.NumberofRecipients = 0;
        mAllRespondents.NumberofRespondents = 0;
        EI_DataTableEmpty (DataTable);

        rc_processing = ReadByGroup (&sp, AllFields.NumberofFields,
            AllFields.FieldName, &KeyDATA, InVarValue, DataTable, &Info);

    }

    proc_exit_code = convert_processing_rc(rc_processing);
    if (proc_exit_code != BRC_SUCCESS) {
        goto error_cleanup;
    }

    IO_PRINT_LINE ("");
    if (VL_is_specified(&sp.dsr_indata.dsr.VL_by_var)) {
        IO_PRINT_LINE ("");
        PrintStatistics (TotalRespondents, TotalRecipients, &TotalInfo,
            RandomSelection);
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgFooterAllByGroup SAS_NEWLINE);
    }
    IO_PRINT_LINE ("");

    TIME_CPU_STOPDIFF(processing);
    TIME_WALL_STOPDIFF(processing);

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

	/* free memory */
	if ((AllRecipients.Key != NULL) &&
		(AllRecipients.RecipientData != NULL)
		) {
		EI_FreeRecipients(&AllRecipients);
		AllRecipients.Key = NULL;
		AllRecipients.RecipientData = NULL;
	}
	if ((mAllRespondents.Key != NULL) &&
		(mAllRespondents.RespondentData != NULL) &&
		(mAllRespondents.RespondentExclusion != NULL)
		) {
		EI_FreeRespondents(&mAllRespondents);
		mAllRespondents.Key = NULL;
		mAllRespondents.RespondentData = NULL;
		mAllRespondents.RespondentExclusion = NULL;
	}
	if ((DataTable != NULL)
		) {
		EI_DataTableFree(DataTable);
		DataTable = NULL;
	}

	if ((KeyDATA.Value != NULL)
		) {
		STC_FreeMemory(KeyDATA.Value);
		KeyDATA.Value = NULL;
	}
	if ((InVarValue != NULL)
		) {
		STC_FreeMemory(InVarValue);
		InVarValue = NULL;
	}

    if (MustImputeVariablesList != NULL) {
        STC_FreeMemory(MustImputeVariablesList);
        MustImputeVariablesList = NULL;
    }

    if (MustImputeFields.FieldName != NULL) {
        for (int i = 0; i < MustImputeFields.NumberofFields; i++) {
            if (MustImputeFields.FieldName[i] != NULL) {
                STC_FreeMemory(MustImputeFields.FieldName[i]);
                MustImputeFields.FieldName[i] = NULL;
            }
        }
        STC_FreeMemory(MustImputeFields.FieldName);
        MustImputeFields.FieldName = NULL;
    }

    if (NumberOfMustMatchVars) {
		if ((MustMatchVariablesList != NULL)
			) {
			STC_FreeMemory(MustMatchVariablesList);
			MustMatchVariablesList = NULL;
		}
       for (i = 0; i < UserMatchingFields.NumberofFields; i++)
		   if ((UserMatchingFields.FieldName != NULL) &&
			   (UserMatchingFields.FieldName[i] != NULL)
			   ) {
			   STC_FreeMemory(UserMatchingFields.FieldName[i]);
			   UserMatchingFields.FieldName[i] = NULL;
		   }
	   if ((UserMatchingFields.FieldName != NULL)
		   ) {
		   STC_FreeMemory(UserMatchingFields.FieldName);
		   UserMatchingFields.FieldName = NULL;
	   }
    }

    for (i = 0; i < AllFields.NumberofFields; i++) {
		if ((AllFields.FieldName != NULL) &&
			(AllFields.FieldName[i] != NULL)
			) {
			STC_FreeMemory(AllFields.FieldName[i]);
			AllFields.FieldName[i] = NULL;
		}
        if ((AllFieldsNameOriginal != NULL) &&
            (AllFieldsNameOriginal[i] != NULL)
            ) {
            STC_FreeMemory(AllFieldsNameOriginal[i]);
            AllFieldsNameOriginal[i] = NULL;
        }
    }
	if ((AllFields.FieldName != NULL)
		) {
		STC_FreeMemory(AllFields.FieldName);
		AllFields.FieldName = NULL;
	}
    if ((AllFieldsNameOriginal != NULL)
        ) {
        STC_FreeMemory(AllFieldsNameOriginal);
        AllFieldsNameOriginal = NULL;
    }

	if ((UserData.DonorKey != NULL)
		) {
		STC_FreeMemory(UserData.DonorKey);
		UserData.DonorKey = NULL;
	}
	if ((UserData.OutVarValue != NULL)
		) {
		STC_FreeMemory(UserData.OutVarValue);
		UserData.OutVarValue = NULL;
	}
	if ((UserData.RecipientKey != NULL)
		) {
		STC_FreeMemory(UserData.RecipientKey);
		UserData.RecipientKey = NULL;
	}
	if ((UserData.RespondentKey != NULL)
		) {
		STC_FreeMemory(UserData.RespondentKey);
		UserData.RespondentKey = NULL;
	}
	if ((EI_mMessageList_Message_is_NULL() != 1)
		) {
		EI_FreeMessageList();
		EI_mMessageList_Message_to_NULL();
	}

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


/*********************************************************************
 AdjustStatusRemoveSomeData

 1.Adjust status for "good" observations:
   Recipient: - all MUSTIMPUTE vars: FIELDMISS -> FIELDFTI
              - MUSTMATCH vars: FIELDMISS->FIELDFTI
                                FIELDOK  ->FIELDMFU
   Donor: -all MUSTIMPUTE vars have FIELDOK (nothing to do)
          -all MUSTMATCH vars: FIELDOK->FIELDMFU
 2.Remove "bad" observations:
   - option specified: all obs with negative values
   - "bad" donor: at least one missing value among MUSTIMPUTE vars
                  at least one missing value among MUSTMATCH vars
**********************************************************************/
static void AdjustStatusRemoveSomeData (
    EIT_DATATABLE * DataTable,
    EIPT_POSITIVITY_OPTION PositivityOptionSet,
    int NumberOfMustImputeVars,
    int NumberAllFields, /* mustimpute + mustmatch */
    T_INFO * Counter)
{
    int CounterMissingMustImpute;
    int CounterMissingMustMatchForDonor;
    EIT_DATA * Data;
    EIT_DATA * DataNext;
    EIT_DATAREC * DataRec;
    int DataRecType; /* 0:donor and 1: recipient */
    int i;
    int j;
    EIT_DATA * PreviousData = NULL;
    EIT_BOOLEAN ToRemove;


    /* Note: in DataTable, status has been
       initialized to FIELDOK or FIELDMISS.
    */
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = DataNext) {

            /* only current data ... */
            DataRec = Data->DataRec[EIE_PERIOD_CURRENT];

            DataRecType = 0; /* put arbitrarily (don't yet known) */
            ToRemove = EIE_FALSE;
            CounterMissingMustImpute = 0;
            CounterMissingMustMatchForDonor = 0;

            /* option specified: identify obs. with negative values */
            if ((PositivityOptionSet == EIPE_REJECT_NEGATIVE_DEFAULT) ||
                     (PositivityOptionSet == EIPE_REJECT_NEGATIVE))
                ToRemove = DataNegative (DataRec);
            if (ToRemove == EIE_TRUE)
                Counter->NegativeData++;
            else {

                /* MUSTIMPUTE fields:
                   - count "missing"
                   - if all these fields are missing, that's
                     a recipient and adjust their status to FIELDFTI.
                */
                for (j = 0; j < NumberOfMustImputeVars; j++) {
                     if (DataRec->StatusFlag[j] == FIELDMISS)
                         CounterMissingMustImpute++;
                }
                if (CounterMissingMustImpute == NumberOfMustImputeVars) {
                    /* one recipient */
                    DataRecType = 1;
                    for (j = 0; j < NumberOfMustImputeVars; j++)
                         DataRec->StatusFlag[j] = FIELDFTI;
                }

                /* MUSTMATCH fields:
                   - if missing: adjust status to FIELDFTI for a recipient
                                 or increment the counter for a potential donor
                   - otherwise: adjust status to FIELDMFU (all obs)
                */
                for (j = NumberOfMustImputeVars; j < NumberAllFields; j++) {
                     if (DataRec->StatusFlag[j] == FIELDMISS) {
                        if (DataRecType)
                            DataRec->StatusFlag[j] = FIELDFTI;
                        else
                            CounterMissingMustMatchForDonor++;
                     }
                     else
                         DataRec->StatusFlag[j] = FIELDMFU;
                }

                if (!DataRecType) {
                    /* identify "bad" donor to be removed */
                    if (CounterMissingMustImpute > 0) {
                        ToRemove = EIE_TRUE;
                        Counter->MissingData++;
                    }
                    else if (CounterMissingMustMatchForDonor > 0) {
                        ToRemove = EIE_TRUE;
                        Counter->Mixed++;
                    }
                }
            }

			/* save the Next pointer now, after the free it is too late */
			DataNext = Data->Next;

            /* Remove observations identified as "bad" */
            if (ToRemove == EIE_TRUE) {
                Counter->ValidObsInByGroup--;
                if (Data == DataTable->Data[i])
                    DataTable->Data[i] = Data->Next;
                else
                    PreviousData->Next = Data->Next;

                EI_DataTableFreeItem (Data);
                DataTable->NumberOfData--;
            }
            else
                PreviousData = Data;

        } /* for (Data = DataTable->Data[i];...;...;) */
    } /* for (i = 0;...; i++) */
}

/*****************************************
 Decide what size to give to DataTable
 and allocate memory.
******************************************/
static EIT_DATATABLE * AllocateDataTable (
    SP_massimpu* sp)
{
    long long NumberOfObservations;

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
***************************************/
static void AllocateDataVariables (
    char ** RespondentKey,
    int LengthKeyData,
    double ** InVarValue,
    int NumberOfMustImputeVars,
    int NumberOfMustMatchVars)
{
    *RespondentKey = STC_AllocateMemory (LengthKeyData+1);
    *InVarValue = STC_AllocateMemory (
        (NumberOfMustImputeVars + NumberOfMustMatchVars) * sizeof **InVarValue);
}

/***************************************
 Allocate memory for Write Variables
****************************************/
static void AllocateWriteVariables (
    char ** RecipientKey,
    double ** OutVarValue,
    int NumberOfMustImputeVars,
    int LengthKeyData)
{
    *RecipientKey = STC_AllocateMemory (LengthKeyData+1);
    *OutVarValue = STC_AllocateMemory (NumberOfMustImputeVars * sizeof **OutVarValue);
}
/*************************************************
 Concatenate all fields names:
 - MUSTIMPUTE list
 - MUSTMATCH list
 (already verified that the lists are mutually
  exclusive)
*************************************************/
static void ConcatenateAllFieldsNames (
    char ** MustImputeFieldsNames,
    int NumberOfMustImputeVars,
    char ** UserMatchingFieldsNames,
    int NumberOfMustMatchVars,
    EIT_FIELDNAMES * NewArray)
{
    int i;

    /* NewArray: MustImpute + MustMatch */
    NewArray->FieldName = STC_AllocateMemory (
        (NumberOfMustImputeVars + NumberOfMustMatchVars) *
        sizeof *NewArray->FieldName);

    /* Copy all MustImputeFieldsNames to NewArray */
    NewArray->NumberofFields = NumberOfMustImputeVars;

    for (i = 0; i < NewArray->NumberofFields; i++) {
        NewArray->FieldName[i] = STC_StrDup (MustImputeFieldsNames[i]);
    }

    /* Add all UserMatchingFieldsNames */
    for (i = 0; i < NumberOfMustMatchVars; i++) {
        NewArray->FieldName[NewArray->NumberofFields] =
            STC_StrDup (UserMatchingFieldsNames[i]);
        NewArray->NumberofFields++;
    }

/*    UTIL_PrintStrings (NewArray->FieldName, NewArray->NumberofFields);*/
}

/**********************************************
 Copy Respondent Data to Recipient Variables
***********************************************/
static void CopyAllRespondentsToAllRecipients (
    EIT_ALL_RESPONDENTS * AllRespondents,
    EIT_ALL_RECIPIENTS * AllRecipients,
    int NumberOfFields) /* number of variables in must impute */
{
    int i; /* respondents index */
    int IdxRecipient;
    int isRecipient; /* Respondent is a "recipient" or not. */

/*    if (DEBUG) EI_RespondentsPrint (AllRespondents);*/

    IdxRecipient = 0;
    for (i = 0; i < AllRespondents->NumberofRespondents; i++) {

        isRecipient = IsRecipient (NumberOfFields,
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

/*****************************************
 Copy "DATATABLE" to "AllRespondents"
******************************************/
static void CopyDataTableToAllRespondents (
    EIT_DATATABLE * DataTable,
    EIT_ALL_RESPONDENTS * AllRespondents)
{
    EIT_DATA * Data;
    EIT_DATAREC * DataRec;
    int i;
    int IdxResp;

    IdxResp = 0;
    for (i = 0; i < DataTable->Size; i++) {
         for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {

              DataRec = Data->DataRec[EIE_PERIOD_CURRENT];

              if (AllRespondents->NumberofRespondents ==
                      AllRespondents->NumberofAllocatedRespondents) {
                  EI_AllocateRespondents (AllRespondents);
              }

              AllRespondents->Key[IdxResp] = Data->Key;

              AllRespondents->RespondentData[IdxResp].NumberofFields =
                  DataRec->NumberofFields;
              AllRespondents->RespondentData[IdxResp].FieldName =
                  DataRec->FieldName;
              AllRespondents->RespondentData[IdxResp].FieldValue =
                  DataRec->FieldValue;
              AllRespondents->RespondentData[IdxResp].StatusFlag =
                  DataRec->StatusFlag;

              /* Increment ... */
              AllRespondents->NumberofRespondents++;
              IdxResp++;
         } /* for (Data = DataTable->Data[i];...;...) */
   } /* for (i = 0;...;...;) */
}

/*********************************************************
 Check if at least one negative value among fields.
 Return: EIE_TRUE --> Data must be removed
         EIE_FALSE -> Data must be kept
 (Called only if "PositivityOptionSet" is
  EIPE_REJECT_NEGATIVE_DEFAULT or EIPE_REJECT_NEGATIVE)
**********************************************************/
static EIT_BOOLEAN DataNegative (
    EIT_DATAREC * DataRec)
{
    int i;
    EIT_BOOLEAN ToRemove;

    ToRemove = EIE_FALSE;

    for (i = 0; i < DataRec->NumberofFields; i++) {
         if ((DataRec->FieldValue[i] != EIM_MISSING_VALUE) &&
                 (DataRec->FieldValue[i] < 0.0)) {
             ToRemove = EIE_TRUE;
             i = DataRec->NumberofFields; /* skip remaining */
         }
    }

  return ToRemove;
}

/***************************************
 Define Gather Write for OUTDATASET
****************************************/
static PROC_RETURN_CODE DefineGatherWriteData (
    SP_massimpu* sp,
    T_CHAR_VARIABLE * KeyDATA,
    char * RecipientKey,
    char ** MustImputeVariablesList, /* original names */
    int NumberOfMustImputeVars,
    double * OutVarValue)
{
    int i;

    DSW_generic* dsw = &sp->dsw_outdata;

    /* Initialize gather write for all variables */
    if (IORC_SUCCESS != DSW_allocate(dsw, NumberOfMustImputeVars + 1)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    /* Define gather write for the key */
    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, KeyDATA->Name, RecipientKey, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* Define gather write for the numeric variables */
    for (i = NumberOfMustImputeVars-1; i >= 0; i--) {
        if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, MustImputeVariablesList[i], (OutVarValue + i), IOVT_NUM)) {
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
*******************************************/
static PROC_RETURN_CODE DefineGatherWriteDonorMap (
    SP_massimpu* sp,
    char * RecipientKey,
    char * DonorKey,
    double * NumberOfAttempts,
	double * DonorLimit)
{
    DSW_generic* dsw = &sp->dsw_outdonormap;

    /* Initialize gather write for all variables */
    if (IORC_SUCCESS != DSW_allocate(dsw, DONORMAP_NB_VARS)) {
        return PRC_FAIL_WRITE_DATA;
    }

    int var_added = 0;

    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_RECIPIENT, RecipientKey, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_DONOR, DonorKey, IOVT_CHAR)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_NB_ATTEMPTS, NumberOfAttempts, IOVT_NUM)) {
        return PRC_FAIL_WRITE_DATA;
    }

    if (IORC_SUCCESS != DSW_define_column(dsw, var_added++, DONORMAP_DONORLIMIT, DonorLimit, IOVT_NUM)) {
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
static PROC_RETURN_CODE DefineGatherWriteStatus(
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

/**************************************
 Define scatter read for INDATAFILE
***************************************/
static void DefineScatterReadData (
    SP_massimpu* sp,
    int NumberOfMustImputeVars,
    int NumberOfMustMatchVars,
    T_CHAR_VARIABLE * KeyDATA,
    double * InVarValue)
{
    int i;
    int j;
    int NumberOfVars;

    DSR_indata* dsr = &sp->dsr_indata;

   /* Initialize scatter read definition */
    NumberOfVars = NumberOfMustImputeVars+NumberOfMustMatchVars;

    /* Define key variable for scatter read */
    dsr->dsr.VL_unit_id.ptrs[0] = KeyDATA->Value;

    /* Define must impute variables for scatter read */
    for (i = 0; i < NumberOfMustImputeVars; i++) {
        dsr->VL_must_impute.ptrs[i] = (InVarValue + i);
    }

    /* Define must match variables for scatter read */
    /* Do this only if there are must match fields  */
    if (NumberOfVars > NumberOfMustImputeVars) {
        for (i = 0; i < NumberOfVars-NumberOfMustImputeVars; i++) {
             j = i+NumberOfMustImputeVars;
            dsr->VL_must_match.ptrs[i] = (InVarValue + j);
        }
    }
}

/********************************************************************
 Finds a donor (hopefully!) for every recipients.
 Calls EI_FindNearestDonors() when a recipient has matching fields
 and keep the first one with smallest distance.
 Calls EI_RandomDonorWithoutPostEdits() when a recipient does not
 have matching field and "RandomSelection".
*********************************************************************/
static PROC_RETURN_CODE DoImputation (
    SP_massimpu* sp,
    EIT_ALL_RESPONDENTS * AllRespondents,
    EIT_ALL_RECIPIENTS * AllRecipients,
    EIT_TRANSFORMED * AllTransformed,
    EIT_ALL_DONORS * AllDonors,
    EIT_FIELDNAMES * MatchingFields,
    int NumberOfMustImputeVars,
    int RandomSelection,
	int DonorLimit,
    EIT_KDTREE * KDTree,
    T_INFO * Info,
    T_WRITEINFO * UserData,
    char** AllFieldsNameOriginal)
{
    EIT_ONERECIPIENTALLOCATE_RETURNCODE ALLOCATEFTI_rcode;
    int DonorRow;
    int * FTIFields;
    int i;
    EIT_NEARESTDONORS * NearestDonors;
    EIT_FINDNEARESTDONORS_RETURNCODE NEARESTDONORSrcode;
    EIT_RANDOMDONOR_RETURNCODE RANDOMDONORrcode;

    /* FTI fields: NumberOfMustImputeVars + 1 for constant term */
    ALLOCATEFTI_rcode = EI_OneRecipient_AllocateFTIFields (&FTIFields, NumberOfMustImputeVars + 1);

    for (i = 0; i < AllRecipients->NumberofRecipients; i++) {

        EI_OneRecipient_GetFTIFields (FTIFields, NumberOfMustImputeVars + 1,
            &AllRecipients->RecipientData[i]);

        if (EI_OneRecipient_HasMatchingFields_TypeMFU (&AllRecipients->RecipientData[i])) {
            /* recipient has matching fields:
               try to find a donor from the nearest donors */
            Info->WithMatchingFields++;
            NEARESTDONORSrcode = EI_FindNearestDonors (1/* find one donor */,
                &AllRecipients->RecipientData[i], AllRecipients->Key[i],
                FTIFields, MatchingFields, AllTransformed, AllDonors,
                KDTree, &NearestDonors, DonorLimit);

            switch (NEARESTDONORSrcode) {
            case EIE_FINDNEARESTDONORS_SUCCEED:
                /* identify first nearest donor without trying it (no post
                   edits), copy donor values and write results */
                DonorRow = EI_OneRecipient_FindNextNearestDonor (0/*the index of the one donor*/,
					NearestDonors);
				AllDonors->Gave[DonorRow]++;
                EI_OneRecipient_CopyDonorValues (DonorRow, AllDonors->Key,
                    AllRespondents, FTIFields, &AllRecipients->RecipientData[i]);
                if (PRC_SUCCESS != WriteResults (sp, UserData, &AllRecipients->RecipientData[i],
                    AllRecipients->Key[i], AllDonors->Key[DonorRow], 1/*one attempt*/, DonorLimit, AllFieldsNameOriginal)) {
                    return PRC_FAIL_WRITE_DATA;
                }
                Info->WithMatchingFieldsImputed++;
                EI_OneRecipient_FreeNearestDonors (NearestDonors);
                break;

            case EIE_FINDNEARESTDONORS_NOTFOUND:
                /* no donor */
                Info->WithMatchingFieldsNotImputed++;
                break;

            case EIE_FINDNEARESTDONORS_FAIL: // fall through
            default:
                EI_OneRecipient_FreeFTIFields (FTIFields);
                return PRC_FAIL_PROCESSING_GENERIC;
            }
        }
        else {
            /* recipient has no matching field:
            try to find a donor randomly */
            Info->WithoutMatchingFields++;
            if (RandomSelection == 1) {
                DonorRow = -1;
                RANDOMDONORrcode = EI_RandomDonorWithoutPostEdits (
                    &AllRecipients->RecipientData[i], FTIFields, AllRespondents,
                    AllDonors, &DonorRow, DonorLimit);
                EI_PrintMessages ();

                switch (RANDOMDONORrcode) {
                case EIE_RANDOMDONOR_SUCCEED:
					AllDonors->Gave[DonorRow]++;
                    if (PRC_SUCCESS != WriteResults (sp, UserData, &AllRecipients->RecipientData[i],
                        AllRecipients->Key[i], AllDonors->Key[DonorRow], 0, DonorLimit, AllFieldsNameOriginal)) {
                        return PRC_FAIL_WRITE_DATA;
                    }
                    Info->WithoutMatchingFieldsImputed++;
                    break;

                case EIE_RANDOMDONOR_NOTFOUND:
                    Info->WithoutMatchingFieldsNotImputed++;
                    break;

                case EIE_FINDNEARESTDONORS_FAIL: // fall through
                default:
                    EI_OneRecipient_FreeFTIFields (FTIFields);
                    return PRC_FAIL_PROCESSING_GENERIC;
                }
            }
        } /* recipient has no matching field */
    }

    EI_OneRecipient_FreeFTIFields (FTIFields);

    return PRC_SUCCESS;
}

/***********************************************
 Verify variables exclusivity between:
 - MUSTIMPUTE vs MUSTMATCH
 - BY-list of num variables vs MUSTIMPUTE
 - BY-list of num variables vs MUSTMATCH
 - ID vs BY-list of char variables
 (via position of variables on InDataFile)
************************************************/
static EIT_RETURNCODE ExclusivityBetweenLists (
    SP_massimpu* sp,
    int* MustImputeVariables,
    int MustImputeNumberOfVar,
    int* MustMatchVariables,
    int MustMatchNumberOfVar)
{
    int* ByList;
    int* ByListTypeChar;
    int* ByListTypeNum;
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

    DSR_indata *dsr = &sp->dsr_indata;
    /* If any BY-variables, identify their position */
    NumberOfByVars = dsr->dsr.VL_by_var.count;
    if (NumberOfByVars != 0)
        ByList = dsr->dsr.VL_by_var.positions;

    /* BY-variables: build array of position for num variables     */
    /*                     array of position for char variables    */
    for (i = 0; i < NumberOfByVars; i++) {
         VariableType = DSR_get_col_type(&dsr->dsr, ByList[i]);
         if (VariableType == IOVT_NUM)
             NumberOfByVarsNum++;
         else
             NumberOfByVarsChar++;
    }

    ByListTypeNum  = STC_AllocateMemory (NumberOfByVarsNum  * sizeof *ByListTypeNum);
    ByListTypeChar = STC_AllocateMemory (NumberOfByVarsChar * sizeof *ByListTypeChar);

    j = 0;
    k = 0;
    for (i = 0; i < NumberOfByVars; i++) {
         VariableType = DSR_get_col_type(&dsr->dsr, ByList[i]);
         if (VariableType == IOVT_NUM) {
             ByListTypeNum[j] = ByList[i];
             j++;
         }
         else {
             ByListTypeChar[k] = ByList[i];
             k++;
         }
    }

    /* ByListTypeChar vs Id */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
	    i = 0;
        while (i < NumberOfByVarsChar) {
            if (ByListTypeChar[i] == dsr->dsr.VL_unit_id.positions[0]) {
                IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, BPN_UNIT_ID, BPN_BY);
                rc = EIE_FAIL;
                break;
            }
            i++;
        }
    }

    /* MUSTIMPUTE vs MUSTMATCH */
    found = 0;
    for (i = 0; i < MustMatchNumberOfVar; i++) {
         for (j = 0; j < MustImputeNumberOfVar; j++) {
              if (MustMatchVariables[i] == MustImputeVariables[j]) {
                  IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, VARIMPUSTMT_LEX, VARMATCHSTMT_LEX);
                  rc = EIE_FAIL;
                  found = 1;
                  break;
              }
         }
         if (found || MustImputeNumberOfVar == 0)
             break;
    }

    /* ByListTypeNum vs MustImputeVariables */
    found = 0;
    for (i = 0; i < NumberOfByVarsNum; i++) {
         for (j = 0; j < MustImputeNumberOfVar; j++) {
              if (ByListTypeNum[i] == MustImputeVariables[j]) {
                  IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, VARIMPUSTMT_LEX, BPN_BY);
                  rc = EIE_FAIL;
                  found = 1;
                  break;
              }
         }
         if (found || MustImputeNumberOfVar == 0)
             break;
    }

    /* ByListTypeNum vs MustMatchVariables */
    found = 0;
    for (i = 0; i < NumberOfByVarsNum; i++) {
         for (j = 0; j < MustMatchNumberOfVar; j++) {
              if (ByListTypeNum[i] == MustMatchVariables[j]) {
                  IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgTwoStatementsExclusive, VARMATCHSTMT_LEX, BPN_BY);
                  rc = EIE_FAIL;
                  found = 1;
                  break;
              }
         }
         if (found || MustMatchNumberOfVar == 0)
             break;
    }

    if (NumberOfByVarsNum)
        STC_FreeMemory (ByListTypeNum);
    if (NumberOfByVarsChar)
        STC_FreeMemory (ByListTypeChar);

    return rc;
}
/******************************************
Fill "Matching Field Names" structure
*******************************************/
static void FillMatchingFieldsStruct (
    char ** UserMatchingFieldNames,
    int NumberOfUserMatchingFields,
    EIT_FIELDNAMES * MatchingFields)
{
    int i;

    MatchingFields->FieldName = STC_AllocateMemory (
        NumberOfUserMatchingFields * sizeof *MatchingFields->FieldName);
    MatchingFields->NumberofFields = NumberOfUserMatchingFields;

    for (i = 0; i < MatchingFields->NumberofFields; i++) {
        MatchingFields->FieldName[i] = STC_StrDup (UserMatchingFieldNames[i]);
    }
}

/**********************************************************
 This is the call back function (from EI_Transform) that
 gets the transformable data for one matching field at
 a time.
 ObsIndex should be set to 0 by the calling function
 before the first time GetMatchingFieldData () is called.
 This function does not read from the SAS data set.
 It will get the data from the AllRespondents structure.
***********************************************************/
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

/*************************************************
 Function to populate a UserFields structure
**************************************************/
static void GetNamesOfFieldsInStatementList (
    DS_varlist* vl,
    EIT_FIELDNAMES * UserFields,
    int NumberOfVars)
{
    int i;

    UserFields->NumberofFields = NumberOfVars;
    UserFields->FieldName = STC_AllocateMemory (
        UserFields->NumberofFields * sizeof *UserFields->FieldName);

    for (i = 0; i < UserFields->NumberofFields; i++) {
        UserFields->FieldName[i] = STC_AllocateMemory (LEGACY_MAXNAME+1);
        IOUtil_copy_varname(UserFields->FieldName[i], vl->names[i]);
    }
}

/*******************************************
 Get procedure parameters except "seed".
 (it'll be get later, inside PrintParms)
********************************************/
static void GetParms (
    SP_massimpu* sp,
    int * NumberOfMustImputeVars,
    int** MustImputeVariablesList,
    int * NumberOfMustMatchVars,
    int** MustMatchVariablesList,
    int * MinimumNumberOfDonors,
    double * MinimumPercentOfDonors,
    int * RandomSelection,
    T_CHAR_VARIABLE * KeyDATA,
    EIPT_POSITIVITY_OPTION * PositivityOptionSet,
	int * NLimit,
	double * Mrl,
	int * ByGroupLoggingLevel)
{
    double DoubleMinimumNumberOfDonors;

   /* ID statement */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        KeyDATA->Position = sp->dsr_indata.dsr.VL_unit_id.positions[0];
        IOUtil_copy_varname(KeyDATA->Name, sp->dsr_indata.dsr.col_names[KeyDATA->Position]);
        KeyDATA->Size = DSR_get_column_length(&sp->dsr_indata.dsr, sp->dsr_indata.dsr.VL_unit_id.positions[0]); //sp->dsr_indata.dsr.unit_id_size;
        KeyDATA->Value = STC_AllocateMemory (KeyDATA->Size+1);
    }

    /* Must Impute Fields - Mandatory */
    if (!VL_is_specified(&sp->dsr_indata.VL_must_impute)) {
        *NumberOfMustImputeVars = 0;
        *MustImputeVariablesList = NULL;
    }
    else {
        int* m;
        int i;

        *NumberOfMustImputeVars = sp->dsr_indata.VL_must_impute.count;
        m = sp->dsr_indata.VL_must_impute.positions;

        *MustImputeVariablesList = STC_AllocateMemory (
            *NumberOfMustImputeVars * sizeof **MustImputeVariablesList);

        for (i = 0; i < *NumberOfMustImputeVars; i++)
            (*MustImputeVariablesList)[i] = m[i];
    }

    /* Must Match Fields - no mandatory */
    if (!VL_is_specified(&sp->dsr_indata.VL_must_match)) {
        *NumberOfMustMatchVars = 0;
        *MustMatchVariablesList = NULL;
    }
    else {
        int* l;
        int i;

        *NumberOfMustMatchVars = sp->dsr_indata.VL_must_match.count;
        l = sp->dsr_indata.VL_must_match.positions;

        *MustMatchVariablesList = STC_AllocateMemory (
            *NumberOfMustMatchVars * sizeof **MustMatchVariablesList);

        for (i = 0; i < *NumberOfMustMatchVars; i++)
            (*MustMatchVariablesList)[i] = l[i];
    }

    /* Minimum Acceptable Number of Donors: default value. */
    /* (an INT in the grammar)                             */
    if (sp->min_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
        *MinimumNumberOfDonors = DEFAULT_MIN_DONORS;
    else {
        DoubleMinimumNumberOfDonors = sp->min_donors.value;
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

    /* Random Selection option.
       - not specified (set to zero):
         no random selection will be performed.
       - specified (set to 1):
         random selection will be performed.
    */
    *RandomSelection = sp->random.meta.is_specified == IOSV_SPECIFIED && sp->random.value == IOB_TRUE ? 1 : 0;

    /* Get the positivity option set by the user */
    if (sp->accept_negative.meta.is_specified == IOSV_NOT_SPECIFIED)
        *PositivityOptionSet = EIPE_REJECT_NEGATIVE_DEFAULT;
    else if (sp->accept_negative.value == IOB_FALSE)
        *PositivityOptionSet = EIPE_REJECT_NEGATIVE;
    else if (sp->accept_negative.value == IOB_TRUE)
        *PositivityOptionSet = EIPE_ACCEPT_NEGATIVE;

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

/*************************************************************
 Increment "Total counter" with "Counter for one BY group"
**************************************************************/
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
    TotalInfo->Mixed += Info->Mixed;
}

/*******************************************
 Initialize counters of structure T_INFO
********************************************/
static void InitCounter (
    T_INFO * Info)
{
    Info->ObsInByGroup = 0;
    Info->ValidObsInByGroup = 0;
	Info->DonorsReachedDonorLimit = 0;;
    Info->WithMatchingFields = 0;
    Info->WithMatchingFieldsImputed = 0;
    Info->WithMatchingFieldsNotImputed = 0;
    Info->WithoutMatchingFields = 0;
    Info->WithoutMatchingFieldsImputed = 0;
    Info->WithoutMatchingFieldsNotImputed = 0;
    Info->WithInsufficientDonors = 0;
    Info->MissingKey = 0;
    Info->MissingData = 0;
    Info->Mixed = 0;
    Info->NegativeData = 0;
}

/******************************************************
 Identify if a respondent is a recipient.
 Recipient is a respondent that have at least one
 a field flagged "FTI" among MUSTIMPUTE fields.
 Note:
 A record "potentially" a donor is already skipped
 from the respondents when at least one of these
 fields was flagged FTI.
*******************************************************/
static int IsRecipient (
    int NumberOfFields,
    EIT_STATUS_OF_FIELDS * StatusFlag)
{
    int i;

    for (i = 0; i < NumberOfFields; i++)
        if (StatusFlag[i] == FIELDFTI)
            return 1;

    return 0;
}

/*******************************************
 Print counters for observations dropped
 in DATA data set for one BY Group.
********************************************/
static void PrintInfoCounters (
    char * KeyName,
    T_INFO Info)
{
    /*if (Info.MissingKey || Info.MissingData || Info.NegativeData ||
            Info.Mixed)
        IO_PRINT_LINE ("");*/

    if (Info.MissingKey > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMissingKeyNameInDataSet,
             Info.MissingKey, DSN_INDATA, KeyName);

    if (Info.MissingData > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedSomeMissingButNotAllMustimpute,
            Info.MissingData);

    if (Info.NegativeData > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedNegativeMustimputeOrMustmatch,
            Info.NegativeData);

    if (Info.Mixed > 0)
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_WARNING MsgNumberDroppedMixedMissingMustmatch,
            Info.Mixed);

    IO_PRINT_LINE ("");
}

/*********************************************************
 Print the values of the parameters to the LOG
 (for "seed", get/print it and start the generator...)
**********************************************************/
static void PrintParms (
    SP_massimpu* sp,
    int MinimumNumberOfDonors,
    double MinimumPercentOfDonors,
    int RandomSelection,
    char * KeyName,
    EIPT_POSITIVITY_OPTION PositivityOptionSet,
	int NLimit,
	double Mrl,
	int ByGroupLoggingLevel)
{
    double DoubleSeed;
    int RandSeed;

    /* All data sets name */
    SUtil_PrintInputDataSetInfo(&sp->dsr_indata.dsr);

    SUtil_PrintOutputDataSetInfo(&sp->dsw_outdata);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outstatus);
    SUtil_PrintOutputDataSetInfo(&sp->dsw_outdonormap);

    /* Minimum acceptable number of donors */
    if ((MinimumNumberOfDonors > EIM_MIN_INT) &&
        (MinimumNumberOfDonors < EIM_MAX_INT)) {
        if (sp->min_donors.meta.is_specified == IOSV_NOT_SPECIFIED)
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger MsgDefault, BPN_MIN_DONORS, MinimumNumberOfDonors);
        else
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualInteger, BPN_MIN_DONORS, MinimumNumberOfDonors);
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
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgSeedEqualIntegerChosenBySystem, RandSeed);
    }
    else {
        DoubleSeed = sp->seed.value;
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

    /* Key variable name */
    if (VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id))
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmEqualString, BPN_UNIT_ID, KeyName);
    else
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_NOTE MsgParmNotSpecified, BPN_UNIT_ID);

    /* List of fields to impute - mandatory */
    SUtil_PrintStatementVars(&sp->must_impute.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.VL_must_impute, VARIMPUSTMT_LEX);

    /* List of matching fields - no mandatory */
    SUtil_PrintStatementVars(&sp->must_match.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.VL_must_match, VARMATCHSTMT_LEX);

    /* List of BY-variables - no mandatory */
    SUtil_PrintStatementVars(&sp->by.meta, &sp->dsr_indata.dsr, &sp->dsr_indata.dsr.VL_by_var, BPN_BY);

    IO_PRINT_LINE ("");
}

/*****************************************************
 Print the imputation statistics to the LOG window
******************************************************/
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
        MsgNumberObservations_LPH,
        Info->ObsInByGroup);

    PRINT_COUNT_NOPERCENT (MsgNumberObservationsDropped,
        MsgNumberObservationsDropped_LPH,
        Info->MissingKey + Info->NegativeData + Info->MissingData +
        Info->Mixed);

    if (Info->MissingKey + Info->NegativeData + Info->MissingData +
        Info->Mixed > 0) {

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
    }

/* Used when printing count and percent */
#define PRINT_LENGTH_FLOAT    6
#define PRINT_LENGTH_INTERSPACES  5 /* nb of blanks between count and percent*/
#define PRINT_LENGTH_SYMBOL   1 /* for symbol % */
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
#define PRINT_SUBTOTAL_PERCENT(Msg, LengthPlaceHolders, LengthHeadingSpace, \
Count, Percent) \
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
        PRINT_SUBTOTAL_PERCENT (
            MsgNumberPercentRecipientsWithoutEnoughDonors,
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
}
/*************************************************
 Read all data for one "by group" and fill
 "DataTable" structure.
 Return: 0 when no more data in DATA data set
         1 when no more data in the "by group"
**************************************************/
static PROC_RETURN_CODE ReadByGroup (
    SP_massimpu* sp,
    int NumberAllFields, /* mustimpute + mustmatch */
    char ** VarNamesAllFields, /* in uppercase by SAS */
    T_CHAR_VARIABLE * KeyDATA,
    double * InVarValue,
    EIT_DATATABLE * DataTable,
    T_INFO * Info)
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

    PROC_RETURN_CODE rc_ReadData = ReadData (sp, NumberAllFields, VarNamesAllFields,
        KeyDATA, InVarValue, DataTable, Info);
    if (rc_ReadData != PRC_SUCCESS_NO_MORE_DATA) {
        return rc_ReadData;
    }

    return PRC_SUCCESS;
}

/**************************
 Read DATA data set.
***************************/
static PROC_RETURN_CODE ReadData (
    SP_massimpu* sp,
    int NumberAllFields, /* mustimpute + mustmatch */
    char ** VarNamesAllFields, /* in uppercase by SAS */
    T_CHAR_VARIABLE * KeyDATA,
    double * InVarValue,
    EIT_DATATABLE * DataTable,
    T_INFO * Info)
{
    EIT_DATA * Data;
    int i;
    EIT_DATATABLELOOKUP_RETURNCODE rcLookup;

    IO_DATASET_RC rc_next_rec;

    DSR_generic* dsr = &sp->dsr_indata.dsr;

    while ((rc_next_rec = DSR_cursor_next_rec(dsr)) == DSRC_NEXT_REC_SUCCESS) {

        Info->ObsInByGroup++;

        IO_RETURN_CODE rc_get_rec = DSR_rec_get(dsr);
        if (rc_get_rec != IORC_SUCCESS) {
            return PRC_FAIL_GET_REC;
        }
        SUtil_NullTerminate (KeyDATA->Value, KeyDATA->Size);

        if (KeyDATA->Value[0] == '\0') {
            Info->MissingKey++;
        }
        else {
            rcLookup = EI_DataTableLookup (DataTable, KeyDATA->Value,
                EIE_DATATABLELOOKUPTYPE_CREATE, &Data);

            switch (rcLookup) {
            case EIE_DATATABLELOOKUP_NOTFOUND:
                /* Cool... continuons */

                Info->ValidObsInByGroup++;

                Data->DataRec[EIE_PERIOD_CURRENT] = STC_AllocateMemory (
                    sizeof *Data->DataRec[EIE_PERIOD_CURRENT]);

                EI_DataRecAllocate (VarNamesAllFields, NumberAllFields,
                    Data->DataRec[EIE_PERIOD_CURRENT]);
                for (i = 0; i < NumberAllFields; i++)
                    if (IOUtil_is_missing (InVarValue[i]))
                        Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] =
                            EIM_MISSING_VALUE;
                    else
                        Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[i] =
                            InVarValue[i];

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

/*************************************
 Validate Procedure Parameters
**************************************/
static EIT_RETURNCODE ValidateParms (
    SP_massimpu* sp,
    int MinimumNumberOfDonors,
    double MinimumPercentOfDonors,
    int NumberOfMustImputeVars,
    int* MustImputeVariablesList,
    int NumberOfMustMatchVars,
    int* MustMatchVariablesList,
    int RandomSelection,
    EIPT_POSITIVITY_OPTION PositivityOptionSet,
	int NLimit,
	double Mrl)
{
    int crc = EIE_SUCCEED; /* cumulative return code */
    int rc;

    /* DATA data set is not NULL */
    if (sp->dsr_indata.dsr.is_specified != IOSV_SPECIFIED) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgDataSetMandatory, DSN_INDATA);
        crc = EIE_FAIL;
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

    if (PositivityOptionSet == EIPE_ACCEPT_AND_REJECT_NEGATIVE) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgPositivityOptionInvalid);
        crc = EIE_FAIL;
    }

    /* ID statement - mandatory */
    if (!VL_is_specified(&sp->dsr_indata.dsr.VL_unit_id)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, BPN_UNIT_ID);
        crc = EIE_FAIL;
    }

    /* Must Impute Fields - Mandatory */
    if (NumberOfMustImputeVars == 0) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmMandatory, VARIMPUSTMT_LEX);
        crc = EIE_FAIL;
    }
    else {
        /* Check if variables are repeated in MUSTIMPUTE */
        if (SUtil_AreDuplicateInListPosition (NumberOfMustImputeVars,
                MustImputeVariablesList)) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmWithDuplicateVariable, VARIMPUSTMT_LEX);
            crc = EIE_FAIL;
        }
    }

    /* Check if variables are repeated in MUSTMATCH */
    /* (MUSTMATCH is no mandatory)                  */
    if (NumberOfMustMatchVars) {
        if (SUtil_AreDuplicateInListPosition (NumberOfMustMatchVars,
                MustMatchVariablesList)) {
            IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgParmWithDuplicateVariable, VARMATCHSTMT_LEX);
            crc = EIE_FAIL;
        }
    }

    /* Validate the exclusivity of variables between lists. */
    rc = ExclusivityBetweenLists (sp, MustImputeVariablesList,
             NumberOfMustImputeVars, MustMatchVariablesList,
             NumberOfMustMatchVars);
    if (rc != EIE_SUCCEED)
        crc = EIE_FAIL;

    /* Consistency between some options */
    if ((NumberOfMustMatchVars == 0) && (RandomSelection == 0)) {
        IO_PRINT_LINE (SAS_MESSAGE_PREFIX_ERROR MsgNoImputationInvalidOptions);
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

	return crc;
}

/*************************
 Write new observation
**************************/
static PROC_RETURN_CODE WriteResults (
    SP_massimpu* sp,
    T_WRITEINFO * UserData,
    EIT_DATAREC * RecipientData,
    char * RecipientKey,
    char * DonorKey,
    int NumberOfAttempts,
	int DonorLimit,
    char** AllFieldsNameOriginal)
{
    int i;

    for (i = 0; i < UserData->RulesNumberOfFields; i++)
        UserData->OutVarValue[i] = RecipientData->FieldValue[i];

    SUtil_CopyPad (UserData->RecipientKey, RecipientKey, UserData->LengthKeyData);

    if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outdata)) {
        return PRC_FAIL_WRITE_DATA;
    }

    /* write out the status IMAS for fields */
    /* that have been imputed.             */
    SUtil_CopyPad(UserData->RespondentKey, RecipientKey,
        UserData->LengthKeyData);
    /* Write name of variables to impute in status data set */
    for (i = UserData->RulesNumberOfFields - 1; i >= 0; i--) {
        if (RecipientData->StatusFlag[i] == FIELDFTI) {
            SUtil_CopyPad(UserData->OutFieldid, AllFieldsNameOriginal[i], LEGACY_MAXNAME);
            SUtil_CopyPad(UserData->OutStatus, EIM_STATUS_IMAS, EIM_STATUS_SIZE);
            // copy value from `outdata` dataset
            UserData->FieldValue = UserData->OutVarValue[i];
            if (IORC_SUCCESS != DSW_add_record(&sp->dsw_outstatus)) {
                return PRC_FAIL_WRITE_DATA;
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
