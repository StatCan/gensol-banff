#include <stdlib.h>

#include "EI_Donor.h"

/*********************************************************************
*********************************************************************/
int EI_CalculateDonorLimit (
	int NLimit,
	double Mrl,
	int NumberOfRecipients,
	int NumberOfDonors)
{
	int DonorLimit;

	if (NLimit == -1 && Mrl == -1.0)
		/* NLimit AND Mrl not specified, set DonorLimit to a big number */
		DonorLimit = INT_MAX;
	else {
		/* NLimit AND / OR Mrl specified, Calculate DonorLimit */
		DonorLimit = 1;
		if (NLimit != -1)
			DonorLimit = NLimit;
		if (Mrl != -1.0) {
			double temp;
			temp = ceil (Mrl * ceil ((double) NumberOfRecipients / NumberOfDonors));
			if ((int) temp > DonorLimit)
				DonorLimit = (int) temp;
		}
	}

	return DonorLimit;
}
/*********************************************************************
*********************************************************************/
int EI_CountDonorsReachedDonorLimit (
	EIT_ALL_DONORS * AllDonors,
	int DonorLimit)
{
	int i, n;

	n = 0;
	for (i = 0; i < AllDonors->NumberOfDonors; i++) {
		if (AllDonors->Gave[i] == DonorLimit)
			n++;
	}

	return n;
}
/***************************************
Is there enough donors
****************************************/
EIT_BOOLEAN EI_EnoughDonors (
    int NumberOfRespondents,
    int NumberOfRecipients,
    int MinimumNumberOfDonors,
    double MinimumPercentOfDonors)
{
    int NumberOfDonors;
    double PercentOfDonors;

    NumberOfDonors = NumberOfRespondents - NumberOfRecipients;
    PercentOfDonors = (double) NumberOfDonors / NumberOfRespondents * 100.0;

    if (NumberOfDonors < MinimumNumberOfDonors || EIM_DBL_LT (PercentOfDonors, MinimumPercentOfDonors))
        return EIE_FALSE;

    return EIE_TRUE;
}
