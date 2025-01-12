#ifndef DONOR_H
#define DONOR_H

#include "EI_Common.h"
#include "EI_Donor.h"

#include "EIP_Common.h"

/* Return codes for the DONOR_AllocateFTIFields function */
/**/
/* Codes de retour pour la fonction DONOR_AllocateFTIFields */
typedef enum {
    DONORE_ALLOCATEFTI_FAIL=EIE_FAIL,
    DONORE_ALLOCATEFTI_SUCCEED=EIE_SUCCEED
} DONORT_ALLOCATEFTI_RETURNCODE;

/* Return codes for the DONOR_AllocateTreeSearch function */
/**/
/* Codes de retour pour la fonction DONOR_AllocateTreeSearch */
typedef enum {
    DONORE_ALLOCATETREESEARCH_FAIL=EIE_FAIL,
    DONORE_ALLOCATETREESEARCH_SUCCEED=EIE_SUCCEED
} DONORT_ALLOCATETREESEARCH_RETURNCODE;

/* Return codes for the DONOR_ReallocateRecipients function */
/**/
/*Codes de retour pour la fonction DONOR_ReallocateRecipients */
typedef enum {
    DONORE_REALLOCATERECIPIENTS_FAIL=EIE_FAIL,
    DONORE_REALLOCATERECIPIENTS_SUCCEED=EIE_SUCCEED
} DONORT_REALLOCATERECIPIENTS_RETURNCODE;

/* Return codes for the DONOR_CreateAllDonors function */
/**/
/*Codes de retour pour la fonction DONOR_CreateAllDonors */
typedef enum {
    DONORE_CREATEALLDONORS_FAIL=EIE_FAIL,
    DONORE_CREATEALLDONORS_SUCCEED=EIE_SUCCEED
} DONORT_CREATEALLDONORS_RETURNCODE;

/* Allocate the memory needed for the nearest donors */
extern EIT_NEARESTDONORS * DONOR_AllocateNearestDonors (
    int);

/* Allocate memory for a "maximum number" of recipient FTI fields */
extern DONORT_ALLOCATEFTI_RETURNCODE DONOR_AllocateFTIFields (
    int **, int);

/* Allocate the memory needed for the search algorithm */
extern DONORT_ALLOCATETREESEARCH_RETURNCODE DONOR_AllocateTreeSearch (
    int **, double **, double **, double **, EIT_TREESEARCH *,
    int, long);

/*
COPY DONOR VALUES TO FTI FIELDS-COPIER LES DONNEES DES DONNEURS AUX CHAMPS FTI
This function will copy the values of a donor into the FTI fields of
a recipient.

Cette fonction copie les valeurs d'un donneur dans les champs à 
imputer d'un receveur.
*/
extern void DONOR_CopyDonorValues (int *, double *, double *);

/* COPY TRANSFORMED VALUES OF MATCH FIELDS*/
extern void DONOR_CopyMatchFields (double *, EIT_TRANSFORMED *, char *, int *);

/* CREATE ALLDONORS - CREER ALLDONORS */
extern DONORT_CREATEALLDONORS_RETURNCODE DONOR_CreateAllDonors (
    EIT_ALL_RESPONDENTS *, EIT_ALL_DONORS *, EIT_FIELDNAMES *,
    EIT_TRANSFORMED *);

/* 
FIND NEXT NEAREST DONOR INDEX IN EIT_ALL_DONORS
IDENTIFIER INDICE DU PROCHAIN PLUS PROCHE DONNEUR DANS EIT_ALL_DONORS 
*/
extern int DONOR_FindNextNearestDonorIndex (int, EIT_NEARESTDONORS *);         
extern int DONOR_FindNextNearestDonorIndexAlt (int, EIT_NEARESTDONORS *, EIT_ALL_DONORS *, tSList *, tADTList *, char *);
      
/*
FREE THE STRUCTURES ALLOCATED BY DONOR_AllocateTreeSearch
LIBERER LA MEMOIRE ALLOUEE PAR DONOR_AllocateTreeSearch
*/
extern void DONOR_FreeTreeSearch (int *, double *, double *, double *,
    EIT_TREESEARCH *);

/* FREE STRUCTURES CREATED BY DONOR_AllocateRandomSearch */
extern void DONOR_FreeRandomSearch (int *);

/* GET INDICES OF FTI FIELDS-CHERCHER LES INDICES DES CHAMPS FTI*/
extern void DONOR_GetFTIFields (int *, int, EIT_DATAREC *);

/* GET INDICES OF MATCH FIELDS-CHERCHER LES INDICES DES CHAMPS D'APPARIEMENT*/
extern void DONOR_GetMatchFields (int *, EIT_DATAREC *, EIT_TRANSFORMED *,
    EIT_FIELDNAMES *);

/*
RETURNS TRUE IF A DONOR HAS AT LEAST ONE FTE STATUS FOR THE RECIPIENT FTI FIELDS
RETOURNE VRAI SI UN DONNEUR A AU MOINS UN STATUS FTE PARMI LES CHAMPS FTI DU RECEVEUR
*/
extern EIT_BOOLEAN DONOR_HasFIELDFTE (
        /* Table of donors/Tableau de donneurs*/
    EIT_ALL_DONORS * AllDonors,
        /* Index in AllDonors of one to test*/
        /* Indice du donneur à tester parmi la structure AllDonors*/
    int DonorIndex,
        /* List of indexes of recipient FTI fields*/
        /* Liste des indices des champs FTI pour le receveur*/
    int * xfti
);

/*
VERIFY AT LEAST ONE MATCHING FIELD OF ANY TYPES FOR A RECIPIENT
VERIFIE AU MOINS UN CHAMP D'APPARIEMENT DE TOUS TYPES POUR UN RECEVEUR
*/
extern int DONOR_HasMatchingFields_AllTypes (EIT_DATAREC *);

/*
VERIFY AT LEAST ONE MATCHING FIELD OF TYPE MFU FOR A RECIPIENT
VERIFIE AU MOINS UN CHAMP D'APPARIEMENT DE TYPE MFU POUR UN RECEVEUR
*/
extern int DONOR_HasMatchingFields_TypeMFU (EIT_DATAREC *);

/*
POPULATE "NEARESTDONORS" FOR A RECIPIENT WITH DONORS FOUND IN KDTREE
REMPLIT "NEARESTDONORS" POUR UN RECEVEUR AVEC LES DONNEURS TROUVÉS DANS KDTREE
*/
extern void DONOR_PopulateNearestDonors (
        /* Number of donors found in KDTree.*/
        /* Nombre de donneurs trouvés dans KDTree.*/
    int NumberofDonorsFound,
        /* Index of these donors in EIT_ALL_DONORS.*/
        /* Index de ces donneurs dans EIT_ALL_DONORS.*/
    int * DonorsIndex,
        /* Table of distances between each donor and the recipient.*/
        /* Tableau des distances entre chaque donneur et le receveur.*/
    double * Distances,
        /* "NearestDonors" structure to populate.*/
        /* Structure "NearestDonors" à peupler.*/
    EIT_NEARESTDONORS * NearestDonors
);

/* */
extern void DONOR_ReduceChoiceofDonors (
        /* Number of Donors that can be searched in AllDonors.*/
        /* Nombre de donneurs qui peuvent être choisis.*/
    int NumberofDonors,
        /* Donor to remove from possible choice of donors*/
        /* Donneur à éliminer du choix de donneurs*/
    int DonortoRemove,
        /* Table of donors/Tableau de donneurs*/
    EIT_ALL_DONORS * AllDonors
);

#endif
