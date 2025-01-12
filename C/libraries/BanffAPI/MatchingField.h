#ifndef MATCHINGFIELD_H
#define MATCHINGFIELD_H

#include "EI_Common.h"
#include "EI_Donor.h"

/* Return codes for the MF_FieldValuesAdd function */
/**/
/*Codes de retour pour la fonction MF_FieldValuesAdd */
typedef enum {
    MFE_FIELDVALUESADD_FAIL=EIE_FAIL,
    MFE_FIELDVALUESADD_SUCCEED=EIE_SUCCEED
} MFT_FIELDVALUESADD_RETURNCODE;

/* ADD VALUES TO MATCHING FIELD/AJOUTER VALEURS AUX CHAMPS D'APPARIEMENT*/
/* This function will add information to the matching field structure*/
/**/
/* Cette fonction ajoute des enregistrements à la structure des*/
/* champs d'appariement.*/
extern MFT_FIELDVALUESADD_RETURNCODE MF_FieldValuesAdd (EIT_MATCHVAL * FieldValues, char *Key, double Value);

/* ALLOCATE MEMORY FOR THE MATCHING FIELDS STRUCTURE/ALLOUE MEMOIRE POUR STRUCTURE DE CHAMPS D'APPARIEMENT*/
/* This function will allocate memory for the matching field structure*/
/**/
/* Cette fonction alloue de la mémoire pour la structure des champs d'appariement.*/
extern EIT_MATCHVAL * MF_FieldValuesAlloc (void);

/* FREE MEMORY OF THE MATCHING FIELDS STRUCTURE/LIBERE LA MEMOIRE DE LA STRUCTURE DE CHAMPS D'APPARIEMENT*/
/* This function will free the memory of the matching field structure*/
/**/
/* Cette fonction libère la mémoire de la structure des champs d'appariement.*/
extern void MF_FieldValuesFree (EIT_MATCHVAL * FieldValues);

/* SORT THE MATCHING FIELDS STRUCTURE/TRIE LES CHAMPS D'APPARIEMENT*/
/* This function will sort the transformed values by key */
/**/
/* Cette fonction trie les champs d'appariement.*/
extern void MF_FieldValuesSort (EIT_MATCHVAL * FieldValues);

#endif
