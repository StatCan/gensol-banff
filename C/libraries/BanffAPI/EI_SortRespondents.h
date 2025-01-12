#ifndef EI_SORTRESPONDENTS_H
#define EI_SORTRESPONDENTS_H

#include "EI_Common.h"

/* Return codes for the EI_SortRespondents_Quicksort function*/
/**/
/* Codes de retour pour la fonction EI_SortRespondents_Quicksort*/
typedef enum {
    EIE_SORTRESPONDENTS_QUICKSORT_FAIL,
    EIE_SORTRESPONDENTS_QUICKSORT_SUCCEED,
    EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK
} EIT_SORTRESPONDENTS_QUICKSORT_RETURNCODE;

/* SORT RESPONDENTS WITH QUICKSORT / TRIER LES REPONDANTS AVEC QUICKSORT*/
/* This function sorts the structure that holds the data for all respondents*/
/* into ascending order of the key.*/
/**/
/* Cette fonction trie la structure qui contient les données de tous les*/
/* répondants en ordre ascendant de "clé".*/
CLASS_DECLSPEC EIT_SORTRESPONDENTS_QUICKSORT_RETURNCODE EI_SortRespondents_Quicksort (
    EIT_ALL_RESPONDENTS *);

/* SORT RESPONDENTS WITH SHELLSORT / TRIER LES REPONDANTS AVEC SHELLSORT*/
/* This function sorts the structure that holds the data for all respondents*/
/* into ascending order of the key.*/
/**/
/* Cette fonction trie la structure qui contient les données de tous les*/
/* répondants en ordre ascendant de "clé".*/
CLASS_DECLSPEC void EI_SortRespondents_Shellsort (EIT_ALL_RESPONDENTS *);

#endif
