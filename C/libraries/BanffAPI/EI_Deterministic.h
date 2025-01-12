#ifndef EI_DETERMINISTIC_H
#define EI_DETERMINISTIC_H

#include "EI_Common.h"
#include "EI_Edits.h"

/*
Return codes for the EI_Deterministic function

Codes de retour pour la fonction EI_Deterministic
*/
typedef enum {
        /* Success, but no imputation done

        Succ�s, avec aucune imputation effectu�e*/
    EIE_DETERMINISTIC_NOTIMPUTED = 0,
        /*Success, with imputation done

        Succ�s, avec imputation effectu�e*/
    EIE_DETERMINISTIC_IMPUTED = 1,
        /*Deterministic imputation failed

        �chec de l'imputation d�terministe*/
    EIE_DETERMINISTIC_FAIL = EIE_FAIL
} EIT_DETERMINISTIC_RETURNCODE;


/*
EI_Deterministic - Deterministic Imputation /Imputation d�terministe

This function receives rules and data for one respondent.
The new value is returned inside the data structure EIT_DATAREC.
The values changed by deterministic imputation have a StatusFlag=FIELDIDE.

Cette fonction re�oit les r�gles de v�rification ainsi que les donn�es pour
un r�pondant. Le r�sultat de l'imputation d�terministe est contenu dans la
structure EIT_DATAREC. Les valeurs modifi�es poss�dent un StatusFlag=FIELDIDE.

returns this function returns three possible values
Cette fonction peut retourner trois valeurs possibles
*/
CLASS_DECLSPEC EIT_DETERMINISTIC_RETURNCODE EI_Deterministic (
        /*Data structure for the edit rules

        Structure de donn�es pour les r�gles de v�rification */
    EIT_EDITS *Edits,
        /*Data structure for the respondent variables

        Structure de donn�es pour les variables d'un r�pondant */
    EIT_DATAREC * Respondent_Data);

#endif
