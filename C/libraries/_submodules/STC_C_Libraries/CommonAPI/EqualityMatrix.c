#include <stdio.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "EI_Message.h"
#include "STC_Memory.h"

#include "EqualityMatrix.h"

/* ------------------------------------------------------------------ */
/*      Allocate space for the equality matrix                        */
/*                                                                    */
/*      The equality matrix is an upper triangular matrix of size     */
/*      (nb_col) * (nb_col + 1).                                     */
/* ------------------------------------------------------------------ */
int EqualityMatrixAllocate (
    int nb_col,
    tEqualityMatrix * EqualityMatrix)
{
    register int i;
    int cnt;

    EqualityMatrix->EditId = STC_AllocateMemory (
        nb_col * sizeof *EqualityMatrix->EditId);
    if (EqualityMatrix->EditId == NULL)
        return -1;

    for (i = 0; i < nb_col; i++)
        EqualityMatrix->EditId[i] = -1;

    EqualityMatrix->equa = STC_AllocateMemory (
        nb_col * sizeof *EqualityMatrix->equa);
    if (EqualityMatrix->equa == NULL)
        return -1;

    for (i = 0; i < nb_col; i++) {
        cnt = nb_col + 1 - i;
        EqualityMatrix->equa[i] = STC_AllocateMemory (
            cnt * sizeof *EqualityMatrix->equa[i]);
        if (EqualityMatrix->equa[i] == NULL)
            return -1;
    }

    /*  EqualityMatrix is a upper square */
    EqualityMatrix->nb_col  = nb_col+1; /*  +1 because edit's constant   */
    EqualityMatrix->nb_row  = nb_col;   /*  # of rows = # of col   */

    EqualityMatrix->nb_equa = 0;  /*  nothing is loaded in EqualityMatrix yet */

    return 0;
}

/* ------------------------------------------------------------------ */
/*      Free space for the equality matrix                            */
/* ------------------------------------------------------------------ */
void EqualityMatrixFree (
    tEqualityMatrix * EqualityMatrix)
{
    register int i;

    for (i = 0; i < EqualityMatrix->nb_col-1; i++)
        STC_FreeMemory (EqualityMatrix->equa[i]);
    STC_FreeMemory (EqualityMatrix->equa);
    STC_FreeMemory (EqualityMatrix->EditId);
}

/* ------------------------------------------------------------------ */
/*      Print the equality matrix                                     */
/* ------------------------------------------------------------------ */
void EqualityMatrixPrint (
    tEqualityMatrix * EqualityMatrix)
{
    EI_AddMessage ("", 4, "EqualityMatrix->nb_col=%d\n"
        "EqualityMatrix->nb_row=%d\n"
        "EqualityMatrix->nb_equa=%d",
        EqualityMatrix->nb_col,
        EqualityMatrix->nb_row,
        EqualityMatrix->nb_equa);
}
