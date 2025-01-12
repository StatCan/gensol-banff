#ifndef EQUALITYMATRIX_H
#define EQUALITYMATRIX_H

typedef struct {
    int     * EditId;  /* array of pointers to edit-id   */
    double ** equa;   /* array of pointers to equations */
    int       nb_equa;  /* # of equations already loaded  */
    int       nb_col;   /* # of columns including the constant */
    int       nb_row;   /* maximum number of rows         */
} tEqualityMatrix;

int EqualityMatrixAllocate (int, tEqualityMatrix *);
void EqualityMatrixFree (tEqualityMatrix *);
void EqualityMatrixPrint (tEqualityMatrix *);

#endif
