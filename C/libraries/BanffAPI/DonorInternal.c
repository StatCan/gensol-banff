/************************************************************************/
/* This module contains functions that are used by the Donor Imputation */
/* method.                                                              */
/* Massimputation method uses some of these functions.                  */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "EI_Message.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "Donor.h"
#include "util.h"

#include "MessageBanffAPI.h"

static void FreeDonorIndex (int *);
static int GetDonorIndex (EIT_ALL_RESPONDENTS *, int *);
static int GetDonorKeys (long, int *, char **, char **);
static int GetDonorValues (EIT_ALL_DONORS *, EIT_TRANSFORMED *);
static int GetDonorStatusFlag (EIT_ALL_DONORS *, int *, EIT_DATAREC *);

/*****************************************************************/
/* Allocate memory for structure EIT_NEARESTDONORS               */
/* Note:                                                         */
/*  Called in EI_FindNearestDonors() if some donors have been    */
/*  found in KDTree  i.e. TreeSearch.dl_size > 0                 */
/*****************************************************************/
EIT_NEARESTDONORS * DONOR_AllocateNearestDonors (
    int NumberofDonorsFound)
{
    EIT_NEARESTDONORS * NearestDonors;
    
	NearestDonors = STC_AllocateMemory (sizeof *NearestDonors);
	if (NearestDonors == NULL)
		return NULL;

    NearestDonors->DonorsIndex = STC_AllocateMemory (
        NumberofDonorsFound * sizeof *NearestDonors->DonorsIndex);
    if (NearestDonors->DonorsIndex == NULL)
        return NULL;

    NearestDonors->Distances = STC_AllocateMemory (
        NumberofDonorsFound * sizeof *NearestDonors->Distances);
    if (NearestDonors->Distances == NULL)
        return NULL;

    return NearestDonors;
}

/******************************************************************/
/* Allocate Memory for a "maximum number" of recipient FTI fields */
/******************************************************************/
DONORT_ALLOCATEFTI_RETURNCODE DONOR_AllocateFTIFields (
    int ** FTIFields,
    int NumberofEditsFields)
{
    *FTIFields = STC_AllocateMemory (NumberofEditsFields * sizeof **FTIFields);
    if (*FTIFields == NULL)
        return DONORE_ALLOCATEFTI_FAIL;

    return DONORE_ALLOCATEFTI_SUCCEED;
}

/***************************************************************/
/* Allocate Memory for all variables needed in the tree search */
/* (except FTIFields: already allocated)                       */
/***************************************************************/
DONORT_ALLOCATETREESEARCH_RETURNCODE DONOR_AllocateTreeSearch (
    int ** MatchFields,
    double ** MatchFieldsValues,
    double ** lbound,
    double ** ubound,
    EIT_TREESEARCH * search,
    int NumberOfMatchingFields,
    long NumberOfDonors)
{
    int i;

    *MatchFields = STC_AllocateMemory ((NumberOfMatchingFields+1) * sizeof **MatchFields);
    if (*MatchFields == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    *MatchFieldsValues = STC_AllocateMemory ((NumberOfMatchingFields+1) * sizeof **MatchFieldsValues);
    if (*MatchFieldsValues == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    for (i = 0; i < NumberOfMatchingFields; i++)
        (*MatchFieldsValues)[i] = 0.0;
    (*MatchFieldsValues)[i] = ENDLIST;

    *lbound = STC_AllocateMemory ((NumberOfMatchingFields+1) * sizeof **lbound);
    if (*lbound == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    *ubound = STC_AllocateMemory ((NumberOfMatchingFields+1) * sizeof **ubound);
    if (*ubound == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    search->d_list = STC_AllocateMemory (NumberOfDonors * sizeof *search->d_list);
    if (search->d_list == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    search->distances = STC_AllocateMemory (NumberOfDonors * sizeof *search->distances);
    if (search->distances == NULL)
        return DONORE_ALLOCATETREESEARCH_FAIL;

    search->m_list = &search->d_list[NumberOfDonors-1];

    return DONORE_ALLOCATETREESEARCH_SUCCEED;
}
/*********************************************************************
*********************************************************************/
void DONOR_CopyDonorValues (
    int    *FTIFields,      /* list indexes of recipient fields to impute   */
    double *donorvalues,    /* values of the current donor                  */
    double *FTIFieldsValues)/* values of the current recipient              */
{
    while (*FTIFields != ENDLIST) {
        FTIFieldsValues[*FTIFields] = donorvalues[*FTIFields];
        FTIFields++;
    }
}
/*********************************************************************
*********************************************************************/
void DONOR_CopyMatchFields (
    double * MatchFieldsValues,
    EIT_TRANSFORMED *AllTransformed,
    char * Key,
    int * MatchFields)
{
    int i;
    int where;

    for (i = 0; MatchFields[i] != ENDLIST; i++) {
        where = UTIL_Binary (Key,
            AllTransformed->TransformedValues[MatchFields[i]].Key,
            AllTransformed->TransformedValues[MatchFields[i]].NumberofValues);
        MatchFieldsValues[MatchFields[i]] =
            AllTransformed->TransformedValues[MatchFields[i]].Value[where];
    }
}

/*********************************************************************
Get the set of keys, values and status flags of the set of donors.
Donors are respondents that have no FTI fields.
*********************************************************************/
DONORT_CREATEALLDONORS_RETURNCODE DONOR_CreateAllDonors (
    EIT_ALL_RESPONDENTS * AllRespondents,
    EIT_ALL_DONORS * AllDonors,
    EIT_FIELDNAMES * MatchingFields,
    EIT_TRANSFORMED * AllTransformed)
{
    int * DonorIndex;
    int rcode;
	int i;

    AllDonors->NumberOfFields = MatchingFields->NumberofFields;

    DonorIndex = STC_AllocateMemory (AllRespondents->NumberofRespondents * sizeof *DonorIndex);
    if (DonorIndex == NULL) return DONORE_CREATEALLDONORS_FAIL;

    AllDonors->NumberOfDonors = GetDonorIndex (AllRespondents, DonorIndex);

    AllDonors->Key = STC_AllocateMemory (AllDonors->NumberOfDonors * sizeof *AllDonors->Key);
    if (AllDonors->Key == NULL) return DONORE_CREATEALLDONORS_FAIL;

    AllDonors->Value = STC_AllocateMemory (AllDonors->NumberOfDonors * sizeof *AllDonors->Value);
    if (AllDonors->Value == NULL) return DONORE_CREATEALLDONORS_FAIL;

    AllDonors->Gave = STC_AllocateMemory (AllDonors->NumberOfDonors * sizeof *AllDonors->Gave);
    if (AllDonors->Gave == NULL) return DONORE_CREATEALLDONORS_FAIL;

	AllDonors->DonorStatusFlag = STC_AllocateMemory (AllDonors->NumberOfDonors * sizeof *AllDonors->DonorStatusFlag);
    if (AllDonors->DonorStatusFlag == NULL) return DONORE_CREATEALLDONORS_FAIL;

    rcode = GetDonorKeys (AllDonors->NumberOfDonors, DonorIndex, AllDonors->Key, AllRespondents->Key);
    if (rcode == -1) return DONORE_CREATEALLDONORS_FAIL;

    rcode = GetDonorValues (AllDonors, AllTransformed);
    if (rcode == -1) return DONORE_CREATEALLDONORS_FAIL;

	for(i = 0; i < AllDonors->NumberOfDonors - 1; i++) {
	   AllDonors->Gave[i] = 0;
	}

    rcode = GetDonorStatusFlag (AllDonors, DonorIndex, AllRespondents->RespondentData);
    if (rcode == -1) return DONORE_CREATEALLDONORS_FAIL;

    FreeDonorIndex (DonorIndex);

    return DONORE_CREATEALLDONORS_SUCCEED;
}

/**************************************************************/
/* Search among the nearest donors found in the KD tree the   */
/* index of the next one having the smallest distance.        */
/* Return:                                                    */
/*   Next Nearest Donor Index (corresponding to index of the  */
/*   donor in the structure EIT_ALL_DONORS)                   */
/**************************************************************/
int DONOR_FindNextNearestDonorIndex (        
    int k, /* Current index in EIT_NEARESTDONORS */        
    EIT_NEARESTDONORS * NearestDonors)
{
    int j, temp_k;
    int choice; /* next donor chosen from among tied donors  */

    for (j = k + 1; j < NearestDonors->NumberofDonors ; j++)
        if (NearestDonors->Distances[k] != NearestDonors->Distances[j])
            break; /* DonorsIndex[k] is the nearest */

    /* Tied donors (same distances) */
    if ((j-k) > 1) {
       choice = UTIL_Random (k, j);
       if (choice != k) {
           temp_k = NearestDonors->DonorsIndex[k];
           NearestDonors->DonorsIndex[k] = NearestDonors->DonorsIndex[choice];
           NearestDonors->DonorsIndex[choice] = temp_k;
       }
    }

    return NearestDonors->DonorsIndex[k];
}

int DONOR_FindNextNearestDonorIndexAlt (
    int k,                             /* Current index in EIT_NEARESTDONORS */
    EIT_NEARESTDONORS * NearestDonors, /* All Nearest Donors */
    EIT_ALL_DONORS * AllDonors,        /* All Donors */
    tSList   * randkey_list,           /* tSList sorted in ascending order containing the keys of all of the records read for the current by group */
    tADTList * randnum_list,           /* tADTList sorted in the same order as "randkey_list" containing the value of the random number field of all of the records read for the current by group */
    char * AllRecipients_Key_i)        /* key of recipient for which a matching donor is sought */
{
    int j, temp_k;
    int choice; /* next donor chosen from among tied donors  */
    char * Key;
    int index_in_list;
    double RandNum;
    double * address_of_randnum_matching_key;

    for (j = k + 1; j < NearestDonors->NumberofDonors ; j++)
        if (NearestDonors->Distances[k] != NearestDonors->Distances[j])
            break; /* DonorsIndex[k] is the nearest */

    /*-k is the index of the next donor that hasn't been tried yet    */
    /*-if not ((j-k) > 1), then j = k+1 and the distance of the jth   */
    /* donor in the list is greater than the distance of the kth      */
    /* donor                                                          */
    /*-if ((j-k) > 1), then there is more than one donor with the     */
    /* distance as the kth donor,                                     */
    /* and j is the index of the first donor following the kth donor  */
    /* whose distance is different from that of the kth donor (and    */
    /* therefor is greater than that of the kth donor, since the list */
    /* is sorted in ascending order of distances)                     */
    /* or is (the index of the last donor in the list)+1 if all of    */
    /* remaining untried donors have the same distance as the kth     */
    /* donor:                                                         */
    if ((j-k) > 1) {
        /* choice = UTIL_Random (k, j); */ /* <-- this is how choice is calculated if the user hasn't supplied a random number */
                                           /*     with each record in the input "data=" dataset                                */
                                           /*     and it expands to "choice = k+(long)(UTIL_Rand ()*(j-k));"                   */
        /*-assign the value of the random number from the recipient record to   */
        /* the variable "RandNum" (all of the random numbers are doubles        */
        /* between 0.0 and 1.0) and use it to decide the order in the sequence  */
        /* tied records of the record that will be selected):                   */
        RandNum = -1;
        Key = AllRecipients_Key_i;
        /* rc = EI_DataTableLookup(DataTable, Key, EIE_DATATABLELOOKUPTYPE_FIND, &Data); */
        index_in_list = SList_StringSearchBinary(randkey_list, Key);
        if (index_in_list == SLIST_NOTFOUND) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalTslistLookupFail "\n");
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalNotFound "\n");
            return -1;
        }
        address_of_randnum_matching_key = (double *)ADTList_Index(randnum_list, index_in_list);
        
        if (address_of_randnum_matching_key == NULL) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalTadtlistIndexFail "\n");
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalNotFound "\n");
            return -1;
        }
        RandNum = *address_of_randnum_matching_key;
        
        if (RandNum < 0.0) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalLessThanZero "\n", RandNum);
            return -1;
        }
        else if (RandNum > 1.0) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarDonorInternalGreaterThanOne "\n", RandNum);
            return -1;
        }
        else if (RandNum > (1.0 - DONOR_RANDNUM_FUZZ)) {
            printf(SAS_MESSAGE_PREFIX_NOTE MsgRandnumvarRandomDonorInvalidRange "\n");
            RandNum = 1.0 - DONOR_RANDNUM_FUZZ;
        }
        choice = k+(long)(RandNum*(j-k));
        if (choice != k) {
            /*-the record selected to be tried next isn't the next untried record in the list of nearest donor    */
            /* records, so swap it with the next untried record in the list (they both have the same distance, so */
            /* the list will still be correctly sorted (and that's also why the only the elements of              */
            /* "NearestDonors->DonorsIndex" have to be swapped, and the corresponding elements of                 */
            /* "NearestDonors->Distances" don't also have to be swapped)), and then return the element of         */
            /* "NearestDonors->DonorsIndex" whose index is now the index of the record selected to be tried next: */
            temp_k = NearestDonors->DonorsIndex[k];
            NearestDonors->DonorsIndex[k] = NearestDonors->DonorsIndex[choice];
            NearestDonors->DonorsIndex[choice] = temp_k;
        }
    }

    return NearestDonors->DonorsIndex[k];
}

/*******************************************************/
/* Free memory allocated by DONOR_AllocateRandomMemory */
/* Note: - unused                                      */
/*       - free with EI_OneRecipient_FreeFTIFields()   */
/*******************************************************/
void DONOR_FreeRandomSearch (
    int * FTIFields)
{
    STC_FreeMemory (FTIFields);
}

/*********************************************************/
/* Free the structures allocated by DONOR_AllocateSearch */
/*********************************************************/
void DONOR_FreeTreeSearch (
    int * MatchFields,
    double * MatchFieldsValues,
    double * lbound,
    double * ubound,
    EIT_TREESEARCH * Search)
{
    STC_FreeMemory (MatchFields);
    STC_FreeMemory (MatchFieldsValues);
    STC_FreeMemory (lbound);
    STC_FreeMemory (ubound);
    STC_FreeMemory (Search->d_list);
    STC_FreeMemory (Search->distances);
}

/*********************************************************************
*********************************************************************/
void DONOR_GetFTIFields (
    int * FTIFields,
    int   NumberOfEditsFields, /* Edits: # of fields + constant term */
    EIT_DATAREC * RecipientData)
{
    int i, NumberOfFTI = 0;

    for (i = 0; i < NumberOfEditsFields - 1; i++) {
        if (RecipientData->StatusFlag[i] == FIELDFTI) {
            FTIFields[NumberOfFTI++] = i;
        }
    }
    FTIFields[NumberOfFTI] = ENDLIST;
}

/*********************************************************************
AllMatchingFields and AllTransformed are filled with the
same fields name and in the same order
*********************************************************************/
void DONOR_GetMatchFields (
    int * MatchFields,
    EIT_DATAREC * RecipientData,
    EIT_TRANSFORMED * AllTransformed,
    EIT_FIELDNAMES * AllMatchingFields)
{
    int i, j, NumberOfMF = 0;

    for (i = 0; i < AllMatchingFields->NumberofFields; i++) {
        for (j = 0; j < RecipientData->NumberofFields; j++) {
            if (strcmp (AllMatchingFields->FieldName[i], RecipientData->FieldName[j]) == 0) {
		        if ((RecipientData->StatusFlag[j] == FIELDMFU) ||
				    (RecipientData->StatusFlag[j] == FIELDMFS) ||
		            (RecipientData->StatusFlag[j] == FIELDMFB)) {
		            MatchFields[NumberOfMF] = i;
		            NumberOfMF++;
		        }
                break;
            }
        }

    }
    MatchFields[NumberOfMF] = ENDLIST;
}

/*---------------------------------------------------------*/
/* Check if a potential donor contains at least one FTE    */
/* field corresponding to any of the recipient FTI fields. */
/*---------------------------------------------------------*/
EIT_BOOLEAN DONOR_HasFIELDFTE (
    EIT_ALL_DONORS *AllDonors, /* ordered list of all donors, their values */
    int     DonorIndex,        /* index in AllDonors of donor to test      */
    int    *xfti)              /* indexes of recipient FTI field columns   */
{
    EIT_STATUS_OF_FIELDS * StatusFlag;

    StatusFlag = AllDonors->DonorStatusFlag[DonorIndex].StatusFlag;
    while (*xfti != ENDLIST) {
        if (StatusFlag[*xfti] == FIELDFTE)
            return EIE_TRUE;
        xfti++;
    }

    return EIE_FALSE;
}

/*********************************************************************
 Check if a recipient has at least one matching fields.
 (verify all types of matching fields: MFU, MFS and MFB)
*********************************************************************/
int DONOR_HasMatchingFields_AllTypes (
    EIT_DATAREC * RecipientData)
{
    int i;

    for (i = 0; i < RecipientData->NumberofFields; i++) {
        if (RecipientData->StatusFlag[i] == FIELDMFU ||
				RecipientData->StatusFlag[i] == FIELDMFS ||
				RecipientData->StatusFlag[i] == FIELDMFB)
			return 1;
    }
    return 0;
}

/*********************************************************************
 Check if a recipient has at least one matching fields.
 (verify only user matching fields: MFU)
*********************************************************************/
int DONOR_HasMatchingFields_TypeMFU (
    EIT_DATAREC * RecipientData)
{
    int i;

    for (i = 0; i < RecipientData->NumberofFields; i++)
        if (RecipientData->StatusFlag[i] == FIELDMFU)
			return 1;
    return 0;
}

/*****************************************************************/
/* Populate structure for nearest donors with ones found in      */
/* KDTree for a recipient.                                       */
/*****************************************************************/
void DONOR_PopulateNearestDonors (
    int NumberofDonorsFound,  /* EIT_TREESEARCH: dl_size            */
    int * DonorsIndex,        /* EIT_TREESEARCH: int * d_list       */
    double * Distances,       /* EIT_TREESEARCH: double * distances */
    EIT_NEARESTDONORS * NearestDonors)
{
    int i;

    NearestDonors->NumberofDonors = NumberofDonorsFound;

    for (i = 0; i < NumberofDonorsFound; i++) {
        NearestDonors->DonorsIndex[i] = DonorsIndex[i];
        NearestDonors->Distances[i] = Distances[i];
    }
}

/*********************************************************************
*********************************************************************/
void DONOR_ReduceChoiceofDonors (
    int NumberofDonors,
    int DonortoRemove,
    EIT_ALL_DONORS * AllDonors)
{
    char * TempKey;
    double * TempValue;
	int TempGave;

    /* Put the selected donor row of values at the end of the table*/
    /* Since the input parameter decreases, then the number of     */
    /* rows that can be searched in AllDonors decreases and the    */
    /* last one, then the two last ones etc. are never available   */
    /* more than once.*/

    TempKey = AllDonors->Key[DonortoRemove];
    TempValue = AllDonors->Value[DonortoRemove];
	TempGave = AllDonors->Gave[DonortoRemove];

	//Trace debug
	/*if (strcmp(AllDonors->Key[DonortoRemove],"17205874") == 0)
	EI_AddMessage("DonorInternal", 4, "Reduce of Donor: %s %d %d", TempKey, TempValue, TempGave);*/

    AllDonors->Key[DonortoRemove] = AllDonors->Key[NumberofDonors-1];
    AllDonors->Value[DonortoRemove] = AllDonors->Value[NumberofDonors-1];
	AllDonors->Gave[DonortoRemove] = AllDonors->Gave[NumberofDonors-1];

    AllDonors->Key[NumberofDonors-1] = TempKey;
    AllDonors->Value[NumberofDonors-1] = TempValue;
	AllDonors->Gave[NumberofDonors-1] = TempGave;
}

/*******************/
/* Free DonorIndex */
/*******************/
static void FreeDonorIndex (
    int * DonorIndex)
{
    STC_FreeMemory (DonorIndex);
}

/*******************************************************/
/* Get the indices of the donors within AllRespondents */
/*******************************************************/
static int GetDonorIndex (
    EIT_ALL_RESPONDENTS * AllRespondents,
    int * DonorIndex)
{
    int i, j, l, NumberOfOKFields;

    l = 0;
    for (i = 0; i < AllRespondents->NumberofRespondents; i++) {
        NumberOfOKFields = 0;
        for (j = 0; j < AllRespondents->RespondentData[i].NumberofFields; j++) {
            if ((AllRespondents->RespondentData[i].StatusFlag[j] != FIELDFTI) &&
                (AllRespondents->RespondentData[i].StatusFlag[j] != FIELDMISS) &&
                (AllRespondents->RespondentData[i].StatusFlag[j] != FIELDNEG))
                NumberOfOKFields++;
        }
        if (NumberOfOKFields == AllRespondents->RespondentData[i].NumberofFields) {
            DonorIndex[l] = i;
            l++;
        }
    }
    return l;
}

/******************************************/
/* Get donor Keys from AllRespondent Keys */
/******************************************/
static int GetDonorKeys (
    long    NumberofDonors,
    int *   DonorIndex,
    char ** DonorKeys,
    char ** RespondentKeys)
{
    int i;

    for (i = 0; i < NumberofDonors; i++) {
        DonorKeys[i] = STC_StrDup (RespondentKeys[DonorIndex[i]]);
        if (DonorKeys[i] == NULL)
            return -1;
    }
    return 1;
}

/********************************************/
/* Get the data for the AllDonors structure */
/********************************************/
static int GetDonorValues (
    EIT_ALL_DONORS  *AllDonors,
    EIT_TRANSFORMED *AllTransformed)
{
    int i, j, TransformIndex;

    for (i = 0; i < AllDonors->NumberOfDonors; i++) {

        /* set to NULL to so free() works later */
        AllDonors->Value[i] = NULL;

        /* do not allocate and copy values if NumberofFields = 0 */
        if (AllDonors->NumberOfFields != 0) {
            AllDonors->Value[i] = STC_AllocateMemory (
                AllDonors->NumberOfFields * sizeof *AllDonors->Value[i]);
            if (AllDonors->Value[i] == NULL)
                return -1;

            for (j = 0; j < AllDonors->NumberOfFields; j++) {

                /* find donor in AllTransformed for that field */
                TransformIndex = UTIL_Binary (AllDonors->Key[i],
                    AllTransformed->TransformedValues[j].Key,
                    AllTransformed->TransformedValues[j].NumberofValues);
                if (TransformIndex == IS_NOT_FOUND) return -1;

                AllDonors->Value[i][j] =
                    AllTransformed->TransformedValues[j].Value[TransformIndex];
            }
        }
    }
    return 1;
}

/**************************************************/
/* Get the StatusFlag for the AllDonors structure */
/* (Done after GetDonorIndex)                     */
/**************************************************/
static int GetDonorStatusFlag (
    EIT_ALL_DONORS * AllDonors,
    int            * DonorIndex,
    EIT_DATAREC    * RespondentData)
{
    int i, j;
    int NumberofEditsFields;
    int RespondentId;

    /* Number of edits fields (the same for each donor) */
    /* Get the value from the respondent corresponding  */
    /* to the first DonorIndex.                         */
    RespondentId = DonorIndex[0];
    NumberofEditsFields = RespondentData[RespondentId].NumberofFields;

    for (i = 0; i < AllDonors->NumberOfDonors; i++) {

        RespondentId = DonorIndex[i];

        /* Get StatusFlag for all edits fields for this donor */
        AllDonors->DonorStatusFlag[i].NumberofFields = NumberofEditsFields;
        AllDonors->DonorStatusFlag[i].StatusFlag = STC_AllocateMemory (
            NumberofEditsFields *
                sizeof *AllDonors->DonorStatusFlag[i].StatusFlag);
        if (AllDonors->DonorStatusFlag[i].StatusFlag == NULL)
           return -1;

        for (j = 0; j < NumberofEditsFields; j++)
            AllDonors->DonorStatusFlag[i].StatusFlag[j] =
                RespondentData[RespondentId].StatusFlag[j];
     }

     return 1;
}
