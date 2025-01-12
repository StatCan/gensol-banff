#ifndef EI_EDITSAPPLYCHERN_H
#define EI_EDITSAPPLYCHERN_H

#include "EI_Common.h"
#include "EI_Edits.h"

/*
Return codes for the EI_EditsApplyChern function

Codes de retour pour la fonction EI_EditsApplyChern
*/
/*EIE_EDITSAPPLYCHERN_FAIL added on oct 21-2003, Maurice D.*/
typedef enum {
  EIE_EDITSAPPLYCHERN_FAIL = EIE_FAIL,
  EIE_EDITSAPPLYCHERN_SUCCEED = 1,
  EIE_EDITSAPPLYCHERN_FAILDOCHERN = 0,
  EIE_EDITSAPPLYCHERN_FAILNOCHERN = 2
} EIT_EDITSAPPLYCHERN_RETURNCODE;

/*
Apply edits / Appliquer un ensemble de regles

This function will apply a group of edits for one respondent.  The function
returns a value to specify whether or not the respondent data should be
put through Chernikova's Algorithm.

EIE_EDITSAPPLYCHERN_SUCCEED: The respondent passed the edits.
EIE_EDITSAPPLYCHERN_FAILDOCHERN: The respondent failed the edits. Send the
respondent to Chernikova's Algorithm (EI_ErrorLocalization).
EIE_EDITSAPPLYCHERN_FAILNOCHERN: The respondent failed the edits.  Flag as FTI
only the fields with invalid data (eg missing, negative, out of bounds etc).
EIE_EDITSAPPLYCHERN_FAIL: LP failed

Cette fonction applique un ensemble de règles à un receveur dont les
champs à imputer ont été remplacés par les valeurs d'un donneur.
La fonction retourne une valeur pour dire si le receveur passe les règles
ou non.
*/
CLASS_DECLSPEC EIT_EDITSAPPLYCHERN_RETURNCODE EI_EditsApplyChern (
        /* indicates whether there is invalid data.
        Missing, Negative, Out of Bounds.

        indique si certaines valeur sont invalides.
        Manquante, negative, hors bornes. */
    int MissingFlag,
        /* Edits.

        Equations. */
    EIT_EDITS * Edits,
        /* Values of fields.

        Valeurs des champs. */
    EIT_DATAREC * Data
);
#endif
