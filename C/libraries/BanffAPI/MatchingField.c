/************************************************************************/
/* This module contains functions that are used by the Donor Imputation */
/* method.                                                              */
/************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "STC_Memory.h"
#include "MatchingField.h"

/* grosseur des chunks lors des allocations dynamique de mémoire */
#define MF_ALLOCATIONSIZE 5000

static int Reallocate (EIT_MATCHVAL *);

MFT_FIELDVALUESADD_RETURNCODE MF_FieldValuesAdd (
    EIT_MATCHVAL * FieldValues,
    char *Key,
    double Value)
{
    if (FieldValues->NumberofValues == FieldValues->NumberofAllocatedValues)
        if ((Reallocate (FieldValues)) == -1)
            return MFE_FIELDVALUESADD_FAIL;

    FieldValues->Key[FieldValues->NumberofValues] = Key;
    FieldValues->Value[FieldValues->NumberofValues] = Value;

    FieldValues->NumberofValues++;

    return MFE_FIELDVALUESADD_SUCCEED;
}

EIT_MATCHVAL * MF_FieldValuesAlloc (void) {

    EIT_MATCHVAL * FieldValues;

    FieldValues = STC_AllocateMemory (sizeof *FieldValues);
    if (FieldValues == NULL)
        return NULL;

    FieldValues->NumberofAllocatedValues = 0;
    FieldValues->NumberofValues = 0;
    FieldValues->Key = NULL;
    FieldValues->Value = NULL;

    if (Reallocate (FieldValues) == -1)
        return NULL;

    return FieldValues;
}

void MF_FieldValuesFree (
    EIT_MATCHVAL * FieldValues)
{
    STC_FreeMemory(FieldValues->Key);
    STC_FreeMemory (FieldValues->Value);
    STC_FreeMemory (FieldValues);
}

void MF_FieldValuesSort (
    EIT_MATCHVAL * FieldValues)
{
    long i, j, gap, hipos, lowpos;
    double value;
    char *ptrchar;

    for (gap = FieldValues->NumberofValues/2; gap > 0; gap /= 2) {
        for (i = gap; i < FieldValues->NumberofValues; i++) {
            for (j = i-gap; j >= 0; j -= gap) {
                lowpos = j;
                hipos  = lowpos + gap;
                if (strcmp(FieldValues->Key[lowpos], FieldValues->Key[hipos])<0)
                    break;

                value                      = FieldValues->Value[lowpos];
                FieldValues->Value[lowpos] = FieldValues->Value[hipos];
                FieldValues->Value[hipos]  = value;

                ptrchar                    = FieldValues->Key[lowpos];
                FieldValues->Key[lowpos]   = FieldValues->Key[hipos];
                FieldValues->Key[hipos]    = ptrchar;
            }
        }
    }
}

static int Reallocate (
    EIT_MATCHVAL * FieldValues)
{
    double * TempValue;
    char ** TempKey;

    FieldValues->NumberofAllocatedValues += MF_ALLOCATIONSIZE;

    TempValue = STC_ReallocateMemory (
        FieldValues->NumberofValues * sizeof *TempValue,
        FieldValues->NumberofAllocatedValues * sizeof *TempValue,
        FieldValues->Value);
    if (TempValue == NULL) {
        MF_FieldValuesFree (FieldValues);
        return -1;
    }
    FieldValues->Value = TempValue;

    TempKey = STC_ReallocateMemory (
        FieldValues->NumberofValues * sizeof *TempKey,
        FieldValues->NumberofAllocatedValues * sizeof *TempKey,
        FieldValues->Key);
    if (TempKey == NULL){
        MF_FieldValuesFree (FieldValues);
        return -1;
    }
    FieldValues->Key = TempKey;

    return 1;
}
