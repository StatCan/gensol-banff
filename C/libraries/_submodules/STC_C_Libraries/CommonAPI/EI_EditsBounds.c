#ifndef EDITS_DISABLED
#include <stdlib.h>

#include "EI_Edits.h"
#include "EI_EditsBounds.h"
#include "EI_Message.h"

#include "util.h"

#include "LPInterface.h"

/*If no upper bound, Ubound[i] == EIM_NOBOUND */
EIT_EDITSBOUNDS_RETURNCODE EI_EditsBounds (
    EIT_EDITS *Edits,
    double *Ubound,
    double *Lbound)
{
    int i;
    char ilperrmsg[1024];
    int lpnb = 0; /* Number of the LP problem object*/
    int MaximumNumberOfIterations;
    int Maxtime;
    int ncols;
    double objvalue;
    EIT_EDITSBOUNDS_RETURNCODE rc = EIE_EDITSBOUNDS_SUCCESS;

    /* MaximumNumberOfIterations and  Maxtime are actually currently not used.
         by LP software      */
    MaximumNumberOfIterations =
                      10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    Maxtime = -1; /* Negative maximum time will be ignored */

    ncols = Edits->NumberofColumns-1;

    /* ncols = LPI_GetNumCols(lpnb, ilperrmsg); */
    /* To find the Lower bounds of a variable, make the variable the
         objective function and minimize it. */
    for (i = 0; i <= ncols-1; i++) {
        /* Creates a problem object*/
        lpnb = LPI_CreateProb(ilperrmsg);
        if (lpnb == 0){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets the optimality tolerance */
        if (LPI_SetControlParmReal(lpnb, LPI_OPT_TOL_TYPE, LPI_OPT_TOL_VAL,
                                                    ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets the feasibility tolerance */
        if (LPI_SetControlParmReal(lpnb, LPI_FEAS_TOL_TYPE, LPI_FEAS_TOL_VAL,
                                                    ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Silences any output message from the optimizer
          (to print these output, use: LPI_CallbackPrint(ilperrmsg))*/
/*        if (LPI_CallbackSilence(ilperrmsg) != LPI_SUCCESS){*/
/*            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);*/
/*            rc = EIE_EDITSBOUNDS_FAIL;*/
/*            goto TERMINATE;*/
/*        }*/
        /* Populates the problem object with data from the Edits*/
        if (LPI_MakeMatrix(lpnb, Edits, Edits->NumberofEquations, ilperrmsg) !=
                                                                   LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets objective function to zero.
        Note: This step not performed in the version prior to
        interface development*/
        if(LPI_SetObjZero(lpnb, ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }

        /* Make variable i the objective fcn and  minimizes); */
        LPI_SetObjCoefSingle(lpnb, i, 1, ilperrmsg);

        if (LPI_MinimizeObj(lpnb, MaximumNumberOfIterations, Maxtime,
                ilperrmsg) == LPI_LP_OPT) {
            if (LPI_GetObjVal(lpnb, &objvalue, ilperrmsg) != LPI_SUCCESS){
                EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
                rc = EIE_EDITSBOUNDS_FAIL;
                goto TERMINATE;
            }
            Lbound[i] = objvalue; /* Lbound[j] = objvalue; */
        }
        else
            Lbound[i] = EIM_NOBOUND; /* Lbound[j] = EIM_NOBOUND;*/

        LPI_DeleteProb(lpnb, ilperrmsg);
        lpnb = 0;

        /* Creates a problem object*/
        lpnb = LPI_CreateProb(ilperrmsg);
        if (lpnb == 0){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets the optimality tolerance */
        if (LPI_SetControlParmReal(lpnb, LPI_OPT_TOL_TYPE, LPI_OPT_TOL_VAL,
                                                    ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets the feasibility tolerance */
        if (LPI_SetControlParmReal(lpnb, LPI_FEAS_TOL_TYPE, LPI_FEAS_TOL_VAL,
                                                    ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Populates the problem object with data from the Edits*/
        if (LPI_MakeMatrix(lpnb, Edits, Edits->NumberofEquations, ilperrmsg) !=
                                                                   LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }
        /* Sets objective function to zero.
        Note: This step not performed in the version prior to
        interface development*/
        if(LPI_SetObjZero(lpnb, ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
            rc = EIE_EDITSBOUNDS_FAIL;
            goto TERMINATE;
        }

        /* Make variable i the objective fcn and  maximize); */
        LPI_SetObjCoefSingle(lpnb, i, 1, ilperrmsg);


        if (LPI_MaximizeObj(lpnb, MaximumNumberOfIterations, Maxtime,
                ilperrmsg) == LPI_LP_OPT){
            if (LPI_GetObjVal(lpnb, &objvalue, ilperrmsg) != LPI_SUCCESS){
                EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
                rc = EIE_EDITSBOUNDS_FAIL;
                goto TERMINATE;
            }
            Ubound[i] = objvalue; /* Ubound[j] = objvalue;*/
        }
        else
            Ubound[i] = EIM_NOBOUND; /* Ubound[j] = EIM_NOBOUND;*/

        LPI_DeleteProb(lpnb, ilperrmsg);
        lpnb = 0;
    }

	return rc;

TERMINATE:
    /* In case of error, delete problem object and exit */
    if (lpnb != 0) {
        if (LPI_DeleteProb(lpnb, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage("EI_EditsBounds", EIE_ERROR, ilperrmsg);
        }
    }
    return rc;
}
#endif
