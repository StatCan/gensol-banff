#ifndef EI_PARSELRFORMULA_H
#define EI_PARSELRFORMULA_H

#include "EI_Common.h"
#include "EI_Stack.h"

/*
Parses edit specifications / Analyse des r�gles de v�rification.

Parses an lr formula specification. Puts the result in a stack.
It is the programmer's responsability to free that structure.

Analyse une formule rl. Met le r�sultat dans une pile.
C'est la responsabilit� du programmeur de lib�rer la m�moire de cette structure.

returns EIE_SUCCEED when successfull, EIE_FAIL otherwise.

retourne: EIE_SUCCEED s'il termine avec succ�s, EIE_FAIL autrement.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_ParseLRFormula (
        /* Edit specifications.
        R�gles de v�rification. */
    char * String,
        /* Stack */
    EIT_STACK * Stack
);

#endif
