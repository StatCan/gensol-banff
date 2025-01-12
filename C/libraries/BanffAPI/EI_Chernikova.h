#ifndef EI_CHERNIKOVA_H
#define EI_CHERNIKOVA_H

/*#include <stdio.h>*/

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Weights.h"

#define EPSILON   (double) 1.0e-6    /* zero precision               */
#define RANDNUM_FUZZ (double) 1.0e-12 /* 1.0-(maximum allowed value for random number values) */

#define FINAL_XTR_POINT  2.0
#define       XTR_POINT  1.0
#define   NOT_XTR_POINT  0.0

                             /* Flag for row that have been used as   */
                             /* pivotal row                           */
#define NOT_PROCESS_ROW 0    /* row has not been chosen for pivot     */
#define PROCESS_ROW     1    /* row has already been chosen for pivot */
#define FAIL_ROW        2    /* row is a fail edit                    */

#define ROW_NOT_PIVOT   0
#define ROW_IS_PIVOT    1

                             /* rowtype for the U part of the matrix  */
#define EQUA_ROW       0     /* row is an equality                    */
#define INEQ_ROW       1     /* row is an inequality                  */

typedef struct {
    /*
    preinit is initialize in EI_ChernikovaAllocate () and used in
    EI_ChernikovaReinit (). 
    The info it contains is use to reinitialize the chernikova matrix
    for a new respondant.
    */
    double**preinit;    /* preinit */
    int preinitRows;    /* # of rows in preinit */
    int preinitColumns; /* # of columns in preinit */

    double**column; /* coefficients of the matrix stored by columns*/
    int    y_cols;  /* number of columns in use in the y part of the matrix*/
    int    nb_cols; /* number of columns in use in the matrix*/
    int    max_cols;/* maximun number of columns currently allocated in matrix*/
    int    u_rows;  /* Number of rows in the U part of the matrix*/
    int    nb_rows; /* Number of rows in the matrix (Upper + Lower matrices)*/

    int cardrow;
    int flagrow;
} EIT_CHERNIKOVA;

typedef struct {
    int * pivolist;
    int * rowtype;
    double * w;

    int nbvar;

    /* used in newcherk () */
    int * I0;   /* List of rows of y with only zero or positive entries */
    int * I0s;  /* List of rows in y with only zero entries for s       */
    int * I0st; /* List of rows in y with only zero entries for s and t */

    /* used in Pivot () */
    int * I0t; /* List of rows in y with only zero entries for s and t */
} EIT_CHERNIKOVA_COMPANION;

CLASS_DECLSPEC int EI_ChernikovaAllocate (EIT_EDITS *, EIT_WEIGHTS *,
    EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *);
CLASS_DECLSPEC int EI_ChernikovaChooseSolution (EIT_CHERNIKOVA *, int, short *, int, double, int);
CLASS_DECLSPEC void EI_ChernikovaFree (EIT_CHERNIKOVA *,
    EIT_CHERNIKOVA_COMPANION *);
CLASS_DECLSPEC void EI_ChernikovaPrint (char*, EIT_CHERNIKOVA *);
CLASS_DECLSPEC void EI_ChernikovaPrintSolutions (EIT_CHERNIKOVA *, EIT_DATAREC *);
CLASS_DECLSPEC void EI_ChernikovaPrintStat (EIT_CHERNIKOVA *);
CLASS_DECLSPEC int EI_ChernikovaReallocate (EIT_CHERNIKOVA *);
CLASS_DECLSPEC void EI_ChernikovaReinit ( EIT_DATAREC *, EIT_EDITS *,
    EIT_CHERNIKOVA *, EIT_CHERNIKOVA_COMPANION *, int *, int *);

#endif
