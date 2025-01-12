/* contient toutes les fonctions reliees aux specification d'estimateur */

/*#include <assert.h>*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Average.h"
#include "EI_EstimatorSpec.h"
#include "EI_Message.h"
#include "EI_Regression.h"
#include "EI_Residual.h"
#include "EI_Stack.h"

#include "STC_Memory.h"

/**********************************************************************
Add an item to the estimator list.
*********************************************************************/
EIT_RETURNCODE EI_EstimatorAdd (
    EIT_ESTIMATORSPEC_LIST * List,
    char * AlgorithmName,
    char * FieldId,
    char * AuxVariables,
    char * WeightVariable,
    char * VarianceVariable,
    double VarianceExponent,
    char * VariancePeriod,
    char * ExcludeImputed,
    char * ExcludeOutliers,
    char * RandomError,
    int CountCriteria,
    double PercentCriteria)
{
    EIT_ESTIMATORSPEC * E;

    if (List->NumberEntries == List->NumberAllocatedEntries) {
        /* if not enough space, double the size */
        EIT_ESTIMATORSPEC * NewEstimator;

        NewEstimator = STC_ReallocateMemory (
            sizeof *List->Estimator * List->NumberAllocatedEntries,
            sizeof *List->Estimator * List->NumberAllocatedEntries * 2,
            List->Estimator);
        if (NewEstimator == NULL) return EIE_FAIL;

        List->Estimator = NewEstimator;
        List->NumberAllocatedEntries *= 2;
    }

    E = &List->Estimator[List->NumberEntries];

    E->AlgorithmName = STC_StrDup (AlgorithmName);
    if (E->AlgorithmName == NULL) return EIE_FAIL;
    E->FieldId = STC_StrDup (FieldId);
    if (E->FieldId == NULL) return EIE_FAIL;
    E->AuxVariables = STC_StrDup (AuxVariables);
    if (E->AuxVariables == NULL) return EIE_FAIL;
    E->WeightVariable = STC_StrDup (WeightVariable);
    if (E->WeightVariable == NULL) return EIE_FAIL;
    E->VarianceVariable = STC_StrDup (VarianceVariable);
    if (E->VarianceVariable == NULL) return EIE_FAIL;
    E->VarianceExponent = VarianceExponent;
    E->VariancePeriod = STC_StrDup (VariancePeriod);
    if (E->VariancePeriod == NULL) return EIE_FAIL;
    E->ExcludeImputed = STC_StrDup (ExcludeImputed);
    if (E->ExcludeImputed == NULL) return EIE_FAIL;
    E->ExcludeOutliers = STC_StrDup (ExcludeOutliers);
    if (E->ExcludeOutliers == NULL) return EIE_FAIL;
    E->RandomError = STC_StrDup (RandomError);
    if (E->RandomError == NULL) return EIE_FAIL;
    E->CountCriteria = CountCriteria;
    E->PercentCriteria = PercentCriteria;

    E->Active = 1;
    E->FieldIdIndex = -1;
    E->LateralVariable.NumberOfVariables = 0;
    E->LateralVariable.Index = NULL;
    E->LateralVariable.Period = NULL;
    E->Average = NULL;
    E->Regression = NULL;
    E->Algorithm = NULL;
    E->Residual = NULL;
    E->RandomErrorResult = NULL;
    E->AcceptableKeysResult = NULL;
    E->Stack = NULL;

    List->NumberEntries++;

    return EIE_SUCCEED;
}
/**********************************************************************
Allocate and initialise estimator list.
*********************************************************************/
EIT_ESTIMATORSPEC_LIST * EI_EstimatorAllocate (void)
{
    EIT_ESTIMATORSPEC_LIST * List;

    List = STC_AllocateMemory (sizeof *List);
    if (List == NULL) return NULL;

    /* an arbitrary number */
    List->NumberAllocatedEntries = 10;
    List->NumberEntries = 0;
    List->Estimator = STC_AllocateMemory (
        sizeof *List->Estimator * List->NumberAllocatedEntries);
    if (List->Estimator == NULL) return NULL;

    return List;
}
#ifdef NEVER
/**********************************************************************
use LINEAR search to find the index of the ITEM in the list.
*********************************************************************/
int EI_EstimatorFind (
    EIT_ESTIMATORSPEC_LIST * List,
    char * Name)
{
    int i;

    for (i = 0; i < List->NumberEntries; i++) {
        if (strcmp (List->Estimator[i].Name, Name) == 0)
            return i;
    }

    return -1;
}
#endif
/**********************************************************************
Free estimator list.
*********************************************************************/
void EI_EstimatorFree (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        Estimator = &EstimatorList->Estimator[i];

        STC_FreeMemory (Estimator->AlgorithmName);
        STC_FreeMemory (Estimator->FieldId);
        STC_FreeMemory (Estimator->AuxVariables);
        STC_FreeMemory (Estimator->WeightVariable);
        STC_FreeMemory (Estimator->VarianceVariable);
        STC_FreeMemory (Estimator->VariancePeriod);
        STC_FreeMemory (Estimator->ExcludeImputed);
        STC_FreeMemory (Estimator->ExcludeOutliers);
        STC_FreeMemory (Estimator->RandomError);

        STC_FreeMemory (Estimator->LateralVariable.Index);
        STC_FreeMemory (Estimator->LateralVariable.Period);
        if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_EF)
            EI_AverageFree (Estimator->Average);
        else
            EI_RegressionFree (Estimator->Regression);
        Estimator->Algorithm = NULL;
        EI_ResidualFree (Estimator->Residual);
        EI_RandomErrorResultFree (Estimator->RandomErrorResult);
        EI_AcceptableKeysResultFree (Estimator->AcceptableKeysResult);
        EI_StackFree (Estimator->Stack);
    }
    STC_FreeMemory (EstimatorList->Estimator);
    STC_FreeMemory (EstimatorList);
}
/**********************************************************************
Print estimator list.
*********************************************************************/
void EI_EstimatorListPrint (
    EIT_ESTIMATORSPEC_LIST * List)
{
    int i;

    printf ("\nNumberEntries=%d NumberAllocatedEntries=%d\n",
        List->NumberEntries, List->NumberAllocatedEntries);

    for (i = 0; i < List->NumberEntries; i++) {
        printf ("i=%d\n", i);
        EI_EstimatorSpecPrint (&List->Estimator[i]);
    }
}
/**********************************************************************
Print estimator spec.
*********************************************************************/
void EI_EstimatorSpecPrint (
    EIT_ESTIMATORSPEC * E)
{
    int i;

    printf ("Active=%d FId=%s FIdIndex=%d AlgoName=%s AuxVar=%s\n"
        "WeightVar=%s VarVar=%s VarExp=%f VarPer=%s\n"
        "ExclImp=%s ExclOut=%s RandomError=%s "
        "CC=%d PC=%f\n",
        E->Active,
        E->FieldId,
        E->FieldIdIndex,
        E->AlgorithmName,
        E->AuxVariables,
        E->WeightVariable,
        E->VarianceVariable,
        E->VarianceExponent,
        E->VariancePeriod,
        E->ExcludeImputed,
        E->ExcludeOutliers,
        E->RandomError,
        E->CountCriteria,
        E->PercentCriteria);

    printf ("NumberOfLateralVariables=%d\n",
        E->LateralVariable.NumberOfVariables);
    for (i = 0; i < E->LateralVariable.NumberOfVariables; i++)
        printf ("Lateral variable Index=%d Period=%d\n",
            E->LateralVariable.Index[i],
            E->LateralVariable.Period[i]);

    if (E->Average != NULL)
        EI_AveragePrint (E->Average);
    if (E->Regression != NULL)
        EI_RegressionPrint (E->Regression);
    if (E->Residual != NULL)
        EI_ResidualPrint (E->Residual);
    if (E->Stack != NULL)
        EI_StackPrint (E->Stack);
}
