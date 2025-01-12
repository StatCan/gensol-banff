#ifndef EI_ERRORLOCALIZATION_H
#define EI_ERRORLOCALIZATION_H

#include <time.h>

#include "EI_Chernikova.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Weights.h"

/* va pas ICI rene */
#define EPSILON   (double) 1.0e-6    /* zero precision               */

/* va pas ICI */
#define END_ITERATION          0    /* chernikova return code                */
#define NO_EXTREME            -1    /* no extreme point exist                */
#define NO_PIVOT              -2    /* no pivotal row found                  */
#define SPACE_EXCEEDED        -3    /* space exceeded, cannot add new column */
#define TIME_EXCEEDED         -4    /* time limit exceeded for record        */
#define CARDINALITY_EXCEEDED  -5    /* cardinality limit exceeded for record */

/**/
#define EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW (0.01)

/*
Return codes for the EI_ErrorLocalization ()
Codes de retour pour EI_ErrorLocalization ()
*/
typedef enum {
    EIE_ERRORLOCALIZATION_FAIL=EIE_FAIL,
    EIE_ERRORLOCALIZATION_SUCCEED=EIE_SUCCEED,
    EIE_ERRORLOCALIZATION_CARDINALITY_EXCEEDED,
    EIE_ERRORLOCALIZATION_TIME_EXCEEDED,
    EIE_ERRORLOCALIZATION_SPACE_EXCEEDED,
	EIE_ERRORLOCALIZATION_OTHER
} EIT_ERRORLOCALIZATION_RETURNCODE;

CLASS_DECLSPEC EIT_ERRORLOCALIZATION_RETURNCODE EI_ErrorLocalization (EIT_EDITS *,
    EIT_WEIGHTS *, EIT_DATAREC *, double, double, EIT_CHERNIKOVA *,
    EIT_CHERNIKOVA_COMPANION *);

#endif
