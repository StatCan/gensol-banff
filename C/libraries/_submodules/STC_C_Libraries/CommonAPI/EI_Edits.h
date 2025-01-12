#ifndef EI_EDITS_H
#ifndef EDITS_DISABLED
#define EI_EDITS_H

#include "EI_EditsStruct.h"
#include "EI_Common.h"

/*
Value used by EI_EditsFormatEdit() to prevent printing the EditId.
Used in EI_Imply.c.
*/
#define EIM_EDITS_FAKEID -99999999

/*
To format double value too big or too small!
(useful when printing or output)
Example:
double d;
d=999999999999;
if (TOOEXTREME (d))
    printf ("%e", d);
else
    printf ("%f", d);
*/
#define TOOEXTREME(d) ((d)!=0.0&&(((d)>=100000||(d)<=-100000)||((d)<.00001&&(d)>-.00001)))


/*
Return codes for the EI_EditsApply function

Codes de retour pour la fonction EI_EditsApply
*/
typedef enum {
    EIE_EDITSAPPLY_FAIL    = EIE_FAIL,
    EIE_EDITSAPPLY_SUCCEED = EIE_SUCCEED
} EIT_EDITSAPPLY_RETURNCODE;


/*
Return codes for the EI_EditsDuplicate function

Codes de retour pour la fonction EI_EditsDuplicate
*/
typedef enum {
    EIE_EDITSDUPLICATE_FAIL    = EIE_FAIL,
    EIE_EDITSDUPLICATE_SUCCEED = EIE_SUCCEED
} EIT_EDITSDUPLICATE_RETURNCODE;

/*
APPLY EDITS / APPLIQUER UN ENSEMBLE DE REGLES

This function will apply a group of edits for one recipient whose
FTI fields have been replaced with the values of a donor.  The function
returns a value to specify whether or not the edits passed.

Cette fonction applique un ensemble de r�gles � un receveur dont les
champs � imputer ont �t� remplac�s par les valeurs d'un donneur.
La fonction retourne une valeur pour dire si le receveur passe les r�gles
ou non.
*/
CLASS_DECLSPEC EIT_EDITSAPPLY_RETURNCODE EI_EditsApply (
        /* Edits.

        Equations. */
    EIT_EDITS * Edits,
        /* Values of fields.

        Valeurs des champs. */
    EIT_DATAREC * Data
);


/*
APPLY EDITS / APPLIQUER UN ENSEMBLE DE REGLES

This function will apply a group of edits for one recipient whose
FTI fields have been replaced with the values of a donor.  The function
returns a value to specify whether or not the edits passed.

Cette fonction applique un ensemble de r�gles � un receveur dont les
champs � imputer ont �t� remplac�s par les valeurs d'un donneur.
La fonction retourne une valeur pour dire si le receveur passe les r�gles
ou non.
*/
CLASS_DECLSPEC EIT_EDITSAPPLY_RETURNCODE EI_EditsApplyOnValues (
        /* Edits.

        Equations. */
    EIT_EDITS * Edits,
        /* Values of fields.

        Valeurs des champs. */
    double * Values
);


/*
CKECK 2 EDITS SET HAVE SAME FIELDS/VERIFIER 2 GROUPES DE REGLES ONT MEMES CHAMPS

This function checks that two edits sets contain the exact same fields.

Cette fonction verifie que deux ensembles de regles de verification
englobent exactement les memes variables.

returns 0 when successfull, -1 otherwise.

retourne: 0 s'il termine avec succ�s, -1 autrement.
*/
CLASS_DECLSPEC int EI_EditsCheckFieldsSet (
        /* First Edits Group

        Premier groupe de regles de verification */
    EIT_EDITS * Edits1,
        /* Second Edits Group

        Deuxieme groupe de regles de verification */
    EIT_EDITS * Edits2
);

CLASS_DECLSPEC void EI_EditsClean (EIT_EDITS * Edits);
CLASS_DECLSPEC void EI_EditsCleanOneEdit (int Equation, EIT_EDITS * Edits);

/*
DROP EQUATION FROM MATRIX/RETIRER UNE EQUATION D'UNE MATRICE

This function will remove an equation from the Edits structure

Cette fonction retire une �quation de la structure Edits
*/
CLASS_DECLSPEC void EI_EditsDropEquation (
        /* Index of equation to drop

        Indice de l'�quation � retirer */
    int Index,
        /* Edits

        Edits */
    EIT_EDITS * Edits
);

/*
DUPLICATE EDITS STRUCTURE/COPIE UNE STRUCTURE D'EDITS
This function will copy the information of one EIT_EDITS structure
into another structure of the same type.

Cette fonction copie l'information d'une structure de type EIT_EDITS
dans une autre structure du m�me type.
*/
CLASS_DECLSPEC EIT_EDITSDUPLICATE_RETURNCODE EI_EditsDuplicate (
        /* Structure to which the information is being copied.

        Structure dans laquelle l'information est copi�e. */
    EIT_EDITS * ToEdits,
        /* Structure from which the information is copied.

        Structure de laquelle l'information est copi�e. */
    EIT_EDITS * FromEdits
);

CLASS_DECLSPEC void EI_EditsFormat (EIT_EDITS *, char *);

CLASS_DECLSPEC void EI_EditsFormatEdit (int, double *, EIT_FIELDNAMES *, char *,
    char *);

CLASS_DECLSPEC int EI_EditsFormatSize (EIT_EDITS *);

/*
Frees EIT_EDITS structure / Lib�re la m�moire d'un EIT_EDITS.

Frees the memory allocated by EI_EditsParse().

Lib�re la m�moire allou� par EI_EditsParse().
*/
CLASS_DECLSPEC void EI_EditsFree (
    /* Edits is a pointer to a structure previously allocated by
    EI_EditsParse().

    Edits pointe � une structure allou�e par EI_EditsParse().  */
    EIT_EDITS * Edits
);

/*
Parses edit specifications / Analyse des r�gles de v�rification.

Parses an edits specification. Puts the result in the Edits structure.
It is the programmer's responsability to free that structure.

Analyse des r�gles de v�rification. Met le r�sultat dans la structure
Edits. C'est la responsabilit� du programmeur de lib�rer la m�moire de
cette structure.

returns EIE_SUCCEED when successfull, EIE_FAIL otherwise.

retourne: EIE_SUCCEED s'il termine avec succ�s, EIE_FAIL autrement.
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_EditsParse (
        /* Edit specifications.

        R�gles de v�rification.  */
    char * String,
        /* Edits are allocated and filled by EI_EditsParse().

        Edits seront allou�es et remplis par EI_EditsParse().*/
    EIT_EDITS * Edits
);

/*
Prints EIT_EDITS structure / Imprime le contenue de la structure EIT_EDITS.

Prints EIT_EDITS structure.

Imprime le contenue de la structure EIT_EDITS.
*/
CLASS_DECLSPEC void EI_EditsPrint (
    /*Edits is a pointer to a structure previously allocated by EI_EditsParse().

    Edits pointe � une structure allou�e par EI_EditsParse().*/
    EIT_EDITS * Edits
);

#endif
#endif
