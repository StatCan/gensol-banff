/*--------------------------------------------------------------------*/
/* Include Files: (Banff library)                                     */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Message.h"
#include "EI_Prorating.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"
#include "MessageBanffAPI.h"


/*--------------------------------------------------------------------*/
/* Function Prototype                                                 */
/*--------------------------------------------------------------------*/
static void GetUpdatedTotal (int EditNumber, EIT_GROUP_DATA *GroupData);

static EIT_RETURNCODE ValidatePrecisionParametersBasic (int NbDecimal,
    double LowerBound, double UpperBound);

static EIT_RETURNCODE ValidatePrecisionParametersScaling (int NbDecimal,
    double LowerBound, double UpperBound);

void EI_ProratingDataGroupPrint (
    EIT_GROUP_DATA * DataGroup)
{
#define CASEFLAG(X) case X: printf ("(PF=" #X ") "); break;
    int i;
    int j;

    for (i = 0; i < DataGroup->NumberOfEdits; i++) {
        printf ("%s=%f\n", DataGroup->Edit[i].TotalName,
            DataGroup->Edit[i].TotalValue);
        for (j = 0; j < DataGroup->Edit[i].NumberOfFields; j++) {
            printf ("    %s=%f ",
                DataGroup->Edit[i].FieldName[j],
                DataGroup->Edit[i].FieldValue[j]);

            switch (DataGroup->Edit[i].ProratingFlag[j]) {
            CASEFLAG (EIE_RAKING_FLAG_NO_PRORATABLE)
            CASEFLAG (EIE_RAKING_FLAG_PRORATABLE)
            CASEFLAG (EIE_RAKING_FLAG_PRORATED)
            CASEFLAG (EIE_RAKING_FLAG_NOTHING_TO_PRORATE)
            default:
                printf ("UNKNOWN");
                exit (-1);
            }
            printf ("(W=%f)\n",
                DataGroup->Edit[i].Weight[j]);
        }
    }
}
/*--------------------------------------------------------------------*/
/* EI_ProratingInitDataGroup function                                 */
/*                                                                    */
/* This function allocates memory and initializes PARTLY the group    */
/* edits data structure on a respondant with information FROM the     */
/* prorating edits structure already filled by the parser.            */
/*                                                                    */
/* For EIT_GROUP_DATA                                                 */
/* 1) Initializes:                                                    */
/*    GroupData -> NumberOfEdits                                      */
/* 2) Allocates memory (NumberOfEdits X EIT_EDIT_DATA):               */
/*    GroupData -> Edit                                               */
/*    (Implicitly, we allocates memory for 2 members of an edit:      */
/*     i) NumberOfFields and ii) TotalValue)                          */
/*                                                                    */
/* For each EIT_EDIT_DATA                                             */
/* 1) Initializes:                                                    */
/*    Edit[i]-> NumberOfFields                                        */
/* 2) Allocates memory and initializes:                               */
/*    Edit[i][j]-> FieldName                                          */
/*    Edit[i][j]-> Weight                                             */
/*    Edit[i][j]-> ProratingFlag                                      */
/*    Edit[i]-> TotalName                                             */
/* 3) Allocates memory:                                               */
/*    Edit[i][j]-> FieldValue                                         */
/*                                                                    */
/* At the end, 2 members of each edit are not initialized but memory  */
/* spaces have been allocated for them:                               */
/*         Edit[i][j]-> FieldValue                                    */
/*         Edit[i]-> TotalValue                                       */
/* These members will be initialized when we will read the record     */
/* data.                                                              */
/*                                                                    */
/* After processing the data for one record, the programmer must      */
/* free memory allocated for that structure with the function         */
/* EI_ProratingFreeDataGroup.                                         */
/*                                                                    */
/* Parameters:                                                        */
/*    1. *ParserEdits - Pointer to a structure EIT_PRORATING_GROUP    */
/*                      already filled by the edits parser.           */
/*    2. *GroupData - Pointer to a structure EIT_GROUP_DATA           */
/*                    (One Group of Edits Data)                       */
/*                                                                    */
/* Returns: EIT_PRORATING_INITDATAGROUP_RETURNCODE                    */
/*--------------------------------------------------------------------*/
EIT_PRORATING_INITDATAGROUP_RETURNCODE EI_ProratingInitDataGroup (
    EIT_PRORATING_GROUP *ParserEdits,
    EIT_GROUP_DATA *GroupData)
{
    int i;  /* Counter for number of edits                 */
    int j;  /* Counter for number of variables in one edit */

    /* For EIT_GROUP_DATA */
    GroupData->NumberOfEdits = ParserEdits->NumberOfEdits;
    if (NULL==(GroupData->Edit = STC_AllocateMemory (
                ParserEdits->NumberOfEdits * sizeof *GroupData->Edit)))
       return EIE_PRORATING_INITDATAGROUP_FAIL;

    /* For each EIT_EDIT_DATA */
    for (i = 0; i < ParserEdits->NumberOfEdits; i++) {
        GroupData->Edit[i].NumberOfFields = ParserEdits->Edit[i].NumberOfFields;
        if (NULL==(GroupData->Edit[i].FieldName  = STC_AllocateMemory (
                   ParserEdits->Edit[i].NumberOfFields *
                       sizeof *GroupData->Edit[i].FieldName)))
            return EIE_PRORATING_INITDATAGROUP_FAIL;
        if (NULL==(GroupData->Edit[i].FieldValue = STC_AllocateMemory (
                   ParserEdits->Edit[i].NumberOfFields *
                       sizeof *GroupData->Edit[i].FieldValue)))
            return EIE_PRORATING_INITDATAGROUP_FAIL;
        if (NULL==(GroupData->Edit[i].Weight = STC_AllocateMemory (
                   ParserEdits->Edit[i].NumberOfFields *
                       sizeof *GroupData->Edit[i].Weight)))
            return EIE_PRORATING_INITDATAGROUP_FAIL;
        if (NULL==(GroupData->Edit[i].ProratingFlag = STC_AllocateMemory (
                   ParserEdits->Edit[i].NumberOfFields *
                       sizeof *GroupData->Edit[i].ProratingFlag)))
            return EIE_PRORATING_INITDATAGROUP_FAIL;
        if (NULL==(GroupData->Edit[i].TotalName = STC_AllocateMemory (
                  (strlen (ParserEdits->Edit[i].TotalName)+1) *
                       sizeof *GroupData->Edit[i].TotalName)))
            return EIE_PRORATING_INITDATAGROUP_FAIL;

        for (j = 0; j < ParserEdits->Edit[i].NumberOfFields; j++) {
            if (NULL==(GroupData->Edit[i].FieldName[j] =
                STC_AllocateMemory (
                (strlen (ParserEdits->Edit[i].FieldName[j])+1) *
                       sizeof *GroupData->Edit[i].FieldName[j])))
                return EIE_PRORATING_INITDATAGROUP_FAIL;
            strncpy (GroupData->Edit[i].FieldName[j],
                    ParserEdits->Edit[i].FieldName[j],
                    strlen (ParserEdits->Edit[i].FieldName[j])+1);
            GroupData->Edit[i].Weight[j] = ParserEdits->Edit[i].Weight[j];
            GroupData->Edit[i].ProratingFlag[j] =
                                          ParserEdits->Edit[i].ProratingFlag[j];
        }
        strncpy (GroupData->Edit[i].TotalName,ParserEdits->Edit[i].TotalName,
                strlen (ParserEdits->Edit[i].TotalName)+1);
    }

    return EIE_PRORATING_INITDATAGROUP_SUCCEED;
}



/*--------------------------------------------------------------------*/
/*EI_ProratingResetProratingFlag function                             */
/*                                                                    */
/* This function  resets the variable "ProratingFlag" in the group    */
/* edits data structure on a respondant with original information     */
/* from the prorating edits structure already filled by the parser.   */
/*                                                                    */
/* Parameters:                                                        */
/*    1. *ParserEdits - Pointer to a structure EIT_PRORATING_GROUP    */
/*                      already filled by the edits parser.           */
/*    2. *GroupData   - Pointer to a structure EIT_GROUP_DATA         */
/*                      (One Group of Edits Data)                     */
/*                                                                    */
/* Returns: VOID                                                      */
/*--------------------------------------------------------------------*/
void EI_ProratingResetProratingFlag (
    EIT_PRORATING_GROUP *ParserEdits,
    EIT_GROUP_DATA *GroupData)
{
    int i;
    int j;
    for (i = 0; i < ParserEdits->NumberOfEdits; i++) {
         for (j = 0; j < ParserEdits->Edit[i].NumberOfFields; j++){
             GroupData->Edit[i].ProratingFlag[j] =
                 ParserEdits->Edit[i].ProratingFlag[j];
        }
    }
}


/*--------------------------------------------------------------------*/
/* EI_ProratingFreeDataGroup function                                 */
/*                                                                    */
/* This function  frees the memory allocated by the function          */
/* EI_ProratingInitDataGroup().                                       */
/*                                                                    */
/* Parameters:                                                        */
/*    1. *GroupData - Pointer to a structure DataGroup                */
/*                    (One Group of Edits Data)                       */
/*                                                                    */
/* Returns: Void                                                      */
/*--------------------------------------------------------------------*/
void EI_ProratingFreeDataGroup (
    EIT_GROUP_DATA *GroupData)
{
    int i; /* Counter for NumberOfEdits              */
    int j; /* Counter for NumberOfFields in one Edit */

    for (i = 0; i < GroupData->NumberOfEdits; i++) {
        for (j = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
             STC_FreeMemory (GroupData->Edit[i].FieldName[j]);
        }
        STC_FreeMemory (GroupData->Edit[i].TotalName);
        STC_FreeMemory (GroupData->Edit[i].ProratingFlag);
        STC_FreeMemory (GroupData->Edit[i].Weight);
        STC_FreeMemory (GroupData->Edit[i].FieldValue);
        STC_FreeMemory (GroupData->Edit[i].FieldName);
    }

    STC_FreeMemory (GroupData->Edit);
}


/*--------------------------------------------------------------------*/
/* EI_ProratingBasic function                                         */
/*                                                                    */
/* This function will process all edits for a respondant.             */
/* For edit "i":                                                      */
/* - verifies if the total variable has ever been adjusted among the  */
/*   variables for all edits processed (edits "0" until "i-1") and    */
/*   updates the total value for edit "i".                            */
/* - compares the sum of the variables for edit "i" with the updated  */
/*   total value.                                                     */
/* - if a difference exists, calls EI_RakingBasic function to adjust  */
/*   the data.                                                        */
/*   (3 parameters of precision are provided)                         */
/*                                                                    */
/* Parameters:                                                        */
/*    1. GroupData - Pointer to structure EIT_GROUP_DATA              */
/*    2. DecimalPlaces - Precision: Number of decimals to keep.       */
/*    3. LowerBound    - Precision: Lower bound.                      */
/*    4. UpperBound    - Precision: Upper bound.                      */
/*    5. EditNumberFail- Pointer to the first edit number with error  */
/*    6. IndexFieldOutOfBound - Index of field:1st ratio out of bounds*/
/*    7. ValueRatioOutOfBound - 1st erroneous ratio value             */
/*                                                                    */
/* Returns: EIT_RAKING_RETURNCODE                                     */
/*--------------------------------------------------------------------*/
EIT_RAKING_RETURNCODE EI_ProratingBasic (
    EIT_GROUP_DATA *GroupData,
    int DecimalPlaces,
    double LowerBound,
    double UpperBound,
    int   *EditNumberFail,
    int   *IndexFieldOutOfBound,
    double *ValueRatioOutOfBound)
{
    int i;
    int j;
    int ProratingDone;
    EIT_RAKING_RETURNCODE rc;
    double sum;

    if (EIE_FAIL == ValidatePrecisionParametersBasic (DecimalPlaces,
                    LowerBound, UpperBound))
        return EIE_RAKING_FAIL;

    ProratingDone = 0;
    for (i = 0; i < GroupData->NumberOfEdits; i++) {
        for (j = 0, sum = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
            sum += GroupData->Edit[i].FieldValue[j];
        }
        GetUpdatedTotal (i,GroupData);
        if (!EIM_DBL_EQ (sum,GroupData->Edit[i].TotalValue)) {
            ProratingDone = 1;
            rc = EI_RakingBasic (&GroupData->Edit[i], DecimalPlaces, LowerBound,
                UpperBound, IndexFieldOutOfBound, ValueRatioOutOfBound);
            if (rc == EIE_RAKING_FAIL)
                return (rc);
            if (rc != EIE_RAKING_SUCCEED) {
                *EditNumberFail = i;
                return (rc);
            }
        }
        else {
            for (j = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
                if (GroupData->Edit[i].ProratingFlag[j] ==
                        EIE_RAKING_FLAG_PRORATABLE)
                    GroupData->Edit[i].ProratingFlag[j] =
                        EIE_RAKING_FLAG_NOTHING_TO_PRORATE;
            }
        }
    }

    if (!ProratingDone)
        rc = EIE_RAKING_SUCCEED_NOTHING_TO_PRORATE;

    return (rc);
}



/*--------------------------------------------------------------------*/
/* EI_ProratingScaling function (Implements the Scaling Approach of   */
/*  prorating, so that sign of variables cannot change)               */
/*                                                                    */
/* This function will process all edits for a respondant.             */
/* For edit "i":                                                      */
/* - verifies if the total variable has ever been adjusted among the  */
/*   variables for all edits processed (edits "0" until "i-1") and    */
/*   updates the total value for edit "i".                            */
/* - compares the sum of the variables for edit "i" with the updated  */
/*   total value.                                                     */
/* - if a difference exists, calls EI_RakingScaling function to       */
/*   adjust the data.                                                 */
/*   (3 parameters of precision are provided)                         */
/*                                                                    */
/* Parameters:                                                        */
/*    1. GroupData - Pointer to structure EIT_GROUP_DATA              */
/*    2. DecimalPlaces - Precision: Number of decimals to keep.       */
/*    3. LowerBound    - Precision: Lower bound.                      */
/*    4. UpperBound    - Precision: Upper bound.                      */
/*    5. EditNumberFail- Pointer to the first edit number with error  */
/*    6. IndexFieldOutOfBound - Index of field:1st ratio out of bounds*/
/*    7. ValueRatioOutOfBound - 1st erroneous ratio value             */
/*                                                                    */
/* Returns: EIT_RAKING_RETURNCODE                                     */
/*--------------------------------------------------------------------*/
EIT_RAKING_RETURNCODE EI_ProratingScaling (
    EIT_GROUP_DATA *GroupData,
    int DecimalPlaces,
    double LowerBound,
    double UpperBound,
    int   *EditNumberFail,
    int   *IndexFieldOutOfBound,
    double *ValueRatioOutOfBound)
{
    int i;
    int j;
    int ProratingDone;
    EIT_RAKING_RETURNCODE rc;
    double sum;

    if (EIE_FAIL == ValidatePrecisionParametersScaling (DecimalPlaces,
                    LowerBound, UpperBound))
        return EIE_RAKING_FAIL;

    ProratingDone = 0;
    for (i = 0; i < GroupData->NumberOfEdits; i++) {
        for (j = 0, sum = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
            sum += GroupData->Edit[i].FieldValue[j];
        }
        GetUpdatedTotal (i,GroupData);
        if (!EIM_DBL_EQ (sum,GroupData->Edit[i].TotalValue)) {
            ProratingDone = 1;
            rc = EI_RakingScaling (&GroupData->Edit[i], DecimalPlaces,
                LowerBound,UpperBound, IndexFieldOutOfBound,
                ValueRatioOutOfBound);
            if (rc == EIE_RAKING_FAIL)
                return (rc);
            if (rc != EIE_RAKING_SUCCEED) {
                *EditNumberFail = i;
                return (rc);
            }
        }
        else {
            for (j = 0; j < GroupData->Edit[i].NumberOfFields; j++) {
                if (GroupData->Edit[i].ProratingFlag[j] ==
                                                     EIE_RAKING_FLAG_PRORATABLE)
                    GroupData->Edit[i].ProratingFlag[j] =
                                             EIE_RAKING_FLAG_NOTHING_TO_PRORATE;
           }
        }
    }

    if (!ProratingDone)
        rc = EIE_RAKING_SUCCEED_NOTHING_TO_PRORATE;

    return (rc);
}



/***********************************************************/
/* Function: EI_ProratingSetFlag                           */
/*                                                         */
/* Purpose : Set the prorating flag (field is imputable    */
/*           or no) based on the modifiers in the edits    */
/*           and the field status from input status table. */
/* a) If flag value is EIE_PRORATING_MODIFIER_NOTSET,      */
/*    substitutes the value of the global parameter        */
/*    "DefaultModifier".                                    */
/* b) Sets "ProratingFlag" with information from           */
/*    the structure FieldStat (fields would be             */
/*    modifying or no).                                    */
/***********************************************************/
void EI_ProratingSetFlag (
    EIT_GROUP_DATA * GroupData,
    int FlagReadStatus,
    tSList *FieldStat,
    EIT_PRORATING_MODIFIER DefaultModifier)
{
    EIT_EDIT_DATA * Edit;
    int i;
    int j;

    for (j = 0; j < GroupData->NumberOfEdits; j++) {
        Edit = &GroupData->Edit[j];
        for (i = 0; i < Edit->NumberOfFields; i++) {
            if (Edit->ProratingFlag[i] == EIE_PRORATING_MODIFIER_NOTSET)
                Edit->ProratingFlag[i] = DefaultModifier;
    
            switch (Edit->ProratingFlag[i]) {
            case EIE_PRORATING_MODIFIER_NEVER:
                Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;
                break;
    
            case EIE_PRORATING_MODIFIER_ALWAYS:
                Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATABLE;
                break;
    
            case EIE_PRORATING_MODIFIER_ORIGINAL:
                if (FlagReadStatus) {
                    /* if variable not found*/
                    if (SList_StringSearch (FieldStat, Edit->FieldName[i]) ==
                        SLIST_NOTFOUND)
                        /* The field has its original value */
                        Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATABLE;
                    else
                        Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;
                }
                else
                    /* The field has its original value */
                    Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATABLE;
                break;
    
            case EIE_PRORATING_MODIFIER_IMPUTED:
                if (FlagReadStatus) {
                    if (SList_StringSearch (FieldStat, Edit->FieldName[i]) ==
                        SLIST_NOTFOUND)
                        /* The field has its original value */
                        Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;
                    else
                        Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATABLE;
                }
                else
                /* The field has its original value */
                Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;
                break;
            } /* switch */
        } /* for each edit field */
    } /* for each edit */
}


/*--------------------------------------------------------------------*/
/* EI_RakingBasic function                                             */
/*                                                                    */
/* This function will balance a summation by distributing the         */
/* differential with the expected total across the summation          */
/* components based on a specific weight associated with each of      */
/* those components.                                                  */
/*                                                                    */
/* Parameters:                                                        */
/*    1. Edit - Struct containing the values, weights and flags       */
/*              for one edit.                                         */
/*    2. DecimalPlaces - Precision: Number of decimals to keep.       */
/*    3. LowerBound    - Precision: Lower bound.                      */
/*    4. UpperBound    - Precision: Upper bound.                      */
/*    5. IndexFieldOutOfBound - Index of field:1st ratio out of bounds*/
/*    6. ValueRatioOutOfBound - 1st erroneous ratio value             */
/*                                                                    */
/* Returns: EIT_RAKING_RETURNCODE                                     */
/*--------------------------------------------------------------------*/
EIT_RAKING_RETURNCODE EI_RakingBasic (
    EIT_EDIT_DATA *Edit,
    int DecimalPlaces,
    double LowerBound,
    double UpperBound,
    int *IndexFieldOutOfBound,
    double *ValueRatioOutOfBound)
{
    double *ArrayValueOut;         /*  Start:   Array of values before raking */
                                 /* Process: Array of values after raking */
    int FirstTime;
    int i;
    double k;                     /* differential / SumWeightProratableValues */
    int NumberProratable             = 0; /* Number of proratable values(nimp)*/
    int PreviousIndex;
    double PreviousValue;
    double Ratio;
    double SumDiff;
    double SumNoProratableValues     = 0;     /* Sum of not proratable values */
    double SumProratableValues       = 0;    /* Sum of proratable values(sum) */
    double SumWeightProratableValues = 0; /* Sum of (value / its weight)-w_sum*/


    if (EIE_FAIL == ValidatePrecisionParametersBasic (DecimalPlaces,
                    LowerBound, UpperBound))
        return EIE_RAKING_FAIL;

    if (NULL==(ArrayValueOut=STC_AllocateMemory (
            Edit->NumberOfFields * sizeof *ArrayValueOut)))
        return (EIE_RAKING_FAIL);

    for (i = 0; i < Edit->NumberOfFields; i++) {
        /* Start: Make a copy of original value */
        ArrayValueOut[i] = Edit->FieldValue[i];

        /*
        If value is 0, mark it as not proratable.
        If it is not marked as not proratable, the zero could be bumped later
        when the values are rounded.
        */
        if (EIM_DBL_EQ (Edit->FieldValue[i], 0))
            Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;

        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            NumberProratable++;
            SumProratableValues += Edit->FieldValue[i];
            SumWeightProratableValues += Edit->FieldValue[i] / Edit->Weight[i];
        } /* if proratable value */
        else
            SumNoProratableValues += Edit->FieldValue[i];
    } /* for each of the input values */

    if (NumberProratable == 0) {
        /* Quit */
        /* no columns are left to prorate - they all have been eliminated */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_NOTHING_TO_PRORATE);
    } /* if no proratable values */

    if (SumWeightProratableValues == 0) {
        /* Quit */
        /* the sum of all proratable column is 0 */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_SUM_NULL);
    } /* if SumWeightProratableValues == 0 */

    /* check if result of total-SumNoProratableValues as more decimals */
    /* than required. if it has, quit with error */
    if (!UTIL_Decimal(Edit->TotalValue - SumNoProratableValues, DecimalPlaces)){
        /* Quit */
        /* too many decimals */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_DECIMAL_ERROR);
    }

    /*
    calculate k as described in BSMD document named
    "A Prorating Module for GEIS - Methodological Descriptions"
    */
    k = (Edit->TotalValue - (SumNoProratableValues + SumProratableValues))
          / SumWeightProratableValues;

    /*
    Prorate data and round result at one more decimal than requested
    for the prorating (that's needed for the rounding algorythm)
    */
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i]== EIE_RAKING_FLAG_PRORATABLE) {
             ArrayValueOut[i] = UTIL_Round (
                Edit->FieldValue[i] + Edit->FieldValue[i] * k / Edit->Weight[i],
                DecimalPlaces+1);
        } /* if prratable */
    } /* for each components */

    /*
    Do rounding according to algorythm provided to us by BSMD
    See paper "Algorithms for adjusting survey data that fail balance edits"
    By Richard Sigman and Dennis Wagner
    of US Bureau of Census
    */
    FirstTime = 1;
    SumDiff = 0.0;
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            if (!FirstTime)
                SumDiff += (PreviousValue - ArrayValueOut[PreviousIndex]);
            PreviousValue = ArrayValueOut[i];
            PreviousIndex = i;
            ArrayValueOut[i] = UTIL_Round(PreviousValue+SumDiff, DecimalPlaces);
            FirstTime = 0;
        }
    }

    /* Check if data has changed more than what the user specified */
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            /*
            Original way to calculate the ratio for out of bounds variables
            Kept for documentation purpose.
            Ratio = 1 + k / Edit->Weight[i];
            */
            Ratio = ArrayValueOut[i]/Edit->FieldValue[i];
            if (EIM_DBL_LT (Ratio, LowerBound) ||
                EIM_DBL_GT (Ratio, UpperBound)) {
                *IndexFieldOutOfBound = i;
                *ValueRatioOutOfBound = Ratio;
                STC_FreeMemory (ArrayValueOut);
                return (EIE_RAKING_FAIL_OUT_OF_BOUNDS);
            } /* if ratio is out of bounds */
        }
    } /* for each of the output components */

    /* Here, all raked data have an acceptable value because they passed the */
    /* check above. (no out of bound was detected.)                          */
    /* Now:                                                                  */
    /* For all data that was proratable:                                     */
    /* - Set flag to EIE_RAKING_FLAG_PRORATED if value changed               */
    /* - Set flag to EIE_RAKING_FLAG_NOTHING_TO_PRORATE if value not changed */
    /* - Put the new value (changed or not) in the input structure           */

    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE){
           if (EIM_DBL_EQ (Edit->FieldValue[i]*10,ArrayValueOut[i]*10))
                  Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NOTHING_TO_PRORATE;
           else
               Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATED;
    }
        Edit->FieldValue[i]=ArrayValueOut[i];
    }
    STC_FreeMemory (ArrayValueOut);
    return (EIE_RAKING_SUCCEED);
} /* raking */


/*--------------------------------------------------------------------*/
/* EI_RakingScaling function (Implements the Scaling Approach of      */
/*  prorating, so that sign of variables cannot change)               */
/* This function will balance a summation by distributing the         */
/* differential with the expected total across the summation          */
/* components based on a specific weight associated with each of      */
/* those components.                                                  */
/*                                                                    */
/* Parameters:                                                        */
/*    1. Edit - Struct containing the values, weights and flags       */
/*              for one edit.                                         */
/*    2. DecimalPlaces - Precision: Number of decimals to keep.       */
/*    3. LowerBound    - Precision: Lower bound.                      */
/*    4. UpperBound    - Precision: Upper bound.                      */
/*    5. IndexFieldOutOfBound - Index of field:1st ratio out of bounds*/
/*    6. ValueRatioOutOfBound - 1st erroneous ratio value             */
/*                                                                    */
/* Returns: EIT_RAKING_RETURNCODE                                     */
/*--------------------------------------------------------------------*/
EIT_RAKING_RETURNCODE EI_RakingScaling (
    EIT_EDIT_DATA *Edit,
    int DecimalPlaces,
    double LowerBound,
    double UpperBound,
    int *IndexFieldOutOfBound,
    double *ValueRatioOutOfBound)
{
    double *ArrayValueOut;         /*  Start:   Array of values before raking */
                                 /* Process: Array of values after raking */
    int FirstTime;
    int i;
    double k;                     /* differential / SumWeightProratableValues */
    int NumberProratable             = 0; /* Number of proratable values(nimp)*/
    int PreviousIndex;
    double PreviousValue;
    double Ratio;
    double SumDiff;
    double SumNoProratableValues  = 0;         /* Sum of not proratable values*/
    double SumProratableValues       = 0;    /* Sum of proratable values(sum) */
    double SumWeightProratableValues = 0; /* Sum of (value / its weight)-w_sum*/


    if (EIE_FAIL == ValidatePrecisionParametersScaling (DecimalPlaces,
                    LowerBound, UpperBound))
        return EIE_RAKING_FAIL;

    if (NULL==(ArrayValueOut=STC_AllocateMemory (
            Edit->NumberOfFields * sizeof *ArrayValueOut)))
        return (EIE_RAKING_FAIL);

    for (i = 0; i < Edit->NumberOfFields; i++) {
        /* Start: Make a copy of original value */
        ArrayValueOut[i] = Edit->FieldValue[i];

        /*
        If value is 0, mark it as not proratable.
        If it is not marked as not proratable, the zero could be bumped later
        when the values are rounded.
        */
        if (EIM_DBL_EQ (Edit->FieldValue[i], 0))
            Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NO_PRORATABLE;

        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            NumberProratable++;
            SumProratableValues += Edit->FieldValue[i];
            /*The 'Scaling Approach' of prorating uses absolute
             values of Xi/Wi ratios: */
            SumWeightProratableValues +=
                fabs (Edit->FieldValue[i]/Edit->Weight[i]);
        } /* if proratable value */
        else
            SumNoProratableValues += Edit->FieldValue[i];
    } /* for each of the input values */

    if (NumberProratable == 0) {
        /* Quit */
        /* no columns are left to prorate - they all have been eliminated */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_NOTHING_TO_PRORATE);
    } /* if no proratable values */

    if (SumWeightProratableValues == 0) {
        /* Quit */
        /* the sum of all proratable column is 0 */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_SUM_NULL);
    } /* if SumWeightProratableValues == 0 */

    /* check if result of total-SumNoProratableValues as more decimals */
    /* than required. if it has, quit with error */
    if (!UTIL_Decimal(Edit->TotalValue - SumNoProratableValues, DecimalPlaces)){
        /* Quit */
        /* too many decimals */
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_DECIMAL_ERROR);
    }


    /*
      1. calculate k as described in BSMD document named
         "A New Prorating Procedure for Banff Using a Scaling Approch
        - Methodological Descriptions", Revised April 17, 2003.
      2. Make a test to make sure that -1 <= k <= 1, as required by
         the scaling method
    */

    k = ((SumNoProratableValues + SumProratableValues) - Edit->TotalValue)
          / SumWeightProratableValues;

    if (k < -1.0) {
        /* Quit, prorating should not be executed for the record if k<-1*/
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_K_LT_MINUS_ONE);
    }
    if (k > 1.0) {
        /* Quit, prorating should not be executed for the record if k> 1*/
        STC_FreeMemory (ArrayValueOut);
        return (EIE_RAKING_FAIL_K_GT_PLUS_ONE);
    }


    /*
    Prorate data and round result at one more decimal than requested
    for the prorating (that's needed for the rounding algorythm)
    */
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i]== EIE_RAKING_FLAG_PRORATABLE) {
         /*In the Methodology revised April 17, 2003, prorating formula
               now depends on the sign of the value of the variable:
             */
            if (Edit->FieldValue[i] > 0.0) {
                ArrayValueOut[i] = UTIL_Round (
                 (1.0 - k/Edit->Weight[i]) * Edit->FieldValue[i],
                 DecimalPlaces+1);
            }
            if (Edit->FieldValue[i] < 0.0) {
                ArrayValueOut[i] = UTIL_Round (
                 (1.0 + k/Edit->Weight[i]) * Edit->FieldValue[i],
                 DecimalPlaces+1);
            }

        } /* if prratable */
    } /* for each components */

    /*
    Do rounding according to algorythm provided to us by BSMD
    See paper "Algorithms for adjusting survey data that fail balance edits"
    By Richard Sigman and Dennis Wagner
    of US Bureau of Census
    */
    FirstTime = 1;
    SumDiff = 0.0;
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            if (!FirstTime)
                SumDiff += (PreviousValue - ArrayValueOut[PreviousIndex]);
            PreviousValue = ArrayValueOut[i];
            PreviousIndex = i;
            ArrayValueOut[i] = UTIL_Round (PreviousValue + SumDiff,
                DecimalPlaces);
            FirstTime = 0;
        }
    }

    /* Check if data has changed more than what the user specified */
    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE) {
            /*
            Original way to calculate the ratio for out of bounds variables
            Kept for documentation purpose.
            Ratio = 1 + k / Edit->Weight[i];
            */
            Ratio = ArrayValueOut[i]/Edit->FieldValue[i];
            if (EIM_DBL_LT (Ratio, LowerBound) ||
                EIM_DBL_GT (Ratio, UpperBound)) {
                *IndexFieldOutOfBound = i;
                *ValueRatioOutOfBound = Ratio;
                STC_FreeMemory (ArrayValueOut);
                return (EIE_RAKING_FAIL_OUT_OF_BOUNDS);
            } /* if ratio is out of bounds */
        }
    } /* for each of the output components */

    /* Here, all raked data have an acceptable value because they passed the */
    /* check above. (no out of bound was detected.)                          */
    /* Now:                                                                  */
    /* For all data that was proratable:                                     */
    /* - Set flag to EIE_RAKING_FLAG_PRORATED if value changed               */
    /* - Set flag to EIE_RAKING_FLAG_NOTHING_TO_PRORATE if value not changed */
    /* - Put the new value (changed or not) in the input structure           */

    for (i = 0; i < Edit->NumberOfFields; i++) {
        if (Edit->ProratingFlag[i] == EIE_RAKING_FLAG_PRORATABLE){
           if (EIM_DBL_EQ (Edit->FieldValue[i]*10,ArrayValueOut[i]*10))
               Edit->ProratingFlag[i] = EIE_RAKING_FLAG_NOTHING_TO_PRORATE;
           else
               Edit->ProratingFlag[i] = EIE_RAKING_FLAG_PRORATED;
    }
        Edit->FieldValue[i]=ArrayValueOut[i];
    }
    STC_FreeMemory (ArrayValueOut);
    return (EIE_RAKING_SUCCEED);
} /* raking */



/*--------------------------------------------------------------------*/
/* GetUpdatedTotal function                                           */
/*                                                                    */
/* This function updates the value of a total field which was updated */
/* in a previous level.                                               */
/*                                                                    */
/* Parameters:                                                        */
/*    1. EditNumber - number of edit in process.                      */
/*    2. GroupData  - struct containing info on variables.            */
/*                                                                    */
/*--------------------------------------------------------------------*/
static void GetUpdatedTotal (
   int EditNumber,
   EIT_GROUP_DATA *GroupData)
{
    int i;
    int j;

    for (j = 0; j < EditNumber; j++) {
        for (i = 0; i < GroupData->Edit[j].NumberOfFields; i++) {
            if (!strcmp (GroupData->Edit[j].FieldName[i],
                         GroupData->Edit[EditNumber].TotalName)) {
                if (GroupData->Edit[j].ProratingFlag[i] ==
                        EIE_RAKING_FLAG_PRORATED){
                    GroupData->Edit[EditNumber].TotalValue =
                        GroupData->Edit[j].FieldValue[i];
                    return;
                }
            } /* if a lower level component found */
        } /* for each of the edit components */
    } /* for each of already processed edit */
} /* GetUpdatedTotal function */


/*--------------------------------------------------------------------*/
/* ValidatePrecisionParametersBasic function                          */
/*                                                                    */
/* This function validates the value of three precision parameters    */
/* used in the methodology.                                           */
/*                                                                    */
/* Parameters:                                                        */
/*    1. NbDecimal  - number of decimals.                             */
/*    2. LowerBound - lower bound (to verify new value obtained).     */
/*    3. UpperBound - lower bound (to verify new value obtained).     */
/*--------------------------------------------------------------------*/
static EIT_RETURNCODE ValidatePrecisionParametersBasic (
    int    NbDecimal,
    double LowerBound,
    double UpperBound)
{
    EIT_RETURNCODE crc = EIE_SUCCEED;

    if ((NbDecimal < EIM_PRORATING_LOWER_DECIMAL) ||
        (NbDecimal > EIM_PRORATING_UPPER_DECIMAL)) {
        EI_AddMessage ("EI_ProratingBasic", EIE_ERROR,
            M30094 "\n", /* DECIMAL must be an integer 
                            between %d and %d inclusively. */
            EIM_PRORATING_LOWER_DECIMAL,
            EIM_PRORATING_UPPER_DECIMAL);
        crc = EIE_FAIL;
    }
    if (LowerBound >= UpperBound) {
        EI_AddMessage ("EI_ProratingBasic", EIE_ERROR,
            M30095 "\n"); /* UPPERBOUND must be greater than LOWERBOUND. */
        crc = EIE_FAIL;
    }
    return crc;
}


/*--------------------------------------------------------------------*/
/* ValidatePrecisionParametersScaling function                        */
/* (Used in the Scaling Approach of prorating. In this case, both     */
/* lower and upper bounds must be positive, which is not required     */
/* in the Basic method)                                               */
/*                                                                    */
/* This function validates the value of three precision parameters    */
/* used in the methodology.                                           */
/*                                                                    */
/* Parameters:                                                        */
/*    1. NbDecimal  - number of decimals.                             */
/*    2. LowerBound - lower bound (to verify new value obtained).     */
/*    3. UpperBound - lower bound (to verify new value obtained).     */
/*--------------------------------------------------------------------*/
static EIT_RETURNCODE ValidatePrecisionParametersScaling (
    int    NbDecimal,
    double LowerBound,
    double UpperBound)
{
    EIT_RETURNCODE crc = EIE_SUCCEED;

    if ((NbDecimal < EIM_PRORATING_LOWER_DECIMAL) ||
        (NbDecimal > EIM_PRORATING_UPPER_DECIMAL)) {
        EI_AddMessage ("EI_ProratingScaling", EIE_ERROR,
            M30094 "\n", /* DECIMAL must be an integer 
                            between %d and %d inclusively. */
            EIM_PRORATING_LOWER_DECIMAL, EIM_PRORATING_UPPER_DECIMAL);
        crc = EIE_FAIL;
    }
    if (LowerBound >= UpperBound) {
        EI_AddMessage ("EI_ProratingScaling", EIE_ERROR,
            M30095 "\n"); /* UPPERBOUND must be greater than LOWERBOUND. */
        crc = EIE_FAIL;
    }

    /* Now make sure both lower and upper bounds are positive number, as required
       by the New Prorating Procedure for Banff Using a Scalling Approch.
       Revised pril 17, 2003
    */
    if (LowerBound < EIM_PRORATING_LOWER_LOWERBOUND ||
        UpperBound < EIM_PRORATING_LOWER_LOWERBOUND) {
        EI_AddMessage ("EI_ProratingScaling", EIE_ERROR,
            M30096 "\n"); /* LOWERBOUND and UPPERBOUND must be both non-negative
                             in the scaling method.*/
        crc = EIE_FAIL;
    }
    return crc;
}
