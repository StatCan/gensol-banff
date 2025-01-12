/* contient toutes les fonctions reliees aux types AcceptableKeysResult */

/*********************************************************************
These functions manage information about acceptable keys for one
estimator with at least a parameter
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_AcceptableKeysResult.h"

#include "STC_Memory.h"

static EIT_ACCEPTABLEKEYSRESULT_LIST * Reallocate (
    EIT_ACCEPTABLEKEYSRESULT_LIST *);

/***********************************************************************
Add an item to the acceptable keys result list (for one estimator).
return the list.
NULL if the list is NULL, or if it fails.
***********************************************************************/
EIT_ACCEPTABLEKEYSRESULT_LIST * EI_AcceptableKeysResultAdd (
    EIT_ACCEPTABLEKEYSRESULT_LIST * List,
    char * Key)
{
    if (List == NULL) return NULL;

    /* need more space? */
    if (List->NumberEntries == List->NumberAllocatedEntries)
        if (Reallocate (List) == NULL) return NULL;

    List->AcceptableKeys[List->NumberEntries] = Key;
    List->NumberEntries++;

    return List;
}
/***********************************************************************
Allocate and initialise acceptable keys result list (for one estimator).
***********************************************************************/
EIT_ACCEPTABLEKEYSRESULT_LIST * EI_AcceptableKeysResultAllocate (void)
{
    EIT_ACCEPTABLEKEYSRESULT_LIST * List;

    List = STC_AllocateMemory (sizeof *List);
    if (List == NULL) return NULL;

    /* an arbitrary number */
    List->NumberAllocatedEntries = 64;
    List->NumberEntries = 0;
    List->AcceptableKeys = STC_AllocateMemory (
        sizeof *List->AcceptableKeys * List->NumberAllocatedEntries);
    if (List->AcceptableKeys == NULL) return NULL;

    return List;
}
/*********************************************************************
Empty an acceptable keys result list (for one estimator).
*********************************************************************/
void EI_AcceptableKeysResultEmpty (
    EIT_ACCEPTABLEKEYSRESULT_LIST * List)
{
    if (List == NULL) return;

    List->NumberEntries = 0;
}
/*********************************************************************
Free acceptable keys result list (for one estimator).
*********************************************************************/
void EI_AcceptableKeysResultFree (
    EIT_ACCEPTABLEKEYSRESULT_LIST * List)
{
    if (List == NULL) return;

    STC_FreeMemory (List->AcceptableKeys);
    STC_FreeMemory (List);
}
/**********************************************************************
Print acceptable keys result list (for one estimator).
*********************************************************************/
void EI_AcceptableKeysResultPrint (
    EIT_ACCEPTABLEKEYSRESULT_LIST * List)
{
    int i;

    if (List == NULL) return;

    printf ("NumberEntries=%d NumberAllocatedEntries=%d\n",
        List->NumberEntries, List->NumberAllocatedEntries);

    for (i = 0; i < List->NumberEntries; i++)
        printf ("NoEntries: %d   Acceptable Key: %s\n", i,
             List->AcceptableKeys[i]);

}

/**********************************************************************
re/allocate as needed
*********************************************************************/
static EIT_ACCEPTABLEKEYSRESULT_LIST * Reallocate (
    EIT_ACCEPTABLEKEYSRESULT_LIST * List)
{
    char ** TempAcceptableKeys;

    TempAcceptableKeys = STC_ReallocateMemory (
        sizeof *TempAcceptableKeys * List->NumberAllocatedEntries,
        sizeof *TempAcceptableKeys * List->NumberAllocatedEntries * 2,
        List->AcceptableKeys);
    if (TempAcceptableKeys == NULL) return NULL;

    List->AcceptableKeys = TempAcceptableKeys;
    List->NumberAllocatedEntries *= 2;

    return List;
}
