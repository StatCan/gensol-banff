/* contient toutes les fonctions reliees aux types RandomErrorResult */

/*********************************************************************
These functions manage information about random error donors
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_RandomErrorResult.h"

#include "STC_Memory.h"

static EIT_RANDOMERRORRESULT_LIST * Reallocate (EIT_RANDOMERRORRESULT_LIST *);

/*********************************************************************
Add an item to the random error result list.
return the list.
NULL if the list is NULL, or if it fails.
*********************************************************************/
EIT_RANDOMERRORRESULT_LIST * EI_RandomErrorResultAdd (
    EIT_RANDOMERRORRESULT_LIST * List,
    char * Recipient,
    char * Donor,
    char * FieldId,
    double Residual,
    double RandomError,
    double OriginalValue,
    double ImputedValue)
{
    if (List == NULL) return NULL;

    /* need more space? */
    if (List->NumberEntries == List->NumberAllocatedEntries)
        if (Reallocate (List) == NULL) return NULL;

    List->RandomErrorResult[List->NumberEntries].Recipient = Recipient;
    List->RandomErrorResult[List->NumberEntries].Donor = Donor;
    List->RandomErrorResult[List->NumberEntries].FieldId = FieldId;
    List->RandomErrorResult[List->NumberEntries].Residual = Residual;
    List->RandomErrorResult[List->NumberEntries].RandomError = RandomError;
    List->RandomErrorResult[List->NumberEntries].OriginalValue = OriginalValue;
    List->RandomErrorResult[List->NumberEntries].ImputedValue = ImputedValue;

    List->NumberEntries++;

    return List;
}
/*********************************************************************
Allocate and initialise random error result list.
*********************************************************************/
EIT_RANDOMERRORRESULT_LIST * EI_RandomErrorResultAllocate (void)
{
    EIT_RANDOMERRORRESULT_LIST * List;

    List = STC_AllocateMemory (sizeof *List);
    if (List == NULL) return NULL;

    /* an arbitrary number */
    List->NumberAllocatedEntries = 64;
    List->NumberEntries = 0;
    List->RandomErrorResult = STC_AllocateMemory (
        sizeof *List->RandomErrorResult * List->NumberAllocatedEntries);
    if (List->RandomErrorResult == NULL) return NULL;

    return List;
}
/*********************************************************************
Empty a random error result list.
*********************************************************************/
void EI_RandomErrorResultEmpty (
    EIT_RANDOMERRORRESULT_LIST * List)
{
    if (List == NULL) return;

    List->NumberEntries = 0;
}
/*********************************************************************
Free random error result list.
*********************************************************************/
void EI_RandomErrorResultFree (
    EIT_RANDOMERRORRESULT_LIST * List)
{
    if (List == NULL) return;

    STC_FreeMemory (List->RandomErrorResult);
    STC_FreeMemory (List);
}
/**********************************************************************
Print random error result list.
*********************************************************************/
void EI_RandomErrorResultPrint (
    EIT_RANDOMERRORRESULT_LIST * List)
{
    int i;
    EIT_RANDOMERRORRESULT * RER;

    if (List == NULL) return;

    printf ("NumberEntries=%d NumberAllocatedEntries=%d\n",
        List->NumberEntries, List->NumberAllocatedEntries);

    for (i = 0; i < List->NumberEntries; i++) {
        RER = &List->RandomErrorResult[i];
        printf ("D=%s R=%s FId=%s Res=%.3f RE=%.3f Orig=%.3f Imp=%.3f\n",
            RER->Donor,
            RER->Recipient,
            RER->FieldId,
            RER->Residual,
            RER->RandomError,
            RER->OriginalValue,
            RER->ImputedValue);
    }
}


/**********************************************************************
re/allocate as needed
*********************************************************************/
static EIT_RANDOMERRORRESULT_LIST * Reallocate (
    EIT_RANDOMERRORRESULT_LIST * List)
{
    EIT_RANDOMERRORRESULT * RandomErrorResult;

    RandomErrorResult = STC_ReallocateMemory (
        sizeof *List->RandomErrorResult * List->NumberAllocatedEntries,
        sizeof *List->RandomErrorResult * List->NumberAllocatedEntries * 2,
        List->RandomErrorResult);
    if (RandomErrorResult == NULL) return NULL;

    List->RandomErrorResult = RandomErrorResult;
    List->NumberAllocatedEntries *= 2;

    return List;
}
