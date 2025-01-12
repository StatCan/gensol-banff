/* contient toutes les fonctions reliees au type EIT_RESIDUAL */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Residual.h"

#include "STC_Memory.h"

#include "util.h"

static EIT_RESIDUAL * Reallocate (EIT_RESIDUAL *);
static int WeightedSelection (double, double[], int);

/*********************************************************************
Add an entry to the residual at the end.
*********************************************************************/
EIT_RESIDUAL * EI_ResidualAdd (
    EIT_RESIDUAL * Residual,
    EIT_DATA * Data,
    double Weight)
{
    if (Weight <= 0.0)
        return NULL;

    /* need more space? */
    if (Residual->NumberEntries == Residual->NumberAllocatedEntries)
        if (Reallocate (Residual) == NULL)
            return NULL;

    Residual->SumOfWeights += Weight;
    Residual->CumulativeWeight[Residual->NumberEntries] =
        Residual->SumOfWeights;
    Residual->Data[Residual->NumberEntries] = Data;
    Residual->Residual[Residual->NumberEntries] = EIM_MISSING_VALUE;
    Residual->NumberEntries++;
    Residual->NumberGoodEntries++;

    return Residual;
}
/*********************************************************************
Allocates a residual.
*********************************************************************/
EIT_RESIDUAL * EI_ResidualAllocate (void) {
    EIT_RESIDUAL * Residual;

    /* allocate the structure */
    Residual = STC_AllocateMemory (sizeof *Residual);
    if (Residual == NULL) return NULL;

    /* initialize structure */
    Residual->IsWeighted = EIE_FALSE;
    Residual->NumberAllocatedEntries = 64;
    Residual->NumberEntries = 0;
    Residual->NumberGoodEntries = 0;
    Residual->SumOfWeights = 0.0;

    /* allocate the structure members */
    Residual->CumulativeWeight = STC_AllocateMemory (
        sizeof *Residual->CumulativeWeight * Residual->NumberAllocatedEntries);
    if (Residual->CumulativeWeight == NULL) return NULL;
    Residual->Data = STC_AllocateMemory (
        sizeof *Residual->Data * Residual->NumberAllocatedEntries);
    if (Residual->Data == NULL) return NULL;
    Residual->Residual = STC_AllocateMemory (
        sizeof *Residual->Residual * Residual->NumberAllocatedEntries);
    if (Residual->Residual == NULL) return NULL;

    return Residual;
}
/*********************************************************************
Empty a residual.
*********************************************************************/
void EI_ResidualEmpty (
    EIT_RESIDUAL * Residual)
{
    if (Residual == NULL) return;

    Residual->NumberEntries = 0;
    Residual->NumberGoodEntries = 0;
    Residual->SumOfWeights = 0.0;
}
/*********************************************************************
Frees a residual.
*********************************************************************/
void EI_ResidualFree (
    EIT_RESIDUAL * Residual)
{
    if (Residual == NULL) return;

    EI_ResidualEmpty (Residual);
    STC_FreeMemory (Residual->CumulativeWeight);
    STC_FreeMemory (Residual->Data);
    STC_FreeMemory (Residual->Residual);
    STC_FreeMemory (Residual);
}
/*********************************************************************
Prints residual.
*********************************************************************/
void EI_ResidualPrint (
    EIT_RESIDUAL * Residual)
{
    int i;
    EIT_DATA * Data;

    if (Residual == NULL) return;

    for (i = 0; i < Residual->NumberEntries; i++) {
        Data = Residual->Data[i];
        if (Data != NULL) {
            printf ("k=%s er=(%d %d) (hash=%d)\n",
                Data->Key,
                Data->ExcludedObs[EIE_PERIOD_CURRENT],
                Data->ExcludedObs[EIE_PERIOD_HISTORICAL],
                i);
            EI_DataRecPrint (Data->DataRec[EIE_PERIOD_CURRENT]);
            if (Data->DataRec[EIE_PERIOD_HISTORICAL] != NULL)
                EI_DataRecPrint (Data->DataRec[EIE_PERIOD_HISTORICAL]);
            else
                puts ("no historical data");
        }
        printf ("r=%g cw=%f sow=%f\n",
            Residual->Residual[i],
            Residual->CumulativeWeight[i],
            Residual->SumOfWeights);
    }
}
/*********************************************************************
Prints residual statistics.
*********************************************************************/
void EI_ResidualPrintStatistics (
    EIT_RESIDUAL * Residual)
{
    int i;
    int NumberCalculated;
    int NumberNotCalculated;
    int NumberNotGood;

    NumberCalculated = 0;
    NumberNotCalculated = 0;
    NumberNotGood = 0;

    for (i = 0; i < Residual->NumberEntries; i++) {
        if (Residual->Residual[i] == EIM_MISSING_VALUE)
            NumberNotCalculated++;
        else if (Residual->Residual[i] == EIM_RESIDUAL_NOTGOOD)
            NumberNotGood++;
        else
            NumberCalculated++;
    }

    printf ("NumberAllocatedEntries = %5d\n", Residual->NumberAllocatedEntries);
    printf ("NumberEntries          = %5d\n", Residual->NumberEntries);
    printf ("NumberGoodEntries      = %5d\n", Residual->NumberGoodEntries);
    printf ("NumberCalculated       = %5d\n", NumberCalculated);
    printf ("NumberNotCalculated    = %5d\n", NumberNotCalculated);
    printf ("NumberNotGood          = %5d\n", NumberNotGood);
    printf ("SumOfWeights           = %f\n", Residual->SumOfWeights);
    puts ("");
}
/*********************************************************************
returns the index of a residual obs taking the weight in account
if necessary
*********************************************************************/
int EI_ResidualSelection (
    EIT_RESIDUAL * Residual)
{
     int Index;
     double RandomNumber;

     if (Residual->IsWeighted == EIE_FALSE) {
         RandomNumber = UTIL_Rand () * Residual->NumberEntries;
         Index = (int) floor (RandomNumber);
     }
     else {
         RandomNumber = UTIL_Rand () * Residual->SumOfWeights;
         Index = WeightedSelection (RandomNumber, Residual->CumulativeWeight,
             Residual->NumberEntries);
     }

     return Index;
}
/*********************************************************************
sets the IsWeighted member of the Residual structure
*********************************************************************/
void EI_ResidualSetIsWeighted (
    EIT_RESIDUAL * Residual,
    EIT_BOOLEAN IsWeighted)
{
    Residual->IsWeighted = IsWeighted;
}
#ifdef NEVERTOBEDEFINED
/*********************************************************************
removes an item at position 'Index' from the list

NOT USED -- TOO SLOW ON LARGE NUMBER OF WEIGHTED DATA
*********************************************************************/
void EI_ResidualRemove (
    EIT_RESIDUAL * Residual,
    int Index)
{
    int i;
    double RemovedWeight;

    if (Index < 0 || Index >= Residual->NumberEntries)
        return;

    if (Residual->IsWeighted == EIE_FALSE) {
        RemovedWeight = 1.0;
    }
    else {
        if (Index == 0)
            RemovedWeight = Residual->CumulativeWeight[Index];
        else
            RemovedWeight = Residual->CumulativeWeight[Index] -
                Residual->CumulativeWeight[Index-1];
    }

    if (Index < Residual->NumberEntries-1) {
        memmove (Residual->Data+Index, Residual->Data+Index+1,
            sizeof *Residual->Data * (Residual->NumberEntries - 1 - Index));
        memmove(Residual->Residual+Index, Residual->Residual+Index+1,
            sizeof *Residual->Residual * (Residual->NumberEntries - 1 - Index));
    }

    if (Residual->IsWeighted != EIE_FALSE) {
        for (i = Index; i < Residual->NumberEntries-1; i++) {
            Residual->CumulativeWeight[i] = Residual->CumulativeWeight[i+1] -
                RemovedWeight;
        }
    }

    Residual->NumberEntries--;

    if (Residual->NumberEntries == 0)
        Residual->SumOfWeights = 0;
    else
        Residual->SumOfWeights =
            Residual->CumulativeWeight[Residual->NumberEntries-1];
}
#endif

/*********************************************************************
re/allocate as needed
*********************************************************************/
static EIT_RESIDUAL * Reallocate (
    EIT_RESIDUAL * Residual)
{
    int NewCount;
    int OldCount;
    void * Ptr;

    OldCount = Residual->NumberAllocatedEntries;
    NewCount = Residual->NumberAllocatedEntries * 2;

    Ptr = STC_ReallocateMemory (
        sizeof *Residual->CumulativeWeight * OldCount,
        sizeof *Residual->CumulativeWeight * NewCount,
        Residual->CumulativeWeight);
    if (Ptr == NULL)
        return NULL;
    Residual->CumulativeWeight = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *Residual->Data * OldCount,
        sizeof *Residual->Data * NewCount,
        Residual->Data);
    if (Ptr == NULL)
        return NULL;
    Residual->Data = Ptr;

    Ptr = STC_ReallocateMemory (
        sizeof *Residual->Residual * OldCount,
        sizeof *Residual->Residual * NewCount,
        Residual->Residual);
    if (Ptr == NULL)
        return NULL;
    Residual->Residual = Ptr;

    Residual->NumberAllocatedEntries = NewCount;

    return Residual;
}
/*********************************************************************
finds (in a binary fashion) the element with its CumulativeWeight greater than
RandomNumber but for which the previous element has a CumulativeWeight smaller
index weight CumulativeWeight
0     1      1
1     1      2
2     1      3

if RandomNumber is 0.5 returns index 0
if RandomNumber is 1.0 returns index 0
if RandomNumber is 1.5 returns index 1
if RandomNumber is 2.0 returns index 1
if RandomNumber is 2.5 returns index 2
if RandomNumber is 3.0 returns index 2

RandomNumber must be between 0 and Residual->SumOfWeights
*********************************************************************/
static int WeightedSelection (
    double RandomNumber,
    double CumulativeWeight[],
    int NumberEntries)
{
     double Difference;
     int High;
     int Low;
     int Mid;

     Low = 0;
     High = NumberEntries - 1;

     while (Low <= High) {
        Mid = (Low + High) / 2;
        Difference = RandomNumber - CumulativeWeight[Mid];
/*        printf ("lo=%d mi=%d hi=%d CW=%f Difference=%f\n",*/
/*            Low, Mid, High, CumulativeWeight[Mid], Difference);*/
        if (Difference > 0.0) {
            Low = Mid + 1;
        }
        else if (Difference < 0.0) {
            if (Mid == 0 || RandomNumber - CumulativeWeight[Mid-1] > 0.0)
                return Mid;
            High = Mid - 1;
        }
        else { /* Difference == 0.0 */
/*            printf ("%g %d %d %d\n", Difference, Low, Mid, High);*/
            return Mid;
        }
     }
     return 0;
}
