#ifndef EI_EDITSBOUNDS_H
#ifndef EDITS_DISABLED
#define EI_EDITSBOUNDS_H

#include "EI_Common.h"
#include "EI_Edits.h"

#define EIM_NOBOUND 3.1415926535897932384626433832795

/*
Return codes for the EI_EditsBounds function

Codes de retour pour la fonction EI_EditsBounds
*/  
typedef enum {
    EIE_EDITSBOUNDS_FAIL,
    EIE_EDITSBOUNDS_SUCCESS
} EIT_EDITSBOUNDS_RETURNCODE;

/* EDITS' BOUNDS */
/* This functions finds the upper and lower bounds of the variables*/
/* in a group of edits.*/
/**/
/* Cette fonction trouve les limites superieur et inferieur des variables des equations*/
CLASS_DECLSPEC EIT_EDITSBOUNDS_RETURNCODE EI_EditsBounds(/* Edits variables*/
						     /**/
						     /* Variables des équations*/
						     EIT_EDITS *Edits, 
						     /*Upper bound*/
						     /**/
						     /*Limite superieur*/
						     double *Ubound, 
						     /*Lower bound*/
						     /**/
						     /*Limite inferieur*/
						     double *Lbound);

#endif
#endif
