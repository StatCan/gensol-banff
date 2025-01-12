#ifndef EI_EDITSCONSISTENCY_H
#ifndef EDITS_DISABLED
#define EI_EDITSCONSISTENCY_H

#include "EI_Common.h"
#include "EI_Edits.h"

/*
Return codes for the EI_EditsConsistency function

Codes de retour pour la fonction EI_EditsConsistency
*/  
typedef enum {
    EIE_EDITSCONSISTENCY_FAIL,
    EIE_EDITSCONSISTENCY_CONSISTENT,
    EIE_EDITSCONSISTENCY_INCONSISTENT
} EIT_EDITSCONSISTENCY_RETURNCODE;

CLASS_DECLSPEC EIT_EDITSCONSISTENCY_RETURNCODE EI_EditsConsistency (EIT_EDITS *);

#endif
#endif
