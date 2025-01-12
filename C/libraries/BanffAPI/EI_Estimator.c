/* contient toutes les fonctions reliees aux estimateurs */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_AcceptableKeysResult.h"
#include "EI_Average.h"
#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_DataTable.h"
#include "EI_Estimator.h"
#include "EI_EstimatorSpec.h"
#include "EI_Info.h"
#include "EI_Message.h"
#include "EI_ParseEFFormula.h"
#include "EI_ParseLRFormula.h"
#include "EI_RandomErrorResult.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "slist.h"
#include "util.h"

#include "MessageBanffAPI.h"

/*
set DEBUG to 1 print debugging statements.
set DEBUG to 0 to silence the debugging print statements.
If DEBUG is zero, most compilers will not generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

/*
let the user know when the number of random error donors is less than
a given number.
*/
#define ACCEPTABLENUMBEROFRANDOMERRORDONOR 5

/*
Use to calculate singular values that we want to thrash.
*/
#define TOLERANCE (1.0e-13)

/*
define type of algorithm with precision than LR and EF
*/
#define TYPE_LR                 0
#define TYPE_EF_WITH_AVERAGE    1
#define TYPE_EF_WITHOUT_AVERAGE 2

static EIT_BOOLEAN AddToPartialSums (EIT_ESTIMATORSPEC *, EIT_DATA *);
static double AddVarianceToResidual (EIT_ESTIMATORSPEC *, EIT_DATA *,
    EIT_DATA *, double);
static EIT_BOOLEAN AreParametersRequestedInEstimator (EIT_ESTIMATORSPEC *);
static EIT_BOOLEAN AtLeastOneAverageRequested (EIT_ESTIMATORSPEC_LIST *);
static EIT_BOOLEAN AtLeastOneEstimatorIsActive (EIT_ESTIMATORSPEC_LIST *);
static EIT_BOOLEAN AtLeastOneEstimatorUsesResidual (EIT_ESTIMATORSPEC_LIST *);
static EIT_RETURNCODE CheckStatus (int, char *, double, int, char *, char *);
static EIT_BOOLEAN ComputeAverages (EIT_ESTIMATORSPEC *);
static EIT_RETURNCODE DoEstimation (EIT_ESTIMATORSPEC_LIST *, EIT_SYMBOL **,
    EIT_DATATABLE *, EIT_BOOLEAN);
static void FindRandomErrorDonors (EIT_ESTIMATORSPEC_LIST *, EIT_DATATABLE *);
static double FindXMax (double **, int, int);
static double FindYMax (double *, int);
static EIT_RETURNCODE Impute (EIT_ESTIMATORSPEC *, EIT_SYMBOL **, EIT_DATA *,
    EIT_BOOLEAN, int);
static EIT_BOOLEAN IsCurrentAverageRequested (EIT_ESTIMATORSPEC *);
static EIT_BOOLEAN IsObsAcceptableForAverage (EIT_ESTIMATORSPEC *, EIT_DATA *);
static EIT_BOOLEAN IsObsAcceptableForRegression (EIT_ESTIMATORSPEC *,
    EIT_DATA *);
static EIT_BOOLEAN IsObsImputable (EIT_ESTIMATORSPEC *, EIT_DATA *);
static EIT_BOOLEAN IsObsRandomErrorDonor (EIT_ESTIMATORSPEC *, EIT_DATA *);
static EIT_BOOLEAN IsVariableValid (EIT_ESTIMATORSPEC *, int);
static double * MakeDMatrix (EIT_ESTIMATORSPEC *, EIT_DATATABLE *, int *);
static double ** MakeXMatrix (EIT_ESTIMATORSPEC *, EIT_DATATABLE *, int *);
static double * MakeYMatrix (EIT_ESTIMATORSPEC *, EIT_DATATABLE *, int *);
static void PrintMat0 (double **, int, int, char *);
static void PrintMat1 (double **, int, int, char *);
static void PrintVec (double *, int, char *);
static double RandomError (EIT_ESTIMATORSPEC *, EIT_SYMBOL **, EIT_DATA *,
    double);
static void SaveAcceptableKeysResult (EIT_ESTIMATORSPEC *, char *);
static void SaveRandomErrorResult (EIT_ESTIMATORSPEC *, EIT_DATA *, EIT_DATA *,
    double, double, double);
static double * ScaleToOrderUnity (double *, double **, int, int);
static void ScaleXColumn (double **, double, int, int);
static void ScaleYColumn (double *, double, int);
static void SetAverages (EIT_SYMBOL **, EIT_AVERAGE *);
static void SetBetas (EIT_SYMBOL **, EIT_REGRESSION *);
static void SetUpAverages (EIT_ESTIMATORSPEC_LIST *, char **, int, char **,
    int);
static void SetUpRegressions (EIT_ESTIMATORSPEC_LIST *, char **, int, char **,
    int);
static void SetValues (EIT_SYMBOL **, EIT_DATA *);
static tSList * String2List (char *, tSList *, char *, char *, int);
static void ValidateCriteria (EIT_ESTIMATORSPEC *, int, int, int);
static EIT_RETURNCODE ValidateCommittedVariable (EIT_DATATABLE *, int, int,
    EIT_PERIOD, char *);

/*********************************************************************
checks if at least one estimator is an estimator function with parameters.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
EIT_BOOLEAN EI_AreEFRequestedWithParameters (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;
    EIT_BOOLEAN rc;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_EF){
            rc = AreParametersRequestedInEstimator (
                &EstimatorList->Estimator[i]);
            if (rc == EIE_TRUE)
                return EIE_TRUE;
        }
    }
    return EIE_FALSE;
}

/*********************************************************************
checks if at least one variable or average is historical
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
EIT_BOOLEAN EI_AreHistVariablesRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;
    int j;
    EIT_STACK * Stack;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        Stack = EstimatorList->Estimator[i].Stack;
        /* check in stack */
        for (j = 0; j < Stack->NumberEntries; j++)
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                   Stack->Entry[j].Period == EIE_PERIOD_HISTORICAL)
                return EIE_TRUE;
        /* check the period of the variance variable, if one is used */
        if (EstimatorList->Estimator[i].VariancePeriod[0] == 
                EIM_ESTIM_PERIOD_HISTORICAL_LETTER)
            return EIE_TRUE;
    }

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one estimator is a linear regression.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
EIT_BOOLEAN EI_AreLRRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++)
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_LR)
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one parameter is an requested in estimator list.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
EIT_BOOLEAN EI_AreParametersRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;
    EIT_BOOLEAN rc;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        rc = AreParametersRequestedInEstimator (
            &EstimatorList->Estimator[i]);
        if (rc == EIE_TRUE)
            return EIE_TRUE;
    }

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one random error is an requested in estimator list.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
EIT_BOOLEAN EI_AreRandomErrorsRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        if (strcmp (EstimatorList->Estimator[i].RandomError, "Y") == 0)
            return EIE_TRUE;
    }

    return EIE_FALSE;
}
/*********************************************************************
Computes averages for all estimators.
De-activate estimators where average cannot be calculated
(saves info in AcceptableKeysResult list as a side effect)
*********************************************************************/
void EI_ComputeAverages (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_DATATABLE * DataTable)
{
    EIT_BOOLEAN Active;
    EIT_DATA * Data;
    EIT_ESTIMATORSPEC * Estimator;
    EIT_AVERAGE * Average;
    EIT_BOOLEAN ObsAcceptable;
    int i;
    int j;

    /* quit if no average */
    if (AtLeastOneAverageRequested (EstimatorList) != EIE_TRUE)
        return;

    for (j = 0; j < EstimatorList->NumberEntries; j++) {
        for (Average = EstimatorList->Estimator[j].Average; Average != NULL;
                Average = Average->Next) {
            Average->Numerator = 0.0;
            Average->Denominator = 0.0;
            Average->Average = 0.0;
            Average->Count = 0;
        }
    }

    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            for (j = 0; j < EstimatorList->NumberEntries; j++) {

                Estimator = &EstimatorList->Estimator[j];

                if (Estimator->Average != NULL) {
                    ObsAcceptable = IsObsAcceptableForAverage (Estimator, Data);
                    if (ObsAcceptable == EIE_TRUE) {
                        if (AddToPartialSums (Estimator, Data) == EIE_TRUE)
                            SaveAcceptableKeysResult (Estimator, Data->Key);
                    }
                }
            }
        }
    }

    for (j = 0; j < EstimatorList->NumberEntries; j++) {
        Estimator = &EstimatorList->Estimator[j];
        if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_EF &&
                Estimator->Average != NULL) {
            if (Estimator->Average->Count == 0) {
                Estimator->Active = EIE_FALSE;
                EI_AddMessage ("Estimator", EIE_INFORMATION,
                    M10011 "\n", j);  /* Estimator %d will not be processed 
                                         because there is no acceptable 
                                         observation to compute the averages. */
            }
            else {
                Active = ComputeAverages (Estimator);
                if (Active == EIE_FALSE) {
                    Estimator->Active = EIE_FALSE;
                    EI_AddMessage ("Estimator", EIE_INFORMATION,
                        M10012 "\n", j); /* Estimator %d will not be processed
                                            because at least one average has 
                                            a zero denominator. */
                }
            }
        }
    }
}
/*********************************************************************
Computes betas for all estimator.
De-activate estimators where betas cannot be calculated
 *
 1.  Get X, Y, D.
 2.  Compute X = X*D, Y= Y*D
 3.  Transpose X => Xt
 4.  U = X * Xt; U1 = unit offset of U
 5.  Decompose U1 => U1, w, V
 6.  Edit the Singular Values, w.
 7.  Compute XtY = Xt * Y
 8.  Find the betas through SVD back substitution using U1, w, V, XtY
 *
 returns EIE_SUCCEED when successfull
 returns EIE_FAIL when memory failure
*********************************************************************/
EIT_RETURNCODE EI_ComputeBetas (
    EIT_ESTIMATORSPEC_LIST *EstList,
    EIT_DATATABLE *DataTable)
{
    /* An abbreviation for
        EstList->Estimator[j].Regression->NumberOfAux +
        EstList->Estimator[j].Regression->Intercept
    This is the number of Columns in X (and other matrixs).  */
    int AuxInt;
    double *D; /*The weights and variances vector.*/
    int i;
    int j;
    int k;
    int NumRecs; /*Number of records (rows) in X and Y */
    int rc;
    double **X; /*The regression matrix.  */
    double **Xt; /*The transpose of the the regression matrix.*/
    double *XtY; /*The matrix Xt * Y.*/
    double *Y; /*The 'FieldId'. i.e. the response vector.*/

    double **U; /* The SVD matrices: */
    double **U1;
    double **V;
    double *w;

    double * Az;

    double wmax;
    double wmin;

    /*------------------------start-----------------------------------------*/
    for (j = 0; j < EstList->NumberEntries; j++) {
        if (EstList->Estimator[j].Algorithm->Type != EIE_ALGORITHM_TYPE_LR)
            continue;

        /* 1.  Get X, Y, D.*/
        Y = MakeYMatrix (&EstList->Estimator[j], DataTable, &NumRecs);
        if (Y == NULL && NumRecs != 0)
            return EIE_FAIL;

        if (NumRecs == 0) {
            /* no acceptable record */
            EstList->Estimator[j].Active = EIE_FALSE;
            EI_AddMessage ("Estimator", EIE_INFORMATION, 
                M10013 "\n", j); /* Estimator %d will not be processed because
                                    there is no acceptable observation to 
                                    compute the betas. */ 
            continue;
        }

        /* Some error checking:
           We pass in NumYRecs to MakeXMatrix () because X and Y should have
           the same number of records.  If they do not there is 'Big Trouble'.
           MakeXMatrix modifies NumYRecs, so we put it in a temp variable. */
        i = NumRecs;
        X = MakeXMatrix (&EstList->Estimator[j], DataTable, &NumRecs);
        if (X == NULL) {
            STC_FreeMemory (Y);
            return EIE_FAIL;
        }

        /*Make sure X and Y have the same number of records.*/
        /*where i = number of Y records and NumRecs = Number of X records.*/
        if (i != NumRecs) {
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30032); /* Imbalanced Matrices: X and Y */
            UTIL_FreeDouble0Matrix (X, NumRecs);
            STC_FreeMemory (Y);
            return EIE_FAIL;
        }

        /*TODO: are the variances actually STDs?  sqrt!*/
        D = MakeDMatrix (&EstList->Estimator[j], DataTable, &NumRecs);

        /*Make sure X and Y and D have the same number of records.*/
        /*Where i=number of X and Y records and NumRecs = Number of D records.*/
        if (i != NumRecs) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30033); /* Imbalanced Matrices: D and X, Y */              
            UTIL_FreeDouble0Matrix (X, i);
            STC_FreeMemory (Y);
            STC_FreeMemory (D);
            return EIE_FAIL;
        }

        /*Now we have the regression matrix, X, and the response vector, Y,
              and the weights and variances vector, D (if it exists). */

        /*A convenient abreviation for the number of columns.*/
        AuxInt = EstList->Estimator[j].Regression->NumberOfAux +
            EstList->Estimator[j].Regression->Intercept;

/*        PrintVec (Y, NumRecs, "Y");*/
/*        PrintMat0 (X, NumRecs, AuxInt, "X");*/

        /* change data to make it lie in the range [-1, 1] */
        Az = ScaleToOrderUnity (Y, X, NumRecs, AuxInt);

/*        PrintVec (Y, NumRecs, "Scaled Y");*/
/*        PrintMat0 (X, NumRecs, AuxInt, "Scaled X");*/
/*        PrintVec (Az, AuxInt+1, "Az");*/

        /* 2.  Compute X = X*D, Y= Y*D.*/
        /*If D exists, make X = DX and Y = DY.*/
        if (D != NULL) {
            for (i = 0; i < NumRecs; i++) {
                Y[i] = Y[i] * D[i];
                for (k = 0; k < AuxInt; k++)
                    X[i][k] = X[i][k] * D[i];
            }
        }

        /*--------------------------------------------------------------*/
        if (DEBUG) {
            PrintMat0(X, NumRecs, AuxInt, "X");
            printf ("MATRIX Y\n");
            for (k = 0; k < NumRecs; k++)
                printf ("%.0f, ", Y[k]);

            printf ("MATRIX Y|X\n");
            for (k = 0; k < NumRecs; k++) {
                printf ("%.0f ", Y[k]);
                for (i = 1; i < AuxInt; i++)
                    printf ("%.0f ", X[k][i]);
                printf (",\n");
            }
        }

        /*--------------------------------------------------------------*/
        /* 3.  Transpose X => Xt */
        Xt = UTIL_Transpose (X, NumRecs, AuxInt);
        if (Xt == NULL) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30034); /* Could not compute Xt,
                            the transpose of matrix X */               
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            if (D != NULL)
                STC_FreeMemory (D);
            return EIE_FAIL;
        }
        if (DEBUG)
            PrintMat0(Xt, AuxInt, NumRecs, "Xt");

        /*-----------------------------------------------------------*/
        /* 4.  U = X * Xt; U1 = unit offset of U. */
        /*U has dimension XtRows x XtRows (= AuxInt).*/
        U = UTIL_MultiplyMatrices (Xt, X, AuxInt, NumRecs, AuxInt);
        if (U == NULL) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30035); /* Could not compute U = X * Xt */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            if (D != NULL)
                STC_FreeMemory (D);
            return EIE_FAIL;
        }
        if (DEBUG)
            PrintMat0(U, AuxInt, AuxInt, "U");

        /* For the Numerical Recipes code, we change U (which is a zero-offset
               matrix) to U1 (a unit-offset matrix).
            */
        U1 = STC_AllocateMemory ((1 + AuxInt) * sizeof *U1);
        if (U1 == NULL) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30036); /* Could not allocate U1 */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            if (D != NULL)
                STC_FreeMemory (D);
            return EIE_FAIL;
        }
        for (i = 0; i < AuxInt; i++)
            U1[i+1] = U[i] - 1;

        if (DEBUG)
            PrintMat1(U1, AuxInt, AuxInt, "U1 =  Xt * X");


        /*-----------------------------------------------------------*/
        /*Allocate memory for other SVD matrices.*/
        w = STC_AllocateMemory ((1 + AuxInt) * sizeof *w);
        if (w == NULL) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR, 
                M30037); /* Could not allocate vector w */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            if (D != NULL)
                STC_FreeMemory (D);
            return EIE_FAIL;
        }

        V = STC_AllocateMemory ((1 + AuxInt) * sizeof *V);
        if (V == NULL) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30038); /* Could not allocate V */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            STC_FreeMemory (w);
            if (D != NULL)
                STC_FreeMemory (D);
            return EIE_FAIL;
        }

        for (i = 1; i <= AuxInt; i++) {
            V[i] = STC_AllocateMemory ((1 + AuxInt) * sizeof *V[i]);
            if (V[i] == NULL) {
                UTIL_FreeDouble1Matrix (V, i - 1);                
                EI_AddMessage ("ComputeBetas", EIE_ERROR,
                    M30038); /* Could not allocate V */
                STC_FreeMemory (Y);
                UTIL_FreeDouble0Matrix (X, NumRecs);
                UTIL_FreeDouble0Matrix (Xt, AuxInt);
                UTIL_FreeDouble0Matrix (U, AuxInt);
                STC_FreeMemory (U1);
                STC_FreeMemory (w);
                if (D != NULL)
                    STC_FreeMemory (D);
                return EIE_FAIL;
            }
        }


        /*------------------------------------------------------------------*/
        /* 5.  Decompose U1 => U1, w, V. */
        rc = UTIL_Svdcmp (U1, AuxInt, AuxInt, w, V);
        if (rc == 0) {
            /* no convergence */
            EstList->Estimator[j].Active = EIE_FALSE;            
            EI_AddMessage ("Estimator", EIE_INFORMATION,
                M30039 "\n", j); /* Estimator %d will not be processed because
                                    no convergence could be achieved when 
                                    computing betas. */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            STC_FreeMemory (w);
            if (D != NULL)
                STC_FreeMemory (D);
            UTIL_FreeDouble1Matrix (V, AuxInt);
            continue;
        } else if (rc == -1) {            
            EI_AddMessage ("ComputeBetas", EIE_ERROR,
                M30040); /* Failure at SVDCMP */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            STC_FreeMemory (w);
            if (D != NULL)
                STC_FreeMemory (D);
            UTIL_FreeDouble1Matrix (V, AuxInt);
            return EIE_FAIL;
        }


        /*----------------------------------------------------------------*/
        /*Now we start using unit-offset arrays and vectors.
              Thank you Numerical Recipes.*/

        if (DEBUG) {
            PrintMat1(U1, AuxInt, AuxInt, "U1");
            PrintMat1(V, AuxInt, AuxInt, "V");
            printf ("MATRIX w\n");
            for (k = 1; k <= AuxInt; k++) {
                printf ("%e  ", w[k]);
                printf ("\n");
                printf ("\n");
            }
        }

        /*-----------------------------------------------------------------*/
        /* 6.  Edit the singular values, w. */
        wmax = 0.0;
        for (k = 1; k <= AuxInt; k++)
            if (w[k] > wmax)
                wmax = w[k];

        wmin = wmax * TOLERANCE;
        for (k = 1; k <= AuxInt; k++)
            if (w[k] < wmin)
                w[k] = 0.0;

        /*-----------------------------------------------------------*/
        /* 7.  Compute XtY = Xt * Y. */
        XtY = STC_AllocateMemory ((AuxInt + 1) * sizeof *XtY);
        if (XtY == NULL) {
            EI_AddMessage ("ComputeBetas", EIE_ERROR, 
                M30041); /* Could not allocate XtY */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            STC_FreeMemory (w);
            if (D != NULL)
                STC_FreeMemory (D);
            UTIL_FreeDouble1Matrix (V, AuxInt);
            return EIE_FAIL;
        }

        for (i = 0; i < AuxInt; i++) {
            XtY[i + 1] = 0.0;
            for (k = 0; k < NumRecs; k++)
                XtY[i + 1] += Xt[i][k] * Y[k];
        }

        /*-----------------------------------------------------------*/
        /*8. Find the betas through SVD back substitution using U1, w, V, XtY.*/
        if (!UTIL_Svbksb (U1, w, V, AuxInt, AuxInt, XtY,
                EstList->Estimator[j].Regression->Betas)) {
            EI_AddMessage ("ComputeBetas", EIE_ERROR, 
                M30042); /* Failure at SVBKSB */
            STC_FreeMemory (Y);
            UTIL_FreeDouble0Matrix (X, NumRecs);
            UTIL_FreeDouble0Matrix (Xt, AuxInt);
            UTIL_FreeDouble0Matrix (U, AuxInt);
            STC_FreeMemory (U1);
            STC_FreeMemory (XtY);
            STC_FreeMemory (w);
            if (D != NULL)
                STC_FreeMemory (D);
            UTIL_FreeDouble1Matrix (V, AuxInt);
            return EIE_FAIL;
        }

        if (DEBUG) {
            printf ("MATRIX betas\n");
            for (k = 1; k <= AuxInt; k++) {
                printf ("%f ", EstList->Estimator[j].Regression->Betas[k]);
                printf ("\n");
            }
        }

        /*Make the Betas zero-offset, but don't reduce the size (because
              the space taken up is insignificant, i.e. 1 double).
              Lets remember to free it all...
            */
        for (i = 0; i < AuxInt; i++) {
            EstList->Estimator[j].Regression->Betas[i] =
                EstList->Estimator[j].Regression->Betas[i + 1];
        }

        /*
        Unscale the betas because we scaled the data.
        */
        for (i = 0; i < AuxInt; i++) {
            if (Az[i+1] != 0.0)
                EstList->Estimator[j].Regression->Betas[i] =
                   EstList->Estimator[j].Regression->Betas[i] * Az[0] / Az[i+1];
        }

        /* Record the number of acceptable records.*/
        EstList->Estimator[j].Regression->Count = NumRecs;

        /*-----------------------------------------------------------*/
        /*Lets free some memory.*/
        UTIL_FreeDouble0Matrix (X, NumRecs);
        STC_FreeMemory (Y);
        if (D != NULL)
            STC_FreeMemory (D);
        UTIL_FreeDouble0Matrix (Xt, AuxInt);
        STC_FreeMemory (XtY);

        STC_FreeMemory (U1);/*U1 is made up of U... */
        UTIL_FreeDouble0Matrix (U, AuxInt);

        UTIL_FreeDouble1Matrix (V, AuxInt);
        STC_FreeMemory (w);

        STC_FreeMemory (Az);

    }/*for each EstList.*/

    return EIE_SUCCEED;
}
/*********************************************************************
estimate (hopefully!) every field to impute.
impute a variable when the calculated value is positive or negative
*********************************************************************/
EIT_RETURNCODE EI_DoEstimation (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable,
    EIT_DATATABLE * DataTable)
{
    return DoEstimation (EstimatorList, SymbolTable, DataTable, EIE_TRUE);
}
/*********************************************************************
estimate (hopefully!) every field to impute.
does not impute a variable when the calculated value is negative
*********************************************************************/
EIT_RETURNCODE EI_DoEstimationRejectNegative (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable,
    EIT_DATATABLE * DataTable)
{
    return DoEstimation (EstimatorList, SymbolTable, DataTable, EIE_FALSE);
}
/*********************************************************************
Creates a string where placeholders are replaced with the real aux variable
names in the estimator formula.
It is the callers responsability to allocate enough space for the
resulting string.
NewFormula is also returned.
*********************************************************************/
char * EI_EstimatorFormatFormula (
    char * Formula,
    char * FieldId,
    char * AuxVariables,
    char * NewFormula)
{
    tSList * AuxVariableList;
    int AuxVariableListIndex;
    size_t FormulaIndex;
    size_t n;
    size_t NewFormulaIndex;

    SList_New (&AuxVariableList);
    SList_Add (FieldId, AuxVariableList);
    String2List (AuxVariables, AuxVariableList, ",", "", 1);

    FormulaIndex = 0;
    NewFormulaIndex = 0;
    while (Formula[FormulaIndex] != '\0') {
        if (strncmp (Formula+FormulaIndex, "AUX", strlen ("AUX")) == 0) {
            AuxVariableListIndex = atoi (Formula+FormulaIndex+strlen ("AUX"));  // nosemgrep  // not seeing this as an attack vector, ignore
            n = strspn (Formula+FormulaIndex+strlen ("AUX"), "1234567890");
            strcpy (NewFormula+NewFormulaIndex,
                SList_Entry (AuxVariableList, AuxVariableListIndex));
            FormulaIndex += strlen ("AUX")+n;
            NewFormulaIndex +=
                strlen (SList_Entry (AuxVariableList, AuxVariableListIndex));
        }
        else if(strncmp(Formula+FormulaIndex,"FIELDID",strlen("FIELDID"))==0) {
            strcpy (NewFormula+NewFormulaIndex,
                SList_Entry (AuxVariableList, 0));
            FormulaIndex += strlen ("FIELDID");
            NewFormulaIndex += strlen (SList_Entry (AuxVariableList, 0));
        }
        else {
            NewFormula[NewFormulaIndex] = Formula[FormulaIndex];
            FormulaIndex++;
            NewFormulaIndex++;
        }
    }
    NewFormula[NewFormulaIndex] = '\0';

    SList_Free (AuxVariableList);

    return NewFormula;
}
/*********************************************************************
make all estimator active
reset counters
*********************************************************************/
void EI_Reactivate (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        Estimator = &EstimatorList->Estimator[i];

        Estimator->Active = EIE_TRUE;

        Estimator->FTI = 0;
        Estimator->Imputed = 0;
        Estimator->DivisionByZero = 0;
        Estimator->NegativeImputation = 0;

        /* empty Residual and RandomErrorResult */
        if (Estimator->Residual != NULL) {
            EI_ResidualEmpty (Estimator->Residual);
            if (Estimator->RandomErrorResult != NULL) {
                EI_RandomErrorResultEmpty (Estimator->RandomErrorResult);
            }
        }

        /* empty AcceptableKeysResult */
        if (Estimator->AcceptableKeysResult != NULL)
            EI_AcceptableKeysResultEmpty (Estimator->AcceptableKeysResult);
    }

}
#ifdef NEVERTOBEDEFINED
/*********************************************************************
rene: sets parameters (averages and betas) in stack for performance reasons
*********************************************************************/
void EI_SetParametersInStack (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    EIT_AVERAGE * Average;
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    EIT_STACK * Stack;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        Estimator = &EstimatorList->Estimator[i];
        Stack = Estimator[i].Stack;
        EI_StackPrint (Estimator->Stack);
        if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_EF) {
            /* set averages if any */
            for (Average = Estimator->Average; Average != NULL;
                    Average = Average->Next) {

            }
        }
        else {
            /* set betas */
        }
        EI_StackPrint (Estimator->Stack);
    }
}
#endif
/*********************************************************************
Sets up all estimators
*********************************************************************/
void EI_SetUpEstimators (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    char ** InDataVariableName,
    int InDataNumberOfVariables,
    char ** InHistVariableName,
    int InHistNumberOfVariables,
    EIT_BOOLEAN ReportRandomErrorResult,
    EIT_BOOLEAN ReportAcceptableKeysResult)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    int j;
    int k;
    EIT_STACK * Stack;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        Estimator = &EstimatorList->Estimator[i];
        Stack = Estimator->Stack;

        Estimator->Residual = NULL;
        Estimator->RandomErrorResult = NULL;
        if (Estimator->RandomError[0] == 'Y') {
            Estimator->Residual = EI_ResidualAllocate ();
            if (ReportRandomErrorResult == EIE_TRUE) {
                Estimator->RandomErrorResult = EI_RandomErrorResultAllocate ();
            }
        }

        Estimator->AcceptableKeysResult = NULL;
        if (ReportAcceptableKeysResult == EIE_TRUE)
            Estimator->AcceptableKeysResult = EI_AcceptableKeysResultAllocate();

        /* find index of fieldid */
        Estimator->FieldIdIndex = UTIL_Binary (Estimator->FieldId,
            InDataVariableName, InDataNumberOfVariables);

        assert (Estimator->FieldIdIndex != -1);

        Estimator->LateralVariable.NumberOfVariables = 0;

        /* count the lateral variables */
        for (j = 0; j < Stack->NumberEntries; j++) {
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                    Stack->Entry[j].Aggregate == EIE_AGGREGATE_VALUE &&
                    strncmp (Stack->Entry[j].PlaceHolder, EIM_BETA_PREFIX,
                        strlen (EIM_BETA_PREFIX)) != 0) {
                Estimator->LateralVariable.NumberOfVariables++;
            }
        }

        Estimator->LateralVariable.Index = STC_AllocateMemory (
            Estimator->LateralVariable.NumberOfVariables *
                sizeof *Estimator->LateralVariable.Index);

        Estimator->LateralVariable.Period = STC_AllocateMemory (
            Estimator->LateralVariable.NumberOfVariables *
                sizeof *Estimator->LateralVariable.Period);

        /* rene: could be optimize by removing repeated variables */

        /* find index and period of lateral variables */
        for (j = 0, k = 0; j < Stack->NumberEntries; j++) {
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                    Stack->Entry[j].Aggregate == EIE_AGGREGATE_VALUE &&
                    strncmp (Stack->Entry[j].PlaceHolder, EIM_BETA_PREFIX,
                        strlen (EIM_BETA_PREFIX)) != 0) {
                Estimator->LateralVariable.Period[k] = Stack->Entry[j].Period;
                if (Estimator->LateralVariable.Period[k] == EIE_PERIOD_CURRENT)
                    Estimator->LateralVariable.Index[k] = UTIL_Binary (
                        Stack->Entry[j].Symbol->Name, InDataVariableName,
                        InDataNumberOfVariables);
                else
                    Estimator->LateralVariable.Index[k] = UTIL_Binary (
                        Stack->Entry[j].Symbol->Name, InHistVariableName,
                        InHistNumberOfVariables);

                assert (Estimator->LateralVariable.Index[k] != -1);

                k++;
            }
        }
        assert (k == Estimator->LateralVariable.NumberOfVariables);
    }

    SetUpAverages (EstimatorList, InDataVariableName,
        InDataNumberOfVariables, InHistVariableName,
        InHistNumberOfVariables);

    SetUpRegressions (EstimatorList, InDataVariableName,
        InDataNumberOfVariables, InHistVariableName,
        InHistNumberOfVariables);
}
/*********************************************************************
check if the count and percent criteria were fullfilled.
if they were not fullfilled, set estimator as inactive.
*********************************************************************/
void EI_ValidateCriteria (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    int NumberOfObs)
{
    EIT_ESTIMATORSPEC * Estimator;
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        Estimator = &EstimatorList->Estimator[i];
        if (Estimator->Active == EIE_TRUE) {
            if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_EF) {
                if (Estimator->Average != NULL) {
                    ValidateCriteria (Estimator, i, Estimator->Average->Count,
                        NumberOfObs);
                }
            }
            else {
                ValidateCriteria (Estimator, i, Estimator->Regression->Count,
                    NumberOfObs);
            }
        }
    }
}
/*********************************************************************
*********************************************************************/
EIT_RETURNCODE EI_ValidateEstimator (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable)
{
    EIT_BOOLEAN AreSymbolsMissing;
    tSList * AuxVariableList;
    int CountCriteria;
    EIT_RETURNCODE crc; /* cumulative return code */
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    int j;
    double PercentCriteria;
    char PlaceHolder[101];
    EIT_RETURNCODE rc;
    EIT_STACK * Stack;
    char * s;
    int Type;

    crc = EIE_SUCCEED;

/*
rene ValidateEstimatorSpec (): a completer, morceler, rendre plus robuste
rene validate name in EstimatorSpec ?
*/

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        /* find all algorithm name specified in estimator spec data set */
        if (EstimatorList->Estimator[i].Algorithm == NULL) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30043, /* Estimator %d: algorithm name '%s' not defined
                           in ALGORITHM data set or as a predefined algorithm.*/
                i, EstimatorList->Estimator[i].AlgorithmName);
            crc = EIE_FAIL;
        }

        /* validate variance period */
        s = EstimatorList->Estimator[i].VariancePeriod;
        if (!(strcmp (s, EIM_ESTIM_PERIOD_CURRENT_STRING) == 0 ||
                strcmp (s, EIM_ESTIM_PERIOD_HISTORICAL_STRING) == 0 ||
                strcmp (s, EIM_ESTIM_PERIOD_NOT_SPECIFIED_STRING) == 0)) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30044,  /* Estimator %d: VARIANCEPERIOD must be 'C', 'H' or 
                            missing. */
                i);
            crc = EIE_FAIL;
        }

        /* validate exclude imputed */
        s = EstimatorList->Estimator[i].ExcludeImputed;
        if (!(strcmp (s, "Y") == 0 ||
                strcmp (s, "N") == 0 ||
                strcmp (s, "") == 0)) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30045, /* Estimator %d: EXCLUDEIMPUTED must be 'Y', 'N' or 
                           missing. */ 
                i);
            crc = EIE_FAIL;
        }

        /* validate exclude outliers */
        s = EstimatorList->Estimator[i].ExcludeOutliers;
        if (!(strcmp (s, "Y") == 0 ||
                strcmp (s, "N") == 0 ||
                strcmp (s, "") == 0)) {
            EI_AddMessage ("Estimator", EIE_ERROR, 
                M30046, /* Estimator %d: EXCLUDEOUTLIERS must be 'Y', 'N' or 
                           missing.*/
                i);
            crc = EIE_FAIL;
        }

        /* validate random error */
        s = EstimatorList->Estimator[i].RandomError;
        if (!(strcmp (s, "Y") == 0 ||
                strcmp (s, "N") == 0)) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30047,  /* Estimator %d: RANDOMERROR must be 'Y' or 'N'. */
                i);
            crc = EIE_FAIL;
        }

        /* validate count criteria */
        CountCriteria = EstimatorList->Estimator[i].CountCriteria;
        if (CountCriteria != EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED) {
            if (CountCriteria < 1) {
                EI_AddMessage ("Estimator", EIE_ERROR, 
                    M30048, /* Estimator %d: COUNTCRITERIA must be 
                               an integer greater than or equal to 1.*/
                    i);
                crc = EIE_FAIL;
            }
        }

        /* validate percent criteria */
        PercentCriteria = EstimatorList->Estimator[i].PercentCriteria;
        if (PercentCriteria != EIM_ESTIM_PERCENTCRITERIA_NOT_SPECIFIED) {
            if (PercentCriteria <= 0 || PercentCriteria >= 100) {
                EI_AddMessage ("Estimator", EIE_ERROR, 
                    M30049, /* Estimator %d: PERCENTCRITERIA must be
                               a number greater than 0 and less than 100. */
                    i);
                crc = EIE_FAIL;
            }
        }

        if (EstimatorList->Estimator[i].Algorithm != NULL) {
            /* validate status */
            s = EstimatorList->Estimator[i].Algorithm->Status;
            if (strlen (s) < 1 || strlen (s) > 3) {
                EI_AddMessage ("Estimator", EIE_ERROR, 
                    M30050, /* Estimator %d: STATUS (%s) should be 1 to 3 
                               characters.*/ 
                    i, s);
                crc = EIE_FAIL;
            }

            /* validate type */
            if (EstimatorList->Estimator[i].Algorithm->Type ==
                    EIE_ALGORITHM_TYPE_INVALID) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30051, /* Estimator %d: TYPE must be '%s' or '%s'. */
                    i, EIM_ALGORITHM_TYPE_EF, EIM_ALGORITHM_TYPE_LR);
                crc = EIE_FAIL;
            }
            else {
                /* parse the formula */
                Stack = EI_StackAllocate ();
                if (Stack == NULL) return EIE_FAIL;

                if (EstimatorList->Estimator[i].Algorithm->Type ==
                       EIE_ALGORITHM_TYPE_EF)
                    rc = EI_ParseEFFormula (
                        EstimatorList->Estimator[i].Algorithm->Formula, Stack);
                else
                    rc = EI_ParseLRFormula (
                        EstimatorList->Estimator[i].Algorithm->Formula, Stack);

                if (rc != EIE_SUCCEED) {
                    EI_StackFree (Stack);
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        "Estimator %d FORMULA.", i);
                    crc = EIE_FAIL;
                }
                else
                    EstimatorList->Estimator[i].Stack = Stack;
            }
        }
    }
    if (crc != EIE_SUCCEED) return EIE_FAIL;

    /* set symbols of stack aux placeholders */
    SList_New (&AuxVariableList);
    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        /* set fieldid */
        EI_StackSetSymbols (EstimatorList->Estimator[i].Stack,
            "FIELDID", SymbolTable, EstimatorList->Estimator[i].FieldId);

        /* set aux variables */
        SList_Empty (AuxVariableList);
        if (EstimatorList->Estimator[i].AuxVariables[0] != '\0') {
            String2List (EstimatorList->Estimator[i].AuxVariables,
                AuxVariableList, ",", "", 0);
            for (j = 0; j < SList_NumEntries (AuxVariableList); j++) {
                sprintf (PlaceHolder, "AUX%d", j+1);
                rc = EI_StackSetSymbols (
                    EstimatorList->Estimator[i].Stack, PlaceHolder,
                    SymbolTable, SList_Entry (AuxVariableList, j));
                if (rc != EIE_SUCCEED) {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30052, /* Estimator %d: number of different auxiliary 
                                   variable names specified is greater than 
                                   number of formula placeholders. */
                        i);
                    crc = EIE_FAIL;
                    break; /* once is enough */
                }
            }
        }

        /* set betas */
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_LR){

            /* We do not care if it fails */
            EI_StackSetSymbols (EstimatorList->Estimator[i].Stack,
                EIM_BETA_PREFIX "0", SymbolTable, EIM_BETA_PREFIX "0");

            j = 0;
            rc = EIE_SUCCEED;
            while (rc == EIE_SUCCEED) {
                sprintf (PlaceHolder, EIM_BETA_PREFIX "%d", j+1);
                rc = EI_StackSetSymbols (
                    EstimatorList->Estimator[i].Stack,
                    PlaceHolder, SymbolTable, PlaceHolder);
                j++;
            }
        }
    }
    SList_Free (AuxVariableList);
    if (crc != EIE_SUCCEED) return EIE_FAIL;

    /* check if all placeholder have been filled */
    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        AreSymbolsMissing = EI_StackAreSymbolsMissing (
            EstimatorList->Estimator[i].Stack);
        if (AreSymbolsMissing == EIE_TRUE) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30053, /* Estimator %d: number of different auxiliary  
                           variable names specified is less than number 
                           of formula placeholders. */
                i);
            crc = EIE_FAIL;
        }
    }
    if (crc != EIE_SUCCEED) return EIE_FAIL;

    /* check that count and percent criteria */
    /* are only specified for EF with average and LR */
    /* and weight and variance variables are specified when not needed */
    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_LR)
            Type = TYPE_LR;
        else if (AreParametersRequestedInEstimator (
                &EstimatorList->Estimator[i]) == EIE_TRUE)
            Type = TYPE_EF_WITH_AVERAGE;
        else
            Type = TYPE_EF_WITHOUT_AVERAGE;

        if (Type == TYPE_EF_WITHOUT_AVERAGE) {
            if (EstimatorList->Estimator[i].RandomError[0] == 'N') {
                /*
                check that
                EXCLUDEIMPUTED and EXCLUDEOUTLIERS
                are not specified for EF without average and without randomerror
                */
                if (EstimatorList->Estimator[i].ExcludeImputed[0] != '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30054, /* Estimator %d: %s cannot be specified for
                                   an EF estimator that does not calculate
                                   an average or does not have random error. */ 
                        i, "EXCLUDEIMPUTED");
                    crc = EIE_FAIL;
                }
                if (EstimatorList->Estimator[i].ExcludeOutliers[0] != '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30054, /* Estimator %d: %s cannot be specified for 
                                   an EF estimator that does not calculate
                                   an average or does not have random error. */                                   
                        i, "EXCLUDEOUTLIERS");
                    crc = EIE_FAIL;
                }
            }
            else {
                /*
                check that
                EXCLUDEIMPUTED and EXCLUDEOUTLIERS
                are specified for EF with random error
                */
                if (EstimatorList->Estimator[i].ExcludeImputed[0] == '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30055, /* Estimator %d: %s must be specified for 
                                   an EF estimator that has random error. */                                   
                        i, "EXCLUDEIMPUTED");
                    crc = EIE_FAIL;
                }
                if (EstimatorList->Estimator[i].ExcludeOutliers[0] == '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30055, /* Estimator %d: %s must be specified for
                                   an EF estimator that has random error. */                                   
                        i, "EXCLUDEOUTLIERS");
                    crc = EIE_FAIL;
                }
            }
            /*
            check that
            COUNTCRITERIA, PERCENTCRITERIA and WEIGHTVARIABLE
            are not specified for EF without average
            */
            if (EstimatorList->Estimator[i].CountCriteria !=
                    EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30056, /* Estimator %d: %s cannot be specified for 
                               an EF estimator that does not calculate 
                               an average.*/                               
                    i, "COUNTCRITERIA");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].PercentCriteria !=
                    EIM_ESTIM_PERCENTCRITERIA_NOT_SPECIFIED) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30056, /* Estimator %d: %s cannot be specified for 
                               an EF estimator that does not calculate 
                               an average.*/                               
                    i, "PERCENTCRITERIA");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].WeightVariable[0] != '\0') {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30056, /* Estimator %d: %s cannot be specified for 
                               an EF estimator that does not calculate 
                               an average.*/  
                    i, "WEIGHTVARIABLE");
                crc = EIE_FAIL;
            }
        }
        else { /* Type != TYPE_EF_WITHOUT_AVERAGE */
            /*
            check that
            EXCLUDEIMPUTED, EXCLUDEOUTLIERS, COUNTCRITERIA and PERCENTCRITERIA
            are specified for LR and EF with average
            */
            if (EstimatorList->Estimator[i].ExcludeImputed[0] == '\0') {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30057,  /* Estimator %d: %s must be specified for 
                                an LR estimator or an EF estimator that 
                                calculates an average.*/
                    i, "EXCLUDEIMPUTED");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].ExcludeOutliers[0] == '\0') {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30057,  /* Estimator %d: %s must be specified for 
                                an LR estimator or an EF estimator that 
                                calculates an average.*/
                    i, "EXCLUDEOUTLIERS");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].CountCriteria ==
                    EIM_ESTIM_COUNTCRITERIA_NOT_SPECIFIED) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30057,  /* Estimator %d: %s must be specified for 
                                an LR estimator or an EF estimator that 
                                calculates an average.*/
                    i, "COUNTCRITERIA");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].PercentCriteria ==
                    EIM_ESTIM_PERCENTCRITERIA_NOT_SPECIFIED) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30057,  /* Estimator %d: %s must be specified for 
                                an LR estimator or an EF estimator that 
                                calculates an average.*/
                    i, "PERCENTCRITERIA");
                crc = EIE_FAIL;
            }
        }

        if (Type == TYPE_LR) {
            if (EstimatorList->Estimator[i].VarianceVariable[0] != '\0') {
                /*
                check that
                VARIANCEPERIOD and VARIANCEEXPONENT
                are specified for LR and VARIANCEVARIABLE is specified
                */
                if (EstimatorList->Estimator[i].VariancePeriod[0] == '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30058, /* Estimator %d: %s must be specified for
                                   an LR estimator when VARIANCEVARIABLE is 
                                   specified. */
                        i, "VARIANCEPERIOD");
                    crc = EIE_FAIL;
                }
                if (EstimatorList->Estimator[i].VarianceExponent ==
                        EIM_MISSING_VALUE) {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30058, /* Estimator %d: %s must be specified for
                                   an LR estimator when VARIANCEVARIABLE is 
                                   specified. */
                        i, "VARIANCEEXPONENT");
                    crc = EIE_FAIL;
                }
            }
            else {
                /*
                check that
                VARIANCEPERIOD and VARIANCEEXPONENT
                are not specified for LR and VARIANCEVARIABLE is not specified
                */
                if (EstimatorList->Estimator[i].VariancePeriod[0] != '\0') {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30059, /* Estimator %d: %s cannot be specified for
                                   an LR estimator when VARIANCEVARIABLE is
                                   not specified. */
                        i, "VARIANCEPERIOD");
                    crc = EIE_FAIL;
                }
                if (EstimatorList->Estimator[i].VarianceExponent !=
                        EIM_MISSING_VALUE) {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30059, /* Estimator %d: %s cannot be specified for
                                   an LR estimator when VARIANCEVARIABLE is
                                   not specified. */
                        i, "VARIANCEEXPONENT");
                    crc = EIE_FAIL;
                }
            }
        }
        else {
            /*
            check that
            VARIANCEVARIABLE, VARIANCEPERIOD and VARIANCEEXPONENT
            are not specified for EF
            */
            if (EstimatorList->Estimator[i].VarianceVariable[0] != '\0') {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30060, /* Estimator %d: %s cannot be specified
                               for an EF estimator. */
                    i, "VARIANCEVARIABLE");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].VariancePeriod[0] != '\0') {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30060, /* Estimator %d: %s cannot be specified
                               for an EF estimator. */
                    i, "VARIANCEPERIOD");
                crc = EIE_FAIL;
            }
            if (EstimatorList->Estimator[i].VarianceExponent !=
                    EIM_MISSING_VALUE) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30060, /* Estimator %d: %s cannot be specified
                               for an EF estimator. */
                    i, "VARIANCEEXPONENT");
                crc = EIE_FAIL;
            }
        }
    }
    if (crc != EIE_SUCCEED) return EIE_FAIL;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        Estimator = &EstimatorList->Estimator[i];
        Stack = Estimator->Stack;

/*        printf ("%d\n", i);*/
/*        EI_StackPrint (Stack);*/
/*        printf ("\n");*/

        if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_LR)
            Type = TYPE_LR;
        else if (AreParametersRequestedInEstimator (Estimator) == EIE_TRUE)
            Type = TYPE_EF_WITH_AVERAGE;
        else
            Type = TYPE_EF_WITHOUT_AVERAGE;

        /*
        check that the
        current value of FIELDID
        is not used as an auxiliary variable
        */
        for (j = 0; j < Stack->NumberEntries; j++) {
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL) {
                if ((strcmp (Stack->Entry[j].Symbol->Name,
                             Estimator->FieldId) == 0) &&
                        Stack->Entry[j].Symbol->Period == EIE_PERIOD_CURRENT &&
                        Stack->Entry[j].Symbol->Aggregate==EIE_AGGREGATE_VALUE){
                    EI_AddMessage ("Estimator", EIE_ERROR, 
                        M30061, /* Estimator %d: the current value of FIELDID %s
                                  is used as an auxiliary variable. */
                        i, Estimator->FieldId);
                    crc = EIE_FAIL;
                    break; /* once is enough */
                }
            }
        }

        /* check FIELDID and WEIGHTVARIABLE */
        if (Type == TYPE_LR) {
            /*
            check that
            FIELDID and WEIGHTVARIABLE
            is not the same variable
            */
            if (strcmp (Estimator->FieldId, Estimator->WeightVariable) == 0) {
                EI_AddMessage ("Estimator", EIE_ERROR,
                    M30062, /* Estimator %d: WEIGHTVARIABLE and FIELDID 
                               cannot be the same variable %s for an LR
                               estimator. */                     
                    i, Estimator->FieldId);
                crc = EIE_FAIL;
            }
        }
        else if (Type == TYPE_EF_WITH_AVERAGE) {
            /*
            check that
            FIELDID and WEIGHTVARIABLE if current
            is not the same variable
            */
            if (strcmp (Estimator->FieldId, Estimator->WeightVariable) == 0) {
                if (IsCurrentAverageRequested (Estimator) == EIE_TRUE) {
                    EI_AddMessage ("Estimator", EIE_ERROR,
                        M30063, /* Estimator %d: WEIGHTVARIABLE and 
                                   FIELDID cannot be the same variable %s
                                   for an EF estimator having at least one
                                   current average to compute. */
                        i, Estimator->FieldId);
                    crc = EIE_FAIL;
                }
            }
        }

        /*
        check that
        FIELDID and VARIANCEVARIABLE if current
        is not the same variable
        */
        if (Estimator->VarianceVariable[0] != '\0' &&
            strcmp (Estimator->VariancePeriod,
                EIM_ESTIM_PERIOD_CURRENT_STRING) == 0 &&
            strcmp (Estimator->FieldId, Estimator->VarianceVariable) == 0) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30064, /* Estimator %d: VARIANCEVARIABLE and FIELDID 
                           cannot be the same variable %s 
                           when VARIANCEPERIOD is current. */
                i, Estimator->FieldId);
            crc = EIE_FAIL;
        }

        /*
        check that
        WEIGHTVARIABLE and VARIANCEVARIABLE if current
        is not the same variable
        */
        if (Estimator->VarianceVariable[0] != '\0' &&
            strcmp (Estimator->VariancePeriod,
                EIM_ESTIM_PERIOD_CURRENT_STRING) == 0 &&
            strcmp (Estimator->VarianceVariable,
                Estimator->WeightVariable) == 0) {
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30065, /* Estimator %d: WEIGHTVARIABLE and VARIANCEVARIABLE 
                           cannot be the same variable %s when VARIANCEPERIOD
                           is current. */
                i, Estimator->WeightVariable);
            crc = EIE_FAIL;
        }

        /*
        check that
        WEIGHTVARIABLE
        is not used as an auxiliary variable
        */   
        for (j = 0; j < Stack->NumberEntries; j++) {
            if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL) {
                if (strcmp (Stack->Entry[j].Symbol->Name,
                        Estimator->WeightVariable) == 0) {
                    if (Type == TYPE_LR) {
                        /* for LR estimator: WEIGHTVARIABLE is a current variable */
                        if (Stack->Entry[j].Symbol->Period == EIE_PERIOD_CURRENT) {
                            EI_AddMessage ("Estimator", EIE_ERROR, 
                                M30066, /* Estimator %d: WEIGHTVARIABLE %s 
                                           cannot be be used as a current 
                                           auxiliary variable for an LR estimator. */
                                i, Estimator->WeightVariable);
                            crc = EIE_FAIL;
                            break; /* once is enough */
                        }
                    }
                    else {
                        EI_AddMessage ("Estimator", EIE_ERROR, 
                            M30067, /* Estimator %d: WEIGHTVARIABLE %s 
                                       cannot be used as an auxiliary variable
                                       for an EF estimator. */
                            i, Estimator->WeightVariable);
                        crc = EIE_FAIL;
                        break; /* once is enough */
                   }
                }
            }
        }

        /*
        LR estimator only:
           check that
           VARIANCEVARIABLE
           is not used as an auxiliary variable 
           for the same period
        */
        if (Type == TYPE_LR) {
            for (j = 0; j < Stack->NumberEntries; j++) {
                if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL) {
                    if (strcmp (Stack->Entry[j].Symbol->Name,
                            Estimator->VarianceVariable) == 0) {
                        if ((Stack->Entry[j].Symbol->Period == EIE_PERIOD_CURRENT) &&                           
                           strcmp (Estimator->VariancePeriod, 
                               EIM_ESTIM_PERIOD_CURRENT_STRING) == 0                 
                          ||
                           (Stack->Entry[j].Symbol->Period == EIE_PERIOD_HISTORICAL) &&   
                           strcmp (Estimator->VariancePeriod,
                               EIM_ESTIM_PERIOD_HISTORICAL_STRING) == 0 ) {

                           EI_AddMessage ("Estimator", EIE_ERROR, 
                               M30068, /* Estimator %d: VARIANCEVARIABLE %s 
                                          cannot be used as an auxiliary variable
                                          for the same period as VARIANCEPERIOD.*/
                               i, Estimator->VarianceVariable);
                           crc = EIE_FAIL;
                           break; /* once is enough */
                        }  
                    }
                }
            }
        }

    }
    if (crc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}
/*********************************************************************
Make sure weight and model variance variables are valid.
Invalid weight model variance variables are illegal.
A program receiving -1 from this function should print the message and quit.
For performance reason we keep a list of variable already checked.
*********************************************************************/
EIT_RETURNCODE EI_ValidateWeightAndVarianceVariables (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_DATATABLE * DataTable)
{
    int After;
    int Before;
    int i;
    tSList * List[2]; /* 2 periods */
    EIT_PERIOD Period;
    EIT_RETURNCODE rc;
    EIT_RETURNCODE crc; /* cumulative return code */

    crc = EIE_SUCCEED;

    SList_New (&List[EIE_PERIOD_CURRENT]);
    SList_New (&List[EIE_PERIOD_HISTORICAL]);

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_EF){
            if (EstimatorList->Estimator[i].Average != NULL) {
                if (EstimatorList->Estimator[i].Average->WeightIndex != -1) {

                    Period = EstimatorList->Estimator[i].Average->Period;

                    Before = SList_NumEntries (List[Period]);
                    SList_AddNoDup (EstimatorList->Estimator[i].WeightVariable,
                        List[Period]);
                    After = SList_NumEntries (List[Period]);

                    if (Before != After) {
                        rc = ValidateCommittedVariable (DataTable, i,
                            EstimatorList->Estimator[i].Average->WeightIndex,
                            Period, "WEIGHTVARIABLE");
                        if (rc != EIE_SUCCEED) crc = EIE_FAIL;
                    }
                }
            }
        }
        else {
            /* LR */
            if (EstimatorList->Estimator[i].Regression->WeightIndex != -1) {

                /* weight of regression always on current data */
                Period = EIE_PERIOD_CURRENT;

                Before = SList_NumEntries (List[Period]);
                SList_AddNoDup (EstimatorList->Estimator[i].WeightVariable,
                    List[Period]);
                After = SList_NumEntries (List[Period]);

                if (Before != After) {
                    rc = ValidateCommittedVariable (DataTable, i,
                        EstimatorList->Estimator[i].Regression->WeightIndex,
                        Period, "WEIGHTVARIABLE");
                    if (rc != EIE_SUCCEED) crc = EIE_FAIL;
                }
            }

            if (EstimatorList->Estimator[i].Regression->VarianceIndex != -1) {

                Period = EstimatorList->Estimator[i].Regression->VariancePeriod;

                Before = SList_NumEntries (List[Period]);
                SList_AddNoDup (EstimatorList->Estimator[i].VarianceVariable,
                    List[Period]);
                After = SList_NumEntries (List[Period]);

                if (Before != After) {
                    rc = ValidateCommittedVariable (DataTable, i,
                        EstimatorList->Estimator[i].Regression->VarianceIndex,
                        Period, "VARIANCEVARIABLE");
                    if (rc != EIE_SUCCEED) crc = EIE_FAIL;
                }
            }
        }
    }

    SList_Free (List[EIE_PERIOD_CURRENT]);
    SList_Free (List[EIE_PERIOD_HISTORICAL]);

    return crc;
}


/*********************************************************************
AddToPartialSums
Returns EIE_TRUE if the value is added to the partial sums (observation
contributes to the average), otherwise returns EIE_FALSE.
*********************************************************************/
static EIT_BOOLEAN AddToPartialSums (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * Data)
{
    EIT_AVERAGE * Average;
    EIT_BOOLEAN rc;
    double Weight;

    rc = EIE_FALSE;
    for (Average = Estimator->Average; Average!=NULL; Average = Average->Next) {
        if (Average->WeightIndex == -1) {
            /* unweighted average */
            Average->Count++;
            Average->Numerator += Data->DataRec[Average->Period]->
                FieldValue[Average->VariableIndex];
            Average->Denominator++;
            rc = EIE_TRUE;
        }
        else {
            /* weighted average */
            Weight = Data->DataRec[Average->Period]->
                FieldValue[Average->WeightIndex];

            /* obs with weight == 0 are rejected */
            if (Weight > 0.0) {
                Average->Count++;
                Average->Numerator += Data->DataRec[Average->Period]->
                    FieldValue[Average->VariableIndex] * Weight;
                Average->Denominator += Weight;
                rc = EIE_TRUE;
            }
        }
    }
    return rc;
}
/*********************************************************************
random error is
when LR and variance is used
RandomError = Residual * Variance of Recipient / Variance of Donor

otherwise it is
RandomError = Residual
*********************************************************************/
static double AddVarianceToResidual (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * RecipientData,
    EIT_DATA * DonorData,
    double Residual)
{
/*
when LR and variance in used the residual is
RandomError[i]=Residual[j]*sqrt((variace[i]^exponent)/(variance[j]^exponent))
or which is the same
RandomError[i]=Residual[j]*sqrt(variace[i]^exponent)/sqrt(variance[j]^exponent)

this define is a shortcut to calculate the numerator and the denominator

D is a ptr to EIT_DATA
V is a ptr to a Variance Info
*/
#define SQRTPOW(D,V) \
    (sqrt(pow((D)->DataRec[(V)->VariancePeriod]->FieldValue[ \
        (V)->VarianceIndex],(V)->VarianceExponent)))

    double RandomError;

    /* check if variance is used */
    if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_LR &&
            Estimator->Regression->VarianceIndex != -1) {
        RandomError = Residual *
            SQRTPOW (RecipientData, Estimator->Regression) /
            SQRTPOW (DonorData, Estimator->Regression);
    }
    else {
        RandomError = Residual;
    }

    return RandomError;
}
/*********************************************************************
checks if at least one parameter is an requested in estimator.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN AreParametersRequestedInEstimator (
    EIT_ESTIMATORSPEC * Estimator)
{
    int i;

    /* if LR return EIE_TRUE */
    if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_LR)
        return EIE_TRUE;

    for (i = 0; i < Estimator->Stack->NumberEntries; i++)
        if (Estimator->Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL &&
                Estimator->Stack->Entry[i].Aggregate == EIE_AGGREGATE_AVERAGE)
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one estimator computes an average
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN AtLeastOneAverageRequested (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++)
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_EF&&
                EstimatorList->Estimator[i].Average != NULL)
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one estimator is a active.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN AtLeastOneEstimatorIsActive (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++)
        if (EstimatorList->Estimator[i].Active == EIE_TRUE)
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
checks if at least one estimator uses residual.
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN AtLeastOneEstimatorUsesResidual (
    EIT_ESTIMATORSPEC_LIST * EstimatorList)
{
    int i;

    for (i = 0; i < EstimatorList->NumberEntries; i++)
        if (EstimatorList->Estimator[i].RandomError[0] == 'Y')
            return EIE_TRUE;

    return EIE_FALSE;
}
/*********************************************************************
Make sure weight or variance variables are valid.
the only valid value is FIELDOK with a positive value.
*********************************************************************/
static EIT_RETURNCODE CheckStatus (
    int EstimatorIndex,
    char * Key,
    double Value,
    int Status,
    char * VariablePeriod, /* must be M00033 ("current" + 'female gender' form) 
                                   or M00034 ("historical") */
    char * VariableType)   /* must be "WEIGHTVARIABLE" or "VARIANCEVARIABLE"  */ 
                           
{
    switch (Status) {
    case FIELDOK:
        if (Value < 0.0) {
            /* negative value */
            EI_AddMessage ("Estimator", EIE_ERROR,
                M30070, /* Estimator %d: key %s has 
                           a negative %s %s. */
                EstimatorIndex, Key, VariablePeriod, VariableType);
            return EIE_FAIL;
        }
        break;

    case FIELDMISS:
        /* missing value */
        EI_AddMessage ("Estimator", EIE_ERROR,
            M30071, /* Estimator %d: key %s has
                       a missing %s %s. */
            EstimatorIndex, Key, VariablePeriod, VariableType);
        return EIE_FAIL;

    default:
        /* anything else is bad */
        EI_AddMessage ("Estimator", EIE_ERROR,
            M30072, /* Estimator %d: key %s has 
                       a status for the %s %s. */
            EstimatorIndex, Key, VariablePeriod, VariableType);
        return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
compute averages of an estimator.
returns EIE_FALSE when one of the average cannot be calculated.
the caller should use this info to set estimator as inactive
*********************************************************************/
static EIT_BOOLEAN ComputeAverages (
    EIT_ESTIMATORSPEC * Estimator)
{
    EIT_AVERAGE * Average;

    for (Average = Estimator->Average; Average != NULL; Average=Average->Next) {
        if (Average->Denominator == 0.0) {
            return EIE_FALSE;
        }
        else {
            Average->Average = Average->Numerator / Average->Denominator;
        }
    }
    return EIE_TRUE;
}
/*********************************************************************
estimate (hopefully!) every field to impute.
*********************************************************************/
static EIT_RETURNCODE DoEstimation (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_SYMBOL ** SymbolTable,
    EIT_DATATABLE * DataTable,
    EIT_BOOLEAN ImputeNegativeValues)
{
    EIT_DATA * Data;
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    int j;
    EIT_BOOLEAN ObsImputable;
    EIT_RETURNCODE rc;

    if (AtLeastOneEstimatorIsActive (EstimatorList) == EIE_FALSE)
        return EIE_SUCCEED;

    /* rene: to do */
    /* EI_SetParametersInStack (EstimatorList);*/

    /* random error */
    if (AtLeastOneEstimatorUsesResidual (EstimatorList) == EIE_TRUE) {
        FindRandomErrorDonors (EstimatorList, DataTable);
    }

    /* do estimation */
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {

            SetValues (SymbolTable, Data);

            for (j = 0; j < EstimatorList->NumberEntries; j++) {

                Estimator = &EstimatorList->Estimator[j];

                if (Estimator->Active == EIE_TRUE) {
                    /* rene: peut on setter les parametres dans les stacks
                    au lieu de la table de symbole? */
                    if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_EF)
                        SetAverages (SymbolTable, Estimator->Average);
                    else
                        SetBetas (SymbolTable, Estimator->Regression);

                    ObsImputable = IsObsImputable (Estimator, Data);

                    if (ObsImputable == EIE_TRUE) {
                        rc = Impute (Estimator, SymbolTable, Data,
                            ImputeNegativeValues, j);
                        if (rc == EIE_FAIL) return EIE_FAIL;
                    }
                }
            }
        }
    }

    return EIE_SUCCEED;
}
/*********************************************************************
find random error donors
*********************************************************************/
static void FindRandomErrorDonors (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    EIT_DATATABLE * DataTable)
{
    EIT_DATA * Data;
    EIT_ESTIMATORSPEC * Estimator;
    int i;
    int j;
    EIT_BOOLEAN ObsRandomErrorDonor;
    double Weight;

    /* accumulate random error donors */
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {

            for (j = 0; j < EstimatorList->NumberEntries; j++) {

                Estimator = &EstimatorList->Estimator[j];

                if (Estimator->Active == EIE_FALSE ||
                        Estimator->Residual == NULL)
                    continue;

                ObsRandomErrorDonor = IsObsRandomErrorDonor (Estimator, Data);

                /* only LR use weighted selection of random error */
                if (ObsRandomErrorDonor == EIE_TRUE) {
                    if (Estimator->Algorithm->Type == EIE_ALGORITHM_TYPE_LR &&
                            Estimator->Regression->WeightIndex != -1)
                        Weight = Data->DataRec[EIE_PERIOD_CURRENT]->
                            FieldValue[Estimator->Regression->WeightIndex];
                    else
                        Weight = 1.0;

                    EI_ResidualAdd (Estimator->Residual, Data, Weight);
                }
            }
        }
    }

    /* print random error donors statistics */
    for (i = 0; i < EstimatorList->NumberEntries; i++) {

        Estimator = &EstimatorList->Estimator[i];

        if (Estimator->Active == EIE_FALSE ||
                Estimator->Residual == NULL)
            continue;

        EI_AddMessage ("Estimator", EIE_INFORMATION, 
            M10014 "\n", /* Estimator %d: 
                            Number of random error donors is %d. */
            i, Estimator->Residual->NumberEntries);

        if (Estimator->Residual->NumberEntries <
                ACCEPTABLENUMBEROFRANDOMERRORDONOR) {
            EI_AddMessage ("Estimator", EIE_WARNING,
                M20024 "\n", /* Estimator %d: Number of random error donors is 
                                smaller than %d. Variance of the imputed values
                                will not be good. */
                i, ACCEPTABLENUMBEROFRANDOMERRORDONOR);
        }
    }
}
/*********************************************************************
Az[i+1] = FindXMax (X, r, c);
*********************************************************************/
static double FindXMax (
    double ** X,
    int r, /* number of rows */
    int c) /* the column calculated */
{
    double AbsXic;
    int i;
    double MaxAbsYi;

    MaxAbsYi = -1;

    for (i = 0; i < r; i++) {
        AbsXic = fabs (X[i][c]);
        if (MaxAbsYi < AbsXic)
            MaxAbsYi = AbsXic;
    }

    if (MaxAbsYi == 0.0)
        MaxAbsYi = 1.0;

    return MaxAbsYi;
}
/*********************************************************************
Az[0] = FindYMax (Y, r);
*********************************************************************/
static double FindYMax (
    double * Y,
    int r) /* number of rows */
{
    double AbsYi;
    int i;
    double MaxAbsYi;

    MaxAbsYi = -1;

    for (i = 0; i < r; i++) {
        AbsYi = fabs (Y[i]);
        if (MaxAbsYi < AbsYi)
            MaxAbsYi = AbsYi;
    }

    if (MaxAbsYi == 0.0)
        MaxAbsYi = 1.0;

    return MaxAbsYi;
}
/*********************************************************************
computes the estimated value, and if all is well updates the datarec
*********************************************************************/
static EIT_RETURNCODE Impute (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_SYMBOL ** SymbolTable,
    EIT_DATA * Data,
    EIT_BOOLEAN ImputeNegativeValues,
    int EstimatorIndex)
{
    EIT_DATAREC * DataRec;
    double ImputedValue;
    EIT_STACKEVALUATE_RETURNCODE rc;
    EIT_SYMBOL * Symbol;

    /* evaluate the estimator for the recipient */
    rc = EI_StackEvaluate (Estimator->Stack, &ImputedValue);

    switch (rc) {
    case EIE_STACKEVALUATE_SUCCEED:
        /* add random error if requested */
        if (Estimator->RandomError[0] == 'Y') {
            ImputedValue += RandomError (Estimator, SymbolTable, Data,
                ImputedValue);
        }

        if (ImputedValue < 0.0 &&
                ImputeNegativeValues == EIE_FALSE) {
            Estimator->NegativeImputation++;
            return EIE_SUCCEED;
        }

        /* use StatusFlag to hold the index of the estimator that imputed
        the data. use a negative value -1-EstimatorIndex, this way we can
        remember that the variable was imputed in this run.
        later we can compute -1-StatusFlag to get the Estimator Index.
        example:
        EstimatorIndex StatusFlag     Back to EstimatorIndex
        0              -1-0 = -1      -1-(-1) = 0
        1              -1-1 = -2      -1-(-2) = 1
        2              -1-2 = -3      -1-(-3) = 2
        etc */
        DataRec = Data->DataRec[EIE_PERIOD_CURRENT];
        DataRec->StatusFlag[Estimator->FieldIdIndex] = -1-EstimatorIndex;
        DataRec->FieldValue[Estimator->FieldIdIndex] = ImputedValue;
        Estimator->Imputed++;

        /* set the value in symbol table, if necessary. the symbol table needs
        to be refreshed when the variable is used later to impute another
        variable */
        Symbol = EI_SymbolTableLookup (SymbolTable,
            DataRec->FieldName[Estimator->FieldIdIndex], EIE_PERIOD_CURRENT,
            EIE_AGGREGATE_VALUE, EIM_MISSING_VALUE, EIE_SYMBOL_LOOKUPTYPE_FIND);
        if (Symbol != NULL)
            Symbol->Value = ImputedValue;

        break;

    case EIE_STACKEVALUATE_DIVISIONBYZERO:
        Estimator->DivisionByZero++;
        EI_AddMessage ("Evaluation", EIE_WARNING,
            M20025 "\n", /* Estimator %d: %s, division by zero. */ 
            EstimatorIndex, Data->Key);
        break;

    case EIE_STACKEVALUATE_FAIL:
    default:
        /* trap EI_StackEvaluate () error */
        /* when this happen stop everything */
        return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
checks if the estimator computes a current average
Returns EIE_TRUE if there is at least one.
EIE_FALSE otherwise.
*********************************************************************/
static EIT_BOOLEAN IsCurrentAverageRequested (
    EIT_ESTIMATORSPEC * Estimator)
{
    int i;
    EIT_STACK * Stack;

    Stack = Estimator->Stack;

    for (i = 0; i < Stack->NumberEntries; i++) {
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL) {
            if (Stack->Entry[i].Symbol->Aggregate == EIE_AGGREGATE_AVERAGE &&
                    Stack->Entry[i].Symbol->Period == EIE_PERIOD_CURRENT) {
                return EIE_TRUE;
            }
        }
    }

    return EIE_FALSE;
}
/*********************************************************************
is this an acceptable observation for all averages of this estimator?
return as soon as one is found unacceptable
*********************************************************************/
static EIT_BOOLEAN IsObsAcceptableForAverage (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * Data)
{
    EIT_AVERAGE * Average;
    EIT_BOOLEAN VariableValid;

    /* check if current obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_CURRENT] == EIE_EXCLUDED)
        return EIE_FALSE;

    /* check if historical obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_HISTORICAL] == EIE_EXCLUDED)
        return EIE_FALSE;

    for (Average = Estimator->Average; Average != NULL; Average=Average->Next) {

        /* check if data exists */
        if (Data->DataRec[Average->Period] == NULL)
            return EIE_FALSE;

        VariableValid = IsVariableValid (Estimator,
            Data->DataRec[Average->Period]->StatusFlag[Average->VariableIndex]);

        if (VariableValid == EIE_FALSE)
            return EIE_FALSE;
    }
    return EIE_TRUE;
}
/*********************************************************************
is this an acceptable observation for the regression of this estimator?
return as soon as one is found unacceptable
*********************************************************************/
static EIT_BOOLEAN IsObsAcceptableForRegression (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * Data)
{
    EIT_AUX * Aux;
    EIT_BOOLEAN VariableValid;

    /* check if current obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_CURRENT] == EIE_EXCLUDED)
        return EIE_FALSE;

    /* check if historical obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_HISTORICAL] == EIE_EXCLUDED)
        return EIE_FALSE;

    /* Check the response variable. */
    VariableValid = IsVariableValid (Estimator,
        Data->DataRec[EIE_PERIOD_CURRENT]->
            StatusFlag[Estimator->Regression->FieldIdIndex]);
    if (VariableValid == EIE_FALSE)
        return EIE_FALSE;

    /* Check the aux variables */
    for (Aux = Estimator->Regression->Aux; Aux != NULL; Aux = Aux->Next) {

        /* check if data exists */
        if (Data->DataRec[Aux->Period] == NULL)
            return EIE_FALSE;

        VariableValid = IsVariableValid (Estimator,
            Data->DataRec[Aux->Period]->StatusFlag[Aux->AuxIndex]);

        if (VariableValid == EIE_FALSE)
            return EIE_FALSE;

    }
    return EIE_TRUE;
}
/*********************************************************************
Observation is imputable if:
the fieldid is FTI, and,
lateral variables are not FTI.

side effect: increment count of FTI FieldId.
*********************************************************************/
static EIT_BOOLEAN IsObsImputable (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * Data)
{
    int i;
    int Status;

    /* FieldId must be FTI */
    if (Data->DataRec[EIE_PERIOD_CURRENT]->
            StatusFlag[Estimator->FieldIdIndex] != FIELDFTI)
        return EIE_FALSE;

    Estimator->FTI++;

    /* all lateral variables must not be FTI */
    for (i = 0; i < Estimator->LateralVariable.NumberOfVariables; i++) {

        /* hist datarec can be NULL */
        if (Data->DataRec[Estimator->LateralVariable.Period[i]] == NULL)
            return EIE_FALSE;

        Status = Data->DataRec[Estimator->LateralVariable.Period[i]]->
            StatusFlag[Estimator->LateralVariable.Index[i]];

        if (Status == FIELDFTI)
            return EIE_FALSE;
    }

    return EIE_TRUE;
}
/*********************************************************************
Observation is a random error donor when:
the fieldid is valid, and,
lateral variables are valids.

see IsVariableValid() for a definition of a valid variable.
*********************************************************************/
static EIT_BOOLEAN IsObsRandomErrorDonor (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * Data)
{
    int i;
    EIT_BOOLEAN VariableValid;

    /* check if current obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_CURRENT] == EIE_EXCLUDED)
        return EIE_FALSE;

    /* check if historical obs was marked as excluded obs */
    if (Data->ExcludedObs[EIE_PERIOD_HISTORICAL] == EIE_EXCLUDED)
        return EIE_FALSE;

    /* FieldId must be valid */
    VariableValid = IsVariableValid (Estimator,
        Data->DataRec[EIE_PERIOD_CURRENT]->StatusFlag[Estimator->FieldIdIndex]);
    if (VariableValid == EIE_FALSE)
        return EIE_FALSE;

    /* all lateral variables must be valid */
    for (i = 0; i < Estimator->LateralVariable.NumberOfVariables; i++) {

        /* hist datarec can be NULL */
        if (Data->DataRec[Estimator->LateralVariable.Period[i]] == NULL)
            return EIE_FALSE;

        /* lateral variable must be valid */
        VariableValid = IsVariableValid (Estimator,
            Data->DataRec[Estimator->LateralVariable.Period[i]]->
                StatusFlag[Estimator->LateralVariable.Index[i]]);
        if (VariableValid == EIE_FALSE)
            return EIE_FALSE;
    }

    return EIE_TRUE;
}
/*********************************************************************
is this variable valid?
a variable is considered valid when:
it is not FTI, and
it is not imputed when imputed variable are excluded, and
it is not an FTE when outliers are excluded.
*********************************************************************/
static EIT_BOOLEAN IsVariableValid (
    EIT_ESTIMATORSPEC * Estimator,
    int Status)
{
    if ((Status == FIELDFTI) ||
            (Estimator->ExcludeImputed[0] == 'Y' && Status == FIELDIMPUTED) ||
            (Estimator->ExcludeOutliers[0] == 'Y' && Status == FIELDFTE))
        return EIE_FALSE;

    return EIE_TRUE;
}
/*********************************************************************
Returns the weights/variances vector.
*********************************************************************/
static double * MakeDMatrix (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATATABLE * DataTable,
    int * NumRecs)
{
    double *D; /* The return vector. */
    double temp;
    int    i;
    int    D_Alloc; /* A check value. */
    EIT_DATA * Data;
    double    VarExponent;
    EIT_PERIOD VarPeriod;
    int    VarIndex;
    int    WeightIndex;

    /* If there is neither weights nor variances we are done. */
    if ((Estimator->Regression->WeightIndex < 0) &&
            (Estimator->Regression->VarianceIndex < 0))
        return NULL;

    D = STC_AllocateMemory (*NumRecs * sizeof *D);
    if (D == NULL)
        return NULL;
    D_Alloc = *NumRecs;
    *NumRecs = 0;

    VarIndex = Estimator->Regression->VarianceIndex;
    VarExponent = Estimator->Regression->VarianceExponent;
    VarPeriod = Estimator->Regression->VariancePeriod;
    WeightIndex = Estimator->Regression->WeightIndex;

    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            if (!IsObsAcceptableForRegression (Estimator, Data))
                continue;

            /* The weight value (always CURRENT) / the variance value. */
            temp = (WeightIndex < 0 ?  1 : Data->DataRec[EIE_PERIOD_CURRENT]->FieldValue[WeightIndex])
             / (VarIndex < 0 ?  1 : pow (Data->DataRec[VarPeriod]->FieldValue[VarIndex], VarExponent));

            D[*NumRecs] = sqrt (temp);
            *NumRecs += 1;
            if (*NumRecs > D_Alloc) /* This should never happen. */
                return NULL;
        }
    }
    return D;
}


/*********************************************************************
 * The parameter 'NumXRecs' is from the previous call to MakeYMatrix.
 * Both matrices should have the same number of records.
 *********************************************************************/
static double ** MakeXMatrix (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATATABLE * DataTable,
    int * NumXRecs)
{
    EIT_AUX * Aux;
    EIT_DATA * Data;
    int i;
    int k;
    double **X; /*The return matrix.*/
    int X_Alloc; /*A check value.*/

    /*-----------------------------------------------*/
    /* Allocate the matrix: */
    X = STC_AllocateMemory (*NumXRecs * sizeof *X);
    if (X == NULL) return NULL;

    X_Alloc = *NumXRecs;

    for (i = 0; i < *NumXRecs; i++) {
        X[i] = STC_AllocateMemory (
            sizeof *X[i] * (Estimator->Regression->NumberOfAux +
                Estimator->Regression->Intercept));
        if (X[i] == NULL) {
            UTIL_FreeDouble0Matrix (X, i - 1);
            return NULL;
        }
    }
    /*-----------------------------------------------*/
    /* Fill the Matrix.*/
    *NumXRecs = 0;
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            if (!IsObsAcceptableForRegression (Estimator, Data)) continue;
            if (Estimator->Regression->Intercept)
                /* The first column if there is an intercept */
                X[*NumXRecs][0] = 1.0;

            for (k = 0, Aux = Estimator->Regression->Aux; Aux != NULL;
                    Aux = Aux->Next, k++)
                X[*NumXRecs][k + Estimator->Regression->Intercept] =
                    pow (Data->DataRec[Aux->Period]->FieldValue[Aux->AuxIndex],
                        Aux->ExponentValue);

            *NumXRecs += 1;
            if (*NumXRecs > X_Alloc)/* This should never happen. */
                return NULL;
        }
    }
    return X;
}
/*********************************************************************
 *Y always uses current data.
 *NumRecs will be the number of records in Y.
 *(saves info in AcceptableKeysResult list as a side effect)
 *to do error detection when you call this function
 *test for Y == NULL && NumYRecs != 0
 *********************************************************************/
static double * MakeYMatrix (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATATABLE * DataTable,
    int * NumYRecs)
{
    EIT_DATA * Data;
    int i;
    double * Y; /* The return vector.*/
    int Y_Alloc; /* The number of columns (a return value).*/

    *NumYRecs = -1; /* keep this for error detection later */

    Y_Alloc = 1000;
    Y = STC_AllocateMemory (Y_Alloc * sizeof *Y);
    if (Y == NULL)
        return NULL;

    *NumYRecs = 0;
    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            if (!IsObsAcceptableForRegression (Estimator, Data))
                continue;

            /* realloc for Y if necessary.*/
            if (*NumYRecs >= Y_Alloc) {
                Y = STC_ReallocateMemory (Y_Alloc * sizeof *Y,
                    (Y_Alloc + 1000) * sizeof *Y, Y);
                if (Y == NULL)
                    return NULL;
                Y_Alloc += 1000;
            }

            Y[*NumYRecs] = Data->DataRec[EIE_PERIOD_CURRENT]->
                FieldValue[Estimator->Regression->FieldIdIndex];
            *NumYRecs += 1;

            SaveAcceptableKeysResult (Estimator, Data->Key);
        }
    }

    Y = STC_ReallocateMemory (Y_Alloc * sizeof *Y,
        (*NumYRecs) * sizeof *Y, Y);
    if (Y == NULL)
        return NULL;

    return Y;
}
/*********************************************************************
Just for debugging.  Prints a 0-offset matrix.
*********************************************************************/
static void PrintMat0 (
    double ** mat,
    int r,
    int c,
    char * Name)
{
    int i;
    int k;

    printf ("MATRIX %s\n", Name);
    for (k=0;k<r;k++) {
        for (i=0;i<c;i++)
            printf ("%e ", mat[k][i]);
        printf (",\n");
    }
    printf ("\n");
}
/*********************************************************************
Just for debugging.  Prints a unit-offset matrix.
*********************************************************************/
static void PrintMat1 (
    double ** mat,
    int r,
    int c,
    char * Name)
{
    int i;
    int k;

    printf ("MATRIX %s\n", Name);
    for (k=1;k<=r;k++) {
        for (i=1;i<=c;i++)
            printf ("%.0f ", mat[k][i]);
        printf ("\n");
    }
    printf ("\n");
}
/*********************************************************************
Just for debugging.  Prints a 0-offset vector.
*********************************************************************/
static void PrintVec (
    double * vec,
    int r,
    char * Name)
{
    int i;

    printf ("VECTOR %s\n", Name);
    for (i = 0; i < r; i++)
        printf ("%e\n", vec[i]);
    printf ("\n");
}
/*********************************************************************
computes a random error and
saves info in RandomErrorResult list as a side effect

residual is real value - fitted value
random error is the residual
or if variance is used
random error is the residual * variance. see AddVarianceToResidual()
for more details

returns 0.0 if it cannot compute a random error
*********************************************************************/
static double RandomError (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_SYMBOL ** SymbolTable,
    EIT_DATA * RecipientData,
    double ImputedValue)
{
    EIT_DATA * DonorData;
    double FittedValue;
    int Index;
    int Iteration;
    double RandomError;
    int rc;
    double RealValue;
    double Residual;

    if (Estimator->Residual->NumberGoodEntries == 0)
        return 0.0;

    Iteration = 0;

    do {
        Index = EI_ResidualSelection (Estimator->Residual);

        DonorData = Estimator->Residual->Data[Index];

        if (Estimator->Residual->Residual[Index] == EIM_MISSING_VALUE) {
            /* donor's residual not already calculated,
            let's calculate it and save the value */

            RealValue = DonorData->DataRec[EIE_PERIOD_CURRENT]->
                FieldValue[Estimator->FieldIdIndex];

            /* put donor values in SymbolTable */
            SetValues (SymbolTable, DonorData);

            /* evaluate the estimator for the donor */
            rc = EI_StackEvaluate (Estimator->Stack, &FittedValue);

            if (rc == EIE_STACKEVALUATE_SUCCEED) {
                /* residual is donor real value - its fitted value */
                Residual = RealValue - FittedValue;

                RandomError = AddVarianceToResidual (Estimator,
                    RecipientData, DonorData, Residual);

                ImputedValue += RandomError;

                SaveRandomErrorResult (Estimator, RecipientData, DonorData,
                    Residual, RandomError, ImputedValue);

                /* save residual for later */
                Estimator->Residual->Residual[Index] = Residual;

                /* reset values of recipient in SymbolTable */
                SetValues (SymbolTable, RecipientData);

                return RandomError;
            }
            else {
                Estimator->Residual->NumberGoodEntries--;
                Estimator->Residual->Residual[Index] = EIM_RESIDUAL_NOTGOOD;
            }
        }
        else if (Estimator->Residual->Residual[Index] == EIM_RESIDUAL_NOTGOOD) {
            /* donor's was found to be bad, nothing we can do with it */
            ;
        }
        else {
            /* donor's residual already calculated, use that saved value */

            /* use saved residual */
            Residual = Estimator->Residual->Residual[Index];

            RandomError = AddVarianceToResidual (Estimator, RecipientData,
                DonorData, Residual);

            ImputedValue += RandomError;

            SaveRandomErrorResult (Estimator, RecipientData, DonorData,
                Residual, RandomError, ImputedValue);

            /* reset values of recipient in SymbolTable */
            SetValues (SymbolTable, RecipientData);

            return RandomError;
        }
    } while (++Iteration < Estimator->Residual->NumberEntries * 2);

    /* reset values of recipient in SymbolTable */
    SetValues (SymbolTable, RecipientData);

    return 0.0;
}
/*********************************************************************
Adds info in EIT_ACCEPTABLEKEYSRESULT_LIST struct for futur use
*********************************************************************/
static void SaveAcceptableKeysResult (
    EIT_ESTIMATORSPEC * Estimator,
    char * CurrentKey)
{
    if (Estimator->AcceptableKeysResult != NULL) {
        Estimator->AcceptableKeysResult =
            EI_AcceptableKeysResultAdd (Estimator->AcceptableKeysResult,
                CurrentKey);
    }
}
/*********************************************************************
Adds info in EIT_RANDOMERRORRESULT struct for futur use
*********************************************************************/
static void SaveRandomErrorResult (
    EIT_ESTIMATORSPEC * Estimator,
    EIT_DATA * RecipientData,
    EIT_DATA * DonorData,
    double Residual,
    double RandomError,
    double ImputedValue)
{
    if (Estimator->RandomErrorResult != NULL) {
        Estimator->RandomErrorResult =
            EI_RandomErrorResultAdd (Estimator->RandomErrorResult,
                RecipientData->Key, DonorData->Key, Estimator->FieldId,
                Residual, RandomError,
                RecipientData->DataRec[EIE_PERIOD_CURRENT]->
                    FieldValue[Estimator->FieldIdIndex],
                ImputedValue);
    }
}
/*********************************************************************
change data to make it lie in the range [-1, 1]
Ay = ScaleToOrderUnity (Y, X, NumRecs, AuxInt);
*********************************************************************/
static double * ScaleToOrderUnity (
    double * Y,
    double ** X,
    int r, /* number of rows */
    int c) /* number of columns */
{
    double * Az;
    int i;

    Az = STC_AllocateMemory ((c+1) * sizeof *Az);

    Az[0] = FindYMax (Y, r);
    ScaleYColumn (Y, Az[0], r);

    for  (i = 0; i < c; i++) {
        Az[i+1] = FindXMax (X, r, i);
        ScaleXColumn (X, Az[i+1], r, i);
    }

    return Az;
}
/*********************************************************************
ScaleXColumn (X, Az[0], r, c);
*********************************************************************/
static void ScaleXColumn (
    double ** X,
    double Az,
    int r, /* number of rows */
    int c) /* the column calculated */
{
    int i;

    for (i = 0; i < r; i++) {
        X[i][c] /= Az;
    }
}
/*********************************************************************
ScaleYColumn (Y, Az[0], r);
*********************************************************************/
static void ScaleYColumn (
    double * Y,
    double Az,
    int r) /* number of rows */
{
    int i;

    for (i = 0; i < r; i++) {
        Y[i] /= Az;
    }
}
/*********************************************************************
set value of averages into symbol table
*********************************************************************/
static void SetAverages (
    EIT_SYMBOL ** SymbolTable,
    EIT_AVERAGE * Average)
{
    EIT_SYMBOL * Symbol;

    for (; Average != NULL; Average = Average->Next) {
        Symbol = EI_SymbolTableLookup (SymbolTable, Average->Name,
            Average->Period, EIE_AGGREGATE_AVERAGE, EIM_MISSING_VALUE,
            EIE_SYMBOL_LOOKUPTYPE_FIND);
        assert (Symbol != NULL);
        Symbol->Value = Average->Average;
    }
}
/*********************************************************************
set value of betas into symbol table
*********************************************************************/
static void SetBetas (
    EIT_SYMBOL ** SymbolTable,
    EIT_REGRESSION * Regression)
{
    EIT_AUX * Aux;
    int i;
    EIT_SYMBOL * Symbol;

    if (Regression->Intercept == EIE_TRUE) {
        Symbol = EI_SymbolTableLookup (SymbolTable, EIM_BETA_PREFIX "0",
            EIE_PERIOD_CURRENT, EIE_AGGREGATE_VALUE, EIM_MISSING_VALUE,
            EIE_SYMBOL_LOOKUPTYPE_FIND);
        assert (Symbol != NULL);
        Symbol->Value = Regression->Betas[0];
    }

    Aux = Regression->Aux;
    for (i = 0; i < Regression->NumberOfAux; i++, Aux = Aux->Next) {
        Symbol = EI_SymbolTableLookup (SymbolTable, Aux->BetaName,
            EIE_PERIOD_CURRENT, EIE_AGGREGATE_VALUE, EIM_MISSING_VALUE,
            EIE_SYMBOL_LOOKUPTYPE_FIND);
        assert (Symbol != NULL);
        Symbol->Value = Regression->Betas[i+Regression->Intercept];
    }
}
/*********************************************************************
Sets up averages for all estimators with averages
*********************************************************************/
static void SetUpAverages (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    char ** InDataVariableName,
    int InDataNumberOfVariables,
    char ** InHistVariableName,
    int InHistNumberOfVariables)
{
    EIT_AVERAGE * AverageCurrent;
    EIT_AVERAGE * AverageHead;
    int i;
    int j;
    char * Name;
    int NumberOfVariables;
    EIT_PERIOD Period;
    EIT_STACK * Stack;
    char ** VariableName;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        AverageHead = NULL;
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_EF){
            Stack = EstimatorList->Estimator[i].Stack;
            for (j = 0; j < Stack->NumberEntries; j++) {
                if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                        Stack->Entry[j].Aggregate == EIE_AGGREGATE_AVERAGE) {

                    Name = Stack->Entry[j].Symbol->Name;
                    Period = Stack->Entry[j].Symbol->Period;

                    /* check if already specified */
                    AverageCurrent = EI_AverageLookup (AverageHead,
                        Name, Period);

                    if (AverageCurrent == NULL) {
                        /* it was not already specified  */
                        AverageHead = EI_AverageAdd (AverageHead, Name, Period);
                        AverageCurrent = AverageHead;
                    }

                    if (Period == EIE_PERIOD_CURRENT) {
                        VariableName = InDataVariableName;
                        NumberOfVariables = InDataNumberOfVariables;
                    }
                    else {
                        VariableName = InHistVariableName;
                        NumberOfVariables = InHistNumberOfVariables;
                    }

                    /* find average name in datarec */
                    AverageCurrent->VariableIndex = UTIL_Binary (
                        Name, VariableName, NumberOfVariables);

                    assert (AverageCurrent->VariableIndex != -1);

                    /* find weight name in datarec */
                    if (EstimatorList->Estimator[i].WeightVariable[0] != '\0') {
                        /*printf ("looking for weight var %s in\n", */
                        /*EstimatorList->Estimator[i].WeightVariable);*/
                        AverageCurrent->WeightIndex = UTIL_Binary (
                            EstimatorList->Estimator[i].WeightVariable,
                            VariableName, NumberOfVariables);

                        assert (AverageCurrent->WeightIndex != -1);
                    }
                }
            }
        }
        EstimatorList->Estimator[i].Average = AverageHead;
    }
}
/*********************************************************************
Sets up regression for all LR estimators
*********************************************************************/
static void SetUpRegressions (
    EIT_ESTIMATORSPEC_LIST * EstimatorList,
    char ** InDataVariableName,
    int InDataNumberOfVariables,
    char ** InHistVariableName,
    int InHistNumberOfVariables)
{
    int i;
    int j;
    char * Name;
    int NumberOfVariables;
    EIT_AUX * NewAux;
    EIT_STACK * Stack;
    char ** VariableName;

    for (i = 0; i < EstimatorList->NumberEntries; i++) {
        if (EstimatorList->Estimator[i].Algorithm->Type==EIE_ALGORITHM_TYPE_LR){

            Stack = EstimatorList->Estimator[i].Stack;
            EstimatorList->Estimator[i].Regression = EI_RegressionAllocate ();

            /*get the index of the 'y' and put it in Regression Struct.*/
            Name = EstimatorList->Estimator[i].FieldId;
            EstimatorList->Estimator[i].Regression->FieldIdIndex =
                UTIL_Binary (Name, InDataVariableName, InDataNumberOfVariables);

            assert (EstimatorList->Estimator[i].Regression->FieldIdIndex != -1);

            /*Get the index of the Weight var, put it in Regression Struct.*/
            /*It will be -1 if it doesn't exist.*/
            Name = EstimatorList->Estimator[i].WeightVariable;
            if (Name[0] != '\0') {
                EstimatorList->Estimator[i].Regression->WeightIndex =
                    UTIL_Binary (Name, InDataVariableName,
                        InDataNumberOfVariables);

                assert (EstimatorList->Estimator[i].Regression->WeightIndex !=
                    -1);

                if (EstimatorList->Estimator[i].RandomError[0] == 'Y')
                    EI_ResidualSetIsWeighted (
                        EstimatorList->Estimator[i].Residual, EIE_TRUE);
            }
            else
                EstimatorList->Estimator[i].Regression->WeightIndex = -1;

            /*Get the index of the Variance var, put it in Regression Struct.*/
            /*It will be -1 if it doesn't exist.*/
            Name = EstimatorList->Estimator[i].VarianceVariable;
            if (Name[0] != '\0') {

                if (EstimatorList->Estimator[i].VariancePeriod[0] ==
                        EIM_ESTIM_PERIOD_CURRENT_LETTER)
                    EstimatorList->Estimator[i].Regression->VariancePeriod =
                        EIE_PERIOD_CURRENT;
                else
                    EstimatorList->Estimator[i].Regression->VariancePeriod =
                        EIE_PERIOD_HISTORICAL;

                if (EstimatorList->Estimator[i].Regression->VariancePeriod ==
                        EIE_PERIOD_CURRENT) {
                    VariableName = InDataVariableName;
                    NumberOfVariables = InDataNumberOfVariables;
                }
                else {
                    VariableName = InHistVariableName;
                    NumberOfVariables = InHistNumberOfVariables;
                }

                EstimatorList->Estimator[i].Regression->VarianceIndex =
                    UTIL_Binary (Name, VariableName, NumberOfVariables);

                assert (EstimatorList->Estimator[i].Regression->VarianceIndex !=
                    -1);

                EstimatorList->Estimator[i].Regression->VarianceExponent =
                    EstimatorList->Estimator[i].VarianceExponent;
            }
            else
                EstimatorList->Estimator[i].Regression->VarianceIndex = -1;

            for (j = 0; j < Stack->NumberEntries; j++) {
                if (Stack->Entry[j].Operation == EIE_OPERATION_PUSHSYMBOL &&
                        strcmp (Stack->Entry[j].PlaceHolder,
                            EIM_BETA_PREFIX "0") == 0) {
                    EstimatorList->Estimator[i].Regression->Intercept=EIE_TRUE;
                }
                else if (Stack->Entry[j].Operation == EIE_OPERATION_POWER) {
                    NewAux = STC_AllocateMemory (sizeof *NewAux);
                    NewAux->ExponentValue = Stack->Entry[j-1].Value;
                    NewAux->Period = Stack->Entry[j-2].Period;

                    /*Get index of Aux vars; put this index in Reg struct.*/
                    Name = Stack->Entry[j-2].Symbol->Name;
                    NewAux->VariableName = STC_StrDup (Name);
                    if (Stack->Entry[j-2].Symbol->Period == EIE_PERIOD_CURRENT)
                        NewAux->AuxIndex = UTIL_Binary(Name, InDataVariableName,
                            InDataNumberOfVariables);
                    else
                        NewAux->AuxIndex = UTIL_Binary(Name, InHistVariableName,
                            InHistNumberOfVariables);

                    assert (NewAux->AuxIndex != -1);

                    Name = Stack->Entry[j-3].Symbol->Name;
                    NewAux->BetaName = STC_StrDup (Name);

                    NewAux->Next = EstimatorList->Estimator[i].Regression->Aux;
                    EstimatorList->Estimator[i].Regression->Aux = NewAux;

                    EstimatorList->Estimator[i].Regression->NumberOfAux++;
                }
            }

            /* Allocate Memory for the betas */
            /* allocate NumberOfAux+2 because: 1 for the intercept and
               1 because the array is used with an offset of 1 by
               Numerical Recipes
            */
            EstimatorList->Estimator[i].Regression->Betas = STC_AllocateMemory (
               (EstimatorList->Estimator[i].Regression->NumberOfAux + 2) *
                   sizeof *EstimatorList->Estimator[i].Regression->Betas);
        }
    }

    if (DEBUG)
        for (i = 0; i < EstimatorList->NumberEntries; i++)
            EI_RegressionPrint (EstimatorList->Estimator[i].Regression);
}
/*********************************************************************
set value of datarec (2 periods) into symbol table
*********************************************************************/
static void SetValues (
    EIT_SYMBOL ** SymbolTable,
    EIT_DATA * Data)
{
    int i;
    int Index;
    EIT_SYMBOL * Symbol;

    for (i = 0; i < EI_SymbolTableGetSize (SymbolTable); i++) {
        for (Symbol = SymbolTable[i]; Symbol != NULL; Symbol = Symbol->Next) {
            /* do not set average or beta */
            if (Symbol->Aggregate == EIE_AGGREGATE_VALUE &&
                    strncmp (Symbol->Name, EIM_BETA_PREFIX,
                        strlen (EIM_BETA_PREFIX)) != 0) {
                if (Data->DataRec[Symbol->Period] != NULL) {
                    Index = EI_DataRecSearch (Data->DataRec[Symbol->Period],
                        Symbol->Name);
                    Symbol->Value =
                        Data->DataRec[Symbol->Period]->FieldValue[Index];
                }
            }
        }
    }
}
/*********************************************************************
Creates a list with a string, the string should look like:
[Del]item1[Del]Sep[Del]item2[Del]Sep...Sep[Del]itemN[Del]
*********************************************************************/
static tSList * String2List (
    char * String,
    tSList * List,
    char * Separator,
    char * Delimitor,
    int KeepDuplicate)
{
    size_t DelimitorLength;
    char * Element;
    size_t ElementLength;
    char * pB;
    char * pE;
    size_t SeparatorLength;
    size_t VariableNameSize;

    DelimitorLength = strlen (Delimitor);
    SeparatorLength = strlen (Separator);
    VariableNameSize = EI_InfoGetVariableNameMaxSize ();

    Element = STC_AllocateMemory (VariableNameSize + 1);

    pB = String;
    pE = strstr (pB, Separator);
    while (pE != NULL) {
        ElementLength = pE - pB - (DelimitorLength*2);
        ElementLength = (ElementLength < VariableNameSize ?
            ElementLength : VariableNameSize);
        strncpy (Element, pB + DelimitorLength, ElementLength);
        Element[ElementLength] = '\0';
        UTIL_DropBlanks (Element);
        if (Element[0] != '\0') {
            if (KeepDuplicate == 0)
                SList_AddNoDup (Element, List);
            else
                SList_Add (Element, List);
        }
        pB = pE + SeparatorLength;
        pE = strstr (pB, Separator);
    }
    ElementLength = (String + strlen (String)) - pB - (DelimitorLength*2);
    ElementLength = (ElementLength < VariableNameSize ?
        ElementLength : VariableNameSize);
    strncpy (Element, pB + DelimitorLength, ElementLength);
    Element[ElementLength] = '\0';
    UTIL_DropBlanks (Element);
    if (Element[0] != '\0') {
        if (KeepDuplicate == 0)
            SList_AddNoDup (Element, List);
        else
            SList_Add (Element, List);
    }

    STC_FreeMemory (Element);

    return List;
}
/*********************************************************************
makes sure a committed variable is valid.
valid means
average variable must be greater than or equal to 0, can't be missing and
cannot have a status.
beta variable must be greater than 0, can't be missing and cannot have a status.
a committed variable is a variable use to calculate a parameter.
*********************************************************************/
static EIT_RETURNCODE ValidateCommittedVariable (
    EIT_DATATABLE * DataTable,
    int EstimatorIndex,
    int VariableIndex,
    EIT_PERIOD Period,
    char * VariableType) /* must be "WEIGHTVARIABLE" or "VARIANCEVARIABLE" */
{
    EIT_DATA * Data;
    int i;
    char * PeriodPtr;

    /* String for period in message:
       M00033 -> "current" ('female gender' form for french grammar) 
       M00034 -> "historical" 
    */
    PeriodPtr = (Period == EIE_PERIOD_CURRENT) ? M00033: M00034; 

    for (i = 0; i < DataTable->Size; i++) {
        for (Data = DataTable->Data[i]; Data != NULL; Data = Data->Next) {
            if (Data->DataRec[Period] != NULL) {
                if (CheckStatus (EstimatorIndex, Data->Key,
                        Data->DataRec[Period]->FieldValue[VariableIndex],
                        Data->DataRec[Period]->StatusFlag[VariableIndex],
                        PeriodPtr, VariableType) != EIE_SUCCEED) {
                    return EIE_FAIL;
                }
                /* VARIANCEVARIABLE cannot be 0.0 */
                if (VariableType[0] == 'V') {
                    if (Data->DataRec[Period]->FieldValue[VariableIndex]==0.0) {
                        EI_AddMessage ("Estimator", EIE_ERROR,
                            M30069,  /* Estimator %d: key %s has a 0.0 value 
                                        for the %s %s. */
                            EstimatorIndex, Data->Key, PeriodPtr, VariableType);                            
                        return EIE_FAIL;
                    }
                }
            }
        }
    }

    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static void ValidateCriteria (
    EIT_ESTIMATORSPEC * Estimator,
    int EstimatorIndex,
    int NumberOfAcceptableRecord,
    int NumberOfObs)
{
    char Line[1001];
    double Percent;

    /* check CountCriteria */
    if (NumberOfAcceptableRecord < Estimator->CountCriteria) {
        Estimator->Active = EIE_FALSE;
        sprintf (Line, M10015, /* Estimator %d did not meet the COUNTCRITERIA:
                                  target %-4d actual %-4d */
            EstimatorIndex, Estimator->CountCriteria, NumberOfAcceptableRecord);            
        EI_AddMessage ("Estimator", EIE_INFORMATION, "%s", Line);
    }

    Percent = 0.0;
    if (NumberOfObs != 0.0)
        Percent = (double) NumberOfAcceptableRecord / NumberOfObs * 100;

    /* check PercentCriteria */
    if (Percent < Estimator->PercentCriteria) {
        Estimator->Active = EIE_FALSE;
        sprintf (Line, M10016, /* Estimator %d did not meet the PERCENTCRITERIA:
                                  target %-4.1f actual %-4.1f */
            EstimatorIndex, Estimator->PercentCriteria, Percent);
        EI_AddMessage ("Estimator", EIE_INFORMATION, "%s", Line);
    }

    if (Estimator->Active == EIE_FALSE)
        EI_AddMessage ("Estimator", EIE_INFORMATION,
            M10017 "\n", /* Estimator %d will not be processed. */
            EstimatorIndex);
}
