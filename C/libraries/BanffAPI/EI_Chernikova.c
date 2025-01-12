/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Chernikova.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Message.h"
#include "EI_Weights.h"

#include "STC_Memory.h"

#include "util.h"

#include "MessageBanffAPI.h"


/* ------------------------------------------------------------------ */
/*      Allocate the chernikova matrix                                */
/*                                                                    */
/*     |     | (B - A*X)         -A           A            | | | |    */
/*     |  U  |           (M x 1)   (M x N)     (M x N)     |R| |P|    */
/*     |     |  X                 I          -I            | | | |    */
/*     |     |      (N x 1)        (N x N)     (N x N)     | | | |    */
/* Y <-| ----------------------------------------------               */
/*     |     | 0                                           | |        */
/*     |     | 0                                           |W|        */
/*     |  L  | 0                  I (2N x 2N)              | |        */
/*     |     | 0                                           | |        */
/*     | ----------------------------------------------               */
/*     |  F  | 1 + (2 * N) zero                                       */
/*     | ----------------------------------------------               */
/*     |  C  |   column cardinality                                   */
/*                                                                    */
/*  A     coefficients of edits excluding the constants               */
/*  A * X scalar product of matrix A and vector X                     */
/*  B     edits' constants                                            */
/*  I     identity matrix corresponding to the implied edits          */
/*  X     data vector                                                 */
/*  C     cardinality of each column                                  */
/*                                                                    */
/*  M     is the number of edits in group, excluding the constant     */
/*  N     is the number of variables in the group of edits            */
/*                                                                    */
/*  U     is the upper part of the matrix Y                           */
/*  L     is the lower part of the matrix Y                           */
/*                                                                    */
/*  R     is the row type in U part (equality or inequality)          */
/*  P     is the pivot list in U part (row has already been process   */
/*                                     yes or no)                     */
/*                                                                    */
/*  F     is the row containing the column flag ( > 0 means extremal  */
/*        0 not extremal)                                             */
/*  W     weight for the lower part of the matrix except the flagrow  */
/*        and cardrow                                                 */
/* ------------------------------------------------------------------ */
int EI_ChernikovaAllocate (
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights,
    EIT_CHERNIKOVA * y,      /* Chernikova matrix         */
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    int       i;
    int       j;
    int       k;
    int       nbcol;
    int       nbrow;
    int       nburow;

    yCompanion->nbvar  = Edits->NumberofColumns - 1;   /* number of variables */

    nbcol  = 2 * yCompanion->nbvar + 1;
    nbrow  = Edits->NumberofEquations + (2 * yCompanion->nbvar) + 2;
    nburow = Edits->NumberofEquations;/* + yCompanion->nbvar;*//**/

    y->flagrow = nbrow - 2;   /* Index of extremal flag in matrix */
    y->cardrow = nbrow - 1;   /* Index of cardinality in matrix */

    /* # of columns in preinit */
    y->preinitColumns = 2 * yCompanion->nbvar;
    /* # of rows in preinit */
    y->preinitRows    = Edits->NumberofEquations + (2 * yCompanion->nbvar) + 2;

    /* Allocate space for preinit */
    y->preinit = STC_AllocateMemory (
        y->preinitColumns * sizeof *y->preinit);
    if (y->preinit == NULL)
        return -1;

    /*  Allocate space for preinit[j]       */
    for (j = 0; j < y->preinitColumns; j++) {
        y->preinit[j] = STC_AllocateMemory (
            y->preinitRows * sizeof *y->preinit[j]);
        if (y->preinit[j] == NULL)
            return -1;
    }

    /*  Allocate array of columns           */
    y->column = STC_AllocateMemory (nbcol * sizeof *y->column);
    if (y->column == NULL)
        return -1;

    /*  Allocate space for column[j]       */
    for (j = 0; j < nbcol; j++) {
        y->column[j] = STC_AllocateMemory (nbrow * sizeof *y->column[j]);
        if (y->column[j] == NULL)
            return -1;
    }

    yCompanion->w = STC_AllocateMemory (
        yCompanion->nbvar * sizeof *yCompanion->w);
    if (yCompanion->w == NULL)
        return -1;

    /*  Allocate space for the type of row  */
    yCompanion->rowtype = STC_AllocateMemory (
        nburow * sizeof *yCompanion->rowtype);
    if (yCompanion->rowtype == NULL)
        return -1;

    /*  Allocate space for the pivot list  */
    yCompanion->pivolist = STC_AllocateMemory (
        nburow * sizeof *yCompanion->pivolist);
    if (yCompanion->pivolist == NULL)
        return -1;

    yCompanion->I0 = STC_AllocateMemory (nbrow * sizeof *yCompanion->I0);
    if (yCompanion->I0 == NULL)
        return -1;

    yCompanion->I0s = STC_AllocateMemory (nbrow * sizeof *yCompanion->I0s);
    if (yCompanion->I0s == NULL)
        return -1;

    yCompanion->I0st = STC_AllocateMemory (nbrow * sizeof *yCompanion->I0st);
    if (yCompanion->I0st == NULL)
        return -1;

/* qué */
    yCompanion->I0t = yCompanion->I0;
    for (j = y->flagrow; j >= nburow; j--)
        *yCompanion->I0t++ = j;
    *yCompanion->I0t = -1;

    /* PRE-INITIALIZE THE ARRAY       */
    /* initialize           -A   A    */
    for (i = 0; i < Edits->NumberofEquations; i++) {
        for (k = 0; k < yCompanion->nbvar; k++) {
            y->preinit[k][i]                     = -Edits->Coefficient[i][k];
            y->preinit[k + yCompanion->nbvar][i] =  Edits->Coefficient[i][k];
        }
    }

/**/
     /* initialize upper part    I   -I  */ /* The lower part of U */
/*     for (i = Edits->NumberofEquations, k = 0; k < yCompanion->nbvar; i++, k++) { */
/*         for (j = 0; j < yCompanion->nbvar; j++) { */
/*             y->preinit[j][i]                     = 0.0; */
/*             y->preinit[j + yCompanion->nbvar][i] = 0.0; */
/*         } */
        /* init.    I   -I     */
/*         y->preinit[k][i]                     =  1.0; */
/*         y->preinit[k + yCompanion->nbvar][i] = -1.0; */
/*     } */

    /* initialize lower part        I            */
    for (i = nburow, k = 0; i < y->flagrow; i++, k++) {
        for (j = 0; j < yCompanion->nbvar; j++) {
            y->preinit[j][i]                     = 0.0;
            y->preinit[j + yCompanion->nbvar][i] = 0.0;
        }
        y->preinit[k][i] = 1.0;
    }

    /* initialize flag row */
    for (k = 0; k < yCompanion->nbvar; k++) {
        y->preinit[k][y->flagrow]                     = NOT_XTR_POINT;
        y->preinit[k + yCompanion->nbvar][y->flagrow] = NOT_XTR_POINT;
    }

    /* initialize cardinality */
    for (k = 0; k < yCompanion->nbvar; k++) {
        yCompanion->w[k]                      = Weights->Value[k];
        y->preinit[k][y->cardrow]    = Weights->Value[k];
        y->preinit[k + yCompanion->nbvar][y->cardrow] =
                                                Weights->Value[k];
    }

    /*  Initialize type of row  */
    for (j = 0; j < Edits->NumberofInequalities; j++)
        yCompanion->rowtype[j] = INEQ_ROW;

    for (j = Edits->NumberofInequalities; j < Edits->NumberofEquations; j++)
        yCompanion->rowtype[j] = EQUA_ROW;

    for (j = Edits->NumberofEquations; j < nburow; j++)
        yCompanion->rowtype[j] = INEQ_ROW;

    /* Initialize parameters  */
    y->y_cols   = nbcol;
    y->nb_cols  = nbcol;
    y->max_cols = nbcol;
    y->u_rows   = nburow;
    y->nb_rows  = nbrow;

    return 0;
}

/*
choose one solution from all possible solutions.
fills an array of index to variables.
*/
int EI_ChernikovaChooseSolution (
    EIT_CHERNIKOVA *y,
    int NumberOfVariables,
    short * VariableIndexList,
    int RandNumFlag,
    double RandomNumber,
    int LoggingVerbosityLevel)
{
    int Choice;
    int i;
    int Index;
    double *TempColumn;
    
    int rc;
    
    rc = 0;

    /* choose one solution from all possible solutions */
    if (!RandNumFlag) {
        Choice = UTIL_Random (0, y->y_cols);
        if (LoggingVerbosityLevel > 0) {
            printf("using random number between %f and %f for random choice\n",
                       (Choice / ((double)(((y->y_cols)-0)+0))),
                       ((Choice + ((Choice < (y->y_cols))?1:0)) / ((double)(((y->y_cols)-0)+0)))
                      );
        }
    } else {
        if (RandomNumber < 0.0) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarErrorlocLessThanZero "\n", RandomNumber);
            rc = -1;
            goto error_cleanup;
        }
        else if (RandomNumber > 1.0) {
            printf(SAS_MESSAGE_PREFIX_ERROR MsgRandnumvarErrorlocGreaterThanOne "\n", RandomNumber);
            rc = -1;
            goto error_cleanup;
        }
        else if (RandomNumber > (1.0 - RANDNUM_FUZZ)) {
            printf(SAS_MESSAGE_PREFIX_NOTE MsgRandnumvarErrorlocInvalid "\n", RandomNumber, RANDNUM_FUZZ, RANDNUM_FUZZ);
            RandomNumber = 1.0 - RANDNUM_FUZZ;
        }
        Choice = 0+(long)(RandomNumber*((y->y_cols)-0));
        if (LoggingVerbosityLevel > 0) {
            printf("Using random number %f for random choice.\n", RandomNumber);
        }
    }

    TempColumn = &y->column[Choice][y->u_rows];
    Index = 0;
    for (i = 0; i < NumberOfVariables; i++) {
        if (TempColumn[i] == 1.0) {
            VariableIndexList[Index++] = i;
        }
    }
    VariableIndexList[Index] = -1;
    
    goto normal_cleanup;
error_cleanup:
    
normal_cleanup:
    return rc;
}

/*  Free space */
void EI_ChernikovaFree (
    EIT_CHERNIKOVA *y,          /* Chernikova matrix */
    EIT_CHERNIKOVA_COMPANION *yCompanion) /* Chernikova matrix companion */
{
    int i;

    for (i = 0; i < y->max_cols; i++)
        STC_FreeMemory (y->column[i]);
    STC_FreeMemory (y->column);

    if (y->preinit != NULL) {
        for (i = 0; i < y->preinitColumns; i++)
            STC_FreeMemory (y->preinit[i]);
        STC_FreeMemory (y->preinit);
    }

    if (yCompanion != NULL) {
        STC_FreeMemory (yCompanion->pivolist);
        STC_FreeMemory (yCompanion->rowtype);
        STC_FreeMemory (yCompanion->w);

        STC_FreeMemory (yCompanion->I0);
        STC_FreeMemory (yCompanion->I0s);
        STC_FreeMemory (yCompanion->I0st);
    }
}

void EI_ChernikovaPrint (
    char * Message,
    EIT_CHERNIKOVA *y)
{
    int i, j;

    EI_AddMessage ("", 4, Message);

    EI_ChernikovaPrintStat (y);

    /* leave if empty */
    if (y->nb_rows == 0 || y->nb_cols == 0) return;

    EI_AddMessage ("", 4, "   ");
    for (j=0; j<y->nb_cols; j++) EI_AddMessage ("", 4, "%6d ", j);
    EI_AddMessage ("", 4, "");

    for (i=0; i<y->nb_rows; i++) {
        EI_AddMessage ("", 4, "%2d", i);
        if (i >= y->u_rows && i < y->u_rows+y->y_cols)
            EI_AddMessage ("", 4, ">");
        else
            EI_AddMessage ("", 4, " ");
        for (j=0; j<y->nb_cols; j++) {
             EI_AddMessage ("", 4, "%06.1f ", y->column[j][i]);
        }
        EI_AddMessage ("", 4, "");
    }
    EI_AddMessage ("", 4, "");
}

void EI_ChernikovaPrintSolutions (
    EIT_CHERNIKOVA * y,
    EIT_DATAREC * Data)
{
    int i, j;
	char s1[1001];
	char s2[101];
    double * tempcol;

    EI_AddMessage ("", 4, "[");
    for (i = 0; i < y->y_cols; i++) {
        sprintf (s1, "%d:", i);
        tempcol = &y->column[i][y->u_rows];
        for (j = 0; j < Data->NumberofFields; j++) {
            if (tempcol[j] == 1.0) {
                sprintf (s2, " %s", Data->FieldName[j]);
				strcat (s1, s2);
            }
        }
        EI_AddMessage ("", 4, "(%s)", s1);
    }
    EI_AddMessage ("", 4, "]");
	EI_PrintMessages ();
}

void EI_ChernikovaPrintStat (
    EIT_CHERNIKOVA *y)
{
    EI_AddMessage ("", 4, "nb_rows %d u_rows %d nb_cols %d y_cols %d          max_cols %d\n",
        y->nb_rows, y->u_rows, y->nb_cols, y->y_cols, y->max_cols);
    EI_AddMessage ("", 4, "");
}

/* -------------------------------------------------------------------*/
/* EI_ChernikovaReallocate                                            */
/*                                                                    */
/* The Chernikova structure includes two logical matrices:            */
/* y and yprime.  The structure has only one physical matrix          */
/* which is referenced by the array of pointer to column:             */
/* "column".  This means that the two matrices are stored             */
/* consecutively.                                                     */
/*                                                                    */
/* "y_cols" indicates the number of columns in the y matrix,          */
/* "nb_cols" indicates the total number of column in both             */
/* matrices y and yprime.                                             */
/*                                                                    */
/* The way to access each of the matrices is as follows:              */
/* the indices from "0" (inclusively) to "y_cols"                     */
/* (exclusively) are for the y matrix and the indices from            */
/* "y_cols" (inclusively) to "nb_cols" (exclusively) are              */
/* for the yprime matrix.                                             */
/*                                                                    */
/*                                                                    */
/* Reallocate a new column for y                                      */
/*                                                                    */
/* This process is done in six steps.                                 */
/* Step 0:  Evaluate sizes                                            */
/* Step 1:  Expend array of pointers                                  */
/* Step 2:  Copy old array of pointers to new one                     */
/* Step 3:  Free old array                                            */
/* Step 4:  Set pointer                                               */
/* Step 5:  Allocate space for new columns                            */
/*                                                                    */
/*               *---------------*---------------*                    */
/*    y->column  | Old array of pointers         |                    */
/*               *---------------*---------------*                    */
/*                 |   |   |   |   |   |   |   |                      */
/*                 v   v   v   v   v   v   v   v                      */
/*               *---*---*---*---*---*---*---*---*---*---*---*---*    */
/*               |   |   |   |   |   |   |   |   |   |   |   |   |    */
/*               |   |   |   |   |   |   |   |   |   |   |   |   |    */
/*    column[i]  |   |   |   |   |   |   |   |   |   |   |   |   |    */
/*               |   |   |   |   |   |   |   |   |   |   |   |   |    */
/*               |   |   |   |   |   |   |   |   |   |   |   |   |    */
/*               *---*---*---*---*---*---*---*---*---*---*---*---*    */
/*                 ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^   ^      */
/*                 |   |   |   |   |   |   |   |   |   |   |   |      */
/*               *---------------*---------------*---------------*    */
/*    new_column | New array of pointers                         |    */
/*               *---------------*---------------*---------------*    */
/*                                                                    */
/* ------------------------------------------------------------------ */
/* ---------------------------------------------------------------------
FUNCTION        Reallocate the chernikova matrix
PARAMETERS      1 - Pointer to the chernikova matrix.
RETURN VALUE    1 - Structure is updated.
--------------------------------------------------------------------- */
int EI_ChernikovaReallocate (
    EIT_CHERNIKOVA *y)          /* Chernikova matrix */
{
    double ** new_column;   /*  Array of pointers to doubles     */
    long new_max_cols;      /*  New maximum number of columns    */

    register int i;

/*EI_ChernikovaPrint ("in EI_ChernikovaReallocate()", y);*/
/*EI_ChernikovaPrintStat (y);*/

    /******************************************************************/
    /*  Step 0:  Evaluate sizes                                       */
    /******************************************************************/

    /* Number of cols needed by new matrix */
    new_max_cols = y->max_cols + y->nb_rows * 10;

    /******************************************************************/
    /*  Step 1:  Expend array of pointers                             */
    /******************************************************************/

    new_column = STC_AllocateMemory (new_max_cols * sizeof *new_column);
    if (new_column == NULL) {
        return -1;
    }

    /******************************************************************/
    /*  Step 2:  Copy old array of pointers to new one                */
    /******************************************************************/

    memcpy (new_column, y->column, y->max_cols * sizeof (double *));

    /******************************************************************/
    /*  Step 3:   Free old array                                      */
    /******************************************************************/

    STC_FreeMemory (y->column);

    /******************************************************************/
    /*  Step 4:   Set pointer                                         */
    /******************************************************************/

    y->column = new_column;

    /******************************************************************/
    /*  Step 5:  Allocate space for new columns                       */
    /******************************************************************/

    while (new_max_cols > y->max_cols) {

        y->column[y->max_cols] = STC_AllocateMemory (
            y->nb_rows * sizeof *y->column[y->max_cols]);

        if (y->column[y->max_cols] == NULL)
            return -1;

        for (i = 0; i < y->nb_rows; i++)
            y->column[y->max_cols][i] = 0.0;

        y->max_cols++;
    }

/*
    EI_AddMessage ("", 4, "End allocate y_y = %d  y_col = %d  y_max = %d  y_row = %d %d\n",
        y->y_cols, y->nb_cols, y->max_cols, y->u_rows, y->nb_rows);
    EI_ChernikovaPrint ("out EI_ChernikovaReallocate()", y);
    EI_ChernikovaPrintStat (y);
*/

    return 0;
}

/* ------------------------------------------------------------------ */
/*      Initialize the chernikova matrix                              */
/*      The matrix Y is initialized in the following order            */
/*                                                                    */
/*      The EQUALITIES are stored first in the matrix -A A B-AX       */
/*      and followed by the INEQUALITIES.                             */
/*                                                                    */
/*     |     | (B - A*X)         -A           A                       */
/*     |  U  |           (M x 1)   (M x N)     (M x N)                */
/*     |     |  X                 I          -I                       */
/*     |     |      (N x 1)        (N x N)     (N x N)                */
/* Y <-| ----------------------------------------------               */
/*     |     | 0                                                      */
/*     |     | 0                                                      */
/*     |  L  | 0                  I (2N x 2N)                         */
/*     |     | 0                                                      */
/*     | ----------------------------------------------               */
/*     |  F  | 1   0000000000000000000000000000000000 <----- flagrow  */
/*     | ----------------------------------------------               */
/*     |  C  | 0                  w[i]         w[i]   <----- cardrow  */
/*                                                                    */
/*  A     coefficients of edits excluding the constants               */
/*  A * X scalar product of matrix A and vector X                     */
/*  B     edits' constants                                            */
/*  I     identity matrix corresponding to the implied edits          */
/*  X     data vector                                                 */
/*  F     flag row indicating extremal point                          */
/*  C     cardinality of each column equal to zero                    */
/* ------------------------------------------------------------------ */
void EI_ChernikovaReinit (
    EIT_DATAREC * Data,  /*  Data record               */
    EIT_EDITS * Edits,   /*  Group of edits            */
    EIT_CHERNIKOVA * y,  /*  Chernikova matrix         */
    EIT_CHERNIKOVA_COMPANION * yCompanion,  /*  Chernikova matrix companion */
    int * xtrm,          /*  count extremal point      */
    int * fxtrm)         /*  count final extremal point*/
{
    double bminusax; /* value of B - AX */

    int i;
    int j;
    int k;

    /* set invalid data to minus 1      */
  /*  for (i = 0; i < yCompanion->nbvar; i++)
      if ((Data->StatusFlag[i] != FIELDOK) && (Data->StatusFlag[i] != FIELDMISS))
	Data->FieldValue[i] = -1.0;
*/

    /* init.   B - AX      */
    for (i = 0; i < Edits->NumberofEquations; i++) {

        bminusax = Edits->Coefficient[i][yCompanion->nbvar];
        for (j = 0; j < yCompanion->nbvar; j++) {
            bminusax -= Edits->Coefficient[i][j] * Data->FieldValue[j];
        }

        if (fabs (bminusax) <= EPSILON)
            y->column[0][i] = 0.0;
        else
            y->column[0][i] = bminusax;
    }

    /* initialize upper part     X */
 /*    for (i = Edits->NumberofEquations, k = 0; k < yCompanion->nbvar; i++, k++) */
/*         y->column[0][i] = Data->FieldValue[k]; */

    /* initialize lower part        0 */
    for (i = y->u_rows; i < y->flagrow; i++)
        y->column[0][i] = 0.0;

    /* initialize flag to extreme point*/
    y->column[0][y->flagrow] = XTR_POINT;

    /* initialize cardinality to zero */
    y->column[0][y->cardrow] = 0.0;

    for (i = 0, k = 1; i < y->preinitColumns; i++, k++)
        memcpy (y->column[k], y->preinit[i], y->preinitRows * sizeof (double));

    y->y_cols  = 2 * yCompanion->nbvar + 1;
    y->nb_cols = 2 * yCompanion->nbvar + 1;   /* empty the yprime part       */

    for (i = 0; i < y->u_rows; i++)
        yCompanion->pivolist[i] = NOT_PROCESS_ROW;

    *xtrm  = 1;                   /* one extremal point and no final*/
    *fxtrm = 0;
}
