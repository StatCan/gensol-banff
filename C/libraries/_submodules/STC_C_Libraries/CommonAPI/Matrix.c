#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "nr.h"
#include "nrutil.h"

#include "EI_Message.h"

#include "STC_Compare.h"

#include "STC_Memory.h"

#include "Matrix.h"
#include "singular_value_decomposition.h"
#include "util.h"

#include "MessageCommonAPI.h"

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
*/
enum {DEBUG = 0};

/*
used in Matrix_Inverse(). If the Matrix is 100 or bigger Matrix_Inverse()
calls Matrix_InverseLU(), otherwise it calls
Matrix_InverseByPartitionRecursive().
*/
#define LU_LIMIT 100

static void * mymemcpy (void *s1, const void *s2, size_t n);
static void CopyMatrixTo1OffsetMatrix (MATRIX * NR_A, MATRIX * A);

/* Functions for Iterative */
typedef double ** matrix_t;
static double GetScale (int size, double scale, matrix_t M);
static void Snorm (int size, matrix_t M, matrix_t B0, double norm);

/* Functions for LU */
#define LU_TINY 1.0e-20
static int dludcmp (double **a, int n, int *indx, double *d);
static void dlubksb (double **a, int n, int *indx, double b[]);

static double FindAbsMax (MATRIX *, int);
static MATRIX * Matrix2NR (MATRIX * A);
static MATRIX * NR2Matrix (MATRIX * NR);
static void PrintVec1 (double * vec, int r, char * Name);
static void ScaleColumn (MATRIX *, int, double);

/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Add (double *C, double *A, double *B, int nrows, int ncols)   */
/*                                                                            */
/*  Description:                                                              */
/*     Add matrices A and B to form the matrix C, i.e. C = A + B.             */
/*     All matrices should be declared as " double X[nrows][ncols] " in the   */
/*     calling routine, where X = A, B, C.                                    */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Address of the first element of the matrix C.             */
/*     double *A    Address of the first element of the matrix A.             */
/*     double *B    Address of the first element of the matrix B.             */
/*     int    nrows The number of rows of matrices A, B, and C.               */
/*     int    ncols The number of columns of the matrices A, B, and C.        */
/*                                                                            */
/*  Return Values:                                                            */
/*      void                                                                  */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], B[M][N], C[M][N];                                      */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Matrix_Add((double *) C, &A[0][0], &B[0][0], M, N);                    */
/*     printf("The matrix C = A + B is \n"); ...                              */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Add (double *C, double *A, double *B, int nrows, int ncols)
{
   register int n = nrows * ncols - 1;

   for (; n >= 0; n--) C[n] = A[n] + B[n];
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_AddScalar (double *A, double x, int nrows, int ncols)         */
/*                                                                            */
/*  Description:                                                              */
/*     Add each element of the matrix A by the scalar x.                      */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     double x     Scalar to multipy each element of the matrix A.           */
/*     int    nrows The number of rows of matrix A.                           */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], x;                                                     */
/*                                                                            */
/*     (your code to initialize the matrix A and scalar x)                    */
/*                                                                            */
/*     Matrix_AddScalar (&A[0][0], x, M, N);                                  */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_AddScalar (double *A, double x, int nrows, int ncols)
{
   register int n = nrows * ncols - 1;

   for (; n >= 0; n--) A[n] += x;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  MATRIX * Matrix_Allocate (int nrows, int ncols)                           */
/*                                                                            */
/*  Description:                                                              */
/*     Allocate a matrix.                                                     */
/*                                                                            */
/*  Arguments:                                                                */
/*     int    nrows The number of rows of matrix A.                           */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     MATRIX *                                                               */
/*                                                                            */
/*  Example:                                                                  */
/*     MATRIX * A                                                             */
/*                                                                            */
/*     A = Matrix_Allocate (M, N);                                            */
/*////////////////////////////////////////////////////////////////////////////*/
MATRIX * Matrix_Allocate (int nrows, int ncols)
{
    MATRIX * A;
    int i;

    A = STC_AllocateMemory (sizeof *A);
    if (!A) {
         return NULL;
    }
    A->valueSq = STC_AllocateMemory (sizeof *A->valueSq * nrows * ncols);
    if (!A->valueSq) {
         STC_FreeMemory (A);
         return NULL;
    }
    A->value = STC_AllocateMemory (sizeof *A->value * nrows);
    if (!A->value) {
         STC_FreeMemory (A->valueSq);
         STC_FreeMemory (A);
         return NULL;
    }
    for (i = 1, *A->value = A->valueSq; i < nrows; i++){
        *(A->value+i) = *(A->value + i - 1) + ncols;
    }
    Matrix_Fill (A->valueSq, 0, nrows, ncols);
    A->nrows = nrows;
    A->ncols = ncols;
    return A;
}
/*////////////////////////////////////////////////////////////////////////////*/
/* print some statistics on the Matrix */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Analyse (MATRIX * A)
{
    register int n = A->nrows * A->ncols - 1;

    double maxnegvalue, minnegvalue;
    double maxposvalue, minposvalue;
    double v;
    int zero = 0;
    int almostzero = 0;

    maxnegvalue = 0.0;
    minnegvalue = -DBL_MAX;
    minposvalue = DBL_MAX;
    maxposvalue = 0.0;
    for (; n >= 0; n--) {
        v = A->valueSq[n];
        if (v < 0 && v < maxnegvalue) maxnegvalue = v;
        if (v < 0 && v > minnegvalue) minnegvalue = v;
        if (v > 0 && v < minposvalue) minposvalue = v;
        if (v > 0 && v > maxposvalue) maxposvalue = v;

        if (v == 0.0) zero++;
        else if (STC_COMPARE_EQ (v, 0.0)) almostzero++;
    }
    EI_AddMessage ("", 4, "nrows=%d ncols=%d\n", A->nrows, A->ncols);
    EI_AddMessage ("", 4, "maxnegvalue=%.9f\n", maxnegvalue);
    EI_AddMessage ("", 4, "minnegvalue=%.9f\n", minnegvalue);
    EI_AddMessage ("", 4, "minposvalue=%.9f\n", minposvalue);
    EI_AddMessage ("", 4, "maxposvalue=%.9f\n", maxposvalue);
    EI_AddMessage ("", 4, "zero=%d\n", zero);
    EI_AddMessage ("", 4, "almost zero=%d\n", almostzero);
    EI_AddMessage ("", 4, "\n");
}
/*////////////////////////////////////////////////////////////////////////////*/
/* if v[i][j] is close to 0 set it to 0                                       */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Clean (MATRIX *A)
{
    register int n = A->nrows * A->ncols - 1;

    for (; n >= 0; n--) {
        if (STC_COMPARE_EQ (A->valueSq[n], 0.0)) {
            A->valueSq[n] = 0.0;
        }
    }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Copy (double *A, double *B, int nrows, int ncols)             */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the nrows x ncols matrix B to the nrows x ncols matrix A.         */
/*     i.e.    A = B.                                                         */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the destination matrix    */
/*                  A[nrows][ncols].                                          */
/*     double *B    Pointer to the first element of the source matrix         */
/*                  B[nrows][ncols].                                          */
/*     int    nrows The number of rows matrices A and B.                      */
/*     int    ncols The number of columns of the matrices A and B.            */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[N][M], B[N][M];                                               */
/*                                                                            */
/*     (your code to initialize the matrix B)                                 */
/*                                                                            */
/*     Matrix_Copy (&A[0][0], &B[0][0], N, M);                                */
/*     printf(" Matrix A is \n");                                             */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Copy (double *A, double *B, int nrows, int ncols)
{
    memcpy (A, B, sizeof (double) * nrows * ncols);
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  MATRIX * Matrix_Duplicate (MATRIX *A)                                     */
/*                                                                            */
/*  Description:                                                              */
/*     Make a duplicate of a matrix.                                          */
/*     i.e.    A = B.                                                         */
/*                                                                            */
/*  Arguments:                                                                */
/*     MATRIX *A    Pointer to the matrix to duplicate                        */
/*                                                                            */
/*  Return Values:                                                            */
/*     MATRIX *                                                               */
/*                                                                            */
/*  Example:                                                                  */
/*     MATRIX * A                                                             */
/*     MATRIX * B                                                             */
/*                                                                            */
/*     (your code to initialize the matrix B)                                 */
/*                                                                            */
/*     A = Matrix_Duplicate (B);                                              */
/*     printf(" Matrix A is \n");                                             */
/*////////////////////////////////////////////////////////////////////////////*/
MATRIX * Matrix_Duplicate (MATRIX * A)
{
    MATRIX * B;

    B = Matrix_Allocate (A->nrows, A->ncols);
    if (!B) {
         return NULL;
    }
    Matrix_Copy (B->valueSq, A->valueSq, A->nrows, A->ncols);
    return B;
}
/*////////////////////////////////////////////////////////////////////////////*/
/* check if 2 matrices are equal. */
/* returns 1 if they are equal, 0 otherwise. */
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_Equal (MATRIX *A, MATRIX *B)
{
    register int n  = A->nrows * A->ncols - 1;

    for (; n >= 0; n--) {
        if (!STC_COMPARE_EQ (A->valueSq[n], B->valueSq[n])) {
            return 0;
        }
    }
    return 1;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Fill (double *A, double x, int nrows, int ncols)              */
/*                                                                            */
/*  Description:                                                              */
/*     Set each element of the matrix A to the scalar x.                      */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the source matrix A.      */
/*     double x     Scalar which replaces the each element of A.              */
/*     int    nrows The number of rows matrix A.                              */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], x;                                                     */
/*                                                                            */
/*     (your code to initialize the matrix A and the scalar x)                */
/*                                                                            */
/*     Matrix_Fill (&A[0][0], x, M, N);                                       */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Fill (double *A, double x, int nrows, int ncols)
{
   *A = x;
   mymemcpy (A+1, A, sizeof (double) * (nrows * ncols - 1));
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Free (MATRIX *A)                                              */
/*                                                                            */
/*  Description:                                                              */
/*     Free a matrix.                                                         */
/*                                                                            */
/*  Arguments:                                                                */
/*     MATRIX *A    Pointer to the matrix to free                             */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     MATRIX * A                                                             */
/*                                                                            */
/*     (your code to initialize the matrix A)                                 */
/*                                                                            */
/*     Matrix_Free (A);                                                       */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Free (MATRIX * A)
{
    if (A != NULL) {
        STC_FreeMemory (A->value);
        STC_FreeMemory (A->valueSq);
        STC_FreeMemory (A);
    }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_GetSubmatrix (double *S, int mrows, int mcols,                */
/*                                   double *A, int ncols, int row, int col)  */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the mrows and mcols of the nrows x ncols matrix A starting with   */
/*     A[row][col] to the submatrix S.                                        */
/*     Note that S should be declared double S[mrows][mcols] in the calling   */
/*     routine.                                                               */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *S    Destination address of the submatrix.                     */
/*     int    mrows The number of rows of the matrix S.                       */
/*     int    mcols The number of columns of the matrix S.                    */
/*     double *A    Pointer to the first element of the matrix A[nrows][ncols]*/
/*     int    ncols The number of columns of the matrix A.                    */
/*     int    row   The row of A corresponding to the first row of S.         */
/*     int    col   The column of A corresponding to the first column of S.   */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define NB                                                             */
/*     #define MB                                                             */
/*     double A[M][N], B[MB][NB];                                             */
/*     int row, col;                                                          */
/*                                                                            */
/*     (your code to set the matrix A, the row number row and column number   */
/*      col)                                                                  */
/*                                                                            */
/*     if ( (row >= 0) && (col >= 0) && ((row + MB) < M) && ((col + NB) < N) )*/
/*        Get_Submatrix(&B[0][0], MB, NB, &A[0][0], N, row, col);             */
/*     printf("The submatrix B is \n"); ... }                                 */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_GetSubmatrix (double *S, int mrows, int mcols,
    double *A, int ncols, int row, int col)
{
   int NumberOfBytes = sizeof (double) * mcols;

   for (A += row * ncols + col; mrows > 0; A += ncols, S+= mcols, mrows--)
       memcpy (S, A, NumberOfBytes);
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Identity (double *A, int n)                                   */
/*                                                                            */
/*  Description:                                                              */
/*     Set the square n x n matrix A equal to the identity matrix, i.e.       */
/*     A[i][j] = 0 if i != j and A[i][i] = 1.                                 */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    n     The number of rows and columns of the matrix A.           */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     double A[N][N];                                                        */
/*                                                                            */
/*     Matrix_Identity (&A[0][0], N);                                         */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Identity (double *A, int n)
{
   *A = 1.0;
   if (n <= 1) return;
   *(A+1) = 0.0;
   mymemcpy (A+2, A+1, sizeof (double) * (n - 1));
   *(A+n+1) = 1.0;
   if (n == 2) return;
   mymemcpy (A+n+2, A+1, sizeof (double) * (n * (n - 1) - 2));
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix.
if the matrix is less than X rows it uses the LU method, otherwise it uses
the partition method.
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_Inverse (MATRIX *inv, MATRIX *A)
{
    return A->nrows < LU_LIMIT
        ? Matrix_InverseLU (inv, A)
        : Matrix_InverseByPartitionRecursive (inv, A);
}
/*////////////////////////////////////////////////////////////////////////////*/
/*inverse a matrix by partition.*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseByPartition (MATRIX * inv, MATRIX *A)
{
    MATRIX * P;
    MATRIX * Q;
    MATRIX * R;
    MATRIX * S;
    MATRIX * Pi;
    MATRIX * PiQ;
    MATRIX * PiQSRPiQIRPi;
    MATRIX * RPi;
    MATRIX * RPiQ;
    MATRIX * SRPiQ;
    MATRIX * SRPiQI;
    int P1;
    int P4;
    int rc;

    P1 = A->nrows / 2;
    P4 = A->nrows - P1;

    P = Matrix_Allocate (P1, P1); if (!P) return -1;
    Q = Matrix_Allocate (P1, P4); if (!Q) return -1;
    R = Matrix_Allocate (P4, P1); if (!R) return -1;
    S = Matrix_Allocate (P4, P4); if (!S) return -1;

    SRPiQI = S;

    Matrix_GetSubmatrix (P->valueSq, P->nrows, P->ncols,
        A->valueSq, A->ncols, 0, 0);
    Matrix_GetSubmatrix (Q->valueSq, Q->nrows, Q->ncols,
        A->valueSq, A->ncols, 0, P1);
    Matrix_GetSubmatrix (R->valueSq, R->nrows, R->ncols,
        A->valueSq, A->ncols, P1, 0);
    Matrix_GetSubmatrix (S->valueSq, S->nrows, S->ncols,
        A->valueSq, A->ncols, P1, P1);

    Pi = Matrix_Allocate (P->nrows, P->ncols); if (!Pi) return -1;
    rc = Matrix_InverseLU (Pi, P);
    if (rc != 0) return -1;
    PiQ = Matrix_Allocate (Pi->nrows, Q->ncols); if (!PiQ) return -1;
    Matrix_Multiply (PiQ->valueSq, Pi->valueSq, Pi->nrows, Pi->ncols,
        Q->valueSq, Q->ncols);
    RPi = Matrix_Allocate (R->nrows, Pi->ncols); if (!RPi) return -1;
    Matrix_Multiply (RPi->valueSq, R->valueSq, R->nrows, R->ncols,
        Pi->valueSq, Pi->ncols);
    RPiQ = Matrix_Allocate (R->nrows, Q->ncols); if (!RPiQ) return -1;
    Matrix_Multiply (RPiQ->valueSq, RPi->valueSq, RPi->nrows, RPi->ncols,
        Q->valueSq, Q->ncols);
    SRPiQ = Matrix_Allocate (S->nrows, Q->ncols); if (!SRPiQ) return -1;
    Matrix_Subtract (SRPiQ->valueSq, S->valueSq, RPiQ->valueSq,
        SRPiQ->nrows, SRPiQ->ncols);
    Matrix_Free (RPiQ);
    rc = Matrix_InverseLU (SRPiQI, SRPiQ);
    if (rc != 0) return -1;
    Matrix_Free (SRPiQ);

    Matrix_Multiply (Q->valueSq, PiQ->valueSq, PiQ->nrows, PiQ->ncols,
        SRPiQI->valueSq, SRPiQI->ncols);
    Matrix_Free (PiQ);
    Matrix_MultiplyScalar (Q->valueSq, -1.0, Q->nrows, Q->ncols);

    Matrix_Multiply (R->valueSq, SRPiQI->valueSq, SRPiQI->nrows, SRPiQI->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_MultiplyScalar (R->valueSq, -1.0, R->nrows, R->ncols);

    PiQSRPiQIRPi = Matrix_Allocate (S->nrows, S->ncols);
    if (!PiQSRPiQIRPi) return -1;
    Matrix_Multiply (PiQSRPiQIRPi->valueSq, Q->valueSq, Q->nrows, Q->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_Free (RPi);
    Matrix_Subtract (P->valueSq, Pi->valueSq, PiQSRPiQIRPi->valueSq,
        P->nrows, P->ncols);
    Matrix_Free (Pi);
    Matrix_Free (PiQSRPiQIRPi);

    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, P->valueSq,
        P->nrows, P->ncols, 0, 0);
    Matrix_Free (P);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Q->valueSq,
        Q->nrows, Q->ncols, 0, P1);
    Matrix_Free (Q);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, R->valueSq,
        R->nrows, R->ncols, P1, 0);
    Matrix_Free (R);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, S->valueSq,
        S->nrows, S->ncols, P1, P1);
    Matrix_Free (S);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by partition.
MH stand for Memory hungry.
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseByPartitionMH (MATRIX * inv, MATRIX *A)
{
    MATRIX * P;
    MATRIX * Q;
    MATRIX * R;
    MATRIX * S;
    MATRIX * Pc;
    MATRIX * Qc;
    MATRIX * Rc;
    MATRIX * Sc;
    MATRIX * Pi;
    MATRIX * PiQ;
    MATRIX * RPi;
    MATRIX * RPiQ;
    MATRIX * SRPiQ;
    MATRIX * SRPiQI;
    MATRIX * tmp;
    int P1;
    int P4;


    P1 = A->nrows / 2;
    P4 = A->nrows - P1;

    P = Matrix_Allocate (P1, P1);
    Q = Matrix_Allocate (P1, P4);
    R = Matrix_Allocate (P4, P1);
    S = Matrix_Allocate (P4, P4);

    Pi = Matrix_Allocate (P->nrows, P->ncols);
    PiQ = Matrix_Allocate (Pi->nrows, Q->ncols);
    RPi = Matrix_Allocate (R->nrows, Pi->ncols);
    RPiQ = Matrix_Allocate (R->nrows, Q->ncols);
    SRPiQ = Matrix_Allocate (S->nrows, Q->ncols);
    SRPiQI = Matrix_Allocate (S->nrows, Q->ncols);

    Pc = Matrix_Allocate (P1, P1);
    Qc = Matrix_Allocate (P1, P4);
    Rc = Matrix_Allocate (P4, P1);
    Sc = Matrix_Allocate (P4, P4);

    Matrix_GetSubmatrix (P->valueSq, P->nrows, P->ncols,
        A->valueSq, A->ncols, 0, 0);
    Matrix_GetSubmatrix (Q->valueSq, Q->nrows, Q->ncols,
        A->valueSq, A->ncols, 0, P1);
    Matrix_GetSubmatrix (R->valueSq, R->nrows, R->ncols,
        A->valueSq, A->ncols, P1, 0);
    Matrix_GetSubmatrix (S->valueSq, S->nrows, S->ncols,
        A->valueSq, A->ncols, P1, P1);

    Matrix_InverseLU (Pi, P);
    Matrix_Multiply (PiQ->valueSq, Pi->valueSq, Pi->nrows, Pi->ncols,
        Q->valueSq, Q->ncols);
    Matrix_Multiply (RPi->valueSq, R->valueSq, R->nrows, R->ncols,
        Pi->valueSq, Pi->ncols);
    Matrix_Multiply (RPiQ->valueSq, RPi->valueSq, RPi->nrows, RPi->ncols,
        Q->valueSq, Q->ncols);
    Matrix_Subtract (SRPiQ->valueSq, S->valueSq, RPiQ->valueSq,
        SRPiQ->nrows, SRPiQ->ncols);
    Matrix_InverseLU (SRPiQI, SRPiQ);

    Matrix_Multiply (Qc->valueSq, PiQ->valueSq, PiQ->nrows, PiQ->ncols,
        SRPiQI->valueSq, SRPiQI->ncols);
    Matrix_MultiplyScalar (Qc->valueSq, -1.0, Qc->nrows, Qc->ncols);

    Matrix_Multiply (Rc->valueSq, SRPiQI->valueSq, SRPiQI->nrows, SRPiQI->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_MultiplyScalar (Rc->valueSq, -1.0, Rc->nrows, Rc->ncols);

    Matrix_Copy (Sc->valueSq, SRPiQI->valueSq, SRPiQI->nrows, SRPiQI->ncols);

    tmp = Matrix_Allocate (S->nrows, S->ncols);
    Matrix_Multiply (tmp->valueSq, Qc->valueSq, Qc->nrows, Qc->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_Subtract (Pc->valueSq, Pi->valueSq, tmp->valueSq,
        Pc->nrows, Pc->ncols);
    Matrix_Free (tmp);

    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Pc->valueSq,
        Pc->nrows, Pc->ncols, 0, 0);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Qc->valueSq,
        Qc->nrows, Qc->ncols, 0, P1);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Rc->valueSq,
        Rc->nrows, Rc->ncols, P1, 0);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Sc->valueSq,
        Sc->nrows, Sc->ncols, P1, P1);

    Matrix_Free (P);
    Matrix_Free (Q);
    Matrix_Free (R);
    Matrix_Free (S);

    Matrix_Free (Pi);
    Matrix_Free (PiQ);
    Matrix_Free (RPi);
    Matrix_Free (RPiQ);
    Matrix_Free (SRPiQ);
    Matrix_Free (SRPiQI);

    Matrix_Free (Pc);
    Matrix_Free (Qc);
    Matrix_Free (Rc);
    Matrix_Free (Sc);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by partition.
uses Matrix_Inverse() to inverse the submatrix
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseByPartitionRecursive (MATRIX * inv, MATRIX *A)
{
    MATRIX * P;
    MATRIX * Q;
    MATRIX * R;
    MATRIX * S;
    MATRIX * Pc;
    MATRIX * Qc;
    MATRIX * Rc;
    MATRIX * Sc;
    MATRIX * Pi;
    MATRIX * PiQ;
    MATRIX * RPi;
    MATRIX * RPiQ;
    MATRIX * SRPiQ;
    MATRIX * SRPiQI;
    MATRIX * tmp;
    int P1;
    int P4;


    P1 = A->nrows / 2;
    P4 = A->nrows - P1;

    P = Matrix_Allocate (P1, P1);
    Q = Matrix_Allocate (P1, P4);
    R = Matrix_Allocate (P4, P1);
    S = Matrix_Allocate (P4, P4);

    Pi = Matrix_Allocate (P->nrows, P->ncols);
    PiQ = Matrix_Allocate (Pi->nrows, Q->ncols);
    RPi = Matrix_Allocate (R->nrows, Pi->ncols);
    RPiQ = Matrix_Allocate (R->nrows, Q->ncols);
    SRPiQ = Matrix_Allocate (S->nrows, Q->ncols);
    SRPiQI = Matrix_Allocate (S->nrows, Q->ncols);

    Pc = Matrix_Allocate (P1, P1);
    Qc = Matrix_Allocate (P1, P4);
    Rc = Matrix_Allocate (P4, P1);
    Sc = Matrix_Allocate (P4, P4);

    Matrix_GetSubmatrix (P->valueSq, P->nrows, P->ncols,
        A->valueSq, A->ncols, 0, 0);
    Matrix_GetSubmatrix (Q->valueSq, Q->nrows, Q->ncols,
        A->valueSq, A->ncols, 0, P1);
    Matrix_GetSubmatrix (R->valueSq, R->nrows, R->ncols,
        A->valueSq, A->ncols, P1, 0);
    Matrix_GetSubmatrix (S->valueSq, S->nrows, S->ncols,
        A->valueSq, A->ncols, P1, P1);

    Matrix_Inverse (Pi, P);
    Matrix_Multiply (PiQ->valueSq, Pi->valueSq, Pi->nrows, Pi->ncols,
        Q->valueSq, Q->ncols);
    Matrix_Multiply (RPi->valueSq, R->valueSq, R->nrows, R->ncols,
        Pi->valueSq, Pi->ncols);
    Matrix_Multiply (RPiQ->valueSq, RPi->valueSq, RPi->nrows, RPi->ncols,
        Q->valueSq, Q->ncols);
    Matrix_Subtract (SRPiQ->valueSq, S->valueSq, RPiQ->valueSq,
        SRPiQ->nrows, SRPiQ->ncols);
    Matrix_Inverse (SRPiQI, SRPiQ);

    Matrix_Multiply (Qc->valueSq, PiQ->valueSq, PiQ->nrows, PiQ->ncols,
        SRPiQI->valueSq, SRPiQI->ncols);
    Matrix_MultiplyScalar (Qc->valueSq, -1.0, Qc->nrows, Qc->ncols);

    Matrix_Multiply (Rc->valueSq, SRPiQI->valueSq, SRPiQI->nrows, SRPiQI->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_MultiplyScalar (Rc->valueSq, -1.0, Rc->nrows, Rc->ncols);

    Matrix_Copy (Sc->valueSq, SRPiQI->valueSq, SRPiQI->nrows, SRPiQI->ncols);

    tmp = Matrix_Allocate (S->nrows, S->ncols);
    Matrix_Multiply (tmp->valueSq, Qc->valueSq, Qc->nrows, Qc->ncols,
        RPi->valueSq, RPi->ncols);
    Matrix_Subtract (Pc->valueSq, Pi->valueSq, tmp->valueSq,
        Pc->nrows, Pc->ncols);
    Matrix_Free (tmp);

    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Pc->valueSq,
        Pc->nrows, Pc->ncols, 0, 0);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Qc->valueSq,
        Qc->nrows, Qc->ncols, 0, P1);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Rc->valueSq,
        Rc->nrows, Rc->ncols, P1, 0);
    Matrix_SetSubmatrix (inv->valueSq, inv->ncols, Sc->valueSq,
        Sc->nrows, Sc->ncols, P1, P1);

    Matrix_Free (P);
    Matrix_Free (Q);
    Matrix_Free (R);
    Matrix_Free (S);

    Matrix_Free (Pi);
    Matrix_Free (PiQ);
    Matrix_Free (RPi);
    Matrix_Free (RPiQ);
    Matrix_Free (SRPiQ);
    Matrix_Free (SRPiQI);

    Matrix_Free (Pc);
    Matrix_Free (Qc);
    Matrix_Free (Rc);
    Matrix_Free (Sc);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by iterative method.
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseIterative (MATRIX * inv, MATRIX *A)
{
    double norm;
    MATRIX * B;
    MATRIX * M0;
    MATRIX * M1;
    MATRIX * tmp;
    int k;

    B = Matrix_Allocate (A->nrows, A->ncols); if (!B) return -1;
    M0 = Matrix_Allocate (A->nrows, A->ncols); if (!M0) return -1;
    M1 = Matrix_Allocate (A->nrows, A->ncols); if (!M1) return -1;
    tmp = Matrix_Allocate (A->nrows, A->ncols); if (!tmp) return -1;

    norm = GetScale (A->nrows, 1.4, A->value);
    Snorm (A->nrows, A->value, B->value, norm);
    Matrix_Copy (M0->valueSq, B->valueSq, A->nrows, A->ncols);
    Matrix_TransposeSquare (M0->valueSq, A->nrows);
    for (k = 0; k < 20; k++){
        Matrix_Multiply (tmp->valueSq, M0->valueSq, A->nrows, A->ncols,
            B->valueSq, B->ncols);
        Matrix_Multiply (M1->valueSq, tmp->valueSq, tmp->nrows, tmp->ncols,
            M0->valueSq, M0->ncols);
        Matrix_MultiplyScalar (M0->valueSq, 2.0, M0->nrows, M0->ncols);
        Matrix_Subtract (M1->valueSq, M0->valueSq, M1->valueSq, M0->nrows,
            M0->ncols);
        Matrix_Copy (M0->valueSq, M1->valueSq, M0->nrows, M0->ncols);
    }
    Matrix_Copy (inv->valueSq, M0->valueSq, M0->nrows, M0->ncols);
    Matrix_MultiplyScalar (inv->valueSq, 1.0 / norm, inv->nrows, inv->ncols);

    Matrix_Free (B);
    Matrix_Free (M0);
    Matrix_Free (M1);
    Matrix_Free (tmp);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by LU method.
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseLU (MATRIX *inv, MATRIX *A)
{
    MATRIX * NR_A; /* unit-offset double matrix */
    int i, j;
    int *indx;
    double d;
    double *col;
    int rc = 0;

    NR_A = Matrix_Allocate (A->nrows+1, A->ncols+1);
    indx = STC_AllocateMemory (sizeof *indx * (A->nrows+1));
    col = STC_AllocateMemory (sizeof *col * (A->nrows+1));

    if (!NR_A || !indx || !col) {
        rc = -1;
        goto TERMINATE;
    }

    /* copy the zero-offset matrix to the one-offset matrix */
    CopyMatrixTo1OffsetMatrix (NR_A, A);

    rc = dludcmp (NR_A->value, NR_A->ncols-1, indx, &d);
    if (rc != 0) {
        rc = -1;
        goto TERMINATE;
    }

    for (j = 1; j <= NR_A->ncols-1; j++) {
        for (i = 1; i <= NR_A->ncols-1; i++) col[i] = 0.0;
        col[j] = 1.0;
        dlubksb (NR_A->value, NR_A->ncols-1, indx, col);
        for (i = 1; i <= NR_A->ncols-1; i++) inv->value[i-1][j-1] = col[i];
    }

TERMINATE:
    if (NR_A != NULL) {
        Matrix_Free (NR_A);
    }
    if (indx != NULL) {
        STC_FreeMemory (indx);
    }
    if (col != NULL) {
        STC_FreeMemory (col);
    }

    return rc;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by simul method. see fortran prototype!
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseSimul (MATRIX *inv, MATRIX *A)
{
    double * IRow;
    double * JCol;
    double * Y;
    double Pivot;
    int N = A->nrows;
    int i, j, k;
    int iscan, jscan;
    double AIJCK;
    int IRowK, JColK, JColI, JColJ;
    int IRowI, IRowJ;

    IRow = STC_AllocateMemory (sizeof *IRow * A->nrows); if (!IRow) return -1;
    JCol = STC_AllocateMemory (sizeof *JCol * A->nrows); if (!JCol) return -1;
    Y = STC_AllocateMemory (sizeof *Y * A->nrows); if (!Y) return -1;

    Matrix_Copy (inv->valueSq, A->valueSq, A->nrows, A->ncols);

    for (i = 0; i < N; i++) {
        IRow[i] = 0.0;
        JCol[i] = 0.0;
    }

    for (k = 0; k < N; k++) {
        Pivot = 0.0;
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                for (iscan = 0; iscan < k; iscan++) {
                    for (jscan = 0; jscan < k; jscan++) {
                        if (i == IRow[iscan]) goto FINI1;
                        if (j == JCol[jscan]) goto FINI1;
                    }
                }

                if (fabs (inv->value[i][j]) <= fabs (Pivot)) goto FINI1;

                Pivot = inv->value[i][j];
                IRow[k] = i;
                JCol[k] = j;

FINI1:          ;
            }
        }

        if (STC_COMPARE_EQ (Pivot, 0.0)) {
            return -1;
        }

        IRowK = (int)IRow[k];
        JColK = (int)JCol[k];

        for (j = 0; j < N; j++) {
            inv->value[IRowK][j] = inv->value[IRowK][j] / Pivot;
        }
        inv->value[IRowK][JColK] = 1.0 / Pivot;
        for (i = 0; i < N; i++) {
            AIJCK = inv->value[i][JColK];
            if (i != IRowK) {
                inv->value[i][JColK] = -AIJCK / Pivot;
                for (j = 0; j < N; j++) {
                    if (j != JColK)
                        inv->value[i][j] -= AIJCK * inv->value[IRowK][j];
                }
            }
        }
    }
    for (j = 0; j < N; j++) {
        for (i = 0; i < N; i++) {
            IRowI = (int)IRow[i];
            JColI = (int)JCol[i];
            Y[JColI] = inv->value[IRowI][j];
        }
        for (i = 0; i < N; i++) {
            inv->value[i][j] = Y[i];
        }
    }
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            IRowJ = (int)IRow[j];
            JColJ = (int)JCol[j];
            Y[IRowJ] = inv->value[i][JColJ];
        }
        for (j = 0; j < N; j++) {
            inv->value[i][j] = Y[j];
        }
    }

    STC_FreeMemory (IRow);
    STC_FreeMemory (JCol);
    STC_FreeMemory (Y);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*
inverse a matrix by svd method.
*/
/*////////////////////////////////////////////////////////////////////////////*/
int Matrix_InverseSvd (MATRIX *inv, MATRIX *A)
{
    int rc;
    MATRIX * U;
    MATRIX * V;
    double * D;
    double * dummy;

    U = Matrix_Allocate (A->nrows, A->ncols); if (!U) return -1;
    V = Matrix_Allocate (A->ncols, A->ncols); if (!V) return -1;
    D = STC_AllocateMemory (sizeof *D * A->nrows); if (!D) return -1;
    dummy = STC_AllocateMemory (sizeof *dummy * A->nrows); if (!dummy)return -1;

    rc = Singular_Value_Decomposition (A->valueSq, A->nrows, A->ncols,
        U->valueSq, D, V->valueSq, dummy);

    STC_FreeMemory (dummy);

    if (rc < 0) {
        EI_AddMessage ("Singular Value Decomposition", EIE_ERROR,
            M30001); /* Failed to converge */
        Matrix_Free (U);
        Matrix_Free (V);
        STC_FreeMemory (D);
        return -1;
    }

    Singular_Value_Decomposition_Inverse (U->valueSq, D, V->valueSq, 0.0,
        U->nrows, U->ncols, inv->valueSq);

    Matrix_Free (U);
    Matrix_Free (V);
    STC_FreeMemory (D);

    return 0;
}
/*********************************************************************
Decompose
A = U w Vt
pour donner
A inverse = V 1/w Ut
*********************************************************************/
int Matrix_InverseSvdNR (
    MATRIX * AInverse,
    MATRIX * A)
{
    MATRIX * U;
    MATRIX * w;
    MATRIX * V;
    MATRIX * Vw;
    int i;
    int rc;
    MATRIX * UNR;
    MATRIX * VNR;
    double * wNR;
    double wmax;
    double wmin;

    UNR = Matrix2NR (A);
    wNR = STC_AllocateMemory ((A->nrows+1) * sizeof *wNR);
    VNR = Matrix_Allocate (A->nrows+1, A->nrows+1);

    if (DEBUG) MATRIX_PRINT (A, 10);
    if (DEBUG) MATRIX_PRINT (UNR, 10);
    /* Decompose U => U, w, V. */
    rc = dsvdcmp (UNR->value, UNR->nrows-1, UNR->ncols-1, wNR, VNR->value);
    if (rc == 0) {
        EI_AddMessage ("dsvdcmp", EIE_ERROR, "No convergence in dsvdcmp\n");
        return 1;
    }
    else if (rc == -1) {
        EI_AddMessage ("dsvdcmp", EIE_ERROR, "Failure at dsvdcmp\n");
        return 2;
    }
    if (DEBUG) MATRIX_PRINT (UNR, 10);

    if (DEBUG) PrintVec1 (wNR, A->nrows, "wNR");
    if (DEBUG) MATRIX_PRINT (VNR, 10);

    /* Edit the singular values, w. */
    wmax = 0.0;
    for (i = 1; i <= A->nrows; i++)
        if (wNR[i] > wmax)
            wmax = wNR[i];
    //wmin = wmax * 10e-6; /*TOLERANCE;*/
    wmin = wmax * DBL_EPSILON * A->nrows;
    for (i = 1; i <= A->nrows; i++)
        if (wNR[i] <= wmin)
            wNR[i] = 0.0;
        else
            wNR[i] = 1.0 / wNR[i]; /* set w to 1/w */
    if (DEBUG) PrintVec1 (wNR, A->nrows, "wNR");

    U = NR2Matrix (UNR);
    if (DEBUG) MATRIX_PRINT (U, 10);
    w = Matrix_Allocate (A->nrows, A->ncols);
    Matrix_SetDiagonal (w->valueSq, wNR+1, A->nrows, A->ncols);
    if (DEBUG) MATRIX_PRINT (w, 10);
    V = NR2Matrix (VNR);
    if (DEBUG) MATRIX_PRINT (V, 10);

    /* I = V * w * U' */
    Vw = Matrix_Allocate (A->nrows, A->ncols);
    Matrix_MultiplyMatrixWithDiagonal (Vw, V, w);
    if (DEBUG) MATRIX_PRINT (Vw, 10);

    Matrix_MultiplyMatrixWithTranspose (AInverse->valueSq,
        Vw->valueSq, Vw->nrows, Vw->ncols,
        U->valueSq, U->nrows);

    Matrix_Free (UNR);
    Matrix_Free (VNR);
    STC_FreeMemory (wNR);
    Matrix_Free (U);
    Matrix_Free (w);
    Matrix_Free (V);
    Matrix_Free (Vw);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_JoinByColumn(double *C, double *A, int nrows, int ncols,      */
/*                                                    double *B, int mrows)   */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the nrows x ncols matrix A into the (nrows + mrows) x ncols       */
/*     matrix C and then copy the mrows x ncols matrix B to starting at       */
/*     the location C[nrows][0], i.e. C' = [A':B']', where ' denotes the      */
/*     transpose.                                                             */
/*     The matrix C should be declared as double C[nrows + mrows][ncols] in   */
/*     the calling routine.                                                   */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrix A.                           */
/*     int    ncols The number of columns of the matrices A and B.            */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    mrows The number of rows of the matrix B.                       */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define MB                                                             */
/*     double A[M][N], B[MB][N], C[M+MB][N];                                  */
/*     int row, col;                                                          */
/*                                                                            */
/*     (your code to set the matrices A and B)                                */
/*                                                                            */
/*     Join_Matrices_by_Column(&C[0][0], &A[0][0], M, N, &B[0][0], MB);       */
/*     printf("The matrix C is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_JoinByColumn (double *C, double *A, int nrows, int ncols,
    double *B, int mrows)
{
   memcpy (C, A, sizeof (double) * nrows * ncols);
   memcpy (C + (nrows * ncols), B, sizeof(double) * mrows * ncols);
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_JoinByRow(double *C, double *A, int nrows, int ncols,         */
/*                                                     double *B, int mcols)  */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the nrows x ncols matrix A into the nrows x (ncols + mcols)       */
/*     matrix C and then copy the nrows x mcols matrix B to starting at       */
/*     the location C[0][ncols], i.e. C = [A:B].                              */
/*     The matrix C should be declared as double C[nrows][ncols + mcols] in   */
/*     the calling routine.                                                   */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrices A and B.                   */
/*     int    ncols The number of columns of the matrix A.                    */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    mcols The number of columns of the matrix B.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define NB                                                             */
/*     double A[M][N], B[M][NB], C[M][N+NB];                                  */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Join_Matrices_by_Row(&C[0][0], &A[0][0], M, N, &B[0][0], NB);          */
/*     printf("The matrix C is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_JoinByRow (double *C, double *A, int nrows, int ncols,
    double *B, int mcols)
{
   for (; nrows > 0; nrows--) {
      memcpy (C, A, sizeof (double) * ncols);
      C += ncols;
      A += ncols;
      memcpy (C, B, sizeof (double) * mcols);
      C += mcols;
      B += mcols;
   }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Multiply (double *C, double *A, int nrows, int ncols,         */
/*                                                    double *B, int mcols)   */
/*                                                                            */
/*  Description:                                                              */
/*     Post multiply the nrows x ncols matrix A by the ncols x mcols matrix   */
/*     B to form the nrows x mcols matrix C, i.e. C = A B.                    */
/*     The matrix C should be declared as double C[nrows][mcols] in the       */
/*     calling routine.  The memory allocated to C should not include any     */
/*     memory allocated to A or B.                                            */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of the matrices A and C.               */
/*     int    ncols The number of columns of the matrices A and the           */
/*                   number of rows of the matrix B.                          */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    mcols The number of columns of the matrices B and C.            */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define NB                                                             */
/*     double A[M][N], B[N][NB], C[M][NB];                                    */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Matrix_Multiply (&C[0][0], &A[0][0], M, N, &B[0][0], NB);              */
/*     printf("The matrix C is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Multiply (double *C, double *A, int nrows, int ncols,
    double *B, int mcols)
{
   double *pA = A;
   double *pB;
   double *p_B;
   int i, j, k;

   for (i = 0; i < nrows; A += ncols, i++)
      for (p_B = B, j = 0; j < mcols; C++, p_B++, j++) {
         pB = p_B;
         pA = A;
         *C = 0.0;
         for (k = 0; k < ncols; pA++, pB += mcols, k++)
            *C += *pA * *pB;
      }
}
/*********************************************************************
Matrix_MultiplyDiagonalWithMatrix: Multiplies the he diagonal matrix D with matrix B to
obtain DB:
DB = D x B
Note: D must be diagonal, and we must have D->ncols = B->nrows, where
nrows and ncols are the numbers of rows and columns of matrices.
Space for DB, D and B must have been allocated in the calling function.
*********************************************************************/
void Matrix_MultiplyDiagonalWithMatrix(MATRIX * DB, MATRIX * D, MATRIX * B)
{
    register int row, col;
    for (row = 0; row < D->nrows; row++){
        for (col = 0; col < B->ncols; col++){
            DB->value[row][col] = D->value[row][row] * B->value[row][col];
        }
    }
}
/*********************************************************************
Matrix_MultiplyMatrixWithDiagonal: Multiplies the matrix A with the diagonal
matrix D to obtain AD:
AD = A x D
Note: D must be diagonal, and we must have A->ncols = D->nrows, where
nrows and ncols are the numbers of rows and columns of matrices.
Space for AD, A and D must have been allocated in the calling function.
*********************************************************************/
void Matrix_MultiplyMatrixWithDiagonal(MATRIX * AD, MATRIX * A, MATRIX * D)
{
    register int row, col;
    for (row = 0; row < A->nrows; row++){
        for (col = 0; col < D->ncols; col++){
            AD->value[row][col] = A->value[row][col] * D->value[col][col];
        }
    }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_MultiplyScalar (double *A, double x, int nrows, int ncols)    */
/*                                                                            */
/*  Description:                                                              */
/*     Multiply each element of the matrix A by the scalar x.                 */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     double x     Scalar to multipy each element of the matrix A.           */
/*     int    nrows The number of rows of matrix A.                           */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], x;                                                     */
/*                                                                            */
/*     (your code to initialize the matrix A and scalar x)                    */
/*                                                                            */
/*     Matrix_MultiplyScalar (&A[0][0], x, M, N);                             */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_MultiplyScalar (double *A, double x, int nrows, int ncols)
{
   register int n = nrows * ncols - 1;

   for (; n >= 0; n--) A[n] *= x;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_MultiplyMatrixWithTranspose(double *C, double *A, int nrows,  */
/*                                         int ncols, double *B, int mrows)   */
/*                                                                            */
/*  Description:                                                              */
/*     Post multiply the nrows x ncols matrix A by the transpose of the       */
/*     mrows x ncols matrix B to form the nrows x mrows matrix C,             */
/*     i.e. C = A B', where ' denotes the transpose.                          */
/*     The matrix C should be declared as double C[nrows][mrows] in the       */
/*     calling routine.  The memory allocated to C should not include any     */
/*     memory allocated to A or B.                                            */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrices A and C.                   */
/*     int    ncols The number of columns of the matrices A and B.            */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    mrows The number of rows of the matrix B and columns of the     */
/*                  matrix C.                                                 */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define MB                                                             */
/*     double A[M][N], B[MB][N], C[M][MB];                                    */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Multiply_Matrix_with_Transpose(&C[0][0], &A[0][0], M, N, &B[0][0], NB);*/
/*     printf("The matrix C = AB' is \n"); ...                                */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_MultiplyMatrixWithTranspose (double *C, double *A, int nrows,
    int ncols, double *B, int mrows)
{
   int i, j, k;
   double *pA = A;
   double *pB;

   for (i = 0; i < nrows; A += ncols, i++)
      for (pB = B, j = 0; j < mrows; C++, j++) {
         pA = A;
         *C = 0.0;
         for (k = 0; k < ncols; pA++, pB ++, k++) *C += *pA * *pB;
      }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_MultiplySelfTranspose(double *C, double *A, int nrows,        */
/*      int ncols)                                                            */
/*                                                                            */
/*  Description:                                                              */
/*     Post multiply an nrows x ncols matrix A by its transpose.   The result */
/*     is an  nrows x nrows square symmetric matrix C, i.e. C = A A', where ' */
/*     denotes the transpose.                                                 */
/*     The matrix C should be declared as double C[nrows][nrows] in the       */
/*     calling routine.  The memory allocated to C should not include any     */
/*     memory allocated to A.                                                 */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrix A.                           */
/*     int    ncols The number of columns of the matrices A.                  */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], C[M][M];                                               */
/*                                                                            */
/*     (your code to initialize the matrix A)                                 */
/*                                                                            */
/*     Multiply_Self_Transpose(&C[0][0], &A[0][0], M, N);                     */
/*     printf("The matrix C = AA ' is \n"); ...                               */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_MultiplySelfTranspose (double *C, double *A, int nrows, int ncols)
{
   int i, j, k;
   double *pA;
   double *p_A = A;
   double *pB;
   double *pCdiag = C;
   double *pC = C;
   double *pCt;

   for (i = 0; i < nrows; pCdiag += nrows + 1, p_A = pA, i++) {
      pC = pCdiag;
      pCt = pCdiag;
      pB = p_A;
      for (j = i; j < nrows; pC++, pCt += nrows, j++) {
         pA = p_A;
         *pC = 0.0;
         for (k = 0; k < ncols; k++) *pC += *(pA++) * *(pB++);
         *pCt = *pC;
      }
   }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_MultiplyTransposeWithMatrix(double *C, double *A, int nrows,  */
/*                                         int ncols, double *B, int mcols)   */
/*                                                                            */
/*  Description:                                                              */
/*     Post multiply the transpose of the nrows x ncols matrix A by the       */
/*     nrows x mcols matrix B to form the ncols x mcols matrix C,             */
/*     i.e. C = A' B, where ' denotes the transpose.                          */
/*     The matrix C should be declared as double C[ncols][mcols] in the       */
/*     calling routine.  The memory allocated to C should not include any     */
/*     memory allocated to A or B.                                            */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrix A and the number of rows     */
/*                  of the matrix B.                                          */
/*     int    ncols The number of columns of the matrices A and the number    */
/*                   number of rows of the matrix C.                          */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    mcols The number of columns of the matrices B and C.            */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define NB                                                             */
/*     double A[M][N], B[M][NB], C[N][NB];                                    */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Multiply_Transpose_with_Matrix(&C[0][0], &A[0][0], M, N, &B[0][0], NB);*/
/*     printf("The matrix C = A'B is \n"); ...                                */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_MultiplyTransposeWithMatrix (double *C, double *A, int nrows,
    int ncols, double *B, int mcols)
{
   int i, j, k;
   double *pA = A;
   double *pB;
   double *p_B;

   for (i = 0; i < ncols; A++, i++)
      for (p_B = B, j = 0; j < mcols; C++, p_B++, j++) {
         pB = p_B;
         pA = A;
         *C = 0.0;
         for (k = 0; k < nrows; pA += ncols, pB += mcols, k++) *C += *pA * *pB;
      }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_PrintWithPrecision (MATRIX *A, int NumberOfDecimals)          */
/*                                                                            */
/*  Description:                                                              */
/*     Print a matrix.                                                        */
/*                                                                            */
/*  Arguments:                                                                */
/*     MATRIX *A    Pointer to the matrix to print                            */
/*     int NumberOfDecimals number of decimals to print                       */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     MATRIX * A                                                             */
/*                                                                            */
/*     (your code to initialize the matrix A)                                 */
/*                                                                            */
/*     Matrix_PrintWithPrecision (A, 8);                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_PrintWithPrecision (MATRIX *A, int Precision)
{
    register int i, j;
    char s1[1001];/* bad rene... but but it's only for debugging! */
	char s2[101];

    if (Precision < 0) Precision = 7;

    for (i = 0; i < A->nrows; i++) {
		s1[0] = '\0';
        for (j = 0; j < A->ncols; j++) {
            sprintf (s2, "%.*g ", Precision, A->value[i][j]);
            strcat (s1, s2);
        }
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%s", s1);
    }
    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "");
    //for (i = 0; i < A->nrows; i++) {
    //    for (j = 0; j < A->ncols; j++) {
    //        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%.*g ", Precision, A->value[i][j]);
    //    }
    //    EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "\n");
    //}
    //EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "\n");
}
/*////////////////////////////////////////////////////////////////////////////*/
/* fill matrix with random data ranging from min to max */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Random (MATRIX * A, double min, double max)
{
    register int n = A->nrows * A->ncols - 1;

    for (; n >= 0; n--) {
        A->valueSq[n] = min + UTIL_Rand () * (max-min);
    }
}
/*********************************************************************
scale each column to make it lie in the range [-1, 1]
*********************************************************************/
void Matrix_ScaleToOrderUnity (
    MATRIX * A)
{
/*    double * Mc;*/
    double AbsMax;
    int c;

/*    Mc = STC_AllocateMemory (A->nrows * sizeof *Mc);*/
    for  (c = 0; c < A->ncols; c++) {
        AbsMax = FindAbsMax (A, c);
        ScaleColumn (A, c, AbsMax);
    }
/*    return Mc;*/
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_SetDiagonal (double *A, double v[], int nrows, int ncols)     */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the vector v to the diagonal A[i][i], where 0 <= i <              */
/*     min (nrows, ncols).                                                    */
/*     Note that v should be declared "double v[N]", N >= min (nrows, ncols)  */
/*     in the calling routine.                                                */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the source matrix A.      */
/*     double v[]   Source of the new diagonal for the matrix A.              */
/*     int    nrows The number of rows matrix A.                              */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], v[N];                                                  */
/*                                                                            */
/*     (your code to initialize the matrix A and the vector v)                */
/*                                                                            */
/*     Set_Diagonal(&A[0][0], v, M, N);                                       */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_SetDiagonal (double *A, double v[], int nrows, int ncols)
{
   int n;

   if (nrows < ncols) n = nrows; else n = ncols;

   for (; n > 0 ; A += (ncols + 1), n--)  *A = *v++;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_SetSubmatrix(double *A, int ncols, double *S,                 */
/*      int mrows, int mcols, int row, int col)                               */
/*                                                                            */
/*  Description:                                                              */
/*     Copy the mrows x mcols submatrix S into the nrows x ncols matrix A     */
/*     starting at the location A[row][col].                                  */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A[n][n].       */
/*     int    ncols The number of columns of the matrix A.                    */
/*     double *S    Destination address of the submatrix.                     */
/*     int    mrows The number of rows matrix S.                              */
/*     int    mcols The number of columns of the matrix S.                    */
/*     int    row   The row of A corresponding to the first row of S.         */
/*     int    col   The column of A corresponding to the first column of S.   */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     #define NB                                                             */
/*     #define MB                                                             */
/*     double A[M][N], B[MB][NB];                                             */
/*     int row, col;                                                          */
/*                                                                            */
/*     (your code to initialize the matrix A, submatrix B, row number row,    */
/*      and column number col )                                               */
/*                                                                            */
/*     if ( (row > 0) && ( row + MB < M ) && ( col > 0 ) && (col + NB < N)    */
/*        Set_Submatrix(&A[0][0], N, &B[0][0], MB, NB, row, col);             */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_SetSubmatrix (double *A, int ncols, double *S, int mrows, int mcols,
    int row, int col)
{
    int i, j;

    for (A += row * ncols + col, i = 0; i < mrows; A += ncols, i++)
        for (j = 0; j < mcols; j++) *(A + j) = *S++;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Subtract (double *C, double *A, double *B, int nrows,         */
/*                                                               int ncols)   */
/*                                                                            */
/*  Description:                                                              */
/*     Subtract the matrix B from the matrix A to form the matrix C, i.e.     */
/*     C = A - B.                                                             */
/*     All matrices should be declared as " double X[nrows][ncols] " in the   */
/*     calling routine, where X = A, B, C.                                    */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *C    Pointer to the first element of the matrix C.             */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     double *B    Pointer to the first element of the matrix B.             */
/*     int    nrows The number of rows of matrices A and B.                   */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], B[M][N], C[M][N];                                      */
/*                                                                            */
/*     (your code to initialize the matrices A and B)                         */
/*                                                                            */
/*     Matrix_Subtract (&C[0][0], &A[0][0], &B[0][0], M, N);                  */
/*     printf("The matrix C = A - B  is \n"); ...                             */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Subtract (double *C, double *A, double *B, int nrows, int ncols)
{
   register int n = nrows * ncols - 1;

   for (; n >= 0; n--) C[n] = A[n] - B[n];
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Transpose (double *At, double *A, int nrows, int ncols)       */
/*                                                                            */
/*  Description:                                                              */
/*     Take the transpose of A and store in At, i.e. At = A'.                 */
/*     The matrix At should be declared as double At[ncols][nrows] in the     */
/*     calling routine, and the matrix A declared as double A[nrows[ncols].   */
/*     In general, At and A should be disjoint i.e. their memory locations    */
/*     should be distinct.                                                    */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *At   Pointer to the first element of the matrix At.            */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of matrix A and number of columns of   */
/*                  the matrix At.                                            */
/*     int    ncols The number of columns of the matrix A and the number of   */
/*                  rows of the matrix At.                                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[M][N], At[N][M];                                              */
/*                                                                            */
/*     (your code to initialize the matrix A)                                 */
/*                                                                            */
/*     Matrix_Transpose (&At[0][0], &A[0][0], M, N);                          */
/*     printf("The transpose of A is the matrix At \n"); ...                  */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Transpose (double *At, double *A, int nrows, int ncols)
{
   double *pA;
   double *pAt;
   int i, j;

   for (i = 0; i < nrows; At += 1, A += ncols, i++) {
      pAt = At;
      pA = A;
      for (j = 0; j < ncols; pAt += nrows, j++) *pAt = pA[j];
   }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_TransposeSquare  (double *A, int n)                           */
/*                                                                            */
/*  Description:                                                              */
/*     Take the transpose of A and store in place.                            */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    n     The number of rows and columns of the matrix A.           */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     double A[N][N];                                                        */
/*                                                                            */
/*     (your code to initialize the matrix A)                                 */
/*                                                                            */
/*     Matrix_TransposeSquare (&A[0][0], N);                                  */
/*     printf("The transpose of A is \n"); ...                                */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_TransposeSquare (double *A, int n)
{
   double *pA, *pAt;
   double temp;
   int i, j;

   for (i = 0; i < n; A += n + 1, i++) {
      pA = A + 1;
      pAt = A + n;
      for (j = i+1; j < n; pA++, pAt += n, j++) {
         temp = *pAt;
         *pAt = *pA;
         *pA = temp;
      }
   }
}
/*////////////////////////////////////////////////////////////////////////////*/
/*  void Matrix_Zero (double *A, int nrows, int ncols)                        */
/*                                                                            */
/*  Description:                                                              */
/*     Set the nrows x ncols matrix A equal to the zero matrix, i.e.          */
/*     A[i][j] = 0 for all i, j.                                              */
/*                                                                            */
/*  Arguments:                                                                */
/*     double *A    Pointer to the first element of the matrix A.             */
/*     int    nrows The number of rows of the matrix A.                       */
/*     int    ncols The number of columns of the matrix A.                    */
/*                                                                            */
/*  Return Values:                                                            */
/*     void                                                                   */
/*                                                                            */
/*  Example:                                                                  */
/*     #define N                                                              */
/*     #define M                                                              */
/*     double A[N][M];                                                        */
/*                                                                            */
/*     Matrix_Zero (&A[0][0], N, M);                                          */
/*     printf("The matrix A is \n"); ...                                      */
/*////////////////////////////////////////////////////////////////////////////*/
void Matrix_Zero (double *A, int nrows, int ncols)
{
   *A = 0.0;
   mymemcpy (A+1, A, sizeof (double) * (nrows * ncols - 1));
}






/*////////////////////////////////////////////////////////////////////////////*/
/*
Implement a version of memcpy() that copies memory from highest memory
address to the lowest. Implementing it that way propagate
the value at the highest memory to the lowest.

if A points to
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
| 1| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0| 0|
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|

calling mymemcpy (A+1, A, sizeof (double) * (nrows * ncols - 1))
will result in
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1| 1|
|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|

We had to do this because Solaris implementation of memcpy()
does not propagate the value at the highest memory to the lowest.
this make Matrix_Identity(), Matrix_Zero() and Matrix_Fill() fail.

I have use source code from the book to implement mymemcpy
The Standard C Library -- P.J. Plauger
*/
/*////////////////////////////////////////////////////////////////////////////*/
static void * mymemcpy (void *s1, const void *s2, size_t n)
{
    char *su1;
    const char *su2;

    for (su1 = s1, su2 = s2; 0 < n; ++su1, ++su2, --n)
        *su1 = *su2;
    return s1;
}
/*////////////////////////////////////////////////////////////////////////////*/
/* copy the zero-offset matrix to the one-offset matrix */
/*////////////////////////////////////////////////////////////////////////////*/
static void CopyMatrixTo1OffsetMatrix (MATRIX * B, MATRIX * A)
{
    Matrix_SetSubmatrix (B->valueSq, B->ncols, A->valueSq, A->nrows, A->ncols,
        1, 1);
}


/* Functions for Iterative */
/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/
static double GetScale (int size, double scale, matrix_t M)
{
    int i, j;
    double normal;

    normal = 0.0;
    for (i = 0; i < size; i++){
        for (j = 0; j < size; j++){
            normal += M[i][j] * M[i][j];
        }
    }
    normal = scale * sqrt (normal);
    return normal;
}
/*////////////////////////////////////////////////////////////////////////////*/
/* B0[][] is the normalized first guess of the root */
/*////////////////////////////////////////////////////////////////////////////*/
static void Snorm (int size, matrix_t M, matrix_t B0, double normal)
{
    int i, j;

    if (normal == 0.0) return;

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            B0[i][j] = M[i][j] / normal;
        }
    }
}


/*////////////////////////////////////////////////////////////////////////////*/
/* Functions for LU */
/*////////////////////////////////////////////////////////////////////////////*/
static int dludcmp (double **a, int n, int *indx, double *d)
{
    int i, imax, j, k;
    double big, dum, sum, temp;
    double *vv; /*vv stores the implicit scaling of each row. */

    vv = STC_AllocateMemory (sizeof *vv * (n+1)); if (!vv) return -1;

    *d = 1.0;   /* No row interchanges yet. */
    /*Loop over rows to get the implicit scaling information.*/
    for (i = 1; i <= n; i++) {
        big = 0.0;
        for (j = 1; j <= n; j++)
            if ((temp = fabs (a[i][j])) > big) big = temp;
        if (big == 0.0) {
            EI_AddMessage ("LU Decomposition", EIE_ERROR,
                M30002); /* Cannot inverse singular matrix in routine dludcmp */
            STC_FreeMemory (vv);
            return -1;
        }
        /* No nonzero largest element.*/
        vv[i] = 1.0 / big; /* Save the scaling. */
    }

    for (j = 1; j <= n; j++){/*This is the loop over columns of Crout's method*/
        for (i = 1; i < j; i++) {/*This is equation (2.3.12) except for i=j*/
            sum = a[i][j];
            for (k = 1; k < i; k++) sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
        }
        big = 0.0;       /*Initialize for the search for largest pivot element*/
        for (i=j; i<=n; i++){/* This is i=j of equation (2.3.12) and i=j+1...N*/
            sum = a[i][j];         /* of equation (2.3.13). */
            for (k = 1; k < j; k++)
            sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
            if ((dum = vv[i] * fabs (sum)) >= big) {
                /* Is the figure of merit for the pivot
                   better than the best so far? */
                big = dum;
                imax = i;
            }
        }
        if (j != imax) {  /* Do we need to interchange rows? */
            for (k = 1; k <= n; k++) { /* Yes, do so... */
                dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k] = dum;
            }
            *d = -(*d); /* ...and change the parity of d. */
            vv[imax] = vv[j];  /*Also interchange the scale factor. */
        }
        indx[j] = imax;
        if (a[j][j] == 0.0) a[j][j] = LU_TINY;
        /* If the pivot element is zero the matrix is singular
           (at least to the precision of the algorithm).
           For some applications on singular matrices,
           it is desirable to substitute LU_TINY for zero.*/
        if (j != n) {  /*Now, finally, divide by the pivot element. */
            dum = 1.0/(a[j][j]);
            for (i = j+1; i <= n; i++) a[i][j] *= dum;
        }
    }  /*Go back for the next column in the reduction. */
    STC_FreeMemory (vv);

    return 0;
}
/*////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////////////////////////////*/
static void dlubksb (double **a, int n, int *indx, double b[])
{
    int i, ii = 0, ip, j;
    double sum;

    for (i = 1; i <= n; i++) {
        /* When ii is set to a positive value, it will become the
           index of the first nonvanishing element of b. We now
           do the forward substitution, equation (2.3.6). The
           only new wrinkle is to unscramble the permutation as we go. */
        ip = indx[i];
        sum = b[ip];
        b[ip] = b[i];
        if (ii)
            for (j = ii; j <= i-1; j++) sum -= a[i][j] * b[j];
        else if (sum) ii = i;/*A nonzero element was encountered, so from now on
                             we will have to do the sums in the loop above.*/
        b[i] = sum;
    }
    for (i = n; i >= 1; i--){/*Now we do the backsubstitution,equation (2.3.7)*/
        sum = b[i];
        for (j = i+1; j <= n; j++) sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i]; /*Store a component of the solution vector X.*/
    }
}
/*********************************************************************
AbsMax = FindAbsMax (m, c);
*********************************************************************/
static double FindAbsMax (
    MATRIX * m,
    int c)
{
    double Abs;
    double AbsMax;
    int r;

    AbsMax = -1;

    for (r = 0; r < m->nrows; r++) {
        Abs = fabs (m->value[r][c]);
        if (AbsMax < Abs)
            AbsMax = Abs;
    }

    if (AbsMax == 0.0)
        AbsMax = 1.0;

    return AbsMax;
}
/*********************************************************************
*********************************************************************/
MATRIX * Matrix2NR (
    MATRIX * A)
{
    MATRIX * NR; /* NR matrices are 1 based */

    NR = Matrix_Allocate (A->nrows+1, A->ncols+1);
    if (NR == NULL) return NULL;

    Matrix_SetSubmatrix (NR->valueSq, NR->ncols, A->valueSq, A->nrows, A->ncols,
        1, 1);

    return NR;
}
/*********************************************************************
*********************************************************************/
MATRIX * NR2Matrix (
    MATRIX * NR) /* NR matrices are 1 based */
{
    MATRIX * A;

    A = Matrix_Allocate (NR->nrows-1, NR->ncols-1);
    if (A == NULL) return NULL;

    Matrix_GetSubmatrix (A->valueSq, A->nrows, A->ncols, NR->valueSq, NR->ncols,
        1, 1);

    return A;
}
/*********************************************************************
Just for debugging.  Prints a 0-offset vector.
*********************************************************************/
static void PrintVec1 (
    double * vec,
    int r,
    char * Name)
{
    int i;

    EI_AddMessage ("", 4, "VECTOR %s\n", Name);
    for (i = 1; i <= r; i++)
        EI_AddMessage ("", 4, "%e\n", vec[i]);
    EI_AddMessage ("", 4, "\n");
}
/*********************************************************************
ScaleColumn (m, c, AbsMax);
*********************************************************************/
static void ScaleColumn (
    MATRIX * m,
    int c,
    double AbsMax)
{
    int r;

    if (AbsMax == 0.0 || AbsMax == 1.0) return;

    for (r = 0; r < m->nrows; r++) {
        m->value[r][c] /= AbsMax;
    }
}
