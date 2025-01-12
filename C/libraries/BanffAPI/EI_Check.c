/*----------------------------------------------------------------------
Performs the analysis of edits.
----------------------------------------------------------------------*/
/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Check.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Message.h"
#include "EI_EditsBounds.h"

#include "STC_Memory.h"

#include "EditsAnalysis.h"
#include "EqualityMatrix.h"
#include "util.h"

#include "LPInterface.h"
#include "MessageBanffAPI.h"

/* ---------------------------------------------------------------------
        Static variables
--------------------------------------------------------------------- */
#define EPS (1.0e-6)  /* Fuzz factor */


/* ---------------------------------------------------------------------
Procedures which are located in this file.
--------------------------------------------------------------------- */
static EIT_RETURNCODE CheckConsistency (EIT_EDITS *, tEditsAnalysisStatistics *);
static int EditsCompare (EIT_EDITS *, EIT_EDITS *);
static void PrintEditsSet (EIT_EDITS *);
static void PrintStat (tEditsAnalysisStatistics *);
static void PrintVariablesBounds (EIT_EDITS *, tEditsAnalysisStatistics *);

static EIT_PRINTF mPrintf;


/* 1. Construct the LP problem, where the constraints are the Edits. */
/* 2. Check the consistency of the Edits. */
/* 3. Do the Equality Analysis. */
/* 4. Do the Inequality Analysis. */

EIT_CHECK_RETURNCODE EI_Check (
    EIT_EDITS * EditsParm,
    EIT_PRINTF Printf)
{
    EIT_EDITS Edits;
    int rc;
    EIT_CHECK_RETURNCODE eicheckrc = EIE_CHECK_SUCCEED;

    tEditsAnalysisStatistics EditsAnalysisStatistics;
    tEqualityMatrix EqualityMatrix;


    mPrintf = Printf;

    mPrintf ("\n\n");
    mPrintf (M00026 "\n\n"); /* Edits Analysis Diagnostics */

    /* make a copy because Edits might change in this module */
    EI_EditsDuplicate (&Edits, EditsParm);

    EditsAnalysisStatisticsInit (&EditsAnalysisStatistics);

    EditsAnalysisStatistics.NbEqualitiesRead =
        Edits.NumberofEquations - Edits.NumberofInequalities;
    EditsAnalysisStatistics.NbInequalitiesRead = Edits.NumberofInequalities;
    EditsAnalysisStatistics.NbVariables = Edits.NumberofColumns - 1;


    if (CheckConsistency(&Edits, &EditsAnalysisStatistics) !=
        EIE_SUCCEED) {
        EI_AddMessage("EI_Check", EIE_ERROR, M30010 "\n",
            "CheckConsistency()"); /* %s failed. */
        eicheckrc = EIE_CHECK_FAIL;
        goto TERMINATE;
    }

    if (EditsAnalysisStatistics.InconsistencyFlag == 0) {

        /* no inconsistency has been met */
        rc = EqualityMatrixAllocate(Edits.NumberofColumns-1, &EqualityMatrix);
        if (rc == -1) {
            EI_AddMessage ("EI_Check", EIE_ERROR, M30010 "\n",
                "EqualityMatrixAllocate()"); /* %s failed. */
            eicheckrc = EIE_CHECK_FAIL;
            goto TERMINATE;
        }

        EqualityAnalysis (&Edits, &EditsAnalysisStatistics, &EqualityMatrix,
            mPrintf);

        /* equalities are deterministic */
        if (EditsAnalysisStatistics.DeterminancyFlag != 0) {

            /* All inequalities are redundant */
            while (Edits.NumberofInequalities > 0) {
                mPrintf (M00023 "\n", /* Outside feasible region */
                    Edits.EditId[0]);
                EditsAnalysisStatistics.NbRedundantInequalities++;

                /* Edits.NumberofInequalities decremented */
                EI_EditsDropEquation (0, &Edits);
            }
        }

        if (EditsAnalysisStatistics.InconsistencyFlag != 0) {
            /*
                Some inconsistency has been met
                Move inequalities before printing inconsistent set

                NOTE: This should never really happen unless, as noted,
                it is a 'Software Problem'.  We have already checked the
                consistency of the edits before doing the Equality
                Analysis.
                  */
            while (Edits.NumberofInequalities > 0) {
                /* Edits.NumberofInequalities decremented */
                EI_EditsDropEquation (0, &Edits);
            }

            mPrintf ("\n\n");
            mPrintf (M00027 "\n\n"); /* Set of Inconsistent Edits */
            PrintEditsSet (&Edits); 
            mPrintf (M00028 "\n");   /* Software problem: simplex vs Given */
        }
        else{

            /* Perform the Inequality Analysis */
            if (InequalityAnalysis(&Edits, &EditsAnalysisStatistics,
                                   &EqualityMatrix, mPrintf) != EIE_SUCCEED) {
                EI_AddMessage("EI_Check", EIE_ERROR, M30010 "\n",
                    "InequalityAnalysis()"); /* %s failed. */
                eicheckrc = EIE_CHECK_FAIL;
                goto TERMINATE;
            }

            if (EditsAnalysisStatistics.InconsistencyFlag == 0) {
                /* Get the variable bounds */
                PrintVariablesBounds (&Edits, &EditsAnalysisStatistics);

                mPrintf ("\n");
                mPrintf (M00029 "\n\n"); /* Minimal Set of Edits */
                if (EditsCompare (EditsParm, &Edits) == 1) {
                    PrintEditsSet (&Edits);
                }
                else
                    mPrintf (M00030 "\n"); /* Reduced edits equivalent */
            }
            else {
                /* Some inconsistency has been met */
                mPrintf ("\n");
                mPrintf (M00027 "\n\n"); /* Set of Inconsistent Edits */
                PrintEditsSet (&Edits);
            }
        }
        EqualityMatrixFree (&EqualityMatrix);

    }
    TERMINATE:
    /* Print run statistics */
    PrintStat (&EditsAnalysisStatistics);
    EI_EditsFree(&Edits);
    return eicheckrc;
}


/* ------------------------------------------------------------------ */
/*      Check the consistency of the edits                            */
/**/
/* Set the objective function to 0 and maximize. */
/* Remove edits until we are left with an inconsistent set. */
/**/
/* ------------------------------------------------------------------ */
static EIT_RETURNCODE CheckConsistency (
    EIT_EDITS *Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{

    int    indx,                     /* Index of the first edit      */
    intncy,                          /* Inconsistency indicator      */
    nb_equa,
    nb_ineq;
    register int i;                  /* Index                        */
    int MaximumNumberOfIterations;
    int Maxtime;
    int simplexrc;
    char ilperrmsg[1024];
    EIT_RETURNCODE rc = EIE_SUCCEED;
    EIT_EDITS EditsLP;
    int lpnb;

  /* In previous version, one LP object was created and kept in memory
     for use in the following loop.
     
     Now: A copy of Edits is made, and is each time changed in the loop to
          create a new LP.
     This is an attemp to avoid possible numerical difficulties when the same LP in
     memory is re-optimized many times.
   */



    MaximumNumberOfIterations = 10 *
    (Edits->NumberofEquations + Edits->NumberofColumns);
    Maxtime = -1; /* Negative maximum time will be ignored */

    /*--------------set obj fcn etc-------------------------------*/
    /* Set the objective function to 0 and maximize.
       */

    /*
        While we still have more edits to process We go down in the
        array until the set of constraints is consistent.  The last edit
        removed is the one causing inconsistency when consistency is reached.
    */
    nb_equa = Edits->NumberofEquations;
    nb_ineq = Edits->NumberofInequalities;
    intncy  = 1;
    indx    = 0;
    i = 0;
    
    EI_EditsDuplicate (&EditsLP, Edits);

    /* While inconsistency still occurs */
    while ((intncy != 0) && (indx < Edits->NumberofEquations)) {

        /* Move forward in the array.*/

        /* Creates & populate a new problem object */
        lpnb = LPI_CreateProb(ilperrmsg);
        if (lpnb == 0){
            EI_AddMessage("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }
        if (LPI_MakeMatrix (lpnb, &EditsLP, EditsLP.NumberofEquations, ilperrmsg) !=
                        LPI_SUCCESS) {
            EI_AddMessage ("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }
            /* Silences any output message from the optimizer
         (to print these output, use: LPICallbackPrint(ilperrmsg))*/
        if (LPI_CallbackSilence(ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        if(LPI_SetObjZero(lpnb, ilperrmsg) != LPI_SUCCESS){
            EI_AddMessage("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }

        simplexrc = LPI_MaximizeObj(lpnb, MaximumNumberOfIterations,
                                 Maxtime, ilperrmsg);

        if ((simplexrc == LPI_LP_IT_LIM) || (simplexrc == LPI_LP_TIME_LIM) ||
            (simplexrc == LPI_LP_OTHER_ERR)){ /*optimization was not completed*/
            EI_AddMessage("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
            goto TERMINATE;
        }
        else if ((simplexrc == LPI_LP_OPT) || (simplexrc == LPI_LP_UNBND) ||
           (simplexrc == LPI_LP_FEAS))
            /* Unbounded or maximum solution: Consistency has been reached */
            intncy = 0;
        else {
            /* The set constraints is inconsistent advance of one in the list */
            indx++;

            EI_EditsDropEquation (0, &EditsLP);

            /* Previous equation are skipped */
            nb_equa--; /* Reduce number of equations */
            if (nb_ineq != 0)
                nb_ineq--; /* Edit is inequality, reduce number of inequality */
        }

        LPI_DeleteProb (lpnb, ilperrmsg);
        lpnb = 0;
    }

    if (indx != 0) {
        /* At least one edit is inconsistent   */
        EditsAnalysisStatistics->InconsistencyFlag = 1;

        /* -1 because of post incrementation in previous loop */
        indx--;

        if (indx >= Edits->NumberofInequalities)
            /* Equality: makes edits inconsistent */
            mPrintf (M00017 "\n", Edits->EditId[indx]);
        else
            /* Inequality: makes edits inconsistent */
            mPrintf (M00031 "\n", Edits->EditId[indx]);

        /* Move other equations at end of array AB */
        for (i = 0; i < indx; i++)
            EI_EditsDropEquation (0, Edits);

        EditsAnalysisStatistics->NbEqualities   = nb_equa - nb_ineq;
        EditsAnalysisStatistics->NbInequalities = nb_ineq;

        /* Some inconsistency has been met */
        mPrintf ("\n");
        mPrintf (M00027 "\n\n"); /* Set of Inconsistent Edits */
        PrintEditsSet (Edits);
    }

TERMINATE:
    /* Free Memory of the Simplex Problem */
    if (lpnb != 0) {
        if (LPI_DeleteProb(lpnb, ilperrmsg) != LPI_SUCCESS) {
            EI_AddMessage("EI_Check:CheckConsistency", EIE_ERROR, ilperrmsg);
            rc = EIE_FAIL;
        }
    }

    EI_EditsFree (&EditsLP);
    return rc;
}

/*
Compare the original edits against the minimal set.
Return 0, if they are the same.
1 otherwise.
*/
static int EditsCompare (
    EIT_EDITS * EditsA,
    EIT_EDITS * EditsB)
{
    if (EditsA->NumberofEquations == EditsB->NumberofEquations &&
        EditsA->NumberofInequalities == EditsB->NumberofInequalities)
        return 0; /* same */
    return 1; /* different */
}


/* ------------------------------------------------------------------ */
/*      Print set of edits                                            */
/* ------------------------------------------------------------------ */
static void PrintEditsSet (
    EIT_EDITS * Edits)
{
    char * TheEdits;

    TheEdits = STC_AllocateMemory (EI_EditsFormatSize (Edits));
    EI_EditsFormat (Edits, TheEdits);
    mPrintf (TheEdits);
    STC_FreeMemory (TheEdits);
    mPrintf ("\n");
}


/* ------------------------------------------------------------------ */
/*      Print run statistics                                          */
/* ------------------------------------------------------------------ */
static void PrintStat (
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
    int NbEditsInMinimalSet;
    int NbRedundantEdits;

/* For LengthHeadingSpace in printing */
#define  ONETAB   4
#define  TWOTABS  8

/* PRINT_COUNT() is used to print the count of observations.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT(Msg, LengthPlaceHolders, Count) \
mPrintf (Msg, \
65 - strlen (Msg) + LengthPlaceHolders, \
"............................................................", \
Count);

/* PRINT_SUBTOTAL() is used to print subtotal of observations,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is usually ONETAB */
#define PRINT_SUBTOTAL(Msg, LengthPlaceHolders, LengthHeadingSpace, Count) \
mPrintf (Msg, \
LengthHeadingSpace, \
"                                                            ",\
65 - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace, \
"............................................................", \
Count);

/* PRINT_SUBTITLE() is used to print subtitle,
   adding some heading spaces before the message.
   - parm LengthHeadingSpace is ONETAB */
#define PRINT_SUBTITLE(Msg, LengthHeadingSpace) \
mPrintf (Msg, \
LengthHeadingSpace, \
"                                                            ");

    NbEditsInMinimalSet = EditsAnalysisStatistics->NbEqualities +
        EditsAnalysisStatistics->NbInequalities +
        EditsAnalysisStatistics->NbHiddenEqualities;

    NbRedundantEdits = EditsAnalysisStatistics->NbRedundantEqualities +
        EditsAnalysisStatistics->NbRedundantInequalities +
        EditsAnalysisStatistics->NbRedundantHiddenEqualities +
        EditsAnalysisStatistics->NbNoRestriction;

    mPrintf ("\n");
    mPrintf (M40001 "\n\n"); /* Edits Analysis Statistics */
    
    PRINT_COUNT (
        /* Number of edits read */
        M40002 "\n", M40002_LPH, 
        EditsAnalysisStatistics->NbEqualitiesRead +
        EditsAnalysisStatistics->NbInequalitiesRead);                                                   
    PRINT_SUBTOTAL (
        /* Number of equalities read */
        M40003 "\n", M40003_LPH, ONETAB,
        EditsAnalysisStatistics->NbEqualitiesRead);                                 
    PRINT_SUBTOTAL (
        /* Number of inequalities read */
        M40004 "\n", M40004_LPH, ONETAB,
        EditsAnalysisStatistics->NbInequalitiesRead); 
    mPrintf ("\n");
    
    PRINT_COUNT (
        /* Number of edits in minimal set */
        M40005 "\n", M40005_LPH, 
        NbEditsInMinimalSet);                                 
    PRINT_SUBTOTAL (
        /* Number of equalities kept */
        M40006 "\n", M40006_LPH, ONETAB,
        EditsAnalysisStatistics->NbEqualities);                                 
    PRINT_SUBTOTAL (
        /* Number of hidden equalities */
        M40007 "\n", M40007_LPH, ONETAB,
        EditsAnalysisStatistics->NbHiddenEqualities);                                 
    PRINT_SUBTOTAL (
        /* Number of inequalities kept */
        M40008 "\n", M40008_LPH, ONETAB,
        EditsAnalysisStatistics->NbInequalities);
    mPrintf ("\n");
    
    PRINT_COUNT (
        /* Number of redundant edits */
        M40009 "\n", M40009_LPH,
        NbRedundantEdits);                                 
    PRINT_SUBTOTAL (
        /* Number of redundant equalities */
        M40010 "\n", M40010_LPH, ONETAB,
        EditsAnalysisStatistics->NbRedundantEqualities);                                 
    PRINT_SUBTOTAL (
        /* Number of redundant inequalities */
        M40011 "\n", M40011_LPH, ONETAB,
        EditsAnalysisStatistics->NbRedundantInequalities);                                 
    PRINT_SUBTOTAL (
        /* Number of hidden redundant equalities */
        M40012 "\n", M40012_LPH, ONETAB,
        EditsAnalysisStatistics->NbRedundantHiddenEqualities);

    /* Number of tight edits which do not restrict feasible region
       --> M40013 on line 1 (ONETAB)
       --> M40014 on line 2 (TWOTABS) */
    PRINT_SUBTITLE (
        /* Number of tight edits which do not */
        M40013 "\n", ONETAB);
    PRINT_SUBTOTAL (
        /* restrict feasible region */
        M40014 "\n", M40014_LPH, TWOTABS,
        EditsAnalysisStatistics->NbNoRestriction);
    mPrintf ("\n");
    
    PRINT_COUNT (
        /* Number of variables */
        M40015 "\n", M40015_LPH,
        EditsAnalysisStatistics->NbVariables);    
    PRINT_COUNT (
        /* Number of unbounded variables */
        M40016 "\n", M40016_LPH,
        EditsAnalysisStatistics->NbUnboundVariables);    
    PRINT_COUNT (
        /* Number of deterministic variables */
        M40017 "\n", M40017_LPH,
        EditsAnalysisStatistics->NbDeterministicVariables);

    mPrintf ("\n");
}



/* ------------------------------------------------------------------ */
/*      Print the lower and upper bounds for each variable            */
/* ------------------------------------------------------------------ */
static void PrintVariablesBounds (
    EIT_EDITS * Edits,
    tEditsAnalysisStatistics * EditsAnalysisStatistics)
{
    int i;
    char lbound[1001];      /* Lower bound in characters */
    char ubound[1001];      /* Upper bound in characters */
    double *lower;
    double *upper;

    mPrintf ("\n");
    mPrintf (M40018 "\n\n"); /* Variable bounds */

    /* Print subtitle:
       Variable Name    Lower      Upper        
       (M40019)         (M40020)   (M40021)  */
    mPrintf ("%-32s %-15s %-15s\n", M40019, M40020, M40021);

    upper = STC_AllocateMemory((Edits->NumberofColumns - 1) * sizeof *upper);
    lower = STC_AllocateMemory((Edits->NumberofColumns - 1) * sizeof *lower);

    EI_EditsBounds(Edits, upper, lower);

    /* Find the bounds for all variables */
    for (i = Edits->NumberofColumns - 2; i >= 0; i--) {

        /* Write bounds in character form */
        sprintf (lbound, "%f", lower[i]);
        UTIL_DropZeros (lbound);
        sprintf (ubound, "%f", upper[i]);
        UTIL_DropZeros (ubound);

        /* Print bounds */
        if ((upper[i] == EIM_NOBOUND) || (lower[i] == EIM_NOBOUND))  {
            if (upper[i] == EIM_NOBOUND)
                strcpy(ubound, "*****");
            if (lower[i] == EIM_NOBOUND)
                strcpy(lbound, "*****");

            mPrintf ("%-32s %-15s %-15s %s\n",
                Edits->FieldName[i], lbound, ubound, M40022 ); /* unbounded */
            EditsAnalysisStatistics->NbUnboundVariables++;
        }

        else if (fabs (upper[i] - lower[i]) <= EPS) {
            mPrintf ("%-32s %-15s %-15s %s\n",
                Edits->FieldName[i], lbound, ubound, M40023); /* determinant */
            EditsAnalysisStatistics->NbDeterministicVariables++;
        }
        else {
            mPrintf ("%-32s %-15s %-15s\n",
                Edits->FieldName[i], lbound, ubound);
        }
    }
    mPrintf ("\n");

    STC_FreeMemory(upper);
    STC_FreeMemory(lower);
}
