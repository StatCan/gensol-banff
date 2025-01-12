#ifndef MATRIX_H
#define MATRIX_H

/* prints the matrix with a default number of decimals */
//#define Matrix_Print(x) Matrix_PrintWithPrecision((x), 6);
#define Matrix_Print(x) MATRIX_PRINT((x),6)

/* prints the matrix name and its content */
#define MATRIX_PRINT(x,d) {EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, #x "\n"); Matrix_PrintWithPrecision ((x), (d));}

struct MATRIX {
    double * valueSq;
    double ** value;
    int nrows;
    int ncols;
};
typedef struct MATRIX MATRIX;


CLASS_DECLSPEC void Matrix_Add (double *C, double *A, double *B, int nrows,
    int ncols);
CLASS_DECLSPEC void Matrix_AddScalar (double *C, double x, int nrows,
    int ncols);
CLASS_DECLSPEC MATRIX * Matrix_Allocate (int nrows, int ncols);
CLASS_DECLSPEC void Matrix_Analyse (MATRIX * A);
CLASS_DECLSPEC void Matrix_Clean (MATRIX *A);
CLASS_DECLSPEC void Matrix_Copy (double *C, double *A, int nrows, int ncols);
CLASS_DECLSPEC MATRIX * Matrix_Duplicate (MATRIX * A);
CLASS_DECLSPEC int Matrix_Equal (MATRIX *A, MATRIX *B);
CLASS_DECLSPEC void Matrix_Fill (double *A, double x, int nrows, int ncols);
CLASS_DECLSPEC void Matrix_Free (MATRIX * A);
CLASS_DECLSPEC void Matrix_GetSubmatrix(double *S, int mrows, int mcols,
    double *A, int ncols, int row, int col);
CLASS_DECLSPEC void Matrix_Identity (double *A, int n);

/* inverse A, met le resultat dans AInverse */
CLASS_DECLSPEC int Matrix_Inverse (MATRIX *AInverse, MATRIX *A); /*do not use!*/
CLASS_DECLSPEC int Matrix_InverseByPartition (MATRIX * AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseByPartitionMH (MATRIX * AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseByPartitionRecursive (MATRIX * AInverse,
    MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseIterative (MATRIX *AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseLU (MATRIX * AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseSimul (MATRIX *AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseSvd (MATRIX *AInverse, MATRIX *A);
CLASS_DECLSPEC int Matrix_InverseSvdNR (MATRIX *AInverse, MATRIX *A);

CLASS_DECLSPEC void Matrix_JoinByColumn (double *C, double *A, int nrows,
    int ncols, double *B, int mrows);
CLASS_DECLSPEC void Matrix_JoinByRow (double *C, double *A, int nrows,
    int ncols, double *B, int mcols);
CLASS_DECLSPEC void Matrix_Multiply (double *C, double *A, int nrows, int ncols,
    double *B, int mcols);
CLASS_DECLSPEC void Matrix_MultiplyDiagonalWithMatrix(MATRIX * DB, MATRIX * D,
    MATRIX * B);
CLASS_DECLSPEC void Matrix_MultiplyMatrixWithDiagonal(MATRIX * AD, MATRIX * A,
    MATRIX * D);
CLASS_DECLSPEC void Matrix_MultiplyMatrixWithTranspose (double *C, double *A,
    int nrows, int ncols, double *B, int mrows);
CLASS_DECLSPEC void Matrix_MultiplyScalar (double *A, double x, int nrows,
    int ncols);
CLASS_DECLSPEC void Matrix_MultiplySelfTranspose (double *C, double *A,
    int nrows, int ncols);
CLASS_DECLSPEC void Matrix_MultiplyTransposeWithMatrix (double *C, double *A,
    int nrows, int ncols, double *B, int mcols);
CLASS_DECLSPEC void Matrix_PrintWithPrecision (MATRIX *A, int Precision);
CLASS_DECLSPEC void Matrix_Random (MATRIX * A, double min, double max);
CLASS_DECLSPEC void Matrix_ScaleToOrderUnity (MATRIX *A);
CLASS_DECLSPEC void Matrix_SetDiagonal (double *A, double v[], int nrows,
    int ncols);
CLASS_DECLSPEC void Matrix_SetSubmatrix(double *A, int ncols, double *S,
    int mrows, int mcols, int row, int col);
CLASS_DECLSPEC void Matrix_Subtract (double *C, double *A, double *B, int nrows,
    int ncols);
CLASS_DECLSPEC void Matrix_Transpose (double *At, double *A, int nrows,
    int ncols);
CLASS_DECLSPEC void Matrix_TransposeSquare (double *A, int n);
CLASS_DECLSPEC void Matrix_Zero (double *A, int nrows, int ncols);

#endif
