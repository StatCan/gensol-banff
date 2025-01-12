#include <stdio.h>

#include "CD.h"
#include "EI_Common.h"
#include "EI_Message.h"
#include "STC_Memory.h"

/* grosseur des chunks lors des allocations dynamique de mémoire */
#define CDM_ALLOCATIONSIZE 100000

static CDT_CURRENTDATA * Reallocate (CDT_CURRENTDATA * CD);

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
CDT_CURRENTDATA * CD_Add (
    CDT_CURRENTDATA * CD,
    char * Id,
    double Current,
	double Weight)
{
    /* need more space? */
    if (CD->NumberEntries == CD->NumberAllocatedEntries) {
        if (Reallocate (CD) == NULL) {
            return NULL;
        }
    }

    CD->Id[CD->NumberEntries] = STC_StrDup (Id);
	if (CD->IsWeighted) {
	    CD->Current[CD->NumberEntries] = Current * Weight;
	    CD->Weight[CD->NumberEntries] = Weight;
	    CD->UnweightedCurrent[CD->NumberEntries] = Current;
	}
	else
	    CD->Current[CD->NumberEntries] = Current;

    CD->NumberEntries++;

    return CD;
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
CDT_CURRENTDATA * CD_Alloc (EIT_BOOLEAN IsWeighted) {
    CDT_CURRENTDATA * CD;

    /* allocate the structure */
    CD = STC_AllocateMemory (sizeof *CD);
    if (CD == NULL) return NULL;

    /* initialize structure */
    CD->NumberAllocatedEntries = 0;
    CD->NumberEntries = 0;
    CD->Id = NULL;
    CD->Current = NULL;
    CD->Weight = NULL;
    CD->UnweightedCurrent = NULL;
	CD->IsWeighted = IsWeighted;

    /* allocate the structure contents */
    return Reallocate (CD);
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void CD_Free (
    CDT_CURRENTDATA * CD)
{
    int i;

    if (CD != NULL) {
        for (i = 0; i < CD->NumberEntries; i++)
            STC_FreeMemory (CD->Id[i]);
        STC_FreeMemory (CD->Id);
        STC_FreeMemory (CD->Current);
        STC_FreeMemory (CD->Weight);
        STC_FreeMemory (CD->UnweightedCurrent);
        CD->NumberEntries = 0;
        STC_FreeMemory (CD);
    }
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void CD_Print (
    CDT_CURRENTDATA * CD)
{
	int i;

	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%12s %12s %12s %12s",
		"Id", "Current", "Weight", "Weighted or Unweighted Current");
	for (i = 0; i < CD->NumberEntries; i++) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%12s %12.6f %12.6f %12.6f",
			CD->Id[i], CD->Current[i], CD->IsWeighted ? CD->Weight[i] : 1.0, CD->IsWeighted ? CD->UnweightedCurrent[i] : CD->Current[i]);
	}
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void CD_PrintInfo (
    CDT_CURRENTDATA * CD)
{
	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberEntries=%d NumberAllocatedEntries=%d IsWeighted=%s", CD->NumberEntries, CD->NumberAllocatedEntries, CD->IsWeighted ? "TRUE" : "FALSE");
}

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void CD_Sort (
    CDT_CURRENTDATA * CD)
{
    int gap;
    int i;
    int j;
    char * TempKey;
    double TempValue;

    for (gap = CD->NumberEntries/2; gap > 0; gap /= 2) {
        for (i = gap; i < CD->NumberEntries; i++) {
            for (j = i-gap; j >= 0; j -= gap) {
		        if (CD->Current[j] <= CD->Current[j+gap])
			        break;

                /* move the key */
                TempKey       = CD->Id[j];
                CD->Id[j]     = CD->Id[j+gap];
                CD->Id[j+gap] = TempKey;

                /* move the value */
                TempValue          = CD->Current[j];
                CD->Current[j]     = CD->Current[j+gap];
                CD->Current[j+gap] = TempValue;

                /* move the weight */
				if (CD->IsWeighted) {
			        TempValue         = CD->Weight[j];
	                CD->Weight[j]     = CD->Weight[j+gap];
		            CD->Weight[j+gap] = TempValue;

	                /* move the unweighted value */
		            TempValue                    = CD->UnweightedCurrent[j];
			        CD->UnweightedCurrent[j]     = CD->UnweightedCurrent[j+gap];
				    CD->UnweightedCurrent[j+gap] = TempValue;
				}
            }
        }
    }
}


/*------------------------------------------------------------------------------
re/allocate as needed to CD->NumberAllocatedEntries size
------------------------------------------------------------------------------*/
static CDT_CURRENTDATA * Reallocate (
    CDT_CURRENTDATA * CD)
{
    void * Ptr;

    CD->NumberAllocatedEntries += CDM_ALLOCATIONSIZE;

    Ptr = STC_ReallocateMemory (
        sizeof *CD->Id * (CD->NumberAllocatedEntries-CDM_ALLOCATIONSIZE),
        sizeof *CD->Id * (CD->NumberAllocatedEntries),
        CD->Id);
    if (Ptr == NULL) return NULL;
    CD->Id = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *CD->Current * (CD->NumberAllocatedEntries-CDM_ALLOCATIONSIZE),
        sizeof *CD->Current * (CD->NumberAllocatedEntries),
        CD->Current);
    if (Ptr == NULL) return NULL;
    CD->Current = Ptr;

	if (CD->IsWeighted) {
		Ptr = STC_ReallocateMemory (
			sizeof *CD->Weight * (CD->NumberAllocatedEntries-CDM_ALLOCATIONSIZE),
			sizeof *CD->Weight * (CD->NumberAllocatedEntries),
			CD->Weight);
		if (Ptr == NULL) return NULL;
		CD->Weight = Ptr;

	    Ptr = STC_ReallocateMemory (
		    sizeof *CD->UnweightedCurrent * (CD->NumberAllocatedEntries-CDM_ALLOCATIONSIZE),
			sizeof *CD->UnweightedCurrent * (CD->NumberAllocatedEntries),
	        CD->UnweightedCurrent);
		if (Ptr == NULL) return NULL;
		CD->UnweightedCurrent = Ptr;
	}

	return CD;
}
