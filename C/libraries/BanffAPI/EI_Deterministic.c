/* ---------------------------------------------------------------------

NAME            EI_Deterministic.c

DESCRIPTION     Performs the deterministic imputation

--------------------------------------------------------------------- */

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */

#include <stdlib.h>

#include "EI_Common.h"
#include "EI_Deterministic.h"
#include "EI_Edits.h"
#include "EI_Message.h" /*added 21 oct 2003, Maurice D.*/

#include "LPInterface.h"
#include "util.h"
#define EPSILON 1.0e-6      /* Fuzz factor*/

/*--------------------------------------------------------------------*/
/* EI_Deterministic function                                          */
/*                                                                    */
/* This function receives rules and data for one recipient and        */
/* performs the following steps for record having one or more         */
/* fields requiring imputation:                                       */
/* - eliminates the rules which are satisfied by the valid fields     */
/*   on the record.                                                   */
/* - substitutes the original values of the fields which are to be    */
/*   retained into these remaining rules and produces a reduced set   */
/*   of rules which involves only the fields to be imputed.           */
/* - try to find the maximum and minimum values for these fields.     */
/* - if the maximum and minimum are the same, the value of the field  */
/*   is updated and an imputed status is given to this field.         */
/*                                                                    */
/* Parameters:                                                        */
/*    1. Edits - Pointer to structure EIT_EDITS.                      */
/*    2. RespondentData - Pointer to structure EIT_DATAREC.           */
/*                                                                    */
/* Returns: EIT_DETERMINISTIC_RETURNCODE                              */
/*--------------------------------------------------------------------*/

/*NOTE:There is no time limit specified, nor is there a max_it.
 There is no return procedures for these errors...*/

EIT_DETERMINISTIC_RETURNCODE EI_Deterministic (
    EIT_EDITS *Edits,
    EIT_DATAREC *RespondentData)
{
  int i,j,k;
  EIT_DETERMINISTIC_RETURNCODE rc;

  double  adjust,minimum,maximum;
  int constant;
  int active;
  int lpnb; /* Number of the LP problem object*/

  int MaxIterations;
  int Maxtime;
  int simplexrc;
  char ilperrmsg[1024];

  EIT_EDITS EditsLP; 
  /* In previous version, LP was made so as to contain only variables that are FIELDFTI,
     in the for (i = 0; i < Edits->NumberofEquations; i++) loop below. This LP was then
     used in the for (i = 0; i < RespondentData->NumberofFields; i++) loop. Edits where not
     changed.
     
     Now: A copy of Edits is made, and changed in the first loop to contain only 
     FIELDFTI variables. This modified Edits (EditsLP) is then used in the second loop
     to recreate a new LP before any optimizer call.
     This is an attemp to avoid possible numerical difficulties when the same LP in
     memory is re-optimized many times.
   */
    EI_EditsDuplicate (&EditsLP, Edits);

    /* MaxIterations and  Maxtime are actually currently not used.
         by LP software      */
    MaxIterations =
                        10 *(Edits->NumberofEquations + Edits->NumberofColumns);
    Maxtime = -1; /* Negative maximum time will be ignored */


    /*----------------------------------------------------------*/

    constant = EditsLP.NumberofColumns - 1;       /* column of the constant */
    i = 0;
    while (i < EditsLP.NumberofEquations) {
        active = 0;
        for (j = 0; j < constant; j++) {
            if ((RespondentData->StatusFlag[j] == FIELDFTI) &&
                    (!EIM_DBL_EQ(EditsLP.Coefficient[i][j], 0.0))) {
                active = 1;
                adjust = EditsLP.Coefficient[i][constant];
                for (k = 0; k < constant; k++) {
                    if (RespondentData->StatusFlag[k] == FIELDFTI) {
                        /* field to impute, keep it in EditsLP */
                    }
                    else{
                        /* not a field to impute, reduce equation */
                        adjust -= (RespondentData->FieldValue[k] *
                            EditsLP.Coefficient[i][k]);
                        EditsLP.Coefficient[i][k] = 0.0;
                        EditsLP.Coefficient[i][constant] = adjust;
                    }
                }

                i++;
                break;
            }
        }
        if (active != 1) {
            EI_EditsDropEquation (i, &EditsLP);
        }
    }
   
    /* Note: Here EditsLP contains only FIELDFTI variables. Other variables 
        have been replaced  by data values, and the constant updated accordingly.*/

    rc = EIE_DETERMINISTIC_NOTIMPUTED;
    j = 0;
    for (i = 0; i < RespondentData->NumberofFields; i++) {
        if (RespondentData->StatusFlag[i] == FIELDFTI) {
            /* Find the maximum value for the variable:
                makes the variable the objective function and maximizes */

            /* Creates & populate a new problem object */
            lpnb = LPI_CreateProb(ilperrmsg);
            if (lpnb == 0){
                EI_AddMessage("EI_Deterministic", EIE_ERROR, ilperrmsg);
                rc = EIE_DETERMINISTIC_FAIL;
                goto TERMINATE;
            }
            if (LPI_MakeMatrix (lpnb, &EditsLP, EditsLP.NumberofEquations, ilperrmsg) !=
                    LPI_SUCCESS) {
                EI_AddMessage ("EI_Deterministic", EIE_ERROR, ilperrmsg);
                rc = EIE_DETERMINISTIC_FAIL;
                goto TERMINATE;
            }
            /* Silences any output message from the optimizer
             (to print these output, use: LPICallbackPrint(ilperrmsg))*/
            if (LPI_CallbackSilence(ilperrmsg) != LPI_SUCCESS){
                EI_AddMessage("EI_Deterministic", EIE_ERROR, ilperrmsg);
                rc = EIE_DETERMINISTIC_FAIL;
                goto TERMINATE;
            }


            LPI_SetObjZero(lpnb, ilperrmsg);
            LPI_SetObjCoefSingle (lpnb, i, 1.0, ilperrmsg); /* or j ...*/
            /*LPI_WriteLpt(lpnb, "MaximizeObjPB", ilperrmsg);*/
            /*LPI_ScaleProb(lpnb, ilperrmsg);*/ /* to avoid numerical difficulties */
            simplexrc = LPI_MaximizeObj (lpnb, MaxIterations,
                                   Maxtime, ilperrmsg);
            LPI_GetObjVal(lpnb, &maximum, ilperrmsg);

            LPI_DeleteProb (lpnb, ilperrmsg);
            lpnb = 0;

            if (simplexrc == LPI_LP_OPT) {
                /* Find the minimum value for the variable */
                /* Creates & populate a new problem object */
                lpnb = LPI_CreateProb(ilperrmsg);
                if (lpnb == 0){
                    EI_AddMessage("EI_Deterministic", EIE_ERROR, ilperrmsg);
                    rc = EIE_DETERMINISTIC_FAIL;
                    goto TERMINATE;
                }
                if (LPI_MakeMatrix (lpnb, &EditsLP, EditsLP.NumberofEquations, ilperrmsg) !=
                        LPI_SUCCESS) {
                    EI_AddMessage ("EI_Deterministic", EIE_ERROR, ilperrmsg);
                    rc = EIE_DETERMINISTIC_FAIL;
                    goto TERMINATE;
                }

                LPI_SetObjZero(lpnb, ilperrmsg);
                LPI_SetObjCoefSingle (lpnb, i, 1.0, ilperrmsg); /* or j ...*/
                /*LPI_WriteLpt(lpnb, "MinimizeObjPB", ilperrmsg);*/
                /*LPI_ScaleProb(lpnb, ilperrmsg);*/ /* to avoid numerical difficulties */
                simplexrc = LPI_MinimizeObj (lpnb, MaxIterations, Maxtime,
                    ilperrmsg);
                LPI_GetObjVal(lpnb, &minimum, ilperrmsg);

                LPI_DeleteProb (lpnb, ilperrmsg);
                lpnb = 0;

                /* Keep the value if min = max  */
                if((simplexrc == LPI_LP_OPT) && (EIM_DBL_EQ(maximum,minimum))) {
                    rc = EIE_DETERMINISTIC_IMPUTED;
                    if (EIM_DBL_EQ(maximum, 0.0))
                        maximum = 0.0;
                    RespondentData->FieldValue[i] = maximum;
                    RespondentData->StatusFlag[i] = FIELDIDE;
                }
            }
            j++; /* or j++ ...... */
        }
    }


TERMINATE:
    /* Free Memory of the Simplex Problem */

    if (lpnb != 0) {
        if (LPI_DeleteProb(lpnb, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage("EI_Deterministic", EIE_ERROR, ilperrmsg);
            rc = EIE_DETERMINISTIC_FAIL;
        }
    }

    EI_EditsFree (&EditsLP);

    return rc;
}
