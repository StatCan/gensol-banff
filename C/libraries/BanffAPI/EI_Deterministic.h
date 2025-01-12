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

        Succès, avec aucune imputation effectuée*/
    EIE_DETERMINISTIC_NOTIMPUTED = 0,
        /*Success, with imputation done

        Succès, avec imputation effectuée*/
    EIE_DETERMINISTIC_IMPUTED = 1,
        /*Deterministic imputation failed

        Échec de l'imputation déterministe*/
    EIE_DETERMINISTIC_FAIL = EIE_FAIL
} EIT_DETERMINISTIC_RETURNCODE;


/*
EI_Deterministic - Deterministic Imputation /Imputation déterministe

This function receives rules and data for one respondent.
The new value is returned inside the data structure EIT_DATAREC.
The values changed by deterministic imputation have a StatusFlag=FIELDIDE.

Cette fonction reçoit les règles de vérification ainsi que les données pour
un répondant. Le résultat de l'imputation déterministe est contenu dans la
structure EIT_DATAREC. Les valeurs modifiées possèdent un StatusFlag=FIELDIDE.

returns this function returns three possible values
Cette fonction peut retourner trois valeurs possibles
*/
CLASS_DECLSPEC EIT_DETERMINISTIC_RETURNCODE EI_Deterministic (
        /*Data structure for the edit rules

        Structure de données pour les règles de vérification */
    EIT_EDITS *Edits,
        /*Data structure for the respondent variables

        Structure de données pour les variables d'un répondant */
    EIT_DATAREC * Respondent_Data);

#endif
