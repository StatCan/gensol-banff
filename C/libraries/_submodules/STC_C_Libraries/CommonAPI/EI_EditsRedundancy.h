#ifndef EI_EDITSREDUNDANCY_H
#ifndef EDITS_DISABLED
#define EI_EDITSREDUNDANCY_H

#include "EI_Common.h"
#include "EI_Edits.h"

/*
Return codes for the EI_EditsRedundancy function

Codes de retour pour la fonction EI_EditsRedundancy
*/
typedef enum {
    EIE_EDITSREDUNDANCY_FAIL,
    EIE_EDITSREDUNDANCY_SUCCESS,
    EIE_EDITSREDUNDANCY_INCONSISTENT
} EIT_EDITSREDUNDANCY_RETURNCODE;

/*
REDUNDANT EDITS
This function reduces the edits to the minimal set.

Cette fonction reduit les equations.
*/
CLASS_DECLSPEC EIT_EDITSREDUNDANCY_RETURNCODE EI_EditsRedundancy (
        /* Edits variables

        Variables des équations*/
    EIT_EDITS * Edits);
#endif
#endif
