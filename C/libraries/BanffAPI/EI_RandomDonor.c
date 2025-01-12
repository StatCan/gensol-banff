/*--------------------------------------------------------------------*/
/* DESCRIPTION: For a recipient without matching fields, randomly     */
/*              select a donor amongst all donors:                    */
/*              - V1 : apply the post-imputation edits,               */
/*              - V2 : no verification about post-imputation edits.   */
/*              Do this until a suitable donor has been found and     */
/*              return the donor index.                               */
/*              tried.                                                */
/* Pre-requisite:                                                     */
/*  1. Observation processed must be a recipient.                     */
/*  2. Structure EIT_ALL_RESPONDENTS must be sorted in ascending      */
/*     order for the "key".                                           */
/*--------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"
#include "EI_Edits.h"
#include "EI_Message.h"

#include "EIP_Common.h"

#include "Donor.h"
#include "util.h"

#include "MessageBanffAPI.h"

EIT_RANDOMDONOR_RETURNCODE EI_RandomDonorWithPostEdits (
    EIT_DATAREC * RecipientData,
    int * FTIFields,
    EIT_ALL_RESPONDENTS * AllRespondents,
    EIT_EDITS * PostEdits,
    EIT_ALL_DONORS * AllDonors,
    int * RamdomDonorRow,
	int DonorLimit,
    int RandNumFlag,            /* 1 if user has included "randnum" statement, 0 otherwise */
    tSList   * randkey_list,    /* tSList sorted in ascending order containing the keys of all of the records read for the current by group */
    tADTList * randnum_list,    /* tADTList sorted in the same order as "randkey_list" containing the value of the random number field of all of the records read for the current by group */
    char * AllRecipients_Key_i, /* key of recipient for which a matching donor is sought */
    int LoggingVerbosityLevel)
{
    int i;
    int donor_row;
    int l;
    int rcode;
    
    char * Key;
    int index_in_list;
    double RandNum;
    double * address_of_randnum_matching_key;
    
    for (i = AllDonors->NumberOfDonors; i > 0; i--) {
		if (!RandNumFlag) {
			donor_row = (int) UTIL_Random (0, i); /* <-- expands to "donor_row = 0+(long)(UTIL_Rand ()*(i-0));" */
		} else {

			if (i > 1) {
				/*-assign the value of the random number from the recipient record to   */
				/* the variable "RandNum" (all of the random numbers are doubles        */
				/* between 0.0 and 1.0) and use it to decide the order in the sequence  */
				/* tied records of the record that will be selected):                   */
				RandNum = -1;
				Key = AllRecipients_Key_i;
				index_in_list = SList_StringSearchBinary(randkey_list, Key);
				if (index_in_list == SLIST_NOTFOUND) {
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorlTslistLookupFail "\n");
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorNotFound "\n");
					return EIE_RANDOMDONOR_FAIL;
				}
				address_of_randnum_matching_key = (double *)ADTList_Index(randnum_list, index_in_list);
				if (address_of_randnum_matching_key == NULL) {
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorTadtlistIndexFail "\n");
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorNotFound "\n");
					return EIE_RANDOMDONOR_FAIL;
				}
				RandNum = *address_of_randnum_matching_key;
				if (RandNum < 0.0) {
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorLessThanZero "\n", RandNum);
					return EIE_RANDOMDONOR_FAIL;
				}
				else if (RandNum > 1.0) {
					printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarRandomDonorGreaterThanOne "\n", RandNum);
					return EIE_RANDOMDONOR_FAIL;
				}
				else if (RandNum > (1.0 - DONOR_RANDNUM_FUZZ)) {
					printf(SAS_MESSAGE_PREFIX_NOTE MsgRandnumvarRandomDonorInvalidRange "\n");
					RandNum = 1.0 - DONOR_RANDNUM_FUZZ;
				}
				donor_row = 0+(long)(RandNum*(i-0));
			} else {
				donor_row = 0;
			}
		}
		if (LoggingVerbosityLevel > 0) {
			printf("next donor considered for recipient \"%s\" (randomly from all donors): \"%s\"\n", AllRecipients_Key_i, (AllDonors->Key)[donor_row]);
		}

		//Trace debug
		/*if (strcmp(AllDonors->Key[donor_row],"17205874") == 0)
		EI_AddMessage("EI_RandomDonorWithPostEdits", 4, "before: %s %d", AllDonors->Key[donor_row], AllDonors->Gave[donor_row]);*/

		if (AllDonors->Gave[donor_row] < DonorLimit) {
			//Trace debug
			/*if (strcmp(AllDonors->Key[donor_row],"17205874") == 0)
		    EI_AddMessage("EI_RandomDonorWithPostEdits", 4, "after : %s %d %d", AllDonors->Key[donor_row], AllDonors->Gave[donor_row], DonorLimit);*/

			if (DONOR_HasFIELDFTE(AllDonors, donor_row, FTIFields) == EIE_FALSE) {

				/* find donor in AllRespondents */
				l = UTIL_Binary (AllDonors->Key[donor_row], AllRespondents->Key,
					AllRespondents->NumberofRespondents);

				DONOR_CopyDonorValues (FTIFields,
					AllRespondents->RespondentData[l].FieldValue,
					RecipientData->FieldValue);

				/* try the donor */
				rcode = EI_EditsApplyOnValues (PostEdits, RecipientData->FieldValue);
				if (rcode == EIE_EDITSAPPLY_SUCCEED) {

					*RamdomDonorRow = donor_row;

					return EIE_RANDOMDONOR_SUCCEED;
				}
			}
		}

        DONOR_ReduceChoiceofDonors (i, donor_row, AllDonors);
    }

    return EIE_RANDOMDONOR_NOTFOUND;
}

EIT_RANDOMDONOR_RETURNCODE EI_RandomDonorWithoutPostEdits (
    EIT_DATAREC * RecipientData,
    int * FTIFields,
    EIT_ALL_RESPONDENTS * AllRespondents,
    EIT_ALL_DONORS * AllDonors,
    int * RamdomDonorRow,
	int DonorLimit)
{
    int i;
    int donor_row;
    int l;

    for (i = AllDonors->NumberOfDonors; i > 0; i--) {

		donor_row = (int) UTIL_Random (0, i);

		//Trace debug
		/*EI_AddMessage("EI_RandomDonorWithPostEdits", 4, "%s %d", AllDonors->Key[donor_row], AllDonors->Gave[donor_row]);*/

		if (AllDonors->Gave[donor_row] < DonorLimit) {

			if (DONOR_HasFIELDFTE(AllDonors, donor_row, FTIFields) == EIE_FALSE) {

				/* find donor in AllRespondents */
				l = UTIL_Binary (AllDonors->Key[donor_row],AllRespondents->Key,
					AllRespondents->NumberofRespondents);

				DONOR_CopyDonorValues (FTIFields,
					AllRespondents->RespondentData[l].FieldValue,
					RecipientData->FieldValue);

				*RamdomDonorRow = donor_row;

				return EIE_RANDOMDONOR_SUCCEED;
			}
		}

        DONOR_ReduceChoiceofDonors (i, donor_row, AllDonors);
    }

    return EIE_RANDOMDONOR_NOTFOUND;
}
