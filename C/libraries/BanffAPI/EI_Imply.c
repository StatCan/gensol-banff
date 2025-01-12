/* ---------------------------------------------------------------------
FUNCTION        Find the implied edits.
PARAMETERS      Edits.
                Maximum number of implied edits.
--------------------------------------------------------------------- */
/* ---------------------------------------------------------------------
    Include Header Files
--------------------------------------------------------------------- */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Chernikova.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Imply.h"
#include "EI_Message.h"

#include "STC_Memory.h"

#include "util.h"

#include "MessageBanffAPI.h"

/* ---------------------------------------------------------------------
structures
--------------------------------------------------------------------- */
typedef struct {
    int nb_vars;    /* Number of variables in group    */
    int nb_edits;   /* Number of edits in group        */
    int nb_equa;    /* Number of equalities            */
    int nb_ineq;    /* Number of inequalities          */
    int impedred;   /* Number of implied edits redundant*/
    int imp_equa;   /* Number of implied equalities    */
    int imp_ineq;   /* Number of implied inequalities  */
    int limit;      /* Submitted limit on number of edits */
} IMPLYSTA;

/* ---------------------------------------------------------------------
Procedures which are located in this file.
--------------------------------------------------------------------- */
static void buildcol (int, int, int, EIT_CHERNIKOVA *, double *);
static int checklim (EIT_CHERNIKOVA *, IMPLYSTA *);
static int checknew (int, double *, EIT_CHERNIKOVA *);
static void derive (EIT_CHERNIKOVA *, IMPLYSTA *);
static int init_y (EIT_EDITS *, EIT_FIELDNAMES *, EIT_CHERNIKOVA *, IMPLYSTA *);
static void prntstat (EIT_CHERNIKOVA *, EIT_FIELDNAMES *, IMPLYSTA *);
static void scale (double *);


/* ---------------------------------------------------------------------
Variables
--------------------------------------------------------------------- */
static int mColumnSize;   /* Length of a column               */
static int mConstantRowIndex; /* Index of constant row in matrix  */
static int mFlagRowIndex; /* Index of flag row in matrix      */

static EIT_PRINTF mPrintf;


/* ---------------------------------------------------------------------
               M A I N   L I N E

    The implied edits structure includes two logical matrices:
    y and y'.  The structure has only one physical matrix
    which is referenced by the array of pointer to column:
    "column".  Moreover, the matrix y is always part of the
    matrix y'.

    "y_cols" indicates the number of columns in the y matrix,
    "nb_cols" the total number of columns in the matrix y' and
    "nb_cols - y_cols" the number of columns created at the
    end of an iteration.

    The way to access each of the matrices is as follows:
    the indices from "0" (inclusively) to "y_cols"
    (exclusively) are for the y matrix and the indices from
    "0" (inclusively) to "nb_cols" (exclusively) are for the
    y' matrix.

    At the end of each iteration, y' becomes y by setting
    y->cols = y->nb_cols.

    The size of the matrix y (number of columns) grows from
    one iteration to the next.
--------------------------------------------------------------------- */
int EI_Imply (
    EIT_EDITS * Edits,
    int NumberOfImpliedEdits,
    EIT_PRINTF Printf)
{
    int rc;
    double *newcol;          /* New column               */
    EIT_CHERNIKOVA y;
    IMPLYSTA stats;          /* Implied edits stats.     */
    int chklim;              /* Count of number of redundent edits found. */
    EIT_FIELDNAMES Fields;   /* Array of variable names  */
    register int s, t;       /* Indices                  */
    register int row;        /* Index of a row           */

    mPrintf = Printf;

    mPrintf ("\n\n");
    mPrintf (M40024 "\n\n"); /* Implied Edits Analysis Diagnostics */
    
    /* Get the initial matrix */
    if (-1 == init_y (Edits, &Fields, &y, &stats)) {
        /* Not enough space to process implied edits algorithm */
        return -1;
    }

/*    EI_ChernikovaPrint("init_y", &y);*/

    /* Set the maximum number of implied edits */
    stats.limit = NumberOfImpliedEdits;

    if (y.column == NULL) {
        /* Not enough space to process implied edits algorithm */
        return -1;
    }

    newcol = STC_AllocateMemory (y.nb_rows * sizeof *newcol);
    if (newcol == NULL)
       return -1;

    /* Process each row in    */
    /* upper part U of matrix */
    for (row = 0; row < y.u_rows; row++) {
        /* Build the next y' */

        /* For each pair (s, t) */
        for (s = 0; s < (y.y_cols - 1); s++) {
            for (t = (s + 1); t < y.y_cols; t++) {
                /* Check to see if they are of different signs */
                if ((y.column[s][row] * y.column[t][row]) >= 0.0)
                    continue; /* Skip to next t */

                /* Build a new column for */
                /* the pair (s, t)        */
                buildcol (row, s, t, &y, newcol);

                /* Determine if the new column will  */
                /* replace another column or will be */
                /* added at the end of the matrix y' */
                rc = checknew (row, newcol, &y);
/*                EI_ChernikovaPrint("checknew", &y);*/

                if (rc == -1) {
                    /* inconsistent!!! */
                    return -1;
                }
                else if (rc == -2) {
                    /* space exceeded */
                    /* set counter to ensure processing is stopped, but */
                    /* we will show the implied edits we could generate */
                    chklim = 0;
                    NumberOfImpliedEdits = y.nb_cols - stats.nb_edits;
                }
                else {
                    chklim = checklim (&y, &stats);
                }


                /* Determine if the user specified maximum number of implied
                   edits has been reached. The value returned,
                   chklim, is used in the test below.  */
                if (y.nb_cols - stats.nb_edits - chklim >=
                        NumberOfImpliedEdits) {
                    row = y.u_rows;    /* Maximum number of implied */
                    s = y.y_cols;      /* edits has been reached    */
                    t = y.y_cols;      /* To get out of the loops   */
                }
            }     /* End of the for t */
        }     /* End of the for s */

        y.y_cols = y.nb_cols; /* Transfer y' into y */

    } /* End of the for row */

    /* Derive implied edits */
    derive (&y, &stats);
/*    EI_ChernikovaPrint("derive", &y);*/

    /* Print implied edits and statistics */
    prntstat (&y, &Fields, &stats);

    EI_ChernikovaFree (&y, NULL);
    STC_FreeMemory (newcol);

    return 0;
}


/* ------------------------------------------------------------------ */
/*      Initial implied edits matrix                                  */
/*                                                                    */
/*      The initial implied edits matrix y looks like this:           */
/*                                                                    */
/*              *---------------*----------*                          */
/*            ^ |               |          |                          */
/*            | |       A'      |    -E'   |                          */
/*            U |    (n x m)    | (n x me) |                          */
/*            | |               |          |                          */
/*            v |               |          |                          */
/*              *---------------*----------*                          */
/*            ^ |                          |                          */
/*            | |                          |                          */
/*            | |                          |                          */
/*            | |                          |                          */
/*            | |            I             |                          */
/*            | |  ((m + me) x (m + me))   |                          */
/*            L |                          |                          */
/*            | |                          |                          */
/*            | |                          |                          */
/*            | |                          |                          */
/*            | *---------------*----------*                          */
/*            | |       b'      |    be'   |                          */
/*            | |    (1 x m)    | (1 x me) |                          */
/*            | *---------------*----------*                          */
/*            | |       z'      |    0'    |                          */
/*            v |    (1 x m)    | (1 x me) |                          */
/*              *---------------*----------*                          */
/*                                                                    */
/*      where:                                                        */
/*          ':           means transpose                              */
/*          A (m x n):   the matrix of all original edits             */
/*                       excluding the constant                       */
/*          E (me x n):  the duplicate matrix of equality edits       */
/*                       excluding the constant                       */
/*          b (m x 1):   the constant of all edits                    */
/*          be (me x 1): the duplicate constant of equality edits     */
/*          z (m x 1):   an array of indicators:                      */
/*                              0:  edit is an equality               */
/*                              1:  edit is an inequality             */
/*                                                                    */
/*          n:  number of variables in the original edits             */
/*          m:  number of equations in the original edits             */
/*          me: number of equality edits                              */
/*          mi: number of inequality the edits                        */
/*                                                                    */
/*          m = me + mi                                               */
/*                                                                    */
/*          the upper part U of matrix y is of dimension:             */
/*              (n x (m + me))                                        */
/*          the lower part L of matrix y is of dimension:             */
/*              ((m + me + 2) x (m + me))                             */
/* ------------------------------------------------------------------ */
static int init_y (
    EIT_EDITS *Edits,          /* Name of the group of edits  */
    EIT_FIELDNAMES *Fields,    /* Array of variable names     */
    EIT_CHERNIKOVA *y,         /* Implied edits matrix        */
    IMPLYSTA *stats)           /* Implied edits statistics    */
{
    double **columns;          /* Array of pointers to doubles  */
    int m,                     /* Number of equations           */
      me,                      /* Number of equalities          */
      mi,                      /* Number of inequalities        */
      n;                       /* Number of variables in edits  */

    register int i, j;


    y->preinit = NULL;

    /* Initialize parameters */
    n  = Edits->NumberofColumns - 1;
    m  = Edits->NumberofEquations;
    mi = Edits->NumberofInequalities;
    me = m - mi;

    /* Initialize y */
    y->u_rows   = n;
    y->nb_rows  = n + m + me + 2;
    y->nb_cols  = m + me;
    y->y_cols   = y->nb_cols;
    y->max_cols = y->nb_cols;
    y->column   = NULL;

    /* Size of a column */
    mColumnSize  = y->nb_rows * sizeof (double);

    /* Index of constant row in the matrix */
    mConstantRowIndex = y->nb_rows - 2;

    /* Index of flag row in the matrix */
    mFlagRowIndex = y->nb_rows - 1;

    columns = STC_AllocateMemory (y->nb_cols * sizeof *columns);
    if (columns == NULL)
       return -1;

    for (j = 0; j < y->nb_cols; j++) {
        columns[j] = STC_AllocateMemory (y->nb_rows * sizeof *columns[j]);
        if (columns[j] == NULL)
            return -1;
    }

    for (i = 0; i < y->nb_cols; i++) {
        for (j = 0; j < y->nb_rows; j++) {
            columns[i][j] = 0.0;
        }
    }
                      /***************************/
                      /* Set upper part U of y */
                      /***************************/
    for (i = 0; i < n; i++) {
                        /* Set the A' part */
       for (j = 0; j < m; j++)
           if (fabs (Edits->Coefficient[j][i]) >= EPSILON)
               columns[j][i] = Edits->Coefficient[j][i];

                        /* Set the E' part */
       for (j = 0; j < me; j++)
           if (fabs (Edits->Coefficient[j + mi][i]) >= EPSILON)
               columns[j + m][i] = -Edits->Coefficient[j + mi][i];
    }

                      /***************************/
                      /* Set lower part L of y */
                      /***************************/

                          /* Set flag row of y */
    for (j = 0; j < mi; j++)
       columns[j][mFlagRowIndex] = 1.0;

                       /* Set second last row of y */
    for (j = 0; j < m; j++)
       if (fabs (Edits->Coefficient[j][n]) >= EPSILON)
           columns[j][mConstantRowIndex] = Edits->Coefficient[j][n];

    for (j = 0; j < me; j++)
       if (fabs (Edits->Coefficient[j + mi][n]) >= EPSILON)
           columns[m + j][mConstantRowIndex] =
               -Edits->Coefficient[j + mi][n];

                      /* Scale original edits only */
    for (j = 0; j < m; j++)
       scale (columns[j]);

    for (j = m + n; j < y->nb_cols; j++)
       scale (columns[j]);

                    /* Set the Identity matrix */
    for (i = n; i < mConstantRowIndex; i++)
       columns[i - n][i] = 1.0;

                         /********************/
                         /* Set parameters */
                         /********************/
    y->column = columns;
    Fields->NumberofFields = Edits->NumberofColumns -1;
    Fields->FieldName = Edits->FieldName;

                  /* Set some implied edits statistics */
    stats->nb_vars  = n;
    stats->nb_edits = m;
    stats->nb_equa  = me;
    stats->nb_ineq  = mi;

    return 0;
}



/* ------------------------------------------------------------------ */
/*      Build a new column                                            */
/*                                                                    */
/*      A pair of column in y contribute to constitute a new          */
/*      column in y'.                                                 */
/*                                                                    */
/*      y'  = | y   | * y   + | y   | * y                             */
/*       ij   |  rs |    it   |  rt |    is                           */
/*                                                                    */
/*      ie. multiply each entry of one column by the other column's   */
/*      entry in the pivotal row (in absolute value).                 */
/*      Then add the pair of column elementwise.                      */
/* ------------------------------------------------------------------ */
static void buildcol (
    int r,                 /* Pivoting row                */
    int s,                 /* Index of the first column   */
    int t,                 /* Index of the second column  */
    EIT_CHERNIKOVA *y,     /* Impied edits matrix         */
    double *newcol)        /* New column built            */
{
    register int i;
    double *cols;
    double *colt;
    double  colsr, coltr;

    cols  = y->column[s];
    colt  = y->column[t];
    colsr = fabs(cols[r]);
    coltr = fabs(colt[r]);


    /* Build new column */
    for (i = 0; i <= mConstantRowIndex; i++) {
        newcol[i] = colsr * colt[i] + coltr * cols[i];
        if (fabs (newcol[i]) < EPSILON)
            newcol[i] = 0.0;
    }
    newcol[mFlagRowIndex] =
        y->column[s][mFlagRowIndex] + y->column[t][mFlagRowIndex];

    /* Put a 0 if it is an equality    */
    /* Put a 1 if it is an inequality  */
    if (newcol[mFlagRowIndex] > 0.0)
        newcol[mFlagRowIndex] = 1.0;
}



/* ------------------------------------------------------------------ */
/*      Check new column                                              */
/*                                                                    */
/*      Check to see if the new column is causing inconsistency,      */
/*      is redundant with another column or if it is to be kept.      */
/*                                                                    */
/*      Inconsistency is when 1) the upper part of a new column       */
/*      consists of zero elements and 2) the new column is an         */
/*      equality and the constant is not zero or the new column is    */
/*      an inequality and the constant is smaller than zero.          */
/*                                                                    */
/*      Redundancy is when a new column is identical in its           */
/*      upper part to another column already in the y matrix.         */
/*                                                                    */
/*      When a new column is redundant with another, we compare       */
/*      the constants which are located in the second last entry      */
/*      of the lower part of the matrices.  If the constant in the    */
/*      new column is smaller than the other one the new column       */
/*      is copied over the other.  If the constant in the new         */
/*      new column is equal to the other one and if the new           */
/*      column is an equality while the other one is an inequality,   */
/*      the new column is copied over the other one. In any case,     */
/*      the newly created column is droped.                           */
/*                                                                    */
/*      If the new column is neither inconsistent nor redundant       */
/*      it is added at the end of the matrix y.                       */
/* ------------------------------------------------------------------ */
static int checknew (
    int row,             /* Pivoting row number   */
    double *newcol,      /* New column            */
    EIT_CHERNIKOVA *y)   /* Implied edits matrix  */
{

    double cons_new;     /* Constant of the new column  */
    double cons_old;     /* Constant of the old column  */
    int k1, k2;          /* Counts                      */
    int rc;

    register int i, j;

    /* Find values k1 and k2 */
    k1  = 0;
    k2  = 0;

    /* Number of zero entries between the first */
    /* and current row of the upper matrix      */
    for (i = 0; i <= row; i++)
        if (fabs (newcol[i]) < EPSILON)
            k1++;

    /* Number of zero entries between the first */
    /* and second last row of the lower matrix  */
    for (i = y->u_rows; i < mConstantRowIndex; i++)
       if (fabs (newcol[i]) < EPSILON)
           k1++;

    /* Number of zero entries between the first */
    /* and last row of the upper matrix         */
    for (i = 0; i < y->u_rows; i++)
       if (fabs (newcol[i]) < EPSILON)
           k2++;

    /* Determine if the newly created   */
    /* column is redundant              */
    if (k2 == y->u_rows) {
        /* Upper part of the new column is zero for all entries.    */
        /* Do not add the column to y' and check for inconsistency  */
        if (((newcol[mFlagRowIndex] == 0.0) &&
                (fabs (newcol[mConstantRowIndex]) >= EPSILON))
                /* Equality with non-zero constant  */
         /* || ((newcol[mFlagRowIndex] == 1.0) &&
                  (newcol[mConstantRowIndex] <= -EPSILON))*/) {
                  /* inequality with negative constant  */

            mPrintf (M30077 "\n"); /* Set of edits inconsistent */
            return -1;
         }
         return 0;
    }

    /* Determine if the newly created column is redundant */
    if (k1 < (y->nb_rows - y->u_rows - 3)) {
        /* Do not consider new column */
        /* Number of zero entries is too low to add column to y' */
        return 0;
    }

    /* Ensure that newly created column is essentially new */

    /* Scale new column  */
    scale (newcol);

    /* Check for duplicate column  */
    for (j = 0; j < y->nb_cols; j++) {
        for (i = 0; i < y->u_rows; i++)
            if (fabs (newcol[i] - y->column[j][i]) >= EPSILON)
                break;

        if (i == y->u_rows) {
            /* Redundant  */

            /* Upper parts of the column j and the new column are identical */
            cons_new = newcol[mConstantRowIndex];
            cons_old = y->column[j][mConstantRowIndex];

            if (cons_new < cons_old - EPSILON) {
               /* New constant is smaller than old constant  */
               /* New column replaces old column in y'       */
                memcpy (y->column[j], newcol, mColumnSize);
                return 0;
            }

            if (cons_new > cons_old + EPSILON)
                /* New constant is greater than old one  */
                /* Discart new column                    */
                return 0;

            /* Identical constant (within an EPSILON)    */
            if ((newcol[mFlagRowIndex] == 0.0) &&
                    (y->column[j][mFlagRowIndex] == 1.0)) {
                /* New column: equality; old column: inequality  */
                /* Keep new column  */
                memcpy (y->column[j], newcol, mColumnSize);
            }
            return 0;
        }
    }

    /* New column must be added to y' */
    if (y->nb_cols >= y->max_cols) {
        rc = EI_ChernikovaReallocate (y);
        if (rc == -1) {
            mPrintf (M30078 "\n"); /* Not enough space */
            return -2;
        }
    }

    memcpy (y->column[y->nb_cols], newcol, mColumnSize);
    y->nb_cols++;

    return 0;
}



/* ------------------------------------------------------------------ */
/*      Derive implied edits                                          */
/*                                                                    */
/*      Before printing the derived edits, we first compare           */
/*      the entries of the upper part of the matrix of the edits      */
/*      and the constant to find the duplicate ones.                  */
/*                                                                    */
/*      Remove all obvious redundant edits that are hidden by         */
/*      equalities with the same coefficients in absolute value,      */
/*      but of different sign.                                        */
/*                                                                    */
/*      No more that two columns could be of opposite signs           */
/*      The step checknew remove all duplicate columns.               */
/* ------------------------------------------------------------------ */
static void derive (
    EIT_CHERNIKOVA *y,      /* Implied edits matrix      */
    IMPLYSTA *stats)        /* Implied edits statistics  */
{

    double *tempcol;        /* Pointer to one column    */
    int col;                /* Column index             */
    register int i, j, k;

    /* Derive implied edits  */
    for (j = (y->y_cols - 1); j >= stats->nb_edits; j--) {
        /* Compare column j to other columns  */
        for (k = (j - 1); k >= 0; k--) {
            /* Check for duplicate column of opposite sign */
            for (i = 0; i < y->u_rows; i++)
                if (fabs (y->column[j][i] + y->column[k][i]) >= EPSILON)
                    break;

            if ((i == y->u_rows) &&
                (fabs (y->column[j][mConstantRowIndex] +
                     y->column[k][mConstantRowIndex]) <= EPSILON)) {
                /* Duplicate column of opposite sign  */

                /* Find the first non-zero element  */
                for (i = 0; fabs (y->column[j][i]) <= EPSILON; i++)
                    /* DO NOTHING */;

                /* Keep the one having its first  */
                /* non-zero coefficient positive  */
                if (y->column[k][i] < -EPSILON) {
                    /* Switch column k with column j  */
                    tempcol = y->column[k];
                    y->column[k] = y->column[j];
                    y->column[j] = tempcol;
                }

                col = --y->y_cols;

                /* Column j will be remove from y  */
                /* and then not process twice      */
                tempcol = y->column[j];
                y->column[j] = y->column[col];
                y->column[col] = tempcol;

                /* Make column k an equality  */
                y->column[k][mFlagRowIndex] = 0.0;

                break;                        /* Only possible pair  */
            }
        }    /* End of for k  */
    }    /* End of for j  */

    /* Update statistics: number of implied edits redundant */
    stats->impedred = y->nb_cols - y->y_cols;

    y->nb_cols = y->y_cols;
}


/* ------------------------------------------------------------------ */
/*      Check implied edits                                           */
/*                                                                    */
/*      By comparing the entries of the upper part of the matrix      */
/*      of the edits and the constant to find the duplicates we       */
/*      can determine if the user specified limit of implied edits    */
/*      has been reached.                                             */
/*                                                                    */
/*      No more that two columns could be of opposite signs           */
/*      The step checknew removed all duplicate columns.              */
/* ------------------------------------------------------------------ */
static int checklim (
    EIT_CHERNIKOVA *y,      /* Implied edits matrix      */
    IMPLYSTA *stats)        /* Implied edits statistics */
{
    int chklim;             /* Count of redundant edits */

    register int i, j, k;


    chklim = 0;

    for (j = (y->nb_cols - 1); j >= stats->nb_edits; j--) {
        /* Compare column j to other columns  */
        for (k = (j - 1); k >= 0; k--) {
            /* Check for duplicate column of opposite sign */
            for (i = 0; i < y->u_rows; i++)
                if (fabs (y->column[j][i] + y->column[k][i]) >= EPSILON)
                    break;

            /* Duplicate column of opposite sign */
            if ((i == y->u_rows) &&
                (fabs (y->column[j][mConstantRowIndex] +
                     y->column[k][mConstantRowIndex]) <= EPSILON)) {
                chklim++; /* Increment number of redundant edits found. */
            }
        }    /* End of for k */
    }    /* End of for j */

    return chklim;
}


/* ------------------------------------------------------------------ */
/*      Print implied edits                                           */
/*      Print implied edits statistics                                */
/* ------------------------------------------------------------------ */
static void prntstat (
    EIT_CHERNIKOVA *y,          /* Implied edits matrix      */
    EIT_FIELDNAMES *Fields,     /* Variables names           */
    IMPLYSTA *stats)            /* Implied edits statistics */
{
    register int j;
    char relation[3];           /* Relation of edit      */
    char Space[1001];

    mPrintf ("\n");
    mPrintf (M40025 "\n\n"); /* Part 1 - List of implied edits */ 

    /* Initialize number of implied equalities and inequalities   */
    stats->imp_equa = 0;
    stats->imp_ineq = 0;

    /* Print original edits */
    mPrintf (M40026 "\n\n"); /* Original edits */
    for (j = 0; j < stats->nb_edits; j++) {
        if (y->column[j][mFlagRowIndex] == 0.0)
            strcpy (relation, " =");
        else
            strcpy (relation, "<=");

        /* Put constant right after the coefficients */
        y->column[j][y->u_rows] = y->column[j][mConstantRowIndex];

        /* Print the edit itself */
        EI_EditsFormatEdit (EIM_EDITS_FAKEID, y->column[j], Fields, relation,
            Space);
        mPrintf (Space);
    }

    /* Print implied edits   */
    if (stats->nb_edits < y->y_cols) {
        mPrintf ("\n\n");
        mPrintf (M40027 "\n\n"); /* Implied edits */
        for (j = stats->nb_edits; j < y->y_cols; j++) {
            if (y->column[j][mFlagRowIndex] == 0.0) {
                stats->imp_equa++;
                strcpy (relation, " =");
            }
            else {
                stats->imp_ineq++;
                strcpy (relation, "<=");
            }

            /* Put constant right after the coefficients */
            y->column[j][y->u_rows] = y->column[j][mConstantRowIndex];

            /* Print the edit itself */
            EI_EditsFormatEdit (EIM_EDITS_FAKEID, y->column[j], Fields,
                relation, Space);
            mPrintf (Space);
        }
    }
    else {
        mPrintf ("\n\n");
        mPrintf (M40028 "\n\n"); /* Implied edits equivalent to Input edits */
    }
 
    mPrintf ("\n\n");
    mPrintf (M40029 "\n\n"); /* Part 2 - Implied edits statistics */ 

/* For LengthHeadingSpace in printing */
#define  ONETAB   4

/* PRINT_DEFAULT() is used to print the default for max. edits.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_DEFAULT(Msg, LengthPlaceHolders, StringDefault) \
mPrintf (Msg, \
65 - strlen (Msg) + LengthPlaceHolders, \
"............................................................", \
StringDefault);

/* PRINT_COUNT() is used to print the count of edits.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_COUNT(Msg, LengthPlaceHolders, Count) \
mPrintf (Msg, \
65 - strlen (Msg) + LengthPlaceHolders, \
"............................................................", \
Count);

/* PRINT_SUBTOTAL() is used to print subtotal of edits,
   adding some heading spaces before the message.
   - parm LengthPlaceHolders is related to Msg
   - parm LengthHeadingSpace is ONETAB */
#define PRINT_SUBTOTAL(Msg, LengthPlaceHolders, LengthHeadingSpace, Count) \
mPrintf (Msg, \
LengthHeadingSpace, \
"                                                            ",\
65 - strlen (Msg) + LengthPlaceHolders - LengthHeadingSpace, \
"............................................................", \
Count);


    if (stats->limit == INT_MAX) {
        PRINT_DEFAULT (
            /*Specified maximum number of implied edits : max. integer (default)
                   M40030 -> Specified maximum number of implied edits :
                   M40031 -> max. integer (default) */
            M40030, M40030_LPH, M40031);            
        mPrintf ("\n");
    }
    else {
        PRINT_COUNT (
            /* Submitted maximum number of implied edits */
            M40032 "\n", M40032_LPH,
            stats->limit);
    }

    PRINT_COUNT (
        /* Number of implied edits */
        M40033 "\n", M40033_LPH,
        stats->imp_equa + stats->imp_ineq);
        
    PRINT_SUBTOTAL (
        /* Number of implied equalities */
        M40034 "\n", M40034_LPH, ONETAB,
        stats->imp_equa);

    PRINT_SUBTOTAL (
        /* Number of implied inequalities */
        M40035 "\n", M40035_LPH, ONETAB,
        stats->imp_ineq);
        
    mPrintf ("\n");
}


/* ------------------------------------------------------------------ */
/*      Scaling a column                                              */
/*                                                                    */
/*      Scaling is to find the first non-zero element of the upper    */
/*      part of the column (in matrix U) and divide the entire        */
/*      column by this value (except for the flag row).               */
/* ------------------------------------------------------------------ */
static void scale (
    double *column) /* Column to be scaled */
{
    register int i;
    double scaling;

    /* Find the first non zero element */
    for (i = 0; (scaling = fabs (column[i])) < EPSILON; i++);

    /* Scaling the columns */
    for (; i <= mConstantRowIndex; i++)
        column[i] /= scaling;
}
