/* ---------------------------------------------------------------------
FUNCTION        Find the extremal point of a feasible region.
PARAMETERS      1 - Name of the group of set of edits.
                2 - User cardinality for extremal point.
--------------------------------------------------------------------- */

/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "EI_Chernikova.h"
#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_ErrorLocalization.h"
#include "EI_Extremal.h"

#include "STC_Memory.h"

#include "util.h"

#include "MessageBanffAPI.h"

/* ---------------------------------------------------------------------
        Local structure
--------------------------------------------------------------------- */

typedef struct {
    int usercard;    /* User defined cardinality       */
    int Cardinality; /* Effective cardinality          */
} XTRMSTAT;

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

/* ---------------------------------------------------------------------
Procedures which are located in this file.
--------------------------------------------------------------------- */
static void add_col (int, int, int, EIT_CHERNIKOVA *);
static void build (EIT_EDITS *, EIT_FIELDNAMES *, EIT_CHERNIKOVA *, int **,
    int **);
static void cherniko (int *, int *, EIT_CHERNIKOVA *, int *, int *, int *,
    long *);
static void exceeded (int *, EIT_CHERNIKOVA *);
static void one_iter (int, int, EIT_CHERNIKOVA *);
static void pivot (EIT_CHERNIKOVA *, int *, int *, int *, long *);
static void pivot_no (int *, EIT_CHERNIKOVA *);
static void printstat (EIT_CHERNIKOVA *, EIT_FIELDNAMES *, XTRMSTAT *);

static EIT_PRINTF mPrintf;

/* ---------------------------------------------------------------------
        Macro to compute the column's cardinality
--------------------------------------------------------------------- */
#define COLUMN_CARDINALITY(y,i,j,card) {\
card=0;\
for(i=y->u_rows;i<y->nb_rows-1;i++)\
if(fabs(y->column[j][i])>EPSILON)\
card++;\
}


/* ---------------------------------------------------------------------
        Macro to switch two columns
--------------------------------------------------------------------- */
#define SWITCH_COLUMN(y,a,b) {\
double * tempcol;\
tempcol=y->column[a];\
y->column[a]=y->column[b];\
y->column[b]=tempcol;\
a++;\
}

/* ---------------------------------------------------------------------
                           M A I N   L I N E
--------------------------------------------------------------------- */
int EI_Extremal (
    EIT_EDITS * Edits,
    int MaxCardinality,
    EIT_PRINTF Printf)
{
    EIT_FIELDNAMES Fields;    /* Variable names         */
    int nb_var;               /* Number of variables    */
                              /* in group               */
    int * pivolist;           /* Indicator flags for    */
                              /* pivot rows             */
    int pivoloop;             /* Working integer        */
    int pivotrow;             /* Index of pivot row     */
    long rcode;               /* Return code            */
    int * rowtype;            /* Indicator flags for    */
                              /* type of rows           */
    XTRMSTAT stats;           /* Statistics             */
    EIT_CHERNIKOVA y;         /* Chernikova matrix      */

    int * I0; /* List of rows of y with only zero or positive entries */
    int * I1; /* List of rows in y with only zero entries for s and t */


    mPrintf = Printf;

    mPrintf ("\n\n");
    mPrintf (M40036 "\n\n"); /* Edits extremal points analysis diagnostics */

    y.preinit = NULL;

    /* Get the initial matrix  */
    build (Edits, &Fields, &y, &rowtype, &pivolist);
    if (y.column == NULL)
        return -1;

    if (DEBUG) EI_EditsPrint (Edits);
    //if (DEBUG) EI_ChernikovaPrint ("after build", &y);

    I0 = STC_AllocateMemory (y.nb_rows * sizeof *I0);
    if (I0 == NULL)
        return -1;
        
    I1 = STC_AllocateMemory (y.nb_rows * sizeof *I1);
    if (I1 == NULL)
        return -1;

                                                     /* Update statistics  */
    stats.usercard = MaxCardinality;   /* User defined cardinality         */

    nb_var = y.nb_cols - 1;            /* Number of variables in group     */

    if ((MaxCardinality == -1) || (MaxCardinality > nb_var))
        MaxCardinality = nb_var;

    pivoloop = 1;

    while (pivoloop != 0) {

        cherniko (rowtype, pivolist, &y, &pivotrow, I0, I1, &rcode);

        switch (rcode) {
        case END_ITERATION:
            /* Successfull iteration of Chernikova  */
            /* A row is never select twice in the   */
            /* Chernikova Algorithm. Set the pivot  */
            /* list flag accordingly.               */
            pivolist[pivotrow] = ROW_IS_PIVOT;
            one_iter (MaxCardinality, nb_var, &y);

            //if (DEBUG) EI_ChernikovaPrint ("after one_iter", &y);

            if (y.nb_cols == 0)
                pivoloop = 0;
            break;

        case NO_EXTREME:
            /* No extreme point  */
            pivoloop = 0;
            y.y_cols = 0;   /* Empty the matrix  */
            break;

        case NO_PIVOT:
            /* No more row to pivot on  */
            pivoloop = 0;
            pivot_no (rowtype, &y);

            //if (DEBUG) EI_ChernikovaPrint ("after pivot_no", &y);

            break;

        default:
            /* Space is exceeded  */
            exceeded (&MaxCardinality, &y);
            if (y.nb_cols == 0)
                pivoloop = 0;
            break;

        } /* end switch */
    } /* end while  */

    /* Update statistics  */
    stats.Cardinality = MaxCardinality;

    /* Print the statictics  */
    printstat (&y, &Fields, &stats);

    EI_ChernikovaFree (&y, NULL);
    STC_FreeMemory (I0);
    STC_FreeMemory (I1);
    STC_FreeMemory (pivolist);
    STC_FreeMemory (rowtype);

    return 0;
}



/* ------------------------------------------------------------------ */
/*      Take linear combination of column s and t                     */
/*      and add the new column to yprime                              */
/* ------------------------------------------------------------------ */
static void add_col (
    int r,                                /* Pivotal row        */
    int s,                                /* Column index in y  */
    int t,                                /* Column index in y  */
    EIT_CHERNIKOVA * y)                   /* Chernikova matrix  */
{

    register int i,                  /* Row index in y and yprime    */
                 last_row;           /* Row index of the last entry  */

    double last_elm;                 /* Last entry of the row        */
    double * newcol;                 /* New column pointer           */
    double * cols;                   /* Column S pointer             */
    double * colt;                   /* Column T pointer             */
    double  colsr,                   /* Absolute value of column S   */
            coltr;                   /* and T on row R               */


                                            /* Add column to yprime  */
    if (y->nb_cols >= y->max_cols) {
                                   /* Allocate new column to yprime  */
        EI_ChernikovaReallocate (y);
        if (y->nb_cols >= y->max_cols)
            return;                             /* Not enough space  */
    }

    newcol = y->column[y->nb_cols];
    cols   = y->column[s];
    colt   = y->column[t];
    colsr  = fabs(cols[r]);
    coltr  = fabs(colt[r]);
                                          /* Build column in yprime  */
    for (i = 0; i < y->nb_rows; i++) {
        newcol[i] = colsr * colt[i] + coltr * cols[i];

                                           /* Correct zero elements  */
        if (fabs(newcol[i]) < EPSILON)
            newcol[i] = 0.0;
    }

                 /* Determine the last element of the processed row  */
    last_row = y->nb_rows - 1;
    last_elm = newcol[last_row];
                                               /* Scaling of yprime  */
    if ((last_elm != 0.0) && (last_elm != 1.0)) {
        for (i = 0; i < last_row; i++) {
            newcol[i] /= last_elm;
                                           /* Correct zero elements  */
            if (fabs(newcol[i]) < EPSILON)
                newcol[i] = 0.0;
        }
        newcol[last_row] = 1.0;
    }

                  /* Update the current number of columns in yprime  */
    y->nb_cols++;

    return;
}



/* ------------------------------------------------------------------ */
/*      Build the initial Chernikova matrix                           */
/*                                                                    */
/*      |    |                   |                                    */
/*      | U  | -A        B       |                                    */
/*      |    |   (M x N)  (M x 1)|                                    */
/*      |    |                   |                                    */
/* Y<---|----|-------------------|                                    */
/*      |    |                   |                                    */
/*      | L  |  Identity of      |                                    */
/*      |    |     size (N + 1)  |                                    */
/*      |    |                   |                                    */
/*      |    |                   |                                    */
/*      |    |                   |                                    */
/*                                                                    */
/*  A   is the coefficients of the edits excluding the constant       */
/*      (-A is the negation of A)                                     */
/*  B   is the edits's constant                                       */
/*  I   is an identity matrix                                         */
/*  M   is the number of edits in the groups                          */
/*  N   is the number of variables in the group of edits              */
/*      excluding the constant                                        */
/*                                                                    */
/*  U   is the upper part of the matrix Y                             */
/*  L   is the lower part of the matrix Y                             */
/*                                                                    */
/* ------------------------------------------------------------------ */
static void build (
    EIT_EDITS * Edits,
    EIT_FIELDNAMES * Fields,      /* Variable names            */
    EIT_CHERNIKOVA * y,           /* Chernikova matrix         */
    int ** rowtype,               /* Indicator flags for type  */
    int ** pivolist)              /* Pivot list flags          */
{
    double ** dptrptr;
    double negate;

    int last_col;
    int collen;
    int * typerow;
    int * listpivo;

    register int i, j;


                                          /* Initialize parameters  */
    y->column   = NULL;
    y->nb_rows  = 0;
    y->nb_cols  = 0;
    y->y_cols   = 0;
    y->max_cols = 0;
    y->u_rows   = 0;

    *rowtype  = NULL;
    *pivolist = NULL;

                                     /* Set the length of a column  */
    collen = Edits->NumberofEquations + Edits->NumberofColumns;

                            /* Allocate space for array of columns  */
    dptrptr = STC_AllocateMemory (Edits->NumberofColumns * sizeof *dptrptr);
    if (dptrptr == NULL)
        return; /*FAIL*/

    last_col = Edits->NumberofColumns - 1;
    negate   = -1.0;

    for (j = 0; j < Edits->NumberofColumns; j++) {
        dptrptr[j] = STC_AllocateMemory (collen * sizeof *dptrptr[j]);
        if (dptrptr[j] == NULL)
           return; /*FAIL*/

                                   /* Last column does not need to  */
       if (j == last_col)          /* be negated                    */
           negate = 1.0;

                     /* Copy Edits' column in U part of the matrix  */
                     /* do not negate 0 coefficients                */
       for (i = 0; i < Edits->NumberofEquations; i++)
           if (Edits->Coefficient[i][j] == 0.0)
               dptrptr[j][i] = 0.0;
           else
               dptrptr[j][i] = negate * Edits->Coefficient[i][j];

                        /* Initialize L part of the matrix to zero  */
       for (i = Edits->NumberofEquations; i < collen; i++)
           dptrptr[j][i] = 0.0;

                    /* Initialize diagonal in L part of the matrix  */
       i = Edits->NumberofEquations + j;
       dptrptr[j][i] = 1.0;
    }

    typerow = STC_AllocateMemory (Edits->NumberofEquations * sizeof *typerow);
    if (typerow == NULL)
       return; /*FAIL*/

    for (i = 0; i < Edits->NumberofEquations; i++)
                                          /* Initialize the rowtype  */
        if (i < Edits->NumberofInequalities)
            typerow[i] = INEQ_ROW;          /* Row is an inequality  */
        else
            typerow[i] = EQUA_ROW;            /* Row is an equality  */

    listpivo = STC_AllocateMemory (Edits->NumberofEquations * sizeof *listpivo);
    if (listpivo == NULL)
        return; /*FAIL*/

    for (i = 0; i < Edits->NumberofEquations; i++)
        listpivo[i] = ROW_NOT_PIVOT;

    y->column   = dptrptr;                 /* Initialize parameters  */
    y->nb_rows  = collen;
    y->nb_cols  = Edits->NumberofColumns;
    y->y_cols   = y->nb_cols;
    y->max_cols = y->nb_cols;
    y->u_rows   = Edits->NumberofEquations;

    *rowtype  = typerow;
    *pivolist = listpivo;

    Fields->FieldName = Edits->FieldName;        /* Return list of variables */
    Fields->NumberofFields = Edits->NumberofColumns - 1;
}


/* ---------------------------------------------------------------------
FUNCTION        Perform one iteration of the Chernikova algotithm.
PARAMETERS      1 - Type of each row in input matrix.
                2 - Indicator of rows already selected as pivot.
                3 - Pointer to the structure of Chernikova matrix.
                4 - Pointer to the index of the pivotal row.
                5 - Pointer to the return code.
RETURN VALUE    void:  The last three parameters are updated.

        The Chernikova structure includes two logical matrices:
        y and yprime.  The structure has only one physical matrix
        which is referenced by the array of pointer to column:
        "column".  This means that the two matrices are stored
        consecutively.

        "y_cols" indicates the number of columns in the y matrix,
        "nb_cols" indicates the total number of column in both
        matrices y and yprime.

        The way to access each of the matrices is as follows:
        the indices from "0" (inclusively) to "y_cols"
        (exclusively) are for the y matrix and the indices from
        "y_cols" (inclusively) to "nb_cols" (exclusively) are
        for the yprime matrix.

--------------------------------------------------------------------- */
static void cherniko (
    int * row_type, /* Type of each row in y                     */
    int * pivolist, /* Indicates rows already selected as pivot  */
    EIT_CHERNIKOVA * y, /* Chernikova matrix                     */
    int * pivotrow, /* Index of the pivotal row                  */
    int * I0,
    int * I1,
    long * rcode)   /* Return code                               */
{
    register int i,            /* Row index                          */
                 j;            /* Column index in y                  */

    int          l,            /* List index                         */
                 s,            /* Column index in y                  */
                 t,            /* Column index in y                  */
                 u;            /* Column index in y                  */

    int nb_I0,                 /* Number of elements in the list I0  */
        nb_I1,                 /* Number of elements in the list I1  */
        oldnbcol,              /* Old number of columns              */
        r;                     /* Pivotal row                        */


                                                 /* Set return code  */
    *rcode = END_ITERATION;

                                            /* Find a pivotal row r  */
    pivot (y, pivolist, row_type, &r, rcode);

    if ((*rcode == NO_PIVOT) || (*rcode == NO_EXTREME))
        return;

                                  /* Construct first part of yprime  */
    y->nb_cols = y->y_cols;
    for (j = 0; j < y->y_cols; j++) {
        if (((y->column[j][r] >= 0) && (row_type[r] == INEQ_ROW)) ||
            ((y->column[j][r] == 0) && (row_type[r] == EQUA_ROW))) {
                                            /* Add column to yprime  */
            if (y->nb_cols >= y->max_cols) {
                                   /* Allocate new column to yprime  */
                EI_ChernikovaReallocate (y);
                if (y->nb_cols >= y->max_cols) {
                    *rcode = SPACE_EXCEEDED;
                    return;                     /* Not enough space  */
                }
            }

                                              /* Copy column from y  */
            memcpy (y->column[y->nb_cols], y->column[j],
                sizeof (double) * y->nb_rows);

            y->nb_cols++;
        }
    }

                       /* Check for special case:  nb_cols in y = 2  */
    if ((y->y_cols == 2) &&
       ((y->column[0][r] * y->column[1][r]) < 0.0)) {

        s = 0;
        t = 1;
                                            /* Add column to yprime  */
        oldnbcol = y->nb_cols;
        add_col (r, s, t, y);

        if (oldnbcol == y->nb_cols) {
            *rcode = SPACE_EXCEEDED;
            return;
        }

        *rcode = END_ITERATION;
        *pivotrow = r;
        return;
    }

                                                     /* Find set I0  */
    nb_I0 = 0;
    for (i = 0; i < y->nb_rows; i++) {
                                        /* Add to I0 only rows with  */
                                        /* positive or zero entries  */
        for (j = 0; j < y->y_cols; j++)
            if (y->column[j][i] < - EPSILON)
                break;

        if (j >= y->y_cols) {
                                    /* No negative entry on the row  */
            I0[nb_I0] = i;
            nb_I0++;
        }
    }

                                    /* Is there another pair (s, t)  */
    for (s = 0; s < (y->y_cols - 1); s++) {
        for (t = (s + 1); t < y->y_cols; t++) {
            if ((y->column[s][r] * y->column[t][r]) < 0.0) {
                                              /* Find set I1 (s, t)  */
                nb_I1 = 0;
                for (i = 0; i < nb_I0; i++) {
                    l = I0[i];
                    if ((y->column[s][l] == 0.0) &&
                        (y->column[t][l] == 0.0)) {
                                   /* Add to I1 only rows with zero  */
                                   /* entries for columns s and t    */
                        I1[nb_I1] = l;
                        nb_I1++;
                    }
                }

                if (nb_I1 != 0) {
                                       /* Find a column u such that  */
                                       /* Y   = 0 for all i          */
                                       /*   iu         of I1 (s, t)   */
                    for (u = 0; u < y->y_cols; u++) {
                        if ((u != s) && (u != t)) {
                            for (i = 0; i < nb_I1; i++) {
                                l = I1[i];
                                if (y->column[u][l] != 0.0)
                                    break;
                            }
                            if (i >= nb_I1)
                                break;
                        }
                    }

                    if (u >= y->y_cols) {
                                                /* No such u column  */
                                            /* Add column to yprime  */
                        oldnbcol = y->nb_cols;
                        add_col (r, s, t, y);

                        if (oldnbcol == y->nb_cols) {
                            *rcode = SPACE_EXCEEDED;
                            return;
                        }
                    }
                }
            }
        }  /* End of the for t  */
    }  /* End of the for s  */

                                             /* End of an iteration  */
    *pivotrow = r;
    return;
}



/* ------------------------------------------------------------------ */
/*      Space exceeded.                                               */
/*      Keep only columns with cardinality less than                  */
/*      maximum cardinality.                                          */
/* ------------------------------------------------------------------ */
static void exceeded (
    int * Cardinality,
    EIT_CHERNIKOVA * y)
{
    register int i, j;
    int maxcard;
    int newcard;
    int ColumnNumber;      /* Column number               */
    int ColumnCardinality; /* Column's cardinality        */
   
    mPrintf (
        /* Error. Space exceeded. Old number of columns: max and y_col */                  
        M30079 "\n", y->max_cols, y->y_cols);                          

    maxcard = -1;

    /* Compute maximum cardinality  */
    for (j = 0; j < y->y_cols; j++) {
        COLUMN_CARDINALITY (y, i, j, ColumnCardinality);

        if (ColumnCardinality > maxcard)
            maxcard = ColumnCardinality;
    }

    ColumnNumber = 0;
    for (j = 0; j < y->y_cols; j++) {
        COLUMN_CARDINALITY (y, i, j, ColumnCardinality);

                               /* Keep only column with cardinality  */
                               /* less than the maximum cardinality  */
        if (ColumnCardinality < maxcard)
            SWITCH_COLUMN (y, ColumnNumber, j);
    }

    y->y_cols = ColumnNumber;                  /* Reset number of columns  */

    newcard = maxcard - 1;

    mPrintf (
        /* Error. Space exceeded. Cardinality has been reduced         
           from 'value' to 'value' because of space constraints. */
        M30080 "\n", *Cardinality, newcard);    

    *Cardinality = newcard;                   /* Reset new cardinality  */
}



/* ------------------------------------------------------------------ */
/*      One iteration of Chernikova has been performed.               */
/*      Move y' into y.                                               */
/*                                                                    */
/*      Two cases:                                                    */
/*                                                                    */
/*      1. Number of columns in y > number of columns in y'           */
/*                                                                    */
/*         Switch all columns of y' with y starting at the end of y'  */
/*                                                                    */
/*                                                                    */
/*        <---- y ----> <- y' ->        <- y' -> <---- y ---->        */
/*       *-------------*--------*      *--------*----*--------*       */
/*       |             |        | ---> |        |\\\\|        |       */
/*       *-------------*--------*      *--------*----*--------*       */
/*                                  (the middle part has not moved)   */
/*                                                                    */
/*      2. Number of columns in y <= number of columns in y'          */
/*                                                                    */
/*         Switch all columns of y with y' starting at the end of y'  */
/*                                                                    */
/*        <- y -> <---- y'----->        <---- y' ----> <- y ->        */
/*       *-------*--------------*      *-------*------*-------*       */
/*       |       |              | ---> |       |\\\\\\|       |       */
/*       *-------*--------------*      *-------*------*-------*       */
/*                                  (the middle part has not moved)   */
/*                                                                    */
/* ------------------------------------------------------------------ */
static void one_iter (
    int Cardinality,
    int nb_var,
    EIT_CHERNIKOVA *y)
{
    int last;                            /* Last column to be moved  */

    register int i, j;                   /* Indices                  */

    int ColumnNumber;      /* Column number               */
    int ColumnCardinality; /* Column's cardinality        */

                           /* Index of the last column to be moved  */
    last = y->nb_cols - 1;

                             /* Move the minimum number of columns  */
                             /* ColumnNumber is updated in macro    */
    ColumnNumber = 0;
    for (j = last; (j >= y->y_cols) && (ColumnNumber < y->y_cols); j--)
        SWITCH_COLUMN (y, ColumnNumber, j);

                           /* Re-initialize the matrix, to avoid    */
                           /* reallocation of the space. Only the   */
                           /* number of columns is reinitialized.   */
                           /* The max_cols is unchanged and indi-   */
                           /* cates the number of columns current-  */
                           /* ly allocated.                         */
    y->nb_cols = y->nb_cols - y->y_cols;
    y->y_cols  = y->nb_cols;

    /* Specified cardinality is less than the  */
    /* number of variables in the group        */
    if (Cardinality < nb_var) {
        ColumnNumber = 0;
        for (j = 0; j < y->y_cols; j++) {
            COLUMN_CARDINALITY (y, i, j, ColumnCardinality);

            /* Keep only column with cardinality    */
            /* less than the specified cardinality  */
            if (ColumnCardinality <= Cardinality)
                SWITCH_COLUMN (y, ColumnNumber, j);
       }
       y->y_cols = ColumnNumber;
   }
}



/* ------------------------------------------------------------------ */
/*      Check negative rows and find pivotal row                      */
/* ------------------------------------------------------------------ */
static void pivot (
    EIT_CHERNIKOVA * y, /* Input Chernikova matrix                   */
    int * pivolist,     /* Indicates rows already selected as pivot  */
    int * row_type,     /* Type of each row in y                     */
    int * r,            /* Pivotal row                               */
    long * rcode)       /* Return code                               */
{

    register int i,         /* Row index                             */
                 j;         /* Column index in y                     */

    int m_i,                /* Summation of row i                    */
        m_min,              /* Minimum row summation                 */
        n_i,                /* Number of negative elements in row i  */
        p_i,                /* Number of positive elements in row i  */
        z_i;                /* Number of zero elements in a row i    */


                                         /* Check all rows of U and  */
                                         /* find a pivotal row r     */
    m_min = INT_MAX;
    *r    = -1;

    for (i = 0; i < y->u_rows; i++) {
                                  /* Check only rows that have not   */
                                  /* been chosen as pivotal already  */
        if (pivolist[i] == ROW_NOT_PIVOT) {
            n_i = 0;
            p_i = 0;
            z_i = 0;
                                    /* Find the number of negative,  */
                                    /* positive and zero elements    */
            for (j = 0; j < y->y_cols; j++) {
                if (y->column[j][i] < - EPSILON)
                    n_i++;
                else if (y->column[j][i] > EPSILON)
                    p_i++;
                else
                    z_i++;
            }

                         /* Check if the row is completely negative  */
            if (n_i == y->y_cols) {
                *rcode = NO_EXTREME;
                return;
            }

                       /* Check if row is a potential candidate ie.  */
                       /* must have at least one negative element    */
            if (n_i != 0) {
                                          /* Evaluate row summation  */
                m_i = z_i + (p_i * n_i);
                if (row_type[i] == INEQ_ROW)
                    m_i += p_i;
                              /* Compare with minimum row summation  */
                if (m_min > m_i) {
                    m_min = m_i;
                    *r    = i;             /* New pivotal row found  */
                }
            }
        }  /* End of the if pivolist  */
    }  /* End of the for  */

                          /* Verify if a pivotal row has been found  */
    if (*r == -1) {
        *rcode = NO_PIVOT;
        return;                             /* No pivotal row found  */
    }

    return;
}



/* ------------------------------------------------------------------ */
/*      No pivotal row has been found.                                */
/*      Check the non zero equality rows and                          */
/*      if there is any extremal points.                              */
/* ------------------------------------------------------------------ */
static void pivot_no (
    int * rowtype,
    EIT_CHERNIKOVA * y)
{
    register int i, j;

    int ColumnNumber;   /* Column number               */

    for (i = 0; i < y->u_rows; i++) {         /* For each row in U  */
                                              /* Row is an equality  */
        if (rowtype[i] == EQUA_ROW) {         /* Verify row  */
            for (j = 0; j < y->y_cols; j++) { /* Non zero element  */
                if (fabs (y->column[j][i]) > EPSILON) {
                    y->y_cols = 0;              /* No extreme point  */
                    i         = y->nb_rows;     /* End outside loop  */
                    break;                      /* End inner loop    */
                }
            }     /* end for j  */
        }     /* end if rowtype  */
    }     /* end for i  */

    ColumnNumber = 0;
    for (j = 0; j < y->y_cols; j++) {
                                      /* Keep only column which are  */
                                      /* an extremal point           */
        if (fabs (y->column[j][y->nb_rows-1]) > EPSILON)
            SWITCH_COLUMN (y, ColumnNumber, j);
    }

    y->y_cols = ColumnNumber;        /* Reset number of columns in matrix  */
}



/* ------------------------------------------------------------------ */
/*        Print statistics                                            */
/* ------------------------------------------------------------------ */
static void printstat (
    EIT_CHERNIKOVA * y,        /* Chernikova matrix  */
    EIT_FIELDNAMES * Fields,   /* Variables names    */
    XTRMSTAT * stats)          /* Statistics         */
{
    char buffer[1001];
    int  nbpoints;
    register int i, j, col;
    int ColumnCardinality; /* Column's cardinality        */
    int *cards;   /* Counts of number of points of different cardinalities */

    nbpoints = 0;

    if (y->y_cols == 0) {
        mPrintf (
            /* Part 1 - No extremal point of cardinality less than or equal 
                        to 'value' has been found. */
            M40037 "\n\n", stats->Cardinality); 
    }
    else {
        cards = STC_AllocateMemory ((stats->Cardinality+1) * sizeof *cards);

        for (i = 0; i <= stats->Cardinality; i++)
            cards[i] = 0;

        mPrintf (M40038 "\n"); /* Part 1 - List of extremal points */
            
        for (j = 0; j < y->y_cols; j++) {
            mPrintf ("\n" M40039 "\n", nbpoints+1); /* Extremal point number */                

            col = Fields->NumberofFields-1;
            for (i = y->nb_rows-2; i>= y->u_rows;i--) {
                if (y->column[j][i] != 0.0) {
                    sprintf (buffer, "%f", y->column[j][i]);
                    UTIL_DropZeros (buffer);
                    mPrintf ("%-32s = %s\n", Fields->FieldName[col], buffer);
                }
                col--;
            }
            nbpoints++;

            /* Compute the column's cardinality  */
            COLUMN_CARDINALITY (y, i, j, ColumnCardinality);
            if (ColumnCardinality == 0)
                mPrintf (M40040 "\n"); /* All variables are 0. */

            cards[ColumnCardinality] += 1;
        }
        mPrintf ("\n");
    }

    mPrintf (M40041 "\n\n"); /* Variables not shown are 0. */

    mPrintf ("\n");
    mPrintf (M40042 "\n\n"); /* Part 2 - Extremal points statistics */

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

/* PRINT_CARDINALTY_COUNT() is used to print the cardinality and
   count of edits.
   - parm LengthPlaceHolders is related to Msg */
#define PRINT_CARDINALTY_COUNT(Msg, LengthPlaceHolders, Cardinality, Count) \
mPrintf (Msg, \
Cardinality, \
65 - strlen (Msg) + LengthPlaceHolders, \
"............................................................", \
Count);

    if (stats->usercard < 0) {
        PRINT_DEFAULT (
            /* Specified maximum cardinality : number of variables (default)
                   M40043 -> Specified maximum cardinality :
                   M40044 -> number of variables (default) */
            M40043, M40043_LPH, M40044);            
        mPrintf ("\n");
    }
    else {       
        PRINT_COUNT (
            /* Specified maximum cardinality */
            M40045 "\n", M40045_LPH, stats->usercard);                                
    }

    PRINT_COUNT (
        /* Effective maximum cardinality */        
        M40046 "\n", M40046_LPH, stats->Cardinality);
        
    PRINT_COUNT (
        /* Number of extremal points */               
        M40047 "\n", M40047_LPH, nbpoints);        

    if (nbpoints != 0) {
        mPrintf ("\n");
        for (i = 0; i <= stats->Cardinality; i++)
            if (cards[i] > 0)
                PRINT_CARDINALTY_COUNT ( 
                    /* Number of points with cardinality 'i' */        
                    M40048 "\n", M40048_LPH, i, cards[i]);                    
        STC_FreeMemory (cards);
    }
}
