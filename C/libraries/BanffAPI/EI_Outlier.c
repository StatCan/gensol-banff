#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "CD.h"
#include "EI_Common.h"
#include "EI_Outlier.h"
#include "EI_Message.h"
#include "EI_UserData.h"
#include "EIP_Common.h"
#include "HT.h"
#include "STC_Memory.h"
#include "util.h"

#include "MessageBanffAPI.h"

/*
set DEBUG to 1 print debugging statements.
set DEBUG to 0 to silence the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

#define OUTLIER_MAX(a,b) ((a)<(b)?(b):(a))


static double CalculatePercentile (double * Variables, int Count, double Percentile);
static EIT_RETURNCODE CalculateDeviationMAD (double Current[], int Count, double * Deviation);
static EIT_RETURNCODE CalculateDeviationSTD (double Current[], int Count, double * Deviation);
static void CDCalculateBoundaries (CDT_CURRENTDATA * CD,
	double MEI, double MII, double MinimumDistanceMultiplier,
	double * LeftODIBoundary, double * LeftODEBoundary, double * RightODEBoundary, double * RightODIBoundary,
	T_OUTSUMINFO * OutSumData);
static EIT_RETURNCODE CDReadData (CDT_CURRENTDATA * CD, EIT_CDREADCALLBACK ReadFunction);
static int CompareAscending (const void *pa, const void *pb);
static EIT_WRITECALLBACK_RETURNCODE DefaultWriteStatus (DSW_generic* dsw_outstatus, DSW_generic* dsw_outstatus_detailed,
	void * UserData, char * Id, EIT_STATUS Status,
    double Current, double Previous, double Weight, double Effect,
	double LeftODIBoundary, double LeftODEBoundary, double RightODEBoundary, double RightODIBoundary,
    double Excl_Sigmagap, double Imp_Sigmagap, double Gap);
static double Effect (double Current, double Previous, double MedianRatio, double Exponent);
static double HBBoundary (EIT_OUTLIER_SIDE Side, double Median, double Quartile,
	double ImputationMultiplier, double MinimumDistanceMultiplier);
static void HTCalculateBoundaries (HTT_HISTORICALTREND * HT,
	double MEI, double MII, double MinimumDistanceMultiplier, double Exponent,
	double * LeftODIBoundary, double * LeftODEBoundary, double * RightODEBoundary, double * RightODIBoundary,
	T_OUTSUMINFO * OutSumData);
static EIT_RETURNCODE HTReadData (HTT_HISTORICALTREND * HT, EIT_HTREADCALLBACK ReadFunction);
static void OutSum_SetMissing(T_OUTSUMINFO *OutSumData);
static void PrintHBBoundaries (double LeftODIBoundary, double LeftODEBoundary,
	double RightODEBoundary, double RightODIBoundary);
static void PrintSGBoundaries (EIT_OUTLIER_SIDE Side, double BetaE, double BetaI,
	double LeftODIBoundary, double LeftODEBoundary, double RightODEBoundary, double RightODIBoundary);
static void PrintSGBoundary (char * Title, double Boundary);
static void PrintSGGaps (double Deviation, double BetaE, double Excl_Sigmagap, double BetaI, double Imp_Sigmagap);
static char * Status (EIT_OUTLIER_SIDE Side, double Value, double LeftODIBoundary,
	double LeftODEBoundary, double RightODEBoundary, double RightODIBoundary);
static double TransformRatio (double Ratio, double MedianRatio);


/*------------------------------------------------------------------------------
Identify outlying observations by the Hidiroglou-Berthelot current method.
------------------------------------------------------------------------------*/
EIT_RETURNCODE EI_CDOutlier (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	EIT_CDREADCALLBACK ReadData,
	EIT_WRITESTATUS_CALLBACK WriteStatus,
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	int MinObs,
	EIT_BOOLEAN IsWeighted)
{
	CDT_CURRENTDATA * CD;
	int i;
	double LeftODIBoundary;
	double LeftODEBoundary;
	EIT_WRITESTATUS_CALLBACK LocalWriteStatus;
	double RightODEBoundary;
	double RightODIBoundary;
	char * VariableStatus;
	void * UserData;
	double NFTI;
	double NFTE;

	UserData = EI_GetUserData ();

	if (WriteStatus == NULL)
		LocalWriteStatus = DefaultWriteStatus;
	else
		LocalWriteStatus = WriteStatus;

	CD = CD_Alloc (IsWeighted);
	if (CD == NULL) return EIE_FAIL;

	if (CDReadData (CD, ReadData) != EIE_SUCCEED) {
		CD_Free (CD);
		return EIE_FAIL;
	}

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10018 "\n", CD->NumberEntries);

	if (CD->NumberEntries < MinObs) {
		//not enough obs.
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_WARNING, M20026 "\n", MinObs);

		/* prepare observation for Out Summary */
		OutSumData->NUsed = (double)CD->NumberEntries;
		OutSum_SetMissing(OutSumData);

		CD_Free (CD);
		return EIE_SUCCEED;
	}

	/* OutSumData will retain the Q1, M, and Q3 values */
	CDCalculateBoundaries (CD, MEI, MII, MinimumDistanceMultiplier,
		&LeftODIBoundary, &LeftODEBoundary, &RightODEBoundary, &RightODIBoundary, OutSumData);

	/* Initialize count of NFT(I|E) for OutSummary dataset */
	NFTI = 0;
	NFTE = 0;
	for (i = 0; i < CD->NumberEntries; i++) {
		VariableStatus = Status (Side, CD->Current[i], LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary);
		/* count number of NFT(I|E) based on each variable's status */
		if (VariableStatus[2] == 'I') {
			NFTI += 1;
		}
		else if (VariableStatus[2] == 'E') {
			NFTE += 1;
		}
		LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
			UserData, CD->Id[i], VariableStatus,
			(CD->IsWeighted ? CD->UnweightedCurrent[i] : CD->Current[i]),
			EIM_MISSING_VALUE, //Previous,
			(CD->IsWeighted ? CD->Weight[i] : 1.0),
			EIM_MISSING_VALUE, //Effect,
			LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
			EIM_MISSING_VALUE, //Excl_Sigmagap,
			EIM_MISSING_VALUE, //Imp_Sigmagap
			EIM_MISSING_VALUE); //gap
	}

	/* prepare observation for Out Summary */
	OutSumData->NUsed       = (double)CD->NumberEntries;
	OutSumData->NFTI        = NFTI;
	OutSumData->NFTE        = NFTE;
	OutSumData->IMP_BND_L   = LeftODIBoundary;
	OutSumData->EXCL_BND_L  = LeftODEBoundary;
	OutSumData->EXCL_BND_R  = RightODEBoundary;
	OutSumData->IMP_BND_R   = RightODIBoundary;

	CD_Free (CD);

	LocalWriteStatus = NULL;

	return EIE_SUCCEED;
}
/*------------------------------------------------------------------------------
Identify outlying observations by the Hidiroglou-Berthelot historical trend method.
------------------------------------------------------------------------------*/
EIT_RETURNCODE EI_HTOutlier (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	EIT_HTREADCALLBACK ReadData,
	EIT_WRITESTATUS_CALLBACK WriteStatus,
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	int MinObs,
	double Exponent,
	EIT_BOOLEAN IsWeighted)
{
	HTT_HISTORICALTREND * HT;
	int i;
	double LeftODIBoundary;
	double LeftODEBoundary;
	EIT_WRITESTATUS_CALLBACK LocalWriteStatus;
	double RightODEBoundary;
	double RightODIBoundary;
	char * VariableStatus;
	void * UserData;
	double NFTI;
	double NFTE;

	UserData = EI_GetUserData ();

	if (WriteStatus == NULL)
		LocalWriteStatus = DefaultWriteStatus;
	else
		LocalWriteStatus = WriteStatus;

	HT = HT_Alloc (IsWeighted);
	if (HT == NULL) return EIE_FAIL;

	if (HTReadData (HT, ReadData) != EIE_SUCCEED) {
		HT_Free (HT);
		return EIE_FAIL;
	}

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10018 "\n", HT->NumberEntries);

	if (HT->NumberEntries < MinObs) {
		//not enough obs.
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_WARNING, M20026 "\n", MinObs);

		/* prepare observation for Out Summary */
		OutSumData->NUsed = (double)HT->NumberEntries;
		OutSum_SetMissing(OutSumData);

		HT_Free (HT);
		return EIE_SUCCEED;
	}

	/* OutSumData will retain the Q1, M, and Q3 values */
	HTCalculateBoundaries (HT, MEI, MII, MinimumDistanceMultiplier, Exponent,
		&LeftODIBoundary, &LeftODEBoundary, &RightODEBoundary, &RightODIBoundary, OutSumData);

	/* Initialize count of NFT(I|E) for OutSummary dataset */
	NFTI = 0;
	NFTE = 0;
	for (i = 0; i < HT->NumberEntries; i++) {
		VariableStatus = Status (Side, HT->Effect[i], LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary);
		/* count number of NFT(I|E) based on each variable's status */
		if (VariableStatus[2] == 'I') {
			NFTI += 1;
		}
		else if (VariableStatus[2] == 'E') {
			NFTE += 1;
		}
		LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
			UserData, HT->Id[i], VariableStatus,
			HT->Current[i],
			HT->Previous[i],
			(HT->IsWeighted ? HT->Weight[i] : 1.0),
			HT->Effect[i],
			LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
			EIM_MISSING_VALUE, //Excl_Sigmagap,
			EIM_MISSING_VALUE, //Imp_Sigmagap
			EIM_MISSING_VALUE); //gap
	}

	/* prepare observation for Out Summary */
	OutSumData->NUsed       = (double)HT->NumberEntries;
	OutSumData->NFTI        = NFTI;
	OutSumData->NFTE        = NFTE;
	OutSumData->IMP_BND_L   = LeftODIBoundary;
	OutSumData->EXCL_BND_L  = LeftODEBoundary;
	OutSumData->EXCL_BND_R  = RightODEBoundary;
	OutSumData->IMP_BND_R   = RightODIBoundary;
	
	HT_Free (HT);

	LocalWriteStatus = NULL;

	return EIE_SUCCEED;
}
/*------------------------------------------------------------------------------
Identify outlying observations by the sigma-gap current method.
------------------------------------------------------------------------------*/
EIT_RETURNCODE EI_OutlierSigmagap (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	EIT_CDREADCALLBACK ReadData,
	EIT_WRITESTATUS_CALLBACK WriteStatus,
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	EIT_OUTLIER_SIGMA Sigma,
	double BetaE,
	double BetaI,
	double StartCentile,
	int MinObs,
	EIT_BOOLEAN IsWeighted)
{
	CDT_CURRENTDATA * CD;
	double Deviation;
	double Excl_Sigmagap;
	int i;
	double Imp_Sigmagap;
	int LeftExclusionIndex;
	int LeftImputationIndex;
	double LeftODEBoundary;
	double LeftODIBoundary;
	int RightExclusionIndex;
	int RightImputationIndex;
	double RightODIBoundary;
	double RightODEBoundary;
	EIT_WRITESTATUS_CALLBACK LocalWriteStatus;
	int StartingPointIndex;
	double StartingPointLeft;
	double StartingPointRight;
	void * UserData;
	double NFTI;
	double NFTE;

	/* Initialize count of NFT(I|E) for OutSummary dataset 
		These counters are increased in various places throughout this function */
	NFTI = 0;
	NFTE = 0;

	UserData = EI_GetUserData ();

	if (WriteStatus == NULL)
		LocalWriteStatus = DefaultWriteStatus;
	else
		LocalWriteStatus = WriteStatus;

	CD = CD_Alloc (IsWeighted);
	if (CD == NULL) return EIE_FAIL;

	if (CDReadData (CD, ReadData) != EIE_SUCCEED) {
		CD_Free (CD);
		return EIE_FAIL;
	}

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10018 "\n", CD->NumberEntries);

	if (CD->NumberEntries < MinObs) {
		//not enough obs.
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_WARNING, M20026 "\n", MinObs);

		/* prepare observation for Out Summary */
		OutSumData->NUsed = (double)CD->NumberEntries;
		OutSum_SetMissing(OutSumData);

		CD_Free (CD);
		return EIE_SUCCEED;
	}

	//CD_Print (CD);
	CD_Sort (CD);
	//CD_Print (CD);

	if (Sigma == EIE_OUTLIER_SIGMA_STD) {
		if (CalculateDeviationSTD (CD->Current, CD->NumberEntries, &Deviation) == EIE_FAIL) {
			CD_Free (CD);
			return EIE_FAIL;
		}
	}
	else { // !(Sigma == EIE_OUTLIER_SIGMA_STD)
		if (CalculateDeviationMAD (CD->Current, CD->NumberEntries, &Deviation) == EIE_FAIL) {
			CD_Free (CD);
			return EIE_FAIL;
		}
	}

	if (Deviation == 0.0) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, M10029);
	}

	if (BetaE != EIM_MISSING_VALUE)
		Excl_Sigmagap = Deviation * BetaE;
	else
		Excl_Sigmagap = EIM_MISSING_VALUE;
	if (BetaI != EIM_MISSING_VALUE)
		Imp_Sigmagap = Deviation * BetaI;
	else
		Imp_Sigmagap = EIM_MISSING_VALUE;

	PrintSGGaps (Deviation, BetaE, Excl_Sigmagap, BetaI, Imp_Sigmagap);

	LeftODEBoundary = LeftODIBoundary = RightODIBoundary = RightODEBoundary = EIM_MISSING_VALUE;

	if (Side == EIE_OUTLIER_SIDE_LEFT || Side == EIE_OUTLIER_SIDE_BOTH) {
		StartingPointLeft = CalculatePercentile (CD->Current, CD->NumberEntries, 100.0-StartCentile);

		//starting from the left find first item greater than or equal to start value
		for (i = 0; i < CD->NumberEntries - 1; i++) {
			if (CD->Current[i] >= StartingPointLeft) {
				break;
			}
		}
		StartingPointIndex = i;
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "LeftStartingPointIndex %3d", StartingPointIndex);

		LeftImputationIndex = -1;//LeftImputationIndex points prior to the first array item
		if (Imp_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i >= 1; i--) {
				if (CD->Current[i] - CD->Current[i-1] > Imp_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "il %3d Imputation %12.6f > %12.6f ?", i, CD->Current[i] - CD->Current[i-1], Imp_Sigmagap);
					LeftImputationIndex = i-1;//ImputationIndex points to the first ODI array item
					LeftODIBoundary = CD->Current[i] - CD->Current[i-1];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "LeftImputationIndex %3d", LeftImputationIndex);

		LeftExclusionIndex = -1;//LeftExclusionIndex points prior to the first array item
		if (Excl_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i >= LeftImputationIndex && i >= 1; i--) {
				if (CD->Current[i] - CD->Current[i-1] > Excl_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "el %3d  Exclusion %12.6f > %12.6f ?", i, CD->Current[i] - CD->Current[i-1], Excl_Sigmagap);
					LeftExclusionIndex = i-1;//ExclusionIndex points to the first ODE array item
					LeftODEBoundary = CD->Current[i] - CD->Current[i-1];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, " LeftExclusionIndex %3d", LeftExclusionIndex);
	}

	if (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) {
		StartingPointRight = CalculatePercentile (CD->Current, CD->NumberEntries, StartCentile);

		//starting from the right find first item less than or equal to start value
		for (i = CD->NumberEntries - 1; i >= 0; i--) {
			if (CD->Current[i] <= StartingPointRight) {
				break;
			}
		}
		StartingPointIndex = i;
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "RightStartingPointIndex %3d", StartingPointIndex);

		RightImputationIndex = CD->NumberEntries;//RightImputationIndex points passed the last array item
		if (Imp_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i < CD->NumberEntries - 1; i++) {
				if (CD->Current[i+1] - CD->Current[i] > Imp_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "ir %3d Imputation %12.6f > %12.6f ?", i, CD->Current[i+1] - CD->Current[i], Imp_Sigmagap);
					RightImputationIndex = i+1;//RightImputationIndex points to the first ODI array item
					RightODIBoundary = CD->Current[i+1] - CD->Current[i];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "RightImputationIndex %3d", RightImputationIndex);

		RightExclusionIndex = CD->NumberEntries;//RightExclusionIndex points passed the last array item
		if (Excl_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i < RightImputationIndex && i < CD->NumberEntries - 1; i++) {
				if (CD->Current[i+1] - CD->Current[i] > Excl_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "er %3d  Exclusion %12.6f > %12.6f ?", i, CD->Current[i+1] - CD->Current[i], Excl_Sigmagap);
					RightExclusionIndex = i+1;//RightExclusionIndex points to the first ODE array item
					RightODEBoundary = CD->Current[i+1] - CD->Current[i];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, " RightExclusionIndex %3d", RightExclusionIndex);
	}

	PrintSGBoundaries (Side, BetaE, BetaI, LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary);

	if (Side == EIE_OUTLIER_SIDE_LEFT || Side == EIE_OUTLIER_SIDE_BOTH) {
		/* write ODEL */
		double TempGap, CurrentGap;
		CurrentGap = 0.0;
		/*CD_Print(CD);*/ /*Debug*/

		for (i = LeftExclusionIndex; LeftImputationIndex + 1 <= i; i--) {
			TempGap = fabs (CD->Current[i+1] - CD->Current[i]);

			if (TempGap != 0.0) CurrentGap = TempGap;

			/*EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Index: %3d CValue: %12.6f TGap: %3.2f CGap: %3.2f", i, CD->Current[i], TempGap, CurrentGap);*/
			NFTE += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, CD->Id[i], EIM_STATUS_ODEL,
				CD->Current[i], //Current,
				EIM_MISSING_VALUE, //Previous,
				(CD->IsWeighted ? CD->Weight[i] : 1.0),
				EIM_MISSING_VALUE, //Effect,
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}

		/* write ODIL */		
		for (i = LeftImputationIndex; 0 <= i; i--) {
			TempGap = fabs (CD->Current[i+1] - CD->Current[i]);
				
			if (TempGap != 0.0) CurrentGap = TempGap;	

			/*EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Index: %3d CValue: %12.6f TGap: %3.2f CGap: %3.2f", i, CD->Current[i], TempGap, CurrentGap);*/
			NFTI += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, CD->Id[i], EIM_STATUS_ODIL,
				CD->Current[i], //Current,
				EIM_MISSING_VALUE, //Previous,
				(CD->IsWeighted ? CD->Weight[i] : 1.0),
				EIM_MISSING_VALUE, //Effect,
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}
	
	}

	if (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) {
		/* write ODER */
		double TempGap, CurrentGap;
		CurrentGap = 0.0;
		for (i = RightExclusionIndex; i < RightImputationIndex; i++) {
			TempGap = fabs (CD->Current[i] - CD->Current[i-1]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTE += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, CD->Id[i], EIM_STATUS_ODER,
				CD->Current[i], //Current,
				EIM_MISSING_VALUE, //Previous,
				(CD->IsWeighted ? CD->Weight[i] : 1.0),
				EIM_MISSING_VALUE, //Effect,
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}

		/* write ODIR */
		
		for (i = RightImputationIndex; i < CD->NumberEntries; i++) {
			TempGap = fabs (CD->Current[i] - CD->Current[i-1]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTI += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, CD->Id[i], EIM_STATUS_ODIR,
				CD->Current[i], //Current,
				EIM_MISSING_VALUE, //Previous,
				(CD->IsWeighted ? CD->Weight[i] : 1.0),
				EIM_MISSING_VALUE, //Effect,
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}
	}

	/* prepare observation for Out Summary */
	OutSumData->NUsed			= (double)CD->NumberEntries;
	OutSumData->NFTI			= NFTI;
	OutSumData->NFTE			= NFTE;
	OutSumData->IMP_BND_L		= LeftODIBoundary;
	OutSumData->EXCL_BND_L		= LeftODEBoundary;
	OutSumData->EXCL_BND_R		= RightODEBoundary;
	OutSumData->IMP_BND_R		= RightODIBoundary;
	OutSumData->SIGMA			= Deviation;
	OutSumData->IMP_SIGMAGAP	= Imp_Sigmagap;
	OutSumData->EXCL_SIGMAGAP	= Excl_Sigmagap;

	CD_Free (CD);

	LocalWriteStatus = NULL;

	return EIE_SUCCEED;
}
/*------------------------------------------------------------------------------
Identify outlying observations by the sigma-gap ratio method.
------------------------------------------------------------------------------*/
EIT_RETURNCODE EI_OutlierSigmagapRatio (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	EIT_HTREADCALLBACK ReadData,
	EIT_WRITESTATUS_CALLBACK WriteStatus,
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	EIT_OUTLIER_SIGMA Sigma,
	double BetaE,
	double BetaI,
	double StartCentile,
	int MinObs,
	EIT_BOOLEAN IsWeighted)
{
	double Deviation;
	double Excl_Sigmagap;
	HTT_HISTORICALTREND * HT;
	int i;
	double Imp_Sigmagap;
	int LeftExclusionIndex;
	int LeftImputationIndex;
	double LeftODEBoundary;
	double LeftODIBoundary;
	int RightExclusionIndex;
	int RightImputationIndex;
	double RightODIBoundary;
	double RightODEBoundary;
	EIT_WRITESTATUS_CALLBACK LocalWriteStatus;
	int StartingPointIndex;
	double StartingPointLeft;
	double StartingPointRight;
	void * UserData;
	double NFTI;
	double NFTE;

	/* Initialize count of NFT(I|E) for OutSummary dataset
		These counters are increased in various places throughout this function */
	NFTI = 0;
	NFTE = 0;

	UserData = EI_GetUserData ();

	if (WriteStatus == NULL)
		LocalWriteStatus = DefaultWriteStatus;
	else
		LocalWriteStatus = WriteStatus;

	HT = HT_Alloc (IsWeighted);
	if (HT == NULL) return EIE_FAIL;

	if (HTReadData (HT, ReadData) != EIE_SUCCEED) {
		HT_Free (HT);
		return EIE_FAIL;
	}

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10018 "\n", HT->NumberEntries);

	if (HT->NumberEntries < MinObs) {
		//not enough obs.
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_WARNING, M20026 "\n", MinObs);

		/* prepare observation for Out Summary */
		OutSumData->NUsed = (double)HT->NumberEntries;
		OutSum_SetMissing(OutSumData);

		HT_Free (HT);
		return EIE_SUCCEED;
	}

	//HT_Print (HT);
	HT_SortRatio (HT);
	//HT_Print (HT);

	if (Sigma == EIE_OUTLIER_SIGMA_STD) {
		if (CalculateDeviationSTD (HT->Ratio, HT->NumberEntries, &Deviation) == EIE_FAIL) {
			HT_Free (HT);
			return EIE_FAIL;
		}
	}
	else { // !(Sigma == EIE_OUTLIER_SIGMA_STD)
		if (CalculateDeviationMAD (HT->Ratio, HT->NumberEntries, &Deviation) == EIE_FAIL) {
			HT_Free (HT);
			return EIE_FAIL;
		}
	}

	if (Deviation == 0.0) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, M10029);
	}

	if (BetaE != EIM_MISSING_VALUE)
		Excl_Sigmagap = Deviation * BetaE;
	else
		Excl_Sigmagap = EIM_MISSING_VALUE;
	if (BetaI != EIM_MISSING_VALUE)
		Imp_Sigmagap = Deviation * BetaI;
	else
		Imp_Sigmagap = EIM_MISSING_VALUE;

	PrintSGGaps (Deviation, BetaE, Excl_Sigmagap, BetaI, Imp_Sigmagap);

	LeftODEBoundary = LeftODIBoundary = RightODIBoundary = RightODEBoundary = EIM_MISSING_VALUE;

	if (Side == EIE_OUTLIER_SIDE_LEFT || Side == EIE_OUTLIER_SIDE_BOTH) {
		StartingPointLeft = CalculatePercentile (HT->Ratio, HT->NumberEntries, 100.0-StartCentile);

		//starting from the left find first item greater than or equal to start value
		for (i = 0; i < HT->NumberEntries - 1; i++) {
			if (HT->Ratio[i] >= StartingPointLeft) {
				break;
			}
		}
		StartingPointIndex = i;
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Left StartingPointIndex %3d", StartingPointIndex);

		LeftImputationIndex = -1;//LeftImputationIndex points prior to the first array item
		if (Imp_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i >= 1; i--) {
				if (HT->Ratio[i] - HT->Ratio[i-1] > Imp_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "il %3d Imputation %12.6f > %12.6f ?", i, HT->Ratio[i] - HT->Ratio[i-1], Imp_Sigmagap);
					LeftImputationIndex = i-1;//ImputationIndex points to the first ODI array item
					LeftODIBoundary = HT->Ratio[i] - HT->Ratio[i-1];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Left ImputationIndex %3d", LeftImputationIndex);

		LeftExclusionIndex = -1;//LeftExclusionIndex points prior to the first array item
		if (Excl_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i >= LeftImputationIndex && i >= 1; i--) {
				if (HT->Ratio[i] - HT->Ratio[i-1] > Excl_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "el %3d  Exclusion %12.6f > %12.6f ?", i, HT->Ratio[i] - HT->Ratio[i-1], Excl_Sigmagap);
					LeftExclusionIndex = i-1;//ExclusionIndex points to the first ODE array item
					LeftODEBoundary = HT->Ratio[i] - HT->Ratio[i-1];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, " Left ExclusionIndex %3d", LeftExclusionIndex);
	}

	if (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) {
		StartingPointRight = CalculatePercentile (HT->Ratio, HT->NumberEntries, StartCentile);

		//starting from the right find first item less than or equal to start value
		for (i = HT->NumberEntries - 1; i >= 0; i--) {
			if (HT->Ratio[i] <= StartingPointRight) {
				break;
			}
		}
		StartingPointIndex = i;
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Right StartingPointIndex %3d", StartingPointIndex);

		RightImputationIndex = HT->NumberEntries;//RightImputationIndex points passed the last array item
		if (Imp_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i < HT->NumberEntries - 1; i++) {
				if (HT->Ratio[i+1] - HT->Ratio[i] > Imp_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "ir %3d Imputation %12.6f > %12.6f ?", i, HT->Ratio[i+1] - HT->Ratio[i], Imp_Sigmagap);
					RightImputationIndex = i+1;//RightImputationIndex points to the first ODI array item
					RightODIBoundary = HT->Ratio[i+1] - HT->Ratio[i];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Right ImputationIndex %3d", RightImputationIndex);

		RightExclusionIndex = HT->NumberEntries;//RightExclusionIndex points passed the last array item
		if (Excl_Sigmagap != EIM_MISSING_VALUE) {
			for (i = StartingPointIndex; i < RightImputationIndex && i < HT->NumberEntries - 1; i++) {
				if (HT->Ratio[i+1] - HT->Ratio[i] > Excl_Sigmagap) {
					if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "er %3d  Exclusion %12.6f > %12.6f ?", i, HT->Ratio[i+1] - HT->Ratio[i], Excl_Sigmagap);
					RightExclusionIndex = i+1;//RightExclusionIndex points to the first ODE array item
					RightODEBoundary = HT->Ratio[i+1] - HT->Ratio[i];
					break;
				}
			}
		}
		if (DEBUG) EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, " Right ExclusionIndex %3d", RightExclusionIndex);
	}

	PrintSGBoundaries (Side, BetaE, BetaI, LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary);

	if (Side == EIE_OUTLIER_SIDE_LEFT || Side == EIE_OUTLIER_SIDE_BOTH) {
		/* write ODEL */
		double TempGap, CurrentGap;
		CurrentGap = 0.0;
		for (i = LeftExclusionIndex; LeftImputationIndex + 1 <= i; i--) {
			TempGap = fabs (HT->Ratio[i+1] - HT->Ratio[i]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTE += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, HT->Id[i], EIM_STATUS_ODEL,
				HT->Current[i],
				HT->Previous[i],
				(HT->IsWeighted ? HT->Weight[i] : 1.0),
				HT->Effect[i],
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}

		/* write ODIL */
		for (i = LeftImputationIndex; 0 <= i; i--) {
			TempGap = fabs (HT->Ratio[i+1] - HT->Ratio[i]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTI += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, HT->Id[i], EIM_STATUS_ODIL,
				HT->Current[i],
				HT->Previous[i],
				(HT->IsWeighted ? HT->Weight[i] : 1.0),
				HT->Effect[i],
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}
	}

	if (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) {
		/* write ODER */
		double TempGap, CurrentGap;
		CurrentGap = 0.0;
		for (i = RightExclusionIndex; i < RightImputationIndex; i++) {
			TempGap = fabs (HT->Ratio[i] - HT->Ratio[i-1]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTE += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, HT->Id[i], EIM_STATUS_ODER,
				HT->Current[i],
				HT->Previous[i],
				(HT->IsWeighted ? HT->Weight[i] : 1.0),
				HT->Effect[i],
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}

		/* write ODIR */
		for (i = RightImputationIndex; i < HT->NumberEntries; i++) {
			TempGap = fabs (HT->Ratio[i] - HT->Ratio[i-1]);
			if (TempGap != 0.0) CurrentGap = TempGap;
			NFTI += 1;
			LocalWriteStatus (dsw_outstatus, dsw_outstatus_detailed,
				UserData, HT->Id[i], EIM_STATUS_ODIR,
				HT->Current[i],
				HT->Previous[i],
				(HT->IsWeighted ? HT->Weight[i] : 1.0),
				HT->Effect[i],
				LeftODIBoundary, LeftODEBoundary, RightODEBoundary, RightODIBoundary,
				Excl_Sigmagap, Imp_Sigmagap, CurrentGap
				);
		}
	}

	/* prepare observation for Out Summary */
	OutSumData->NUsed           = (double)HT->NumberEntries;
	OutSumData->NFTI            = NFTI;
	OutSumData->NFTE            = NFTE;
	OutSumData->IMP_BND_L       = LeftODIBoundary;
	OutSumData->EXCL_BND_L      = LeftODEBoundary;
	OutSumData->EXCL_BND_R      = RightODEBoundary;
	OutSumData->IMP_BND_R       = RightODIBoundary;
	OutSumData->SIGMA			= Deviation;
	OutSumData->IMP_SIGMAGAP    = Imp_Sigmagap;
	OutSumData->EXCL_SIGMAGAP   = Excl_Sigmagap;

	HT_Free (HT);

	LocalWriteStatus = NULL;

	return EIE_SUCCEED;
}
/*
Calcule un pourcentile selon la formule
percentile = (1-D)*Valeur a la position W + D*Valeur a la position W+1
ou W et D sont egale a
Percentile/100 * (Count+1) = W.D

Entrées:
	Tableau de valeurs
	Nombre d'éléments dans le tableau
	Pourcentile voulu (valeur de 1 a 99)
Sorties:
	Aucune
Retourne:
	pourcentile

Voir algorythme dans manuel
GEIS Functional description of the generalized edit and imputation system
Appendix A
*/
static double CalculatePercentile (
	double * Current,
	int Count,
	double Percentile)
{
	double D;
	double T;
	int   W;

	/* if T = 2.333 then W=2 and D=0.333 */
	T = (Percentile/100.00) * (Count+1);
	W = (int)T; /* Keep integer part only */
	D = T-W;

	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "trace D=%f 1-D=%f W=%d Count=%d", D, 1.0-D, W, Count);

	if (W < 1) {
		//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "trace W est trop petit. retourne Current[0] = %f" , Current[0]);
		return Current[0];//first element
	}
	else if (W >= Count) {
		//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "trace W est trop grand. retourne Current[W-1] = %f" , Current[W-1]);
		return Current[Count-1];//last element
	}

	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "((1-D) * Current[W-1]) + (D * Current[W])");
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "((%f) * Current[%d]) + (%f * Current[%d])", 1.0-D, W-1, D, W);
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "((%f) * %f) + (%f * %f)", 1-D, Current[W-1], D, Current[W]);
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "(%f) + (%f)", ((1.0-D) * Current[W-1]), (D * Current[W]));
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "(%f)", ((1.0-D) * Current[W-1]) + (D * Current[W]));

	/* ici W-1 et W parce que les vecteurs en C sont indexé a partir de 0. */
	return ((1.0-D) * Current[W-1]) + (D * Current[W]);//weighted average
}
/*
Calculate the MAD deviation, Median Absolute Deviation
MAD=med(|x-med(x)|)
precondition: Current is already sorted
*/
static EIT_RETURNCODE CalculateDeviationMAD (
	double Current[],
	int Count,
	double * Deviation)
{
	int i;
	double Median;
	double * v;

	v = STC_AllocateMemory (Count * sizeof *v);
	if (v == NULL) return EIE_FAIL;

	Median = CalculatePercentile (Current, Count, 50.0);

	for (i = 0; i < Count; i++) {
		v[i] = Current[i] - Median;
		if (v[i] < 0.0) v[i] = -v[i];
	}

	//EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "v not sorted");
	//for (i = 0; i < Count; i++) {
	//	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "i=%d v=%f", i, v[i]);
	//}
	qsort (v, Count, sizeof *v, CompareAscending);
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "v sorted");
	//for (i = 0; i < Count; i++) {
	//	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "i=%d v=%f", i, v[i]);
	//}
	//EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "");

	*Deviation = 1.4826*CalculatePercentile (v, Count, 50.0);

	STC_FreeMemory (v);

	return EIE_SUCCEED;
}
/*
Calculate the standard deviation.
return EIT_RETURNCODE to mimic CalculateDeviationMAD
*/
static EIT_RETURNCODE CalculateDeviationSTD (
	double Current[],
	int Count,
	double * Deviation)
{
	double Average;
	double ep;
	int i;
	double s;
	double Variance;
	double Sum;

	if (Count <= 1) {
		*Deviation = 0.0;//prevents division by zero. defensive programming only since outlier stops when # obs < 3
	}
	else {
		Sum = 0.0;
		for (i = 0; i < Count; i++) {
			Sum += Current[i];
		}
		Average = Sum/Count;

		ep = 0.0;
		Variance = 0.0;
		for (i = 0; i < Count; i++) {
			s = Current[i] - Average;
			ep += s;
			Variance += (s*s);
		}
		Variance = (Variance-ep*ep/Count)/(Count-1);//it's safe of division by zero !
		*Deviation = sqrt (Variance);
	}
	return EIE_SUCCEED;
}
/*
Calculate boundaries for current data method.
*/
static void CDCalculateBoundaries (
	CDT_CURRENTDATA * CD,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	double * LeftODIBoundary,
	double * LeftODEBoundary,
	double * RightODEBoundary,
	double * RightODIBoundary,
	T_OUTSUMINFO * OutSumData
	)
{
	double FirstQuartile;
	double Median;
	double ThirdQuartile;

	//CD_Print (CD);
	CD_Sort (CD);
	//CD_Print (CD);

	FirstQuartile = CalculatePercentile (CD->Current, CD->NumberEntries, 25);
	Median = CalculatePercentile (CD->Current, CD->NumberEntries, 50);
	ThirdQuartile = CalculatePercentile (CD->Current, CD->NumberEntries, 75);

	/* retain these values for use on OutSummary dataset */
	OutSumData->Q1 = FirstQuartile;
	OutSumData->M = Median;
	OutSumData->Q3 = ThirdQuartile;

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION,
		"%s %.5f\n%s %.5f\n%s %.5f\n",
		M10019, FirstQuartile,
		M10020, Median,
		M10021, ThirdQuartile);

	if (EIM_DBL_EQ (Median, 0.0))
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10022 "\n");

	*LeftODIBoundary  = HBBoundary (EIE_OUTLIER_SIDE_LEFT, Median, FirstQuartile, MII, MinimumDistanceMultiplier);
	*LeftODEBoundary  = HBBoundary (EIE_OUTLIER_SIDE_LEFT, Median, FirstQuartile, MEI, MinimumDistanceMultiplier);
	*RightODEBoundary = HBBoundary (EIE_OUTLIER_SIDE_RIGHT, Median, ThirdQuartile, MEI, MinimumDistanceMultiplier);
	*RightODIBoundary = HBBoundary (EIE_OUTLIER_SIDE_RIGHT, Median, ThirdQuartile, MII, MinimumDistanceMultiplier);

	PrintHBBoundaries (*LeftODIBoundary, *LeftODEBoundary, *RightODEBoundary, *RightODIBoundary);
}
/*
Call the read callback function until it fails or source is exhausted.
*/
static EIT_RETURNCODE CDReadData (
	CDT_CURRENTDATA * CD,
	EIT_CDREADCALLBACK ReadFunction)
{
	double Current;
	char * Id;
	EIT_READCALLBACK_RETURNCODE rc;
	void * UserData;
	double Weight;

	UserData = EI_GetUserData ();
	rc = ReadFunction (UserData, &Id, &Current, &Weight);
	if (rc == EIE_READCALLBACK_FAIL) return EIE_FAIL;
	while (rc == EIE_READCALLBACK_FOUND) {
		CD = CD_Add (CD, Id, Current, Weight);
		if (CD == NULL) return EIE_FAIL;

		rc = ReadFunction (UserData, &Id, &Current, &Weight);
		if (rc == EIE_READCALLBACK_FAIL) return EIE_FAIL;
	}

	return EIE_SUCCEED;
}
/*
Compares two double, ascending
*/
static int CompareAscending (
	const void *pa,
	const void *pb)
{
	double a = *(double *)pa;
	double b = *(double *)pb;
	if (a < b)
		return -1;
	else if (a > b)
		return 1;
	return 0;
}
/*
default write status function, it's the callers responsability to overright it.
*/
static EIT_WRITECALLBACK_RETURNCODE DefaultWriteStatus (
	DSW_generic* dsw_outstatus,
	DSW_generic* dsw_outstatus_detailed,
	void * UserData, /* unused */
	char * Id,
	EIT_STATUS Status,
	/* all these are unused */
    double Current,
	double Previous,
    double Weight,
	double Effect,
	double LeftODIBoundary,
    double LeftODEBoundary,
    double RightODEBoundary,
    double RightODIBoundary,
    double Excl_Sigmagap,
	double Imp_Sigmagap,
	double Gap)
{
	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s %s\n", Id, Status);

	return EIE_WRITECALLBACK_SUCCEED;
}
/*
Calcule Ei = Si * (max [Xit, Xit-1))^exp
où:
	Si = 1 - (Rm/Ri), si 0<Ri<Rm ou
	Si = (Ri/Rm) - 1, si Ri>=Rm

où:
	Ri = Xit/Xit-1
	Rm = est la médiane des ratio

Entrées:
	Xit
	Xit-1
	Rm
	exp
Sorties:
	Aucune
Retourne:
	l'effet
*/
static double Effect (
	double Current,
	double Previous,
	double MedianRatio,
	double Exponent)
{
	/* Previous cannot be <= 0 */
	return TransformRatio (Current/Previous, MedianRatio) *
		pow (OUTLIER_MAX (Current, Previous), Exponent);
}
/*
Calcule une borne selon la formule
borne de gauche = Median - multiplicateur * MAX[Median-Quartile,Median*Minimum]
ou
borne de droite = Median + multiplicateur * MAX[Quartile-Median,Median*Minimum]

Entrées:
	Indicateur gauche/droite
	mediane,
	1er ou 3e quartiles,
	multiplicateur d'interval: d'exclusion ou d'imputation
	multiplicateur de distance minimal
Sorties:
	Aucune
Retourne:
	borne demandée
*/
static double HBBoundary (
	EIT_OUTLIER_SIDE Side,
	double Median,
	double Quartile,
	double IntervalMultiplier,
	double MinimumDistanceMultiplier)
{
	if (IntervalMultiplier == EIM_MISSING_VALUE)
		return EIM_MISSING_VALUE;

	if (Side == EIE_OUTLIER_SIDE_LEFT)
		return Median -
			IntervalMultiplier * OUTLIER_MAX (
				(Median - Quartile),
				fabs (Median * MinimumDistanceMultiplier));
	else
		return Median +
			IntervalMultiplier * OUTLIER_MAX (
				(Quartile - Median),
				fabs (Median * MinimumDistanceMultiplier));
}
/*
Calculate boundaries for historical trend method.
*/
static void HTCalculateBoundaries (
	HTT_HISTORICALTREND * HT,
	double MEI,
	double MII,
	double MinimumDistanceMultiplier,
	double Exponent,
	double * LeftODIBoundary,
	double * LeftODEBoundary,
	double * RightODEBoundary,
	double * RightODIBoundary,
	T_OUTSUMINFO * OutSumData)
{
	int i;
	double FirstQuartile;
	double Median;
	double ThirdQuartile;

	//HT_Print (HT);
	HT_SortRatio (HT);
	//HT_Print (HT);

	Median = CalculatePercentile (HT->Ratio, HT->NumberEntries, 50);
	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10023 "\n", Median);

	for (i = 0; i < HT->NumberEntries; i++) {
		HT->Effect[i] = Effect (HT->Current[i] * (HT->IsWeighted ? HT->Weight[i]: 1.0), HT->Previous[i], Median, Exponent);
	}

	//HT_Print (HT);
	HT_SortEffect (HT);
	//HT_Print (HT);

	FirstQuartile = CalculatePercentile (HT->Effect, HT->NumberEntries, 25);
	Median  = CalculatePercentile (HT->Effect, HT->NumberEntries, 50);
	ThirdQuartile = CalculatePercentile (HT->Effect, HT->NumberEntries, 75);

	/* retain these values for use on OutSummary dataset */
	OutSumData->Q1 = FirstQuartile;
	OutSumData->M = Median;
	OutSumData->Q3 = ThirdQuartile;

	EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION,
		"%s %.5f\n%s %.5f\n%s %.5f\n",
		M10019, FirstQuartile,
		M10020, Median,
		M10021, ThirdQuartile);


	if (EIM_DBL_EQ (Median, 0.0))
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, M10024 "\n");

	*LeftODIBoundary = HBBoundary (EIE_OUTLIER_SIDE_LEFT, Median, FirstQuartile, MII, MinimumDistanceMultiplier);
	*LeftODEBoundary = HBBoundary (EIE_OUTLIER_SIDE_LEFT, Median, FirstQuartile, MEI, MinimumDistanceMultiplier);
	*RightODEBoundary= HBBoundary (EIE_OUTLIER_SIDE_RIGHT, Median, ThirdQuartile, MEI, MinimumDistanceMultiplier);
	*RightODIBoundary= HBBoundary (EIE_OUTLIER_SIDE_RIGHT, Median, ThirdQuartile, MII, MinimumDistanceMultiplier);

	PrintHBBoundaries (*LeftODIBoundary, *LeftODEBoundary, *RightODEBoundary, *RightODIBoundary);
}
/*
Call the read callback function until it fails or source is exhausted.
*/
static EIT_RETURNCODE HTReadData (
	HTT_HISTORICALTREND * HT,
	EIT_HTREADCALLBACK ReadFunction)
{
	double Current;
	char * Id;
	double Previous;
	EIT_READCALLBACK_RETURNCODE rc;
	void * UserData;
	double Weight;

	UserData = EI_GetUserData ();
	rc = ReadFunction (UserData, &Id, &Current, &Previous, &Weight);
	if (rc == EIE_READCALLBACK_FAIL) return EIE_FAIL;
	while (rc == EIE_READCALLBACK_FOUND) {
		HT = HT_Add (HT, Id, Current, Previous, Weight);
		if (HT == NULL) return EIE_FAIL;

		rc = ReadFunction (UserData, &Id, &Current, &Previous, &Weight);
		if (rc == EIE_READCALLBACK_FAIL) return EIE_FAIL;
	}

	return EIE_SUCCEED;
}

/*************************************************
 Call this when a particular VAR-level iteration is skipped due to insufficient observations (MinObs)
 This function sets the calculated values to missing while letting rejection counters 
 and BY-level information to remain intact 
**************************************************/
static void OutSum_SetMissing(T_OUTSUMINFO *OutSumData)
{
	if (OutSumData->IsEnabled) {
		/* COMMON  */
		OutSumData->NFTI = EIM_MISSING_VALUE;
		OutSumData->NFTE = EIM_MISSING_VALUE;

		/* for HB method */
		OutSumData->Q1 = EIM_MISSING_VALUE;
		OutSumData->M = EIM_MISSING_VALUE;
		OutSumData->Q3 = EIM_MISSING_VALUE;

		/* for SG method */
		OutSumData->SIGMA = EIM_MISSING_VALUE;
		OutSumData->IMP_SIGMAGAP = EIM_MISSING_VALUE;
		OutSumData->EXCL_SIGMAGAP = EIM_MISSING_VALUE;

		/* for both methods */
		OutSumData->IMP_BND_L = EIM_MISSING_VALUE;
		OutSumData->EXCL_BND_L = EIM_MISSING_VALUE;
		OutSumData->IMP_BND_R = EIM_MISSING_VALUE;
		OutSumData->EXCL_BND_R = EIM_MISSING_VALUE;
	}
}
/*
Imprime les intervalles HB.
*/
static void PrintHBBoundaries (
	double LeftODIBoundary,
	double LeftODEBoundary,
	double RightODEBoundary,
	double RightODIBoundary)
{
	if (LeftODIBoundary != EIM_MISSING_VALUE &&
		RightODIBoundary != EIM_MISSING_VALUE &&
		LeftODEBoundary != EIM_MISSING_VALUE &&
		RightODEBoundary != EIM_MISSING_VALUE) {
		/* both intervals have been specified. */
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION,
			M10025 "\n" M10026 "\n" M10027 "\n",
			LeftODEBoundary, RightODEBoundary,
			LeftODIBoundary, LeftODEBoundary,
			RightODEBoundary, RightODIBoundary,
			LeftODIBoundary, RightODIBoundary);
	}
	else if (LeftODIBoundary == EIM_MISSING_VALUE &&
				RightODIBoundary == EIM_MISSING_VALUE) {
		/* only exclusion interval has been specified. */
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION,
			M10025 "\n" M10028 "\n",
			LeftODEBoundary, RightODEBoundary,
			LeftODEBoundary, RightODEBoundary);
	}
	else {
		/* only imputation interval has been specified. */
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION,
			M10025 "\n" M10027 "\n",
			LeftODIBoundary, RightODIBoundary,
			LeftODIBoundary, RightODIBoundary);
	}
}
/*
Imprime les intervalles SG.
*/
static void PrintSGBoundaries (
	EIT_OUTLIER_SIDE Side,
	double BetaE,
	double BetaI,
	double LeftODIBoundary,
	double LeftODEBoundary,
	double RightODEBoundary,
	double RightODIBoundary)
{
	if (BetaI != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT))
		PrintSGBoundary (M10035, LeftODIBoundary);
	if (BetaE != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT))
		PrintSGBoundary (M10036, LeftODEBoundary);
	if (BetaE != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT))
		PrintSGBoundary (M10037, RightODEBoundary);
	if (BetaI != EIM_MISSING_VALUE && (Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT))
		PrintSGBoundary (M10038, RightODIBoundary);
	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "");
}
/*
Imprime les intervalles SG.
*/
static void PrintSGBoundary (
	char * Title,
	double Boundary)
{
	if (Boundary == EIM_MISSING_VALUE)
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, "%30s = ------------", Title);
	else
		EI_AddMessage ("EI_Outlier", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f", Title, Boundary);
}
/*
Imprime les gaps
*/
static void PrintSGGaps (
	double Deviation,
	double BetaE,
	double Excl_Sigmagap,
	double BetaI,
	double Imp_Sigmagap)
{
	EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f\n", M10030, Deviation);
	if (BetaE != EIM_MISSING_VALUE) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f", M10031, BetaE);
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f\n", M10033, Excl_Sigmagap);
	}
	if (BetaI != EIM_MISSING_VALUE) {
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f", M10032, BetaI);
		EI_AddMessage ("", EIE_MESSAGESEVERITY_INFORMATION, "%30s = %12.6f\n", M10034, Imp_Sigmagap);
	}
}
/*
Dit si la variable est left ODI, left ODE, acceptable, right ODE ou right ODI

Entrées:
	Side
	variable
	BorneLeftODI
	BorneLeftODE
	BorneRightODE
	BorneRightODI
Sorties:
	Aucune
Retourne:
	etat (left ODI, left ODE, acceptable, right ODE ou right ODI)
*/
static char * Status (
	EIT_OUTLIER_SIDE Side,
	double Value,
	double LeftODIBoundary,
	double LeftODEBoundary,
	double RightODEBoundary,
	double RightODIBoundary)
{
	/*
	NOTE: order is important. Check ODI first than ODE, because if
	the multiplier for imputation interval is equal to
	multiplier for exclusion interval, we want the ODI to have priority.
	*/
	if (LeftODIBoundary != EIM_MISSING_VALUE && RightODIBoundary != EIM_MISSING_VALUE) {
		if ((Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT) && Value < LeftODIBoundary)   return EIM_STATUS_ODIL;
		if ((Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) && Value > RightODIBoundary) return EIM_STATUS_ODIR;
	}
	if (LeftODEBoundary != EIM_MISSING_VALUE && RightODEBoundary != EIM_MISSING_VALUE) {
		if ((Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_LEFT) && Value < LeftODEBoundary)   return EIM_STATUS_ODEL;
		if ((Side == EIE_OUTLIER_SIDE_BOTH || Side == EIE_OUTLIER_SIDE_RIGHT) && Value > RightODEBoundary) return EIM_STATUS_ODER;
	}
	return EIM_STATUS_GOOD;
}
/*
Calcule la transformation d'un ratio
Si = 1 - Rm/Ri, si 0<Ri<Rm ou
Si = Ri/Rm - 1, si Ri>=Rm
où:
	Ri = Xit/Xit-1
	Rm = est la médiane des ratio

Entrées:
	Valeur courante
	Valeur precedente
	Mediane
Sorties:
	Aucune
Retourne:
	Valeur transformée
*/
static double TransformRatio (
	double Ratio,
	double MedianRatio)
{
	/* Ratio and MedianRatio cannot be <= 0 */
	return (Ratio < MedianRatio)
		? 1 - MedianRatio / Ratio
		: Ratio / MedianRatio - 1;
}
