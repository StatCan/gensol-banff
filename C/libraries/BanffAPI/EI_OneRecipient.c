#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "EI_Common.h"
#include "EI_Donor.h"

#include "STC_Memory.h"

#include "Donor.h"
#include "util.h"

/********************************************************************/
/* Allocate Memory for a "maximum number" of FTI fields for one     */
/* recipient                                                        */
/********************************************************************/
EIT_ONERECIPIENTALLOCATE_RETURNCODE EI_OneRecipient_AllocateFTIFields (
    int ** FTIFields,
    int NumberofEditsFields)
{
    DONORT_ALLOCATEFTI_RETURNCODE rc;

    rc = DONOR_AllocateFTIFields (FTIFields, NumberofEditsFields);
    if (rc == DONORE_ALLOCATEFTI_FAIL)
        return EIE_ONERECIPIENTALLOCATE_FAIL; 

    return EIE_ONERECIPIENTALLOCATE_SUCCEED;
}

/********************************************************************/
/* Copy values of the nearest donor to FTI fields for one recipient */
/* Pre-requisite:                                                   */
/* 1. Observation processed MUST be a recipient.                    */
/* 2. Structure EIT_ALL_RESPONDENTS must be sorted by ascending     */
/*    order of "key".                                               */
/********************************************************************/
void EI_OneRecipient_CopyDonorValues (
        /* Donor index in EIT_ALL_DONORS */
    int DonorIndex, 
        /* All Donors keys*/
    char ** AllDonorsKeys,
        /* All Respondents data (sorted in ascending order of "key")*/
    EIT_ALL_RESPONDENTS * AllRespondents,
        /* List indexes of recipient fields to impute*/ 
    int * FTIFields,
        /* Data of the recipient*/
    EIT_DATAREC * RecipientData)
{
    int l;  

    /* Find donor in AllRespondents
      ("AllRespondents" structure is sorted in ascending order for Key)*/
    l = UTIL_Binary(AllDonorsKeys[DonorIndex], AllRespondents->Key,
        AllRespondents->NumberofRespondents);

    DONOR_CopyDonorValues (FTIFields,
        AllRespondents->RespondentData[l].FieldValue,
        RecipientData->FieldValue);

}

/*****************************************************************/
/* For one recipient with matching fields, search among the      */
/* nearest donors found in the KD tree the index of the next     */
/* one having the smallest distance with this recipient.         */
/* Pre-requisite:                                                */
/*     Return code of EI_FindNearestDonors() is "SUCCEED"        */
/* Return: Next Nearest Donor Index (corresponding to index of   */
/*         the donor in the structure EIT_ALL_DONORS)            */
/*****************************************************************/
int EI_OneRecipient_FindNextNearestDonor (        
    int k, /* Current index in EIT_NEARESTDONORS */        
    EIT_NEARESTDONORS * NearestDonors) /* All Nearest Donors */
{
    return DONOR_FindNextNearestDonorIndex (k, NearestDonors); 

}

int EI_OneRecipient_FindNextNearestDnAlt (
    int k,                             /* Current index in EIT_NEARESTDONORS */
    EIT_NEARESTDONORS * NearestDonors, /* All Nearest Donors */
    EIT_ALL_DONORS * AllDonors,        /* All Donors */
    tSList   * randkey_list,           /* tSList sorted in ascending order containing the keys of all of the records read for the current by group */
    tADTList * randnum_list,           /* tADTList sorted in the same order as "randkey_list" containing the value of the random number field of all of the records read for the current by group */
    char     * AllRecipients_Key_i)    /* the key of the recipient record */
{
    return DONOR_FindNextNearestDonorIndexAlt (k, NearestDonors, AllDonors, randkey_list, randnum_list, AllRecipients_Key_i);

}

/********************************************************************/
/* Free Memory for FTI fields for one recipient                     */
/********************************************************************/
void EI_OneRecipient_FreeFTIFields (
    int * FTIFields)
{
    STC_FreeMemory (FTIFields);
}

/********************************************************************/
/* Free Memory for "Nearest donors" structure                       */
/* Pre-requisite: EI_FindNearestDonors() return code MUST be        */
/*                EIE_FINDNEARESTDONORS_SUCCEED.                    */
/********************************************************************/
void EI_OneRecipient_FreeNearestDonors (
    EIT_NEARESTDONORS  * NearestDonors)
{
    STC_FreeMemory(NearestDonors->DonorsIndex);
    STC_FreeMemory(NearestDonors->Distances);
	STC_FreeMemory(NearestDonors);
}

/********************************************************************/
/* Get FTI fields for one recipient                                 */
/********************************************************************/
void EI_OneRecipient_GetFTIFields (
    int * FTIFields,
    int   NumberofEditsFields, /* Edits: # of fields + constant term */
    EIT_DATAREC * RecipientData)
{
    DONOR_GetFTIFields (FTIFields, NumberofEditsFields, RecipientData);
}

/*********************************************************************
 Check if a recipient has at least one matching fields.
 (verify all types of matching fields: MFU, MFS and MFB)
*********************************************************************/
int EI_OneRecipient_HasMatchingFields_AllTypes (
    EIT_DATAREC * RecipientData)
{
    return DONOR_HasMatchingFields_AllTypes (RecipientData);
}

/*********************************************************************
 Check if a recipient has at least one matching fields.
 (verify only user matching fields: MFU)
*********************************************************************/
int EI_OneRecipient_HasMatchingFields_TypeMFU (
    EIT_DATAREC * RecipientData)
{
    return  DONOR_HasMatchingFields_TypeMFU (RecipientData);
}


