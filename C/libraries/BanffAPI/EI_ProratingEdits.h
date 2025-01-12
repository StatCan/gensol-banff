#ifndef EI_PRORATINGEDITS_H
#define EI_PRORATINGEDITS_H

#include "EI_Prorating.h"


/* Return codes for EI_Raking & EI_Prorating functions/ Codes de retour pour les fonctions EI_Raking & EI_Prorating*/
typedef enum {
    EIE_PRORATING_EDITS_PARSE_FAIL_MULTIPLE_GROUP = -3,
    EIE_PRORATING_EDITS_PARSE_FAIL                = EIE_FAIL,
    EIE_PRORATING_EDITS_PARSE_SUCCEED             = EIE_SUCCEED
} EIT_PRORATING_EDITS_PARSE_RETURNCODE;



/*
Frees EIT_PRORATING_GROUP structure / Libère la mémoire d'un EIT_PRORATING_GROUP

Frees the memory allocated by EI_ProratingEditsParse(). Only call
EI_ProratingEditsFree() when EI_ProratingEditsParse() succeeds.

Libère la mémoire alloué par EI_ProratingEditsParse() quand cette derniere
a terminé avec succes.
*/
CLASS_DECLSPEC void EI_ProratingEditsFree (
    /* ProratingEdits is a pointer to a structure previously allocated by
    EI_ProratingEditsParse().

    ProratingEdits pointe à une structure allouée par EI_ProratingEditsParse().  */
    EIT_PRORATING_GROUP * ProratingEdits
);

/*
Parses edit specifications / Analyse des règles de vérification.

Parses an edits specification. Puts the result in the ProratingEdits structure.
It is the programmer's responsability to free that structure.
ProratingEdits will be allocated only if EI_ProratingEditsParse succeeds.

Analyse des règles de vérification. Met le résultat dans la structure
ProratingEdits. C'est la responsabilité du programmeur de libérer la mémoire de
cette structure. ProratingEdits sera alloué seulement si
EI_ProratingEditsParse() termine avec succes.

returns 0 when successfull, 1 otherwise.

retourne: 0 s'il termine avec succès, 1 autrement.
*/
CLASS_DECLSPEC EIT_PRORATING_EDITS_PARSE_RETURNCODE EI_ProratingEditsParse(
        /* Edit specifications.

        Règles de vérification. */
    char * String,
        /* ProratingEdits is a pointer to a structure allocated by
        EI_ProratingEditsParse().

        ProratingEdits pointe à une structure allouée par
        EI_ProratingEditsParse().*/
    EIT_PRORATING_GROUP * ProratingEdits
);

/*
Prints EIT_PRORATING_GROUP structure / Imprime la structure EIT_PRORATING_GROUP.

Prints EIT_PRORATING_GROUP structure.

Imprime le contenue de la structure EIT_PRORATING_GROUP.
*/
CLASS_DECLSPEC void EI_ProratingEditsPrint (
        /* ProratingEdits is a pointer to a structure previously allocated by
        EI_ProratingEditsParse().

        ProratingEdits pointe à une structure allouée par
        EI_ProratingEditsParse().*/
    EIT_PRORATING_GROUP * ProratingEdits
);

#endif
