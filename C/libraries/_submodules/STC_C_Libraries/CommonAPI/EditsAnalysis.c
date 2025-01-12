/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#ifndef EDITS_DISABLED
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Message.h"

#include "STC_Compare.h"
#include "STC_Memory.h"

#include "EditsAnalysis.h"
#include "EqualityMatrix.h"
#include "util.h"

#include "LPInterface.h"

#include "MessageCommonAPI.h"

enum CheckEqualitiesEnum {
    eCheckEqualitiesIndependant,
    eCheckEqualitiesRedundant,
    eCheckEqualitiesInconsistent,
    eCheckEqualitiesCauseDeterminancy
};
typedef enum CheckEqualitiesEnum tCheckEqualities;


/* ---------------------------------------------------------------------
Procedures which are located in this file
--------------------------------------------------------------------- */
static tCheckEqualities CheckEqualities (tEqualityMatrix *, double *, int *);
static EIT_RETURNCODE CheckHiddenEquality (EIT_EDITS *,
    tEditsAnalysisStatistics *, tEqualityMatrix *);
static EIT_RETURNCODE CheckRedundancy (EIT_EDITS *, tEditsAnalysisStatistics *);
static EIT_RETURNCODE CheckRestriction(EIT_EDITS *, tEditsAnalysisStatistics *);
static void Given (int, int, double *, double *);
static void InequalityToEquality (int, EIT_EDITS *);

static EIT_PRINTF mPrintf;

/* ---------------------------------------------------------------------
--------------------------------------------------------------------- */
void EditsAnalysisStatisticsInit (
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
    EditsAnalysisStatistics->NbEqualitiesRead            = 0;
    EditsAnalysisStatistics->NbInequalitiesRead          = 0;

    EditsAnalysisStatistics->NbEqualities                = 0;
    EditsAnalysisStatistics->NbInequalities              = 0;
    EditsAnalysisStatistics->NbHiddenEqualities          = 0;

    EditsAnalysisStatistics->NbRedundantEqualities       = 0;
    EditsAnalysisStatistics->NbRedundantInequalities     = 0;
    EditsAnalysisStatistics->NbRedundantHiddenEqualities = 0;
    EditsAnalysisStatistics->NbNoRestriction             = 0;

    EditsAnalysisStatistics->NbVariables                 = 0;
    EditsAnalysisStatistics->NbUnboundVariables          = 0;
    EditsAnalysisStatistics->NbDeterministicVariables    = 0;

    EditsAnalysisStatistics->InconsistencyFlag           = 0;
    EditsAnalysisStatistics->DeterminancyFlag            = 0;
}

/* ---------------------------------------------------------------------
--------------------------------------------------------------------- */
void EditsAnalysisStatisticsPrint (
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
#define PRINTSTAT(X) EI_AddMessage ("", 4, "%-52s = %d", #X, X)

    PRINTSTAT (EditsAnalysisStatistics->NbEqualitiesRead);
    PRINTSTAT (EditsAnalysisStatistics->NbInequalitiesRead);

    PRINTSTAT (EditsAnalysisStatistics->NbEqualities);
    PRINTSTAT (EditsAnalysisStatistics->NbInequalities);
    PRINTSTAT (EditsAnalysisStatistics->NbHiddenEqualities);

    PRINTSTAT (EditsAnalysisStatistics->NbRedundantEqualities);
    PRINTSTAT (EditsAnalysisStatistics->NbRedundantInequalities);
    PRINTSTAT (EditsAnalysisStatistics->NbRedundantHiddenEqualities);
    PRINTSTAT (EditsAnalysisStatistics->NbNoRestriction);

    PRINTSTAT (EditsAnalysisStatistics->NbVariables);
    PRINTSTAT (EditsAnalysisStatistics->NbUnboundVariables);
    PRINTSTAT (EditsAnalysisStatistics->NbDeterministicVariables);

    PRINTSTAT (EditsAnalysisStatistics->InconsistencyFlag);
    PRINTSTAT (EditsAnalysisStatistics->DeterminancyFlag);
}

/* ---------------------------------------------------------------------
FUNCTION        Perform the Equality Analysis.
PARAMETERS      1 - Pointer to a structure containing the edits
                2 - Pointer to the edits statistics
                3 - Pointer to the equality matrix
--------------------------------------------------------------------- */
void EqualityAnalysis (
    EIT_EDITS * edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics,
    tEqualityMatrix * EqualityMatrix,
    EIT_PRINTF Printf)
{
    register int i, j;
    int colj;                /* column j                  */
    int rc;                  /* Equation return code      */
    double * row;            /* Vector of current values  */
    int sizerow;             /* Vector size               */

    mPrintf = Printf;

    /* Allocate an array of doubles as working area  */
    sizerow = edits->NumberofColumns * sizeof *row;
    row = STC_AllocateMemory (sizerow);

    /* Process only the equality */
    i = edits->NumberofInequalities;
    while (i < edits->NumberofEquations) {
        /* Copy equation to working space */
        memcpy (row, edits->Coefficient[i], sizerow);

        /* Check the equality */
        rc = CheckEqualities (EqualityMatrix, row, &colj);

        switch (rc) {
        case eCheckEqualitiesIndependant:
            /* Edit with no problem    */
            /* Should be added to      */
            /* set of equality matrix  */
            EqualityMatrix->EditId[colj] = edits->EditId[i];
            memcpy (EqualityMatrix->equa[colj], &row[colj],
                (edits->NumberofColumns - colj) * sizeof (double));
            EqualityMatrix->nb_equa++;
            EditsAnalysisStatistics->NbEqualities++;
            i++;
            break;

        case eCheckEqualitiesRedundant:
            /* Redundant edit  */
            if (mPrintf)
                mPrintf (M00016 "\n", /* linear combination */
                    edits->EditId[i]);

            EditsAnalysisStatistics->NbRedundantEqualities++;

            /* edits->NumberofEquations decremented  */
            EI_EditsDropEquation (i, edits);
            break;

        case eCheckEqualitiesInconsistent:
            /* Inconsistent edit  */
            if (mPrintf)
                mPrintf (M00017 "\n", /* makes edits inconsistent */
                    edits->EditId[i]);

            EditsAnalysisStatistics->InconsistencyFlag = 1;
            EditsAnalysisStatistics->NbEqualities++;

            /* Set end of loop to stop processing */
            i = edits->NumberofEquations;
            break;

        case eCheckEqualitiesCauseDeterminancy:
            /* Determinant edit */
            if (mPrintf)
                mPrintf (M00015 "\n", /* causes determinacy */
                    edits->EditId[i]);

            EditsAnalysisStatistics->DeterminancyFlag = 1;
            EditsAnalysisStatistics->NbEqualities++;

            /* All other equalities are redundant */
            j = i + 1;
            while (j < edits->NumberofEquations) {
                EditsAnalysisStatistics->NbRedundantEqualities++;
                if (mPrintf)
                    mPrintf (M00016 "\n", /* linear combination */
                        edits->EditId[j]);

                /* edits->NumberofEquations decremented */
                EI_EditsDropEquation (j, edits);
            }

            EqualityMatrix->EditId[colj] = edits->EditId[i];
            memcpy (EqualityMatrix->equa[colj], &row[colj],
                (edits->NumberofColumns - colj) * sizeof (double));
            EqualityMatrix->nb_equa++;
            i++;
            break;
        } /* end switch */
    }
    STC_FreeMemory (row);

    return;
}

EIT_RETURNCODE InequalityAnalysis (
    EIT_EDITS * Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics,
    tEqualityMatrix * EqualityMatrix,
    EIT_PRINTF Printf)
{
    /* If # of Inequalities is zero, there is nothing to do. */
    if (Edits->NumberofInequalities == 0) {
        return EIE_SUCCEED;
    }

    mPrintf = Printf;

    /* ------------- Perform the Inequality Analysis ------------- */
    /* Check for redundant inequalities */
    if (CheckRedundancy (Edits, EditsAnalysisStatistics) != EIE_SUCCEED) {
        EI_AddMessage ("InequalityAnalysis", EIE_ERROR,
            M30010, "CheckRedundancy()"); /* %s failed. */
        return EIE_FAIL;
    }

    /* Check for hidden equalities */
    if (CheckHiddenEquality (Edits, EditsAnalysisStatistics, EqualityMatrix) !=
                EIE_SUCCEED) {
        EI_AddMessage ("InequalityAnalysis", EIE_ERROR,
            M30010, "CheckHiddenEquality ()"); /* %s failed. */
        return EIE_FAIL;
    }

    if (EditsAnalysisStatistics->InconsistencyFlag) {
        /* Some inconsistency has been met */
        EditsAnalysisStatistics->NbInequalities = Edits->NumberofInequalities;
        return EIE_FAIL;
    }

    if (EditsAnalysisStatistics->DeterminancyFlag == 0) {
         /* Check feasible region for restriction */
        if (CheckRestriction (Edits, EditsAnalysisStatistics) != EIE_SUCCEED) {
            EI_AddMessage ("InequalityAnalysis", EIE_ERROR,
                M30010, "CheckRestriction()"); /* %s failed. */
            return EIE_FAIL;
        }
        EditsAnalysisStatistics->NbInequalities = Edits->NumberofInequalities;
    }

    return EIE_SUCCEED;
}

/* ------------------------------------------------------------------ */
/*      Check if there is redundant inequality                        */
/* ------------------------------------------------------------------ */
static EIT_RETURNCODE CheckRedundancy (
    EIT_EDITS * Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
    int i;
    char ilperrmsg[1024];
    int lpnb;
    int MaxIter;
    int MaxTime;
    double objval;
    int rcode;
    double rhsval;  /* Right hand side */

    /* MaxIter and MaxTime are currently not used by LP software */
    MaxIter = 10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    MaxTime = -1; /* Negative maximum time will be ignored */

    /*-----------------Find Redundancy-----------------------------*/
    /* To find redundant edits:
       For each linear constraint, make it the objective function
       (but not the constant).
       Maximize the function.
       If the maximized value > the bound of the constraint, then
       the constraint is redundant.
       Remove the constraint from the Edits.
    */
    for (i = 0; i < Edits->NumberofInequalities; i++) {

        /* Creates a problem object: Nouveau, 10 aout 2007*/
        lpnb = LPI_CreateProb (ilperrmsg);
        if (lpnb == 0) {
            EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Populates the problem object: Nouveau, 10 aout 2007 */
        if (LPI_MakeMatrix (lpnb, Edits, Edits->NumberofEquations, ilperrmsg) !=
                    LPI_SUCCESS) {
            EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
            LPI_DeleteProb (lpnb, ilperrmsg);
            return EIE_FAIL;
        }

        /* Make the constraint the objective function */
        if (LPI_SetObjRow (lpnb, i, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
            LPI_DeleteProb (lpnb, ilperrmsg);
            return EIE_FAIL;
        }

        /* Maximizes the objective function */
        rcode = LPI_MaximizeObj (lpnb, MaxIter, MaxTime, ilperrmsg);
        if ((rcode == LPI_LP_IT_LIM) || (rcode == LPI_LP_TIME_LIM) ||
            (rcode == LPI_LP_OTHER_ERR)) {
            /* optimization was not completed */
            EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
            LPI_DeleteProb (lpnb, ilperrmsg);
            return EIE_FAIL;
        }

        if (rcode == LPI_LP_OPT) {
            /* optimization completed & optimum obtained*/
            if (LPI_GetObjVal (lpnb, &objval, ilperrmsg) != LPI_SUCCESS) {
                EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
                LPI_DeleteProb (lpnb, ilperrmsg);
                return EIE_FAIL;
            }
            if (LPI_GetRhsVal (lpnb, i, &rhsval, ilperrmsg) != LPI_SUCCESS) {
                EI_AddMessage ("CheckRedundancy", EIE_ERROR, ilperrmsg);
                LPI_DeleteProb (lpnb, ilperrmsg);
                return EIE_FAIL;
            }

            //if (!EIM_DBL_EQ (objval, rhsval)) {

			//Trace de debug
			//EI_AddMessage ("egale ?", 4, "%.15f %.15f", objval, rhsval);

			if (!STC_COMPARE_EQ (objval, rhsval)) {

                if (mPrintf)
                    mPrintf (M00023 "\n", /* outside feasible region */
                        Edits->EditId[i]);

                /* Remove the constraint from the Edits */
                EI_EditsDropEquation (i, Edits);

                /* Update the statistics */
                EditsAnalysisStatistics->NbRedundantInequalities++;

                i--; /* because rows in edits are re-numbered */
            }
        }
        LPI_DeleteProb (lpnb, ilperrmsg);
    } /* for */
    return EIE_SUCCEED;
}

/* ------------------------------------------------------------------ */
/*      Check for hidden equality                                     */
/* ------------------------------------------------------------------ */
static EIT_RETURNCODE CheckHiddenEquality (
    EIT_EDITS * Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics,
    tEqualityMatrix * EqualityMatrix)
{
    register int i, j;
    int colj;                  /* Column index */
    char ilperrmsg[1024];
    int lpnb;
    int MaxIter;
    int MaxTime;
    double objval;  /* objective function value */
    EIT_RETURNCODE rc = EIE_SUCCEED;
    int rcode;
    double rhsval;  /* Right hand side */
    double *row;

    /* MaxIter and MaxTime are currently not used by LP software */
    MaxIter = 10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    MaxTime = -1; /* Negative maximum time will be ignored */

    row = STC_AllocateMemory (Edits->NumberofColumns * sizeof *row);

    i = 0;
    while (i < Edits->NumberofInequalities) {

        /* Creates a problem object*/
        lpnb = LPI_CreateProb (ilperrmsg);
        if (lpnb == 0) {
            EI_AddMessage ("InequalityAnalysis", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        /* Populates the problem object with data from the Edits*/
        if (LPI_MakeMatrix (lpnb, Edits, Edits->NumberofEquations, ilperrmsg) !=
                    LPI_SUCCESS) {
            EI_AddMessage ("InequalityAnalysis", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        /* Make the constraint the objective function */
        if (LPI_SetObjRow (lpnb, i, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckHiddenEquality", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        /* Minimizes the objective function */
        rcode = LPI_MinimizeObj (lpnb, MaxIter, MaxTime, ilperrmsg);
        if ((rcode == LPI_LP_IT_LIM) || (rcode == LPI_LP_TIME_LIM) ||
            (rcode == LPI_LP_OTHER_ERR)) {
            /* optimization was not completed */
            EI_AddMessage ("CheckHiddenEquality", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        if (rcode == LPI_LP_OPT) {
            /* optimization completed & optimum obtained*/
            if (LPI_GetObjVal (lpnb, &objval, ilperrmsg) != LPI_SUCCESS) {
                EI_AddMessage ("CheckHiddenEquality", EIE_ERROR, ilperrmsg);
                rc = EIE_FAIL;
                goto TERMINATE;
            }

            if (LPI_GetRhsVal (lpnb, i, &rhsval, ilperrmsg) != LPI_SUCCESS) {
                EI_AddMessage ("CheckHiddenEquality", EIE_ERROR, ilperrmsg);
                rc = EIE_FAIL;
                goto TERMINATE;
            }
        }

        if ((rcode == LPI_LP_OPT) && EIM_DBL_EQ (objval,rhsval)) {

            /* make a working area for CheckEqualities() */
            for (j = 0; j < Edits->NumberofColumns; j++)
                row[j] = -Edits->Coefficient[i][j];

            switch (CheckEqualities (EqualityMatrix, row, &colj)) {
            case eCheckEqualitiesIndependant:
                /* Edit with no problem should be added to equality matrix */
                EqualityMatrix->EditId[colj] = Edits->EditId[i];
                memcpy (EqualityMatrix->equa[colj], &row[colj],
                    (Edits->NumberofColumns - colj) * sizeof (double));
                EqualityMatrix->nb_equa++;
                if (mPrintf)
                    mPrintf (M00020 "\n", /* hidden equality */
                        Edits->EditId[i]);

                /* Convert inequality to equality */
                /* Edits->NumberofInequalities is decremented */
                EditsAnalysisStatistics->NbHiddenEqualities++;
                InequalityToEquality (i, Edits);
                break;

            case eCheckEqualitiesRedundant:
                /* Redundant hidden equality */
                if (mPrintf)
                    mPrintf (M00021 "\n", /* redundant hidden equality */
                        Edits->EditId[i]);

                EditsAnalysisStatistics->NbRedundantHiddenEqualities++;

                /* Edits->NumberofEquations is decremented */
                /* Edits->NumberofInequalities is decremented*/
                EI_EditsDropEquation (i, Edits);
                break;

            case eCheckEqualitiesCauseDeterminancy:
                /* System deterministic   */
                /* Inequalities redundant */
                EditsAnalysisStatistics->DeterminancyFlag = 1;

                if (mPrintf) {
                    mPrintf (M00020 "\n", /* hidden equality */
                        Edits->EditId[i]);

                    mPrintf (M00018 "\n", /* causes determinacy */
                        Edits->EditId[i]);
                }

                EditsAnalysisStatistics->NbHiddenEqualities++;
                InequalityToEquality (i, Edits);

                j = 0;
                while (Edits->NumberofInequalities > 0) {
                    if (mPrintf)
                        mPrintf (M00023 "\n", /* outside feasible region */
                            Edits->EditId[j]);

                    EditsAnalysisStatistics->NbRedundantHiddenEqualities++;

                    /* Edits->NumberofInequalities decremented */
                    EI_EditsDropEquation (j, Edits);
                }
                EditsAnalysisStatistics->NbInequalities = 0;
                break;

            case eCheckEqualitiesInconsistent:
                /* Inconsistent edit */

                if (mPrintf) {
                    mPrintf (M00020 "\n", /* hidden equality */
                        Edits->EditId[i]);

                    mPrintf (M00019 "\n", /* makes edits inconsistent */
                        Edits->EditId[i]);
                }
                EditsAnalysisStatistics->InconsistencyFlag = 1;
                EditsAnalysisStatistics->NbHiddenEqualities++;
                InequalityToEquality (i, Edits);

                EditsAnalysisStatistics->NbInequalities = 0;
                /* drop all inequalities */
                while (Edits->NumberofInequalities > 0) {
                    /* Edits->NumberofInequalities is decremented */
                    EI_EditsDropEquation (0, Edits);
                }
                break;
            }/*switch*/
        }/*if*/
        else {/* Unbounded or min Aij Xj - Bi != 0 */
            i++;
        }
        LPI_DeleteProb (lpnb, ilperrmsg);
    } /*for each inequality*/

TERMINATE:
    STC_FreeMemory (row);
    return rc;
}

/* ------------------------------------------------------------------ */
/*      Check if the edits do not restrict the feasible region        */
/*      when added to all edits.                                      */
/*                                                                    */
/*      Geometrically the edit may lie on a boundary of the           */
/*      acceptance region as defined by other edits or may hit        */
/*      the boundary at only one point of the acceptance region.      */
/*      For each edit, make it the objective fcn (not the constant),  */
/*      and then maximize.                                            */
/* ------------------------------------------------------------------ */
static EIT_RETURNCODE CheckRestriction (
    EIT_EDITS * Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
    int i;
    char ilperrmsg[1024];
    int lpnb;
    int MaxIter;
    int MaxTime;
    double objval;  /* objective function value */
    int rcode;
    double rhsval;  /* Right hand side */

    /* MaxIter and MaxTime are currently not used by LP software */
    MaxIter = 10 * (Edits->NumberofEquations + Edits->NumberofColumns);
    MaxTime = -1; /* Negative maximum time will be ignored */

    /*If there is only one edit, there is nothing to do.*/
    if (Edits->NumberofInequalities == 1)
        return EIE_SUCCEED;

    for (i = 0; i < Edits->NumberofInequalities; i++) {
        /* Creates a problem object*/
        lpnb = LPI_CreateProb (ilperrmsg);
        if (lpnb == 0) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Silences any output message from the optimizer
         (to print these output, use: LPI_CallbackPrint(ilperrmsg))*/
        if (LPI_CallbackSilence (ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Populates problem object with data from Edits (only inequalities)*/
        if (LPI_MakeMatrix (lpnb, Edits, Edits->NumberofEquations, ilperrmsg) !=
                    LPI_SUCCESS) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Make the edit the objective function (except constant) */
        if (LPI_SetObjRow (lpnb, i, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Delete the edit from the problem object */
        if (LPI_DeleteRow (lpnb, i, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }

        /* Only one Edit*/
        if (Edits->NumberofEquations == 1)
            rcode = LPI_LP_UNBND;
        else {
            rcode = LPI_MaximizeObj (lpnb, MaxIter, MaxTime, ilperrmsg);
            if ((rcode == LPI_LP_IT_LIM) || (rcode == LPI_LP_TIME_LIM) ||
                (rcode == LPI_LP_OTHER_ERR)) {
                /* optimization was not completed */
                EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
                return EIE_FAIL;
            }

            if ((rcode == LPI_LP_OPT) &&
                (LPI_GetObjVal (lpnb, &objval, ilperrmsg) != LPI_SUCCESS)) {
                EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
                return EIE_FAIL;
            }

            rhsval = Edits->Coefficient[i][Edits->NumberofColumns-1];
        }

        if ((rcode == LPI_LP_UNBND) ||
            ((rcode == LPI_LP_OPT) && !EIM_DBL_EQ (objval,rhsval))) {
            ;
        }
        else {
            if (mPrintf)
                mPrintf (M00022 "\n", /* tight but does not restrict */
                    Edits->EditId[i]);
            EI_EditsDropEquation (i, Edits);

            i--;

            /* Update the statistics */
            EditsAnalysisStatistics->NbNoRestriction++;
        }

        if (LPI_DeleteProb (lpnb, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage ("CheckRestriction", EIE_ERROR, ilperrmsg);
            return EIE_FAIL;
        }
    }
    return EIE_SUCCEED;
}

/* ------------------------------------------------------------------ */
/*      Convert an inequality into and equality                       */
/* ------------------------------------------------------------------ */
static void InequalityToEquality (
    int indx,          /* Index of the equation to drop */
    EIT_EDITS * Edits)
{
    register int i;
    double * tempCoefficient;
    int tempEditId;

    /* to be converted */
    tempEditId = Edits->EditId[indx];
    tempCoefficient = Edits->Coefficient[indx];

    /* Reduce inequality count */
    Edits->NumberofInequalities--;

    /* Move up the subsequent edits */
    for (i = indx; i < Edits->NumberofInequalities ; i++) {
        Edits->EditId[i] = Edits->EditId[i + 1];
        Edits->Coefficient[i] = Edits->Coefficient[i + 1];
    }

    /* Move inequality at beginning of equality */
    Edits->EditId[Edits->NumberofInequalities] = tempEditId;
    Edits->Coefficient[Edits->NumberofInequalities] = tempCoefficient;

    /* change coefficient of equality when constant is 0.0 */
    EI_EditsCleanOneEdit (Edits->NumberofInequalities, Edits);
}

/* --------------------------------------------------------------------
FUNCTION        Check equality.
PARAMETERS      1 - Pointer to equality matrix.
                2 - Pointer to equation to be checked
                3 - Column index of modification
                4 - Return code
--------------------------------------------------------------------- */
static tCheckEqualities CheckEqualities (
    tEqualityMatrix * EqualityMatrix,
    double * row,    /* Pointer to equality to be checked    */
    int * colj)      /* Column index                         */
{
    double eps = 1e-10;  /* Epsilon value                     */

    int constant;        /* Index:  location of the constant  */
    int i;
    int not_loaded;      /* Indicator: is equation loaded     */

    constant = EqualityMatrix->nb_col - 1;
    not_loaded = 0;

    /* while all column are not zero or column not in EqualityMatrix */
    for (i = 0; i < constant; i++) {
        if (fabs (row[i]) > eps) {
            /* Compare row with EqualityMatrix        */
            if (EqualityMatrix->EditId[i] != -1 ) {
                Given (i, constant, EqualityMatrix->equa[i], row);
            }
            else {
                not_loaded = 1;     /* Equation not in EqualityMatrix, vector */
                break;              /* is linearly independant        */
            }
        }
    }
                                     /* Determine the return code  */
    if (not_loaded != 0) {
        *colj = i;
        if ((EqualityMatrix->nb_equa + 1) == EqualityMatrix->nb_row)
            /* cause determinancy */
            return eCheckEqualitiesCauseDeterminancy;
        else
            /* linearly independant */
            return eCheckEqualitiesIndependant;
    }
    else {
        *colj = --i;
        if (fabs (row[constant]) <= eps)
            /* all columns and constant are zero. Redundant edit */
            return eCheckEqualitiesRedundant;
        else
            /* all columns are zero and constant greater than zero.
            Inconsistent edit. */
            return eCheckEqualitiesInconsistent;
    }
}

/* ------------------------------------------------------------------ */
/*      Perform the Given transform algorithm                         */
/* ------------------------------------------------------------------ */
static void Given (
    int col,      /* Column number                    */
    int constant, /* Constant index                   */
    double * rdce,/* Pointer to the reduced equation  */
    double * row) /* Array of transformed elements    */
{

    double d1, d2, s, c, d, x, y;       /* Temporary variables  */
    int    vk, rk;                      /* Temporary indices    */

    d1 = rdce[0];
    d2 = row[col];
    d  = sqrt ((d1*d1) + (d2*d2));
    c  = d1 / d;
    s  = d2 / d;

    for (rk = 0, vk = col; vk <= constant; rk++, vk++) {
        x = (rdce[rk] * c) + (row[vk] * s);
        y = (rdce[rk] * s) - (row[vk] * c);
        rdce[rk] = x;
        row[vk] = y;
    }
}
#endif
