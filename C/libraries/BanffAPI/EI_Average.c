/* contient toutes les fonctions reliees au type EIT_AVERAGE  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Average.h"

#include "STC_Memory.h"

/*********************************************************************
Creates a average item.
*********************************************************************/
EIT_AVERAGE * EI_AverageAdd (
    EIT_AVERAGE * Average,
    char * Name,
    EIT_PERIOD Period)
{
    EIT_AVERAGE * AverageCurrent;

    AverageCurrent = STC_AllocateMemory (sizeof *AverageCurrent);
    if (AverageCurrent == NULL) return NULL;
    AverageCurrent->Name = STC_StrDup (Name);
    if (AverageCurrent->Name == NULL) return NULL;
    AverageCurrent->Period = Period;
    AverageCurrent->VariableIndex = -1;
    AverageCurrent->WeightIndex = -1;
    AverageCurrent->Numerator = 0.0;
    AverageCurrent->Denominator = 0.0;
    AverageCurrent->Average = 0.0;
    AverageCurrent->Count = 0;
    AverageCurrent->Next = Average;

    return AverageCurrent;
}
/*********************************************************************
Frees a average list.
*********************************************************************/
void EI_AverageFree (
    EIT_AVERAGE * Average)
{
    EIT_AVERAGE * AverageNext;

    for (; Average != NULL; Average = AverageNext) {
        AverageNext = Average->Next;
        STC_FreeMemory (Average->Name);
        STC_FreeMemory (Average);
    }
}
/*********************************************************************
Find name in average list.
*********************************************************************/
EIT_AVERAGE * EI_AverageLookup (
    EIT_AVERAGE * Average,
    char * Name,
    EIT_PERIOD Period)
{
    for (; Average != NULL; Average = Average->Next)
        if (strcmp (Average->Name, Name) == 0 &&
                Average->Period == Period)
            return Average;

    return NULL;
}
/*********************************************************************
Prints average list.
*********************************************************************/
void EI_AveragePrint (
    EIT_AVERAGE * Average)
{
    for (; Average != NULL; Average = Average->Next) {
        printf (
            "Var=%s Per=%d VInd=%d WInd=%d Count=%d Num(%.2f)/Den(%.2f)=%.2f\n",
            Average->Name, Average->Period,
            Average->VariableIndex, Average->WeightIndex, Average->Count,
            Average->Numerator, Average->Denominator, Average->Average);
    }
}
