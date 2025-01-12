/*----------------------------------------------------------------------
Computes the beta values used in regression.
----------------------------------------------------------------------*/
/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#include <stdio.h>

#include "EI_DataTable.h"
#include "EI_EstimatorSpec.h"
#include "EI_Message.h"
#include "EI_Regression.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

/*****************************************************
 * Print the auxiliary variables' info.
 *****************************************************/
void EI_PrintAux (
    EIT_REGRESSION * r)
{
    EIT_AUX * a;

    for (a = r->Aux; a != NULL; a = a->Next) {
        printf ("\n");
        printf ("Var name  : %s\n", a->VariableName);
        printf ("Beta name : %s\n", a->BetaName);
        printf ("Aux index : %d\n", a->AuxIndex);
        printf ("Period    : %d\n", a->Period);
        printf ("Exponent  : %f\n", a->ExponentValue);
    }
}
/****************************************************
 * beta[0] -> intercept (if there is one)
 * beta[i] -> the i'th member of the Aux list.
****************************************************/
void EI_PrintBetas (
    EIT_REGRESSION * r)
{
    int i;

    if (r->Betas == NULL) return;

    printf("Betas:\n");
    for (i = 0; i < r->NumberOfAux + r->Intercept; i++)
        printf("\t%f\n",r->Betas[i]);
}
/*****************************************************
 *****************************************************/
EIT_REGRESSION* EI_RegressionAllocate (void) {
    EIT_REGRESSION * r;

    r = STC_AllocateMemory (sizeof *r);
    if (r == NULL) return NULL;

    r->FieldIdIndex = -1;
    r->WeightIndex = -1;
    r->VarianceIndex = -1;
    r->VariancePeriod = 0;
    r->VarianceExponent = 0;
    r->Intercept = EIE_FALSE;
    r->NumberOfAux = 0;
    r->Count = 0;
    r->Aux = NULL;
    r->Betas = NULL;

    return r;
}
/*****************************************************
 *****************************************************/
void EI_RegressionFree (
    EIT_REGRESSION * r)
{
    EIT_AUX *Aux;
    EIT_AUX *AuxNext;

    if (r == NULL) return;

    for (Aux = r->Aux; Aux != NULL; Aux = AuxNext){
        AuxNext = Aux->Next;
        STC_FreeMemory (Aux->VariableName);
        STC_FreeMemory (Aux->BetaName);
        STC_FreeMemory (Aux);
    }
    STC_FreeMemory (r->Betas);
    STC_FreeMemory (r);
}
/*****************************************************
 *****************************************************/
void EI_RegressionPrint (
    EIT_REGRESSION * r)
{
    if (r == NULL) {
        printf ("\nNo Regression Struct allocated\n");
        return;
    }
    printf ("\nField Id Index           : %d\n", r->FieldIdIndex);
    printf   ("Weight Index             : %d\n", r->WeightIndex);
    printf   ("Variance Index           : %d\n", r->VarianceIndex);
    printf   ("Variance Period          : %d\n", r->VariancePeriod);
    printf   ("Variance Expon           : %e\n", r->VarianceExponent);
    printf   ("Intercept                : %d\n", r->Intercept);
    printf   ("Number of Aux            : %d\n", r->NumberOfAux);
    printf   ("# of Acceptable Records  : %d\n", r->Count);

    EI_PrintAux(r);
    EI_PrintBetas(r);
}
