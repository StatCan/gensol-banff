#include <stdlib.h>

#include "EI_Common.h"
#include "EI_Donor.h"

#include "STC_Memory.h"

void EI_FreeDonors (
    EIT_ALL_DONORS * AllDonors)
{
    int i;

    for(i=0;i<AllDonors->NumberOfDonors;i++)
    {
        STC_FreeMemory(AllDonors->Key[i]);
        STC_FreeMemory(AllDonors->Value[i]);
        STC_FreeMemory(AllDonors->DonorStatusFlag[i].StatusFlag);
    }
    STC_FreeMemory(AllDonors->Key);
    STC_FreeMemory(AllDonors->Value);
    STC_FreeMemory(AllDonors->DonorStatusFlag);
	STC_FreeMemory(AllDonors->Gave);
}

void EI_FreeTransformed (
    EIT_TRANSFORMED *AllTransformed)
{
    int i;

    for(i=0;i<AllTransformed->NumberofFields;i++) {
        STC_FreeMemory(AllTransformed->TransformedValues[i].Key);
        STC_FreeMemory(AllTransformed->TransformedValues[i].Value);
    }
    STC_FreeMemory(AllTransformed->TransformedValues);
}
