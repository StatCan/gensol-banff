/* ---------------------------------------------------------------------
Localize the field to impute in a data record.
                The minimum number of fields to impute will be
                identified.

PARAMETERS      1 - Group of edits to apply to the data record
                2 - Data record
                3 - Field weights
                4 - Cardinality of field to impute
                5 - Chernikova Matrix
                7 - Return Code
--------------------------------------------------------------------- */

/* ---------------------------------------------------------------------
        Include Header Files
--------------------------------------------------------------------- */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "EI_Chernikova.h"
#include "EI_Edits.h"
#include "EI_ErrorLocalization.h"
#include "EI_Message.h"
#include "EI_Weights.h"

#include "MessageBanffAPI.h"


#include "util.h"

/* ---------------------------------------------------------------------
        Define MACROS
--------------------------------------------------------------------- */

/* ****************************************************************** */
/*                                                                    */
/*      Macro to keep a column                                        */
/*                                                                    */
/*    A             B                                                 */
/*    |             |                                                 */
/*    v             v                                                 */
/*    _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _                                 */
/*   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_| Y array of pointers to columns */
/*                                                                    */
/*      The column pointers B and A are exchanged and                 */
/*      A is incremented.                                             */
/*                                                                    */
/*   At the end A contains the number of columns kept and they        */
/*   are in the first position of the array Y.                        */
/*                                                                    */
/* ****************************************************************** */

#define KEEP_COLUMN(y,a,b) \
{\
double * tempcol;\
tempcol=y->column[a];\
y->column[a]=y->column[b];\
y->column[b]=tempcol;\
a++;\
}

#define ENDLIST  -1
#define MINUSEPS -EPSILON
#define PLUSEPS   EPSILON
#define EVENWEIGHT 1.0
#define NO_U 1
#define U    0

#define DEBUG 0

#define CPU_TIME()    ((double)clock()/(double)CLOCKS_PER_SEC)

/* ---------------------------------------------------------------------
--------------------------------------------------------------------- */
typedef struct {
    double max_time;  /* maximum time per record  */
    double init_time; /* initial time for record  */
    double curr_time; /* current time for record  */
} RECTIME;


/* ---------------------------------------------------------------------
Procedures which are located in this file.
--------------------------------------------------------------------- */
static void AddColumn (double *, double *, double, int *, double *,
    EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *);
static void BuildI (double *, int *, int *);
static double CalculateMinimumCardinality (EIT_DATAREC *, EIT_WEIGHTS *);
static int CheckU (int, int, int *, EIT_CHERNIKOVA *);
static double Complement (double *, double *, double *, EIT_CHERNIKOVA *,
    EIT_CHERNIKOVA_COMPANION *);
static void Exceeded (double *, int *, int *, EIT_CHERNIKOVA *);
static void Failed (int, int, EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *);
static int IsFinalExtremePoint (int *, double *, int);
static void newcherk (double *, int, int, int *, int *, RECTIME *,
    EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *);
static void OneIteration (int, double *, int *, int *, EIT_CHERNIKOVA *);
static void Pivot (int, int, int *, EIT_CHERNIKOVA *,
    EIT_CHERNIKOVA_COMPANION *);
static void PrintIntArray (char *, int *);
static void Purge (int, int, EIT_CHERNIKOVA *);
static void PurgeGenPattern(int *, int, EIT_CHERNIKOVA *, EIT_DATAREC *);
static void Remove_Column(int, EIT_CHERNIKOVA *);
/*static void RecompColCard( int , EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *);*/

/* ---------------------------------------------------------------------
void EI_AppendMissing();
For each missing field in Data, a '1.0' is added to the lower rows of Y.
Essetially does a logical 'OR' with the vector (d0,d1,...,dn) [where di
is the ith data value and di == 0 if di is not missing, and di == 1 if di
is missing, n == Number of Fields] and the vector (l0, ...,ln) which is
the lower part of Y.
--------------------------------------------------------------------- */
/* void EI_AppendMissing( */
/*       EIT_DATAREC * Data,   */
/*       EIT_CHERNIKOVA * y,   */
/*       int fxtrm */
/*       ) */
/* { */
/*   int i,j; */

/*   for (i = 0; i < fxtrm; i++){ */
/*     for (j = y->u_rows; j < y->u_rows + Data->NumberofFields; j++){ */
/*       if (Data->StatusFlag[j - y->u_rows] != FIELDOK) */
/* 	y->column[i][j] = 1; */
/*     }   */
/*   } */
/* } */

/* ---------------------------------------------------------------------
EIT_ERRORLOCALIZATION_RETURNCODE EI_ErrorLocalization ()
--------------------------------------------------------------------- */
EIT_ERRORLOCALIZATION_RETURNCODE EI_ErrorLocalization (
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights,
    EIT_DATAREC * Data,
    double Cardinality,
    double AllowedTime,   /*  Maximum time allowed for a record */
    EIT_CHERNIKOVA * y,     /*  Chernikova matrix           */
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    int morerow;
    int pivotrow;
    int p_o_y_t_n;
    int xtrm;                     /*  count extremal point       */
    int fxtrm;                    /*  count final extremal point  */
    double MinimumCardinality;
    double NewCardinality;
    RECTIME RecTime;

#if DEBUG
    int iter;/*number of chernikovications*/
#endif

    /* Set to CARDINALITY_EXCCEEDED, SPACE_EXCEEDED or TIME_EXCEEDED,
       or leave to EIE_ERRORLOCALIZATION_SUCCEED */
    int rcode = EIE_ERRORLOCALIZATION_SUCCEED;

    NewCardinality = Cardinality;
    RecTime.max_time = AllowedTime;
    if (RecTime.max_time < EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW) {
        RecTime.max_time = EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW;
        
        /* Minimum time per record is %.2f. This parameter has been reset. */
        EI_AddMessage ("EI_ErrorLocalization", EIE_INFORMATION,
            M10010 "\n", EIM_ERRORLOCALIZATION_MINIMUMTIMEPERROW);
    }

    /*
    calculate the cardinality base on missing and negative values.
    If the cardinality is greater than the requested cardinality,
    there is no need to continue.
    */
    MinimumCardinality = CalculateMinimumCardinality (Data, Weights);

    if (MinimumCardinality > Cardinality) {
#if DEBUG
      printf ("%f %f\nMinimumCardinality is too high...\n",
	      MinimumCardinality, Cardinality);
#endif
        return EIE_ERRORLOCALIZATION_CARDINALITY_EXCEEDED;
    }


    /*Ignore the below comment.*/
    /* Set the missing flag, and set weights.
       If a field is missing, its weight should be set, temporarily,
       to a very low number (0.000001).
       Before returning, it must be reset.
     */
/*     missing = 0; */
/*     for (morerow = 0; morerow < Data->NumberofFields; morerow++) */
/*       if (Data->StatusFlag[morerow] != FIELDOK){ */
/* 	missing = 1; */
	/*This guarantees the field will be marked as FTI.
	  See the documentation.*/

/* 	yCompanion->w[morerow] = EVENWEIGHT; */
/* 	y->preinit[morerow][y->cardrow] = EVENWEIGHT; */
/* 	y->preinit[morerow + yCompanion->nbvar][y->cardrow] = EVENWEIGHT; */
 /*      } */

    /* initialize chernikova matrix */
    EI_ChernikovaReinit (Data, Edits, y, yCompanion, &xtrm, &fxtrm);

    RecTime.init_time = CPU_TIME();

    morerow = y->u_rows;

#if DEBUG
    EI_ChernikovaPrint("First", y);
    iter = 0;
#endif

    while (morerow > 0) {
        newcherk (&NewCardinality, fxtrm, xtrm, &p_o_y_t_n, &pivotrow,
            &RecTime, y, yCompanion);

#if DEBUG0
	printf("pivotal row is %d \n\n", pivotrow);
	printf("p_o_y_t_n == %d fxtrm == %d xtrm == %d\n\n",
	       p_o_y_t_n,fxtrm,xtrm);
	printf("\n u = %d max = %d nbrow = %d y = %d nbcols = %d\n",
	       y->u_rows, y->max_cols, y->nb_rows, y->y_cols, y->nb_cols);
#endif

        if (pivotrow >= END_ITERATION){
	  OneIteration (p_o_y_t_n, &NewCardinality, &xtrm, &fxtrm, y);

	  PurgeGenPattern(&fxtrm, (y->nb_rows - y->u_rows - 2) / 2 , y, Data);
#if DEBUG
	    EI_ChernikovaPrint("asdf", y);
	    printf("fxtrm = %d\n", fxtrm);
	    iter++;
#endif
	}
        else if (pivotrow == NO_PIVOT)
            morerow = 0;
        else if (pivotrow == NO_EXTREME)
            xtrm = 0;
        else if (pivotrow == TIME_EXCEEDED) {
            xtrm = 0;
            fxtrm = 0;
            rcode = EIE_ERRORLOCALIZATION_TIME_EXCEEDED;
        } else {
            Exceeded (&NewCardinality, &xtrm, &fxtrm, y);
            rcode = EIE_ERRORLOCALIZATION_SPACE_EXCEEDED;
        }

        /* no extreme point */
        if (xtrm == 0) {
            y->y_cols = 0;                   /* empty matrix */
            morerow   = 0;
        }
    } /* end while morerow */

#if DEBUG
	    EI_ChernikovaPrint("asdf", y);
	    printf("fxtrm = %d\n", fxtrm);
	    iter++;
#endif

 /*    if (missing){ */
/*       EI_AppendMissing(Data, y, fxtrm); */
/*       RecompColCard(fxtrm, y, yCompanion); */
/*       fxtrm = y->y_cols; */
      /* Reset the weights everywhere. */
/*       for (morerow = 0; morerow < Data->NumberofFields; morerow++) */
/* 	if (Data->StatusFlag[morerow] != FIELDOK){ */
/* 	  yCompanion->w[morerow] = Weights->Value[morerow]; */
/* 	  y->preinit[morerow][y->cardrow] = Weights->Value[morerow]; */
/* 	  y->preinit[morerow + yCompanion->nbvar][y->cardrow] =  */
/* 	    Weights->Value[morerow]; */
/* 	} */
/*     } */
#if DEBUG
	    EI_ChernikovaPrint("asdf", y);
	    printf("fxtrm = %d\n", fxtrm);
	    iter++;
#endif

    Purge (fxtrm, yCompanion->nbvar, y);

#if DEBUG
    EI_ChernikovaPrint("asdf", y);
    EI_ChernikovaPrintSolutions( y, Data);
    EI_ChernikovaPrintStat (y);
	EI_AddMessage ("EI_ErrorLocalization", 4, "NewCardinality (Trong) = %e\n", NewCardinality);
	EI_AddMessage ("EI_ErrorLocalization", 4, "Cardinality (Trong) = %e\n", Cardinality);
#endif

    if (rcode == EIE_ERRORLOCALIZATION_SUCCEED && y->y_cols == 0) {
        if (NewCardinality > Cardinality)
            rcode = EIE_ERRORLOCALIZATION_CARDINALITY_EXCEEDED;
		else
			if  (NewCardinality == Cardinality)
				rcode = EIE_ERRORLOCALIZATION_OTHER ;
			else
				rcode = EIE_ERRORLOCALIZATION_FAIL; /* Oh oh big trouble rene */
    }
#if DEBUG
	EI_AddMessage ("EI_ErrorLocalization", 4, "Number of interations2 = %d\n", iter);
#endif
    return rcode;
}

/* ------------------------------------------------------------------ */
/*      Take linear combination of column s and t                     */
/*      and add the new column to yprime                              */
/* ------------------------------------------------------------------ */
static void AddColumn (
    double  *cols,      /*  Column S pointer             */
    double  *colt,      /*  Column T pointer             */
    double   colcard,
    int     *r,         /*  Pivotal row        */
    double  *Cardinality,
    EIT_CHERNIKOVA * y,  /*  Chernikova matrix  */
    EIT_CHERNIKOVA_COMPANION * yCompanion)  /*  Chernikova matrix companion */
{
    double last_elm;                 /*  Last entry of the row        */
    double *newcol;                  /*  New column pointer           */
    double  colsr,                   /*  Absolute value of column S   */
            coltr;                   /*  and T on row R               */
    int  i;
                                            /*  Add column to yprime  */
    if (y->nb_cols >= y->max_cols) {
                                   /*  Allocate new column to yprime  */
        EI_ChernikovaReallocate (y);
        if (y->nb_cols >= y->max_cols) {
            *r = SPACE_EXCEEDED;
            return;                               /*  Not enough space  */
        }
    }

    newcol = y->column[y->nb_cols];
    colsr  = fabs (cols[*r]);
    coltr  = fabs (colt[*r]);
                                          /*  Build column in yprime  */
    for (i = 0; i <= y->flagrow; i++) {
        /*UTIL_DebugPrint ("%e\n%e\n%e\n%e\n%d\n\n", colsr, colt[i], coltr, cols[i], i);*/
		
		newcol[i]  = colsr * colt[i] + coltr * cols[i];

        if (fabs (newcol[i]) < PLUSEPS)
            newcol[i] = 0.0;
    }

    last_elm = newcol[y->flagrow];
                                               /*  Scaling newcol */
    if (last_elm != NOT_XTR_POINT) {
        if (last_elm != XTR_POINT)
            for (i = 0; i < y->flagrow; i++) {
                newcol[i] /= last_elm;

                if (fabs (newcol[i]) < PLUSEPS)
                    newcol[i] = 0.0;
            }

        if (IsFinalExtremePoint (yCompanion->rowtype, newcol, y->u_rows) == 1) {
            newcol[y->flagrow] = FINAL_XTR_POINT;
            if (colcard < *Cardinality)
                *Cardinality = colcard;
        }
        else
            newcol[y->flagrow] = XTR_POINT;
    }
    else
        newcol[y->flagrow] = NOT_XTR_POINT;

    newcol[y->cardrow] = colcard;
    y->nb_cols++;
/*
printf("  nbcol %d flagrow %10.8f  cardrow %10.8f\n", y->nb_cols,
          newcol[y->flagrow], newcol[y->cardrow]);
*/
    return;
}


/**************************************************************/
/*  build I1 list                                              */
/***************************************************************/
static void BuildI (
    double *coli,
    int    *IZ,
    int    *IL)
{
    /*  Find set Ii(s, t)  */
    while (*IZ != ENDLIST) {
        if (coli[*IZ] == 0.0)
            *IL++ = *IZ;         /*  Add to IL only rows with zero  */
                                 /*  entries for column i           */
        IZ++;
    }
    *IL = ENDLIST;

    return;
}


/* ------------------------------------------------------------------ */
/*
calculate the minimum cardinality base on missing and negative values
*/
/* ------------------------------------------------------------------ */
static double CalculateMinimumCardinality (
    EIT_DATAREC * Data,
    EIT_WEIGHTS * Weights)
{
    int i;
    double MinimumCardinality;

    MinimumCardinality = 0.0;

    for (i = 0; i < Data->NumberofFields; i++)
        if (Data->StatusFlag[i] != FIELDOK)
            MinimumCardinality += Weights->Value[i];

    return MinimumCardinality;
}


/***************************************************************/
/*  check U                                                    */
/*                                                             */
/* column s and t must be skip when checking for the zero      */
/***************************************************************/
static int CheckU (
    int      s,
    int      t,
    int     *I0st,
    EIT_CHERNIKOVA *y)
{
                                       /*  Find a column u such that  */
    int u;                             /*  Y   = 0 for all i          */
    double *colu;                      /*   iu         of I1 (s, t)   */
    int    *rowi;                      /* If such column U exist no   */
                                       /* new column will be generated*/
    for (u = 0; u < y->y_cols; u++) {
        if ((u != s) && (u != t)) {
            colu = y->column[u];
            rowi = I0st;
            while (*rowi != ENDLIST) {
               if (colu[*rowi] != 0.0)
                  break;
               rowi++;
            }
            if (*rowi == ENDLIST)
                return U;
        }
    }
    return NO_U;
}


/* ------------------------------------------------------------------ */
/*      Compute the cardinality and check the complementary condition */
/*                                                                    */
/* ------------------------------------------------------------------ */
static double Complement (
    double  *cols,              /*  Column S pointer             */
    double  *colt,              /*  Column T pointer             */
    double  *Cardinality,
    EIT_CHERNIKOVA *y,
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    double  maxcard, colcard, temp;
    int     i, r1, r2;

    colcard = 0.0;
    maxcard = *Cardinality + PLUSEPS;

    r1 = y->u_rows;
    r2 = y->u_rows + yCompanion->nbvar;
    for (i = 0; i < yCompanion->nbvar; i++, r1++, r2++) {
        /* fail complementary condition between columns*/
        if (((colt[r1] * cols[r2]) > PLUSEPS) ||
            ((colt[r2] * cols[r1]) > PLUSEPS))
                return -1.0;

         /* compute Cardinality */
         temp = colt[r1] + cols[r1];
         temp +=colt[r2] + cols[r2];
         if (temp > PLUSEPS)
             colcard += yCompanion->w[i];
      }

    if (colcard > maxcard)
        return -1.0;

    return colcard;
}


/* ------------------------------------------------------------------ */
/*      Limitation on memory space has been reached.                  */
/*      Space exceeded.                                               */
/*                                                                    */
/*      Reduce the column cardinality of Y.                           */
/*                                                                    */
/*      NOTE. Once the cardinality has been reduced, the cardinality  */
/*            will never exceed the new cardinality.                  */
/* ------------------------------------------------------------------ */
static void Exceeded (
    double * Cardinality,
    int * xtrm,
    int * fxtrm,
    EIT_CHERNIKOVA * y)
{
    double    cardmax, minuscard;           /*  cardinality maximun    */
    int    onextrm,                /*  at least one extremal  point */
    onefxtrm;
    int    k, j;

    cardmax = -1.0;                    /* keep maximun cardinality  */
    /* NOTE the Epsilon value is not*/
    /* required because the machine */
    /* precision is smaller then Epsilon*/
    for (j = 0; j < y->y_cols; j++)
        if (y->column[j][y->cardrow] > cardmax)
            cardmax = y->column[j][y->cardrow] ;

    minuscard = (double) EPSILON;
    minuscard *= 3;
    cardmax -= minuscard;

    /* move extremal point at beginning*/
    for (k = *fxtrm, j = *fxtrm; j < y->y_cols; j++)
        if (y->column[j][y->flagrow] > NOT_XTR_POINT) {
            /* extremal point */
            KEEP_COLUMN(y, k, j);
        }

    *xtrm    = k;

    onextrm = 0;                  /* no extremal point              */
    onefxtrm = 0;                 /* no final extremal point        */
    k        = 0;                 /* position to beginning of array Y */

    /* column's cardinality is less than */
    /* maximun, keep the column          */
    for (j = 0; j < y->y_cols; j++)
        if (y->column[j][y->cardrow] < cardmax) {
            KEEP_COLUMN(y, k, j);

            if (j < *xtrm) {
                /* at least one extremal point*/
                if (j < *fxtrm)
                    onefxtrm++;
                else
                    onextrm++;
            }
        }

    y->y_cols = k; /* reset number of columns in Y */
    *Cardinality = cardmax; /* reset cardinality maximun  */
    *xtrm     = onextrm + onefxtrm;  /* number of extremal point    */
    *fxtrm    = onefxtrm;            /* number of final extremal point*/

    return;
}


/* ------------------------------------------------------------------ */
/*   Identify the fail edit row                                       */
/*                                                                    */
/*   An equality is fail if the row contains a non zero element and   */
/*   the flag (column flag) corresponding to this element is non zero.*/
/*                                                                    */
/*   An inequality is fail if the row contains a negative element and */
/*   the flag (column flag) corresponding to this element is non zero.*/
/*                                                                    */
/*   The non zero column flag are delimited by fxtrm and xtrm.        */
/* ------------------------------------------------------------------ */
static void Failed (
    int fxtrm,
    int xtrm,
    EIT_CHERNIKOVA *y , /*  Input Chernikova matrix                   */
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    int k, j;

    for (k = 0; k < y->u_rows; k++) {
                               /* reset the fail row before next */
                               /* iteration. A previous fail row */
                               /* may become a pass row for this */
                               /* iteration                      */
        if (yCompanion->pivolist[k] != PROCESS_ROW) {
            yCompanion->pivolist[k] = NOT_PROCESS_ROW;

            if (yCompanion->rowtype[k] == INEQ_ROW) {
                for (j = fxtrm; j < xtrm; j++) {
                    if (y->column[j][k] < MINUSEPS) {
                        yCompanion->pivolist[k] = FAIL_ROW;
                        break;
                    }
                }
            }
            else {
                for (j = fxtrm; j < xtrm; j++) {
                    if ((y->column[j][k] < MINUSEPS)  ||
                        (y->column[j][k] > PLUSEPS)) {
                        yCompanion->pivolist[k] = FAIL_ROW;
                        break;
                    }
                }
            }
        }
    }
}


/* ------------------------------------------------------------------ */
/*      check if a column is a final extremal point.                  */
/*                                                                    */
/*      If the last element  of the column is equal to 1 and          */
/*                                                                    */
/*         the U part of Y is checked as follows:                     */
/*                                                                    */
/*         FOR each row in U part of the COLUMN do:                   */
/*            IF row is an equality AND                               */
/*               the column's coefficient is not zero                 */
/*            THEN                                                    */
/*               the column is not a final extremal point             */
/*                                                                    */
/*            IF row is an inequality AND                             */
/*               the column's coefficient is less than zero           */
/*            THEN                                                    */
/*               the column is not a final extremal point             */
/*                                                                    */
/*            OTHERWISE the column is a final extremal point          */
/*                                                                    */
/*       ELSE the column is not a final extremal point                */
/*                                                                    */
/* ------------------------------------------------------------------ */
static int IsFinalExtremePoint (
    int     *rowtype,
    double  *column,
    int      upart)
{
    int   i;

    for (i = 0; i < upart; i++) {
        if (column[i] < MINUSEPS)
            return 0;
        if ((rowtype[i] == EQUA_ROW) && (column[i] != 0.0))
            return 0;
    }

    return 1;
}
/* ---------------------------------------------------------------------
NAME            newcherk()
FUNCTION        Perform one iteration of the Chernikova algotithm.
PARAMETERS      1 - Type of each row in input matrix.
                2 - Cardinality
                3 -
                4 - Number of final extremal points.
                5 - Number of extremal points, including final.
                6 - Pointer to the structure of Chernikova matrix.
                7 - Part of Y to new YPRIME.
                8 - Indicator of rows already selected as pivot.
                9 - Pointer to the index of the pivotal row.
               10 - Pointer to time structure.

        The Chernikova structure includes two logical matrices:
        y and yprime.  The structure has only one physical matrix
        which is referenced by the array of pointer to column:
        "column".  This means that the two matrices are stored
        consecutively.

        "y_cols"    indicates the number of columns in the y matrix,
        "nb_cols"   indicates the total number of column in both
                    matrices y and yprime.
        "p_o_y_t_n" indicates the number of columns from y which are
                    also part of yprime.

        The way to access each of the matrices is as follows:
        matrix y       == from "0" (inclusively) to "y_cols"
                          (exclusively)
        matrix yprime  == from "y_cols" (inclusively) to "nb_cols"
                          (exclusively)
        part of y also == from "0" (inclusively) to "p_o_y_t_n"
        part of yprime    (exclusively)

   The optimization of this program has been done using some property
   of the matrix. It may happen that not all the properties have been
   documented but we had tried to document the program as much as we
   can. Here is a list of the procedure used for the optimization:

   1) Never combine the final extremal point with other column. The
      new column generated will have an higher cardinality and purged
      immediatly.

   2) Based on the type of the pivotal row, the columns are reordered
      in such a way that is not necessary to do a multiplication to
      identify the columns which are of opposite sign.

   3) A partial purge has been added when the column is generated.

   4) The columns of Y which need to be copied to Yprime are not copied
      but only reordered. This save memory and CPU.

   5) A flag for each row is kept to avoid reprocessing a row that
      contains only non negative elements. Those rows are used to build
      the set I0.

   6) Two steps have been combined in a single step (Build I0 and find
      the pivotal row).

   7) The input matrix must be sorted in proper sequence. Final extremal
      point first, followed by the non final extremal point and in the
      last position the columns which have a flag equal to zero.

   8) A new heuristic has been added to identify the fail edits called
      the fail row.

   9) The column cardinality is computed only once and stored into the
      matrix.

  10) The variables start, end, fxtrm and xtrm delimit the processing.

  11) The step BUILDI1 is broken into two parts. First build the set
      I0S which contains the row index of the zero element for the
          column S.
      and for each T build the set
      I0ST which contains the row index of the zero element for the
          column S and T.

WARNING: Any person who will have to modify the program will have to
         keep in mind the points described above. If not it may lead
         to a point that the program ERRORLOC will not run anymore.

--------------------------------------------------------------------- */
static void newcherk (
    double   *Cardinality,
    int       fxtrm, /* number of final extremal point             */
    int       xtrm, /* number of extremal point including final   */
    int      *p_o_y_t_n, /* part of y to new yprime                    */
    int      *pivotrow, /* pivotal row if any                         */
    RECTIME  *RecTime, /* Pointer to record timing information       */
    EIT_CHERNIKOVA  *y, /*  Chernikova matrix                         */
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    int j, k;                  /*  Row index                          */

    int s, t;                   /*  Column index in y                  */
    int s_start;
    int s_end;
    int t_start;
    int t_end;

    double colcard;

    int positive;              /* count negative and positive elements*/
    int negative;              /* on pivotal row                      */


    y->nb_cols = y->y_cols;       /* empty the matrix Yprime          */

    *pivotrow = NO_PIVOT;
                                           /*  Find a pivotal row r  */
    Pivot (fxtrm, xtrm, pivotrow, y, yCompanion);

/*
printf ("exiting Pivot %d\n", *pivotrow);
*/

    if ((*pivotrow == NO_PIVOT) || (*pivotrow == NO_EXTREME))
        return;

          /* Construct first part of yprime by reordering the columns */
          /* of Y and  counting the number of columns to  be copied at*/
          /* the end of the iteration                                 */

          /* see comment below to know the ordering of the columns    */

          /* The loop start from fxtrm to avoid combining the final   */
          /* extremal point with others columns                       */
/******** step 2 of the algorithm **********/
    for (j = fxtrm, k = fxtrm; j < y->y_cols; j++) {
        if ((y->column[j][*pivotrow] == 0) ||
            ((y->column[j][*pivotrow] >= 0) &&
             (yCompanion->rowtype[*pivotrow] == INEQ_ROW))) {
            KEEP_COLUMN (y, k, j);            /* see macro in header file */
        }
    }
    *p_o_y_t_n = k;
/*
printf("after copy part of y to n  %d \n", k);
*/
                       /*  Check for special case:  nb_cols in y = 2  */
    if ((y->y_cols == 2) &&
       ((y->column[0][*pivotrow] * y->column[1][*pivotrow]) < 0.0)) {
                                            /*  Add column to yprime  */
        colcard = Complement (y->column[0], y->column[1], Cardinality,
            y, yCompanion);

        if (colcard > 0) {
            AddColumn (y->column[0], y->column[1], colcard,
                pivotrow, Cardinality, y, yCompanion);
            if (*pivotrow == SPACE_EXCEEDED)
                return;
        }
        yCompanion->pivolist[*pivotrow] = PROCESS_ROW;
        return;
    }

            /*row has been ordered by the previous step (build yprime)*/
            /*                EQUALITY ROW                            */
            /* ------------------------------------------------------ */
            /* |000000000000000|000000000000000000|---+-+--++++-+--+| */
            /* ------------------------------------------------------ */
            /*  final extremal ^  zero entries    ^ non zero entries^ */
            /*     point       |                  |                 | */
            /*             fxtrm          p_o_y_t_n         y->y_cols */
            /*                                                        */
            /* reorder the row has followed:                          */
            /* ------------------------------------------------------ */
            /* |000000000000000|000000000000000000|++++++++---------| */
            /* ------------------------------------------------------ */
            /*  final extremal ^  zero entries    ^        ^        ^ */
            /*     point       |                  |        |        | */
            /*             fxtrm         p_o_y_t_n  negative   y_cols */
            /*                                                        */
            /*                                                        */
            /* ****************************************************** */
            /*                INEQUALITY ROW                          */
            /* ------------------------------------------------------ */
            /* |000000000000000|00+0000++0++0+0+0+|-----------------| */
            /* ------------------------------------------------------ */
            /*  final extremal ^ non negative     ^ negative entries^ */
            /*     point       |                  |                 | */
            /*             fxtrm          p_o_y_t_n         y->y_cols */
            /*                                                        */
            /* reorder the row has followed:                          */
            /* ------------------------------------------------------ */
            /* |000000000000000|++++++++0000000000|-----------------| */
            /* ------------------------------------------------------ */
            /*  final extremal ^        ^         ^                 ^ */
            /*     point       |        |         |                 | */
            /*             fxtrm        | p_o_y_t_n            y_cols */
            /*                   negative                             */
            /*                                                        */
            /* ****************************************************** */

    positive = 0;
    if (yCompanion->rowtype[*pivotrow] == EQUA_ROW) {
        for (j = *p_o_y_t_n, k = *p_o_y_t_n; j < y->y_cols; j++)
            if (y->column[j][*pivotrow] > PLUSEPS) {
                KEEP_COLUMN (y, k, j);
                positive++;
            }
        negative = *p_o_y_t_n + positive;

        s_start = *p_o_y_t_n;
        s_end   = negative;
        t_start = negative;
        t_end   = y->y_cols;
    }
    else {
        for (j = fxtrm, k = fxtrm; j < *p_o_y_t_n; j++)
            if (y->column[j][*pivotrow] > PLUSEPS) {
                KEEP_COLUMN (y, k, j);
                positive++;
            }
        negative = fxtrm + positive;

        s_start = fxtrm;
        s_end   = negative;
        t_start = *p_o_y_t_n;
        t_end   = y->y_cols;
    }
/*
printf("after sorting sstart %d send %d tstart %d tend %d \n",
       s_start, s_end, t_start, t_end);
prntmtrx(y->column, y->nb_rows, y->y_cols);
*/
                                    /* Generate new column for  each  */
                                    /* pair of (s,t) of oppposite sign*/
    for (s = s_start; s < s_end; s++) {
        BuildI (y->column[s], yCompanion->I0, yCompanion->I0s);

/*PrintIntArray ("I0s ", yCompanion->I0s);*/

        for (t = t_start; t < t_end; t++) {
            RecTime->curr_time = CPU_TIME();
/*UTIL_DebugPrint ("s=%d\n", s);*/
/*UTIL_DebugPrint ("t=%d\n", t);*/
/*UTIL_DebugPrint ("%f %f %f %f\n",*/
/*RecTime->curr_time-RecTime->init_time,*/
/*RecTime->max_time,*/
/*RecTime->init_time,*/
/*RecTime->curr_time*/
/*);*/

            if ((RecTime->curr_time-RecTime->init_time) >= RecTime->max_time) {
                *pivotrow = TIME_EXCEEDED;
                return;
            }

            colcard = Complement (y->column[s], y->column[t], Cardinality,
                y, yCompanion);

            if (colcard > 0) {
                BuildI (y->column[t], yCompanion->I0s, yCompanion->I0st);

/*PrintIntArray ("I0st", yCompanion->I0st); */

                if (*yCompanion->I0st != ENDLIST) {
                    if (CheckU (s, t, yCompanion->I0st, y) == NO_U) {
                        AddColumn (y->column[s], y->column[t], colcard,
                            pivotrow, Cardinality, y, yCompanion);

                        if (*pivotrow == SPACE_EXCEEDED)
                            return;
                    }
                }
            }                 /* end if colcard   */
        }               /* end for t   */
    }              /* end for s  */
    yCompanion->pivolist[*pivotrow] = PROCESS_ROW;
/*
prntmtrx(y->column, y->nb_rows, y->nb_cols);
*/
    return;
}


/* ------------------------------------------------------------------ */
/*      One iteration of Chernikova has been performed.               */
/*                                                                    */
/*       The matrix Y is structured as in following:                  */
/*                                                                    */
/*           final      non  final           other                    */
/*        ext. points   ext. points          points                   */
/*       *-----------*----------------*-----------------------*       */
/*       |           |                |                       |       */
/*       *-----------*----------------*-----------------------*       */
/*                                                                    */
/*                                                                    */
/*       before                                                       */
/*        <--------- Y ---------------><----- Y' ---->                */
/*       *---*---------*--------*------*-------------*--------*       */
/*       |   |         |        |      |             |        |       */
/*       *---*---------*--------*------*-------------*--------*       */
/*           ^         ^        ^      ^             ^        ^       */
/*           |         |        |      |             |        |       */
/*       fxtrm      xtrm   p_o_y_t_n   |             |        |       */
/*                                y->y_cols    y->nb_cols   y->maxcols*/
/*       Y' = FROM 0 TO p_o_y_t_n (inclusively) +                     */
/*            FROM y->y_cols (inclusively) TO y->nb_cols (exclusively)*/
/*                                                                    */
/*      The columns between y->nb_cols and y->maxcols are allocated   */
/*      but not used.                                                 */
/*                                                                    */
/*      Only the minimum number of columns of Y' will be moved        */
/*      mincol = minimum of ((y->y_cols - p_o_y_t_n),                 */
/*                           (y->nb_cols - y->y_cols))                */
/*                                                                    */
/*       after                                                        */
/*        <--------- -----Y --------------->   not used               */
/*       *------*------------*-------------*--------------------*     */
/*       |      |            |             |                    |     */
/*       *------*------------*-------------*--------------------*     */
/*              ^            ^             ^                    ^     */
/*              |            |             |                    |     */
/*          fxtrm         xtrm             |                    |     */
/*                                y->y_cols                 y->maxcols*/
/*                                y->nb_cols                          */
/*                                                                    */
/*     After the move Y' become Y.                                    */
/*                                                                    */
/*        All columns having a cardinality less than the minimum      */
/*        cardinality are kept.                                       */
/* ------------------------------------------------------------------ */
static void OneIteration (
    int p_o_y_t_n,
    double * Cardinality,
    int * xtrm,
    int * fxtrm,
    EIT_CHERNIKOVA * y)
{
    double    minpepsi;            /*  minimum + epsilon      */

    int    j, k;
    int    xtpoint, fxtpoint;

    /*
    prntmtrx(y->column, y->nb_rows, y->nb_cols);
    */

    /*  Move the minimum number of columns  */
    /*  from Y' to Y                        */
    for (j = y->nb_cols - 1;  (j >= y->y_cols) && (p_o_y_t_n < y->y_cols);
        j--) {
        KEEP_COLUMN(y, p_o_y_t_n, j);         /* see macro in header file */
    }
    /* Re-initialize the matrix */
    y->y_cols = p_o_y_t_n + j - y->y_cols + 1;

    /* move extremal point at beginning*/
    for (k = *fxtrm, j = *fxtrm; j < y->y_cols; j++)
        if (y->column[j][y->flagrow] > NOT_XTR_POINT) {
            /* column is an extremal point */
            KEEP_COLUMN(y, k, j);
        }
    *xtrm    = k;
    /* move final extremal point at beginning*/
    for (k = *fxtrm, j = *fxtrm; j < *xtrm; j++)
        if (y->column[j][y->flagrow] == FINAL_XTR_POINT) {
            KEEP_COLUMN(y, k, j);
        }
    *fxtrm = k;

    p_o_y_t_n = 0;                 /* position to beginning of array Y*/
    fxtpoint = 0;
    xtpoint  = 0;
    /* keep column with cardinality less */
    /* than minimun, keep column         */
    minpepsi = *Cardinality + EPSILON;
    for (j = 0; j < y->y_cols; j++)
        if (y->column[j][y->cardrow] <= minpepsi) {
            KEEP_COLUMN(y, p_o_y_t_n, j);

            if (j < *xtrm) {
                if (j < *fxtrm)
                    fxtpoint++;
                else
                    xtpoint++;
            }
        }
    /*
       prntmtrx(y->column, y->nb_rows, p_o_y_t_n);
    */
    y->y_cols = p_o_y_t_n;            /* reset number of columns in Y */
    *xtrm     = xtpoint + fxtpoint;  /* number of extremal point    */
    *fxtrm    = fxtpoint;            /* number of final extremal point*/

    return;
}



/* ------------------------------------------------------------------ */
/*      Check negative rows and find pivotal row                      */
/* ------------------------------------------------------------------ */
static void Pivot (
    int      fxtrm,
    int      xtrm,
    int     *r, /* Pivotal row */
    EIT_CHERNIKOVA *y, /* Input Chernikova matrix */
    EIT_CHERNIKOVA_COMPANION * yCompanion)
{
    int j, k;               /*  loop index                            */

    unsigned long int m_i,  /*  Summation of row i                    */
        m_min,              /*  Minimum row summation                 */
        n_i,                /*  Number of negative elements in row i  */
        p_i,                /*  Number of positive elements in row i  */
        z_i;                /*  Number of zero elements in a row i    */

    int * I0;

    I0 = yCompanion->I0t;

    m_min = INT_MAX;

                        /* if no fail equality find the fail inequality */
    Failed (fxtrm, xtrm, y, yCompanion);

    for (k = 0; k < y->u_rows; k++) {
        if (yCompanion->pivolist[k] == FAIL_ROW) {
            n_i = 0;
            p_i = 0;
                                    /*  Find the number of negative,  */
                                    /*  positive and zero elements    */
            for (j = fxtrm; j < y->y_cols; j++) {
                if (y->column[j][k] < MINUSEPS)
                    n_i++;
                else if (y->column[j][k] > PLUSEPS)
                    p_i++;
            }
                                   /*  Check if the row is completely */
                                   /*  negative or positive           */
                                   /* Will be true only if no final   */
                                   /* extremal point (fxtrm == 0)     */
            if ((n_i == y->y_cols) || (p_i == y->y_cols)) {
                *r = NO_EXTREME;
                return;
            }

            z_i = y->y_cols - p_i - n_i;
            m_i = z_i + (p_i * n_i);

            if (yCompanion->rowtype[k] == INEQ_ROW)
                m_i += p_i;
                              /*  Compare with minimum row summation  */
            if (m_min > m_i) {
                m_min = m_i;
                *r    = k;                  /*  New pivotal row found  */
            }
            else             /* fail row contains only positive or zero */
                           /* it is a fail equality. Add row to set I0*/
                if (n_i == 0)
                    *I0++ = k;
        }                                 /*  End of fail row         */
        else if (yCompanion->pivolist[k] == NOT_PROCESS_ROW) {
            for (j = fxtrm; j < y->y_cols; j++)
                if (y->column[j][k] < MINUSEPS)
                    break;
                            /* row has not been processed but contains*/
                            /* only positive or zero entries. We will */
                            /* never pivot on such row, if the row is */
                            /* an inequality.                         */
            if (j >= y->y_cols) {
                if (yCompanion->rowtype[k] == INEQ_ROW)
                    yCompanion->pivolist[k] = PROCESS_ROW;
                 *I0++ = k;
            }
        }
        else
            *I0++ = k;    /* The row is a processed row, it contains*/
                          /* only non negative entries. Add the row */
                          /* index to the set I0.                   */

    } /*  End of the for  */

    *I0 = ENDLIST;
    return;
}


/* ------------------------------------------------------------------ */
/*                                                                    */
/*                                                                    */
/* ------------------------------------------------------------------ */
static void PrintIntArray (
    char * msg,
    int * I)
{
    printf ("%s: ", msg);
    while (*I != ENDLIST) {
        printf ("%2d, ", *I);
        I++;
    }
    puts ("");
}


/* ------------------------------------------------------------------ */
/*                                                                    */
/* Check if the algorithm ended correctly, if so keep only final      */
/* extremal point with unique pattern                                 */
/*                                                                    */
/* ------------------------------------------------------------------ */
static void Purge (
    int fxtrm,
    int nbvar,
    EIT_CHERNIKOVA * y)
{
    int    coljpat,                /*  column J as a unique pattern */
    p_o_y_t_n;
    double    * pattrn1;               /*  point to the column's pattern*/
    double    * pattrn2;               /*  point to the column's pattern*/
    int    i, j, k;

  p_o_y_t_n = 0;                 /* position to beginning of array Y*/

    /* process the column which are final      */
    /* extremal point with minimum cardinality */
    for (j = 0; j < fxtrm; j++) {
        /* compute column J pattern */
        pattrn1 = &y->column[j][y->u_rows];
        pattrn2 = &pattrn1[nbvar];
        for (i = 0; i < nbvar; i++, pattrn1++, pattrn2++)
            if (fabs (*pattrn1 - *pattrn2) > EPSILON)
                *pattrn1 = 1.0;
            else
                *pattrn1 = 0.0;
    }

    for (j = 0; j < fxtrm; j++) {
        coljpat = 0;
        for (k = j + 1; k < fxtrm; k++) {
            pattrn1 = &y->column[j][y->u_rows];
            pattrn2 = &y->column[k][y->u_rows];
            for (i = 0; i < nbvar; i++, pattrn1++, pattrn2++) {
                if (*pattrn1 == 1.0) {
                    if (*pattrn2 == 0.0)
                        /* column J pattern is 1.0 and column K pattern is 0.0*/
                        break;
                }
                else {
                    if (*pattrn2 == 1.0)
                        /* column J pattern is 0.0 and column K pattern is 1.0*/
                        break;
                }
            }

            if (i >= nbvar) {
                coljpat = 1;      /* column J pattern = column K pattern */
                break;
            }
        } /* end for k = j + 1       */

        if (coljpat == 0) { /* column J has a unique pattern */
            KEEP_COLUMN(y, p_o_y_t_n, j); /* at least one extremal point */
        }
    } /* end for j < *fxtrm           */

    y->y_cols = p_o_y_t_n;            /* reset number of columns in Y */
    return;
}


static void PurgeGenPattern(
			    int *fxtrm,
			    int nbvar,
			    EIT_CHERNIKOVA *y,
			    EIT_DATAREC *Data)
{
  double *pattern1;
  double *pattern2;
  int i, j, k;

  for (j = 0; j < *fxtrm; j++) {
    for (k = y->y_cols - 1; k >= *fxtrm; k--) {
      pattern1 = &y->column[j][y->u_rows];
      pattern2 = &y->column[k][y->u_rows];
      /*if pat1 = pat2 remove col[k]*/
      for (i = 0; i < nbvar * 2; i++, pattern1++, pattern2++) {
	if ((*pattern1 > EPSILON) ||
	    (i < nbvar ? (Data->StatusFlag[i] != FIELDOK):
	     (Data->StatusFlag[i - nbvar] != FIELDOK) ) ){
	  if ((*pattern2 < EPSILON) &&
	      (i < nbvar ?  (Data->StatusFlag[i] == FIELDOK):
	       (Data->StatusFlag[i - nbvar] == FIELDOK) ))
	    break;
	}
	else {
	  if ((*pattern2 > EPSILON) ||
	      (i < nbvar ? (Data->StatusFlag[i] != FIELDOK):
	       (Data->StatusFlag[i - nbvar] != FIELDOK) ))
	    break;
	}
      }
      if (i >= nbvar * 2) {
	Remove_Column(k, y);
	break;
      }
    }
  }
  return;
}

/* ------------------------------------------------------------------ */
/*                                                                    */
/*  Instead of using KEEP_COLUMN inside a loop, which keeps all       */
/*  columns over and over again.                                      */
/*                                                                    */
/* ------------------------------------------------------------------ */
static void Remove_Column (
	int j,
	EIT_CHERNIKOVA *y)
{
  int i, p_o_y_t_n;

  p_o_y_t_n = 0;

  for (i= 0; i < y->y_cols; i++)
    if (i != j)
      KEEP_COLUMN(y, p_o_y_t_n, i);

  y->y_cols = p_o_y_t_n;
  return;
}

/*recompute column cardinality, and delete the necessary columns*/
/* static void RecompColCard( int fxtrm, */
/* 			   EIT_CHERNIKOVA *y, */
/* 			   EIT_CHERNIKOVA_COMPANION * yCompanion */
/* 			   ) */
/* { */
/*   int r1,r2; */
/*   int i,j; */
/*   double card = 0; */
/*   double cardMin; */

/*   cardMin = y->column[0][y->cardrow]; */
/*   for (j = 0; j < fxtrm; j++) { */
/*     r1 = y->u_rows; */
/*     r2 = y->u_rows + yCompanion->nbvar; */

/*     for (i = 0; i < yCompanion->nbvar; i++, r1++, r2++){ */
/*       if ((y->column[j][r1] + y->column[j][r2]) > PLUSEPS) */
/* 	card += yCompanion->w[i]; */
/*     } */
/*     y->column[j][y->cardrow] = card; */
/*     if(j==0)  */
/*       cardMin = card; */
/*     else */
/*       cardMin = (card <= cardMin ? card : cardMin); */
/*     card = 0; */
/*   } */

/*   for (j = fxtrm - 1; j >=0; j--) { */
/*     if (y->column[j][y->cardrow] > cardMin){ */
/*       Remove_Column(j, y); */
/*       fxtrm--; */
/*     } */
/*   } */

/*   y->y_cols = fxtrm; */
/* } */
