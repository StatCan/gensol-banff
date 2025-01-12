/* contient toutes les fonctions reliees au EIT_DATAREC */

#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Message.h"

#include "STC_Memory.h"

/***************************************************
Allocates and initializes new datarec structure
***************************************************/
EIT_RETURNCODE EI_DataRecAllocate (
    char ** FieldName,
    int NumberofFields,
    EIT_DATAREC * DataRec)
{
    DataRec->FieldName = FieldName;
    DataRec->NumberofFields = NumberofFields;

    /*  Allocate space for data     */
    DataRec->FieldValue = STC_AllocateMemory (
        DataRec->NumberofFields * sizeof *DataRec->FieldValue);
    if (DataRec->FieldValue == NULL) {
        return EIE_FAIL;
    }

    /*  Allocate space for data flag*/
    DataRec->StatusFlag = STC_AllocateMemory (
        DataRec->NumberofFields * sizeof *DataRec->StatusFlag);
    if (DataRec->StatusFlag == NULL) {
        STC_FreeMemory (DataRec->FieldValue);
        return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/***************************************************
Frees datarec structure
***************************************************/
void EI_DataRecFree (
    EIT_DATAREC * DataRec)
{
    if (DataRec != NULL) {
        STC_FreeMemory (DataRec->FieldValue);
        STC_FreeMemory (DataRec->StatusFlag);
    }
}
/***************************************************
Prints datarec structure
***************************************************/
void EI_DataRecPrint (
    EIT_DATAREC * DataRec)
{
#define CASEFLAG(X) case X: EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "(" #X ") "); break;
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++) {
        if (DataRec->FieldValue[i] != EIM_MISSING_VALUE)
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s=%.1f", DataRec->FieldName[i], DataRec->FieldValue[i]);
        else
            EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s=MISSING", DataRec->FieldName[i]);

        switch (DataRec->StatusFlag[i]) {
        CASEFLAG (FIELDOK)
        CASEFLAG (FIELDFTI)
        CASEFLAG (FIELDMFU)
        CASEFLAG (FIELDMFS)
        CASEFLAG (FIELDMFB)
        CASEFLAG (FIELDIDE)
        CASEFLAG (FIELDMISS)
        CASEFLAG (FIELDNEG)
        CASEFLAG (FIELDFTE)
        CASEFLAG (FIELDIMPUTED) /* by any methods */
        default:
            /*
            estimator uses a negative status to remember which estimator was
            used to impute the value
            */
            if (DataRec->StatusFlag[i] < 0)
                EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "(estimator %d) ", -1-DataRec->StatusFlag[i]);
            else {
                EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "(UNKNOWN) ");
                exit (-1);
            }
            break;
        }
    }

    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "");
}
/***************************************************
rene: create one that does a binary search.
Search a field name using a linear search. Returns the index of the field name
or -1 if not found.
***************************************************/
int EI_DataRecSearch (
    EIT_DATAREC * DataRec,
    char * FieldName)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++)
        if (strcmp (DataRec->FieldName[i], FieldName) == 0)
            return i;

    return EIM_NOTFOUND;
}
/***************************************************
Set the StatusFlag of a field based on the value of FieldValue.
Must be called before the fieldstat is read.
Considers negative and missing values.
***************************************************/
void EI_DataRecSetFlag (
    EIT_DATAREC * DataRec)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++) {
        if (DataRec->FieldValue[i] >= 0.0)
            DataRec->StatusFlag[i] = FIELDOK;
        else if (DataRec->FieldValue[i] == EIM_MISSING_VALUE)
            DataRec->StatusFlag[i] = FIELDMISS;
        else
            DataRec->StatusFlag[i] = FIELDNEG;
    }
}
/***************************************************
for development use only!
Set the StatusFlag of a field based on the value of FieldValue.
Set StatusFlag to FIELDFTI if FieldValue is negative
***************************************************/
void EI_DataRecSetFlagForDebug (
    EIT_DATAREC * DataRec)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++) {
        if (DataRec->FieldValue[i] >= 0.0)
            DataRec->StatusFlag[i] = FIELDOK;
        else
            DataRec->StatusFlag[i] = FIELDFTI;
    }
}
/***************************************************
Set the StatusFlag to FIELDMISS if FieldValue is missing.
Must be called before the fieldstat is read.
Considers missing values only.
***************************************************/
void EI_DataRecSetMissingFlag (
    EIT_DATAREC * DataRec)
{
    int i;

    for (i = 0; i < DataRec->NumberofFields; i++) {
        if (DataRec->FieldValue[i] == EIM_MISSING_VALUE)
            DataRec->StatusFlag[i] = FIELDMISS;
        else
            DataRec->StatusFlag[i] = FIELDOK;
    }
}
