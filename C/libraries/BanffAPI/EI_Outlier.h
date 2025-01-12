#ifndef EI_OUTLIER_H
#define EI_OUTLIER_H

#include "EI_Common.h"
#include "IOUtil.h"

#define EIM_OUTLIER_DEFAULT_EXPONENT (0.0)
#define EIM_OUTLIER_DEFAULT_MDM (0.05)
#define EIM_OUTLIER_MINIMUM_MULTIPLIER (0.0)

#define EIM_OUTLIER_DEFAULT_MINOBS (10)
#define EIM_OUTLIER_MINIMUM_MINOBS_HB (3) //Hidiroglou-Berthelot
#define EIM_OUTLIER_MINIMUM_MINOBS_SIGMAGAP (5)

//typedef  char       *ptr; /* from uwproc.h */
/* Structure to hold info for OutSummary dataset.*/
typedef struct {
	char* XVPutOutSum;
	char* OutSumFile;

	unsigned int IsEnabled : 1;

	/* BY-level counters  */
	double NObs;
	double NValid;
	double NRej_NoMatch;

	/* VAR-level counters */
	double NUsed;
	double NRej_Total;
	double NRej_VarMiss;
	double NRej_VarZero;
	double NRej_VarNegative;
	double NRej_AuxMiss;
	double NRej_AuxZero;
	double NRej_AuxNegative;
	double NFTI;
	double NFTE;

	/* VAR-level: for HB method */
	double Q1;
	double M;
	double Q3;

	/* VAR-level: for SG method */
	double SIGMA;
	double IMP_SIGMAGAP;
	double EXCL_SIGMAGAP;

	/* VAR-levelL: for both methods */
	double IMP_BND_L;
	double EXCL_BND_L;
	double IMP_BND_R;
	double EXCL_BND_R;

} T_OUTSUMINFO;

typedef EIT_READCALLBACK_RETURNCODE (*EIT_CDREADCALLBACK) (
    void * UserData,
    char ** Id,
    double * CurrentValue,
	double * Weight
);

typedef EIT_READCALLBACK_RETURNCODE (*EIT_HTREADCALLBACK) (
    void * UserData,
    char ** Id,
    double * CurrentValue,
    double * PreviousValue,
	double * Weight
);

typedef EIT_WRITECALLBACK_RETURNCODE (*EIT_WRITESTATUS_CALLBACK) (
    DSW_generic* dsw_outstatus,
    DSW_generic* dsw_outstatus_detailed,
    void * UserData,
    char * Id,
    EIT_STATUS Status,
    double CurrentValue,
	double PreviousValue,
    double Weight,
	double Effect,
	double LeftODIBoundary,
    double LeftODEBoundary,
    double RightODEBoundary,
    double RightODIBoundary,
    double Excl_Sigmagap,
	double Imp_Sigmagap,
	double Gap
);

enum EIT_OUTLIER_SIGMA {
    EIE_OUTLIER_SIGMA_STD,
    EIE_OUTLIER_SIGMA_MAD
};
typedef enum EIT_OUTLIER_SIGMA EIT_OUTLIER_SIGMA;

#define EIE_OUTLIER_SIGMA_DEFAULT EIE_OUTLIER_SIGMA_MAD

enum EIT_OUTLIER_SIDE {
	EIE_OUTLIER_SIDE_LEFT,
	EIE_OUTLIER_SIDE_BOTH,
	EIE_OUTLIER_SIDE_RIGHT
};
typedef enum EIT_OUTLIER_SIDE EIT_OUTLIER_SIDE;

#define EIE_OUTLIER_SIDE_DEFAULT EIE_OUTLIER_SIDE_BOTH

/*
Identify outlying observations / Repère les valeurs aberrantes.

Identify outlying observations by the Hidiroglou-Berthelot current method.

Repère les valeurs aberrantes par la méthode des données courantes.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_CDOutlier (
    DSW_generic* dsw_outstatus,
    DSW_generic* dsw_outstatus_detailed,
        /* Callback function to read data (In).

        Fonction de rappel de lecture (Entrée). */
    EIT_CDREADCALLBACK ReadData,
        /* Callback function to write result (In).

        Fonction de rappel d'écriture (Entrée). */
    EIT_WRITESTATUS_CALLBACK WriteStatus,
	/* struct contains most data bound for OutSummary dataset */
	T_OUTSUMINFO * OutSumData,
        /* Multiplier exclusion interval (In).

        Multiplicateur pour l'intervalle des valeurs à exclure (Entrée). */
	EIT_OUTLIER_SIDE Side,
    double MultiplierExclusionInterval,
        /* Multiplier imputation interval (In).

        Multiplicateur pour l'intervalle des valeurs à imputer (Entrée). */
    double MultiplierImputationInterval,
        /* Minimum distance multiplier (In).

        Multiplicateur de distance minimale (Entrée). */
    double MinimumDistanceMultiplier,
        /* MinObs (In).

        MinObs (Entrée). */
    int MinObs,
	EIT_BOOLEAN IsWeighted
);

/*
Identify outlying observations / Repère les valeurs aberrantes.

Identify outlying observations by the Hidiroglou-Berthelot historical trend method.

Repère les valeurs aberrantes par la méthode des tendances historiques.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_HTOutlier (
    DSW_generic* dsw_outstatus,
    DSW_generic* dsw_outstatus_detailed,
        /* Callback function to read data (In).

        Fonction de rappel de lecture (Entrée). */
    EIT_HTREADCALLBACK ReadData,
        /* Callback function to write result (In).

        Fonction de rappel d'écriture (Entrée). */
    EIT_WRITESTATUS_CALLBACK WriteStatus,
	/* struct contains most data bound for OutSummary dataset */
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
        /* Multiplier exclusion interval (In).

        Multiplicateur pour l'intervalle des valeurs à exclure (Entrée). */
    double MultiplierExclusionInterval,
        /* Multiplier imputation interval (In).

        Multiplicateur pour l'intervalle des valeurs à imputer (Entrée). */
    double MultiplierImputationInterval,
        /* Minimum distance multiplier (In).

        Multiplicateur de distance minimale (Entrée). */
    double MinimumDistanceMultiplier,
        /* MinObs (In).

        MinObs (Entrée). */
    int MinObs,
        /* Exponent (In).

        Exposant (Entrée). */
    double Exponent,
	EIT_BOOLEAN IsWeighted
);

/*
Identify outlying observations / Repère les valeurs aberrantes.

Identify outlying observations by the sigma-gap current method.

Repère les valeurs aberrantes par la méthode des données courantes.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_OutlierSigmagap (
    DSW_generic* dsw_outstatus,
    DSW_generic* dsw_outstatus_detailed,
        /* Callback function to read data (In).

        Fonction de rappel de lecture (Entrée). */
    EIT_CDREADCALLBACK ReadData,
        /* Callback function to write result (In).

        Fonction de rappel d'écriture (Entrée). */
    EIT_WRITESTATUS_CALLBACK WriteStatus,
	/* struct contains most data bound for OutSummary dataset */
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	EIT_OUTLIER_SIGMA Sigma,
    double BetaExclusion,
    double BetaImputation,
	double StartCentile,
	int MinObs,
	EIT_BOOLEAN IsWeighted
);

/*
Identify outlying observations / Repère les valeurs aberrantes.

Identify outlying observations by the sigma-gap ratio method.

Repère les valeurs aberrantes par la méthode sigma-gap des ratios.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_OutlierSigmagapRatio (
    DSW_generic* dsw_outstatus,
    DSW_generic* dsw_outstatus_detailed,
        /* Callback function to read data (In).

        Fonction de rappel de lecture (Entrée). */
    EIT_HTREADCALLBACK ReadData,
        /* Callback function to write result (In).

        Fonction de rappel d'écriture (Entrée). */
    EIT_WRITESTATUS_CALLBACK WriteStatus,
	/* struct contains most data bound for OutSummary dataset */
	T_OUTSUMINFO * OutSumData,
	EIT_OUTLIER_SIDE Side,
	EIT_OUTLIER_SIGMA Sigma,
    double BetaExclusion,
    double BetaImputation,
	double StartCentile,
	int MinObs,
	EIT_BOOLEAN IsWeighted
);

#endif
