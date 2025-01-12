#include <stdio.h>

#include "EI_Common.h"
#include "EI_Message.h"
#include "STC_Memory.h"
#include "HT.h"

/* grosseur des chunks lors des allocations dynamique de mémoire */
#define HTM_ALLOCATIONSIZE 100000

static HTT_HISTORICALTREND * Reallocate (HTT_HISTORICALTREND * HT);

/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
HTT_HISTORICALTREND * HT_Add (
    HTT_HISTORICALTREND * HT,
    char * Id,
    double Current,
    double Previous,
	double Weight)
{
    /* need more space? */
    if (HT->NumberEntries == HT->NumberAllocatedEntries) {
        if (Reallocate (HT) == NULL) {
            return NULL;
        }
    }

    HT->Id[HT->NumberEntries] = STC_StrDup (Id);
	HT->Current[HT->NumberEntries] = Current;
    HT->Previous[HT->NumberEntries] = Previous;
	if (HT->IsWeighted) {
	    HT->Weight[HT->NumberEntries] = Weight;
	    HT->Ratio[HT->NumberEntries] = Current/Previous * Weight;
	}
	else
	    HT->Ratio[HT->NumberEntries] = Current/Previous;

    HT->NumberEntries++;

    return HT;
}


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
HTT_HISTORICALTREND * HT_Alloc (EIT_BOOLEAN IsWeighted) {
    HTT_HISTORICALTREND * HT;

    /* allocate the structure */
    HT = STC_AllocateMemory (sizeof *HT);
    if (HT == NULL) return NULL;

    /* initialize structure */
    HT->NumberAllocatedEntries = 0;
    HT->NumberEntries = 0;
    HT->Id = NULL;
    HT->Current = NULL;
    HT->Previous = NULL;
    HT->Weight = NULL;
    HT->Ratio = NULL;
    HT->Effect = NULL;
	HT->IsWeighted = IsWeighted;

    /* allocate the structure contents */
    return Reallocate (HT);
}


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void HT_Free (
    HTT_HISTORICALTREND * HT)
{
    int i;

    if (HT != NULL) {
        for (i = 0; i < HT->NumberEntries; i++)
            STC_FreeMemory (HT->Id[i]);
        STC_FreeMemory (HT->Id);
        STC_FreeMemory (HT->Current);
        STC_FreeMemory (HT->Previous);
        STC_FreeMemory (HT->Ratio);
        STC_FreeMemory (HT->Effect);
        STC_FreeMemory (HT->Weight);
        HT->NumberEntries = 0;
        STC_FreeMemory (HT);
    }
}
/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void HT_Print (
    HTT_HISTORICALTREND * HT)
{
	int i;

	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%12s %12s %12s %12s %12s %12s",
		"Id", "Current", "Previous", "Weight", "Ratio", "Effect");
	for (i = 0; i < HT->NumberEntries; i++) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%12s %12.6f %12.6f %12.6f %12.6f %12.6f",
			HT->Id[i], HT->Current[i], HT->Previous[i], HT->IsWeighted ? HT->Weight[i] : 1.0, HT->Ratio[i], HT->Effect[i]);
	}
}
/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void HT_PrintInfo (
    HTT_HISTORICALTREND * HT)
{
	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "NumberEntries=%d NumberAllocatedEntries=%d IsWeighted=%s", HT->NumberEntries, HT->NumberAllocatedEntries, HT->IsWeighted ? "TRUE" : "FALSE");
}
/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void HT_SortEffect (
    HTT_HISTORICALTREND * HT)
{
    int gap;
    int i;
    int j;
    char * TempKey;
    double TempValue;

    for (gap = HT->NumberEntries/2; gap > 0; gap /= 2) {
        for (i = gap; i < HT->NumberEntries; i++) {
            for (j = i-gap; j >= 0; j -= gap) {
                if (HT->Effect[j] <= HT->Effect[j+gap])
                    break;

                /* move the key */
                TempKey       = HT->Id[j];
                HT->Id[j]     = HT->Id[j+gap];
                HT->Id[j+gap] = TempKey;

                /* move the current value */
                TempValue          = HT->Current[j];
                HT->Current[j]     = HT->Current[j+gap];
                HT->Current[j+gap] = TempValue;

                /* move the previous value */
                TempValue           = HT->Previous[j];
                HT->Previous[j]     = HT->Previous[j+gap];
                HT->Previous[j+gap] = TempValue;

                /* move the ratio */
                TempValue        = HT->Ratio[j];
                HT->Ratio[j]     = HT->Ratio[j+gap];
                HT->Ratio[j+gap] = TempValue;

                /* move the effect */
                TempValue         = HT->Effect[j];
                HT->Effect[j]     = HT->Effect[j+gap];
                HT->Effect[j+gap] = TempValue;

                /* move the weight */
				if (HT->IsWeighted) {
	                TempValue         = HT->Weight[j];
		            HT->Weight[j]     = HT->Weight[j+gap];
			        HT->Weight[j+gap] = TempValue;
				}
            }
        }
    }
}


/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
void HT_SortRatio (
    HTT_HISTORICALTREND * HT)
{
    int gap;
    int i;
    int j;
    char * TempKey;
    double TempValue;

    for (gap = HT->NumberEntries/2; gap > 0; gap /= 2) {
        for (i = gap; i < HT->NumberEntries; i++) {
            for (j = i-gap; j >= 0; j -= gap) {
	            if (HT->Ratio[j] <= HT->Ratio[j+gap])
		            break;

                /* move the key */
                TempKey       = HT->Id[j];
                HT->Id[j]     = HT->Id[j+gap];
                HT->Id[j+gap] = TempKey;

                /* move the current value */
                TempValue          = HT->Current[j];
                HT->Current[j]     = HT->Current[j+gap];
                HT->Current[j+gap] = TempValue;

                /* move the previous value */
                TempValue           = HT->Previous[j];
                HT->Previous[j]     = HT->Previous[j+gap];
                HT->Previous[j+gap] = TempValue;

				/* move the ratio */
                TempValue        = HT->Ratio[j];
                HT->Ratio[j]     = HT->Ratio[j+gap];
                HT->Ratio[j+gap] = TempValue;

                /* move the effect */
                TempValue         = HT->Effect[j];
                HT->Effect[j]     = HT->Effect[j+gap];
                HT->Effect[j+gap] = TempValue;

				/* move the weight */
				if (HT->IsWeighted) {
	                TempValue         = HT->Weight[j];
		            HT->Weight[j]     = HT->Weight[j+gap];
			        HT->Weight[j+gap] = TempValue;
				}
			}
        }
    }
}


/*------------------------------------------------------------------------------
re/allocate as needed to HT->NumberAllocatedEntries size
------------------------------------------------------------------------------*/
static HTT_HISTORICALTREND * Reallocate (
    HTT_HISTORICALTREND * HT)
{
    void * Ptr;

    HT->NumberAllocatedEntries += HTM_ALLOCATIONSIZE;

    Ptr = STC_ReallocateMemory (
        sizeof *HT->Id * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
        sizeof *HT->Id * (HT->NumberAllocatedEntries),
        HT->Id);
    if (Ptr == NULL) return NULL;
    HT->Id = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *HT->Current * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
        sizeof *HT->Current * (HT->NumberAllocatedEntries),
        HT->Current);
    if (Ptr == NULL) return NULL;
    HT->Current = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *HT->Previous * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
        sizeof *HT->Previous * (HT->NumberAllocatedEntries),
        HT->Previous);
    if (Ptr == NULL) return NULL;
    HT->Previous = Ptr;

	Ptr = STC_ReallocateMemory (
        sizeof *HT->Ratio * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
        sizeof *HT->Ratio * (HT->NumberAllocatedEntries),
        HT->Ratio);
    if (Ptr == NULL) return NULL;
    HT->Ratio = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *HT->Effect * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
        sizeof *HT->Effect * (HT->NumberAllocatedEntries),
        HT->Effect);
    if (Ptr == NULL) return NULL;
    HT->Effect = Ptr;

	if (HT->IsWeighted) {
		Ptr = STC_ReallocateMemory (
			sizeof *HT->Weight * (HT->NumberAllocatedEntries-HTM_ALLOCATIONSIZE),
			sizeof *HT->Weight * (HT->NumberAllocatedEntries),
			HT->Weight);
		if (Ptr == NULL) return NULL;
		HT->Weight = Ptr;
	}

    return HT;
}
