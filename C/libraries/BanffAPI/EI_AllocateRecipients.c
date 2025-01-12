#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_Donor.h"

#include "STC_Memory.h"

#define RECIPIENT_ALLOCATIONSIZE 1000

EIT_ALLOCATERECIPIENTS_RETURNCODE EI_AllocateRecipients (
    EIT_ALL_RECIPIENTS * Recipients)
{
    EIT_ALL_RECIPIENTS Tmp;

    if (Recipients->NumberofRecipients ==
            Recipients->NumberofAllocatedRecipients) {

        if (Recipients->NumberofRecipients <= 0)
            Recipients->NumberofAllocatedRecipients = RECIPIENT_ALLOCATIONSIZE;
        else
            Recipients->NumberofAllocatedRecipients =
                Recipients->NumberofRecipients * 2;

        Tmp.Key = STC_ReallocateMemory (
            Recipients->NumberofRecipients * sizeof *Tmp.Key,
            Recipients->NumberofAllocatedRecipients * sizeof *Tmp.Key,
            Recipients->Key);
        if (Tmp.Key == NULL) {
            EI_FreeRecipients (Recipients);
            return EIE_ALLOCATERECIPIENTS_FAIL;
        }
        Recipients->Key = Tmp.Key;

        Tmp.RecipientData = STC_ReallocateMemory (
            Recipients->NumberofRecipients * sizeof *Tmp.RecipientData,
            Recipients->NumberofAllocatedRecipients * sizeof *Tmp.RecipientData,
            Recipients->RecipientData);
        if (Tmp.RecipientData == NULL) {
            EI_FreeRecipients (Recipients);
            return EIE_ALLOCATERECIPIENTS_FAIL;
        }
        Recipients->RecipientData = Tmp.RecipientData;
    }

    return EIE_ALLOCATERECIPIENTS_SUCCEED;
}

void EI_FreeRecipients (
    EIT_ALL_RECIPIENTS * AllRecipients)
{
    STC_FreeMemory(AllRecipients->Key);
    STC_FreeMemory(AllRecipients->RecipientData);
}

void EI_RecipientsPrint (
    EIT_ALL_RECIPIENTS * r)
{
    int i;

    printf ("Number of Recipients : %d\n", r->NumberofRecipients);
    for (i = 0; i < r->RecipientData[0].NumberofFields; i++)
        printf ("%s ", r->RecipientData[0].FieldName[i]);
    printf ("\n");

    for (i = 0; i < r->NumberofRecipients; i++) {
        printf ("%s ", r->Key[i]);
        EI_DataRecPrint (&r->RecipientData[i]);
    }
}
