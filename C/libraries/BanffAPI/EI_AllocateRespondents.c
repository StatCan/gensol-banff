#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Donor.h"

#include "STC_Memory.h"

#define RESPONDENT_ALLOCATIONSIZE  2000

EIT_ALLOCATERESPONDENTS_RETURNCODE EI_AllocateRespondents (
    EIT_ALL_RESPONDENTS * Respondents)
{
    EIT_ALL_RESPONDENTS Tmp;

    if (Respondents->NumberofRespondents ==
            Respondents->NumberofAllocatedRespondents) {

        if (Respondents->NumberofRespondents <= 0)
            Respondents->NumberofAllocatedRespondents =
                RESPONDENT_ALLOCATIONSIZE;
        else
            Respondents->NumberofAllocatedRespondents =
                Respondents->NumberofRespondents * 2;

        Tmp.Key = STC_ReallocateMemory (
            Respondents->NumberofRespondents * sizeof *Tmp.Key,
            Respondents->NumberofAllocatedRespondents * sizeof *Tmp.Key,
            Respondents->Key);
        if (Tmp.Key == NULL) {
            EI_FreeRespondents (Respondents);
            return EIE_ALLOCATERESPONDENTS_FAIL;
        }
        Respondents->Key = Tmp.Key;

        Tmp.RespondentData = STC_ReallocateMemory (
            Respondents->NumberofRespondents * sizeof *Tmp.RespondentData,
            Respondents->NumberofAllocatedRespondents *
                sizeof *Tmp.RespondentData,
            Respondents->RespondentData);
        if (Tmp.RespondentData == NULL) {
            EI_FreeRespondents (Respondents);
            return EIE_ALLOCATERESPONDENTS_FAIL;
        }
        Respondents->RespondentData = Tmp.RespondentData;

        Tmp.RespondentExclusion = STC_ReallocateMemory (
            Respondents->NumberofRespondents * sizeof *Tmp.RespondentExclusion,
            Respondents->NumberofAllocatedRespondents *
                sizeof *Tmp.RespondentExclusion,
            Respondents->RespondentExclusion);
        if (Tmp.RespondentExclusion == NULL) {
            EI_FreeRespondents (Respondents);
            return EIE_ALLOCATERESPONDENTS_FAIL;
        }
        Respondents->RespondentExclusion = Tmp.RespondentExclusion;
    }

    return EIE_ALLOCATERESPONDENTS_SUCCEED;
}

void EI_FreeRespondents (
    EIT_ALL_RESPONDENTS * AllRespondents)
{
    STC_FreeMemory(AllRespondents->Key);
    STC_FreeMemory(AllRespondents->RespondentData);
    STC_FreeMemory(AllRespondents->RespondentExclusion);
}

void EI_RespondentsPrint (
    EIT_ALL_RESPONDENTS * r)
{
    int i;

    printf ("Number of Respondents : %d\n", r->NumberofRespondents);
    for (i = 0; i < r->RespondentData[0].NumberofFields; i++)
        printf ("%s ", r->RespondentData[0].FieldName[i]);
    printf ("\n");

    for (i = 0; i < r->NumberofRespondents; i++) {
        printf ("%s ", r->Key[i]);
        EI_DataRecPrint (&r->RespondentData[i]);
    }
}
