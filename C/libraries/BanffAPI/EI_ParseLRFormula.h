#ifndef EI_PARSELRFORMULA_H
#define EI_PARSELRFORMULA_H

#include "EI_Common.h"
#include "EI_Stack.h"

/*
Parses edit specifications / Analyse des règles de vérification.

Parses an lr formula specification. Puts the result in a stack.
It is the programmer's responsability to free that structure.

Analyse une formule rl. Met le résultat dans une pile.
C'est la responsabilité du programmeur de libérer la mémoire de cette structure.

returns EIE_SUCCEED when successfull, EIE_FAIL otherwise.

retourne: EIE_SUCCEED s'il termine avec succès, EIE_FAIL autrement.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_ParseLRFormula (
        /* Edit specifications.
        Règles de vérification. */
    char * String,
        /* Stack */
    EIT_STACK * Stack
);

#endif
