#ifndef EDITS_DISABLED
#include <stdlib.h>

#include "EI_Edits.h"
#include "EI_EditsConsistency.h"
#include "EI_Message.h"

#include "util.h"

#include "LPInterface.h"

EIT_EDITSCONSISTENCY_RETURNCODE EI_EditsConsistency (
    EIT_EDITS * Edits)
{
    char lperrmsg[1024];
    int lpnb; /* Number of the LP problem object*/
    int MaxIterations;
    int MaxTime;
    EIT_EDITSCONSISTENCY_RETURNCODE rc;
    int simplexrc;

    /* MaxIterations and MaxTime are currently not used by LP software */
    MaxIterations = 10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    MaxTime = -1; /* Negative maximum time will be ignored */

    /* Creates a problem object*/
    lpnb = LPI_CreateProb (lperrmsg);
    if (lpnb == 0) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        return EIE_EDITSCONSISTENCY_FAIL;
    }

    /* Silences or prints any output message from the optimizer
      (use: LPI_CallbackPrint (lperrmsg)) or LPI_CallbackSilence (lperrmsg)*/
    if (LPI_CallbackSilence (lperrmsg) != LPI_SUCCESS) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
        goto TERMINATE;
    }

    /* Populates the problem object with data from the Edits*/
    if (LPI_MakeMatrix (lpnb, Edits, Edits->NumberofEquations, lperrmsg) !=
                                                                  LPI_SUCCESS) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
        goto TERMINATE;
    }

    /* Sets objective function to zero*/
    if (LPI_SetObjZero (lpnb, lperrmsg) != LPI_SUCCESS) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
        goto TERMINATE;
    }

    /* Sets the optimimization sense to 'maximize' */
    /*
    if(LPI_SetOptimizationSense(lpnb,LPI_OPT_SENSE_MAX,lperrmsg)!=LPI_SUCCESS) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
        goto TERMINATE;
    }
    */

    /* Maximizes the objective function */
    /* Analyses solution then decide about consistency of the equations system*/
    simplexrc = LPI_MaximizeObj (lpnb, MaxIterations, MaxTime, lperrmsg);
    switch (simplexrc) {
    case LPI_LP_OPT:
    case LPI_LP_UNBND:
    case LPI_LP_FEAS:
        rc = EIE_EDITSCONSISTENCY_CONSISTENT;
        break;
    case LPI_LP_IT_LIM:
    case LPI_LP_TIME_LIM:
    case LPI_LP_OTHER_ERR:
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
        break;
    case LPI_LP_COMPLETED_NO_OPT_NOR_FEAS_NOR_UNBND:
        rc = EIE_EDITSCONSISTENCY_INCONSISTENT;
        break;
    default:
        rc = EIE_EDITSCONSISTENCY_FAIL;
    } /*switch*/

TERMINATE:
    /* Deletes problem object, free LP environment and exit*/
    if (LPI_DeleteProb (lpnb, lperrmsg) != LPI_SUCCESS) {
        EI_AddMessage ("EI_EditsConsistency", EIE_ERROR, lperrmsg);
        rc = EIE_EDITSCONSISTENCY_FAIL;
    }

    return rc;
}
#endif
