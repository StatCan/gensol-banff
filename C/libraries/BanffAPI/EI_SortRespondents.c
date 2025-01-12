#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_DataRec.h"
#include "EI_SortRespondents.h"

#include "STC_Memory.h"

#define M 7 /* Quicksort: optimal "subarrays size" suggested to do straight
               insertion (when a subarray has gotten down to some size M,
               it becomes faster to sort it by straight insertion) */

static void SwapElements (char **, EIT_DATAREC *, EIT_EXCLUSION *, long, long);


/* (C) Copr. 1986-92 Numerical Recipes Software s@#=.
       QUICKSORT "array1" (AllRespondents.Key) into ascending order of "key"
       while making the corresponding rearrangement of the "array2"
       (AllRespondents.RespondentData) and "array3"
       (AllRespondents.RespondentExclusion).*/
EIT_SORTRESPONDENTS_QUICKSORT_RETURNCODE EI_SortRespondents_Quicksort (
    EIT_ALL_RESPONDENTS * AllRespondents)
{
    EIT_DATAREC DataRec;
    EIT_EXCLUSION Exclusion;
    long i;
    long ir;
    int * istack;
    long j;
    int jstack;
    long k;
    char * Key;
    long l;
    EIT_EXCLUSION * ptrWorkExclusion;
    char ** ptrWorkKey;
    EIT_DATAREC * ptrWorkRespondentData;

    ir = AllRespondents->NumberofRespondents;
    jstack = 0;
    l = 1;

    istack = STC_AllocateMemory (AllRespondents->NumberofRespondents *
        sizeof *istack);

    if (istack == NULL)
        return EIE_SORTRESPONDENTS_QUICKSORT_FAIL;

    /* Adjusts index of "arrays". Algorithm uses arr[1..n]
       instead of arr[0..n-1].*/
    ptrWorkKey = AllRespondents->Key;
    ptrWorkKey--;
    ptrWorkRespondentData = AllRespondents->RespondentData;
    ptrWorkRespondentData--;
    ptrWorkExclusion = AllRespondents->RespondentExclusion;
    ptrWorkExclusion--;

    for (;;) { /* Insertion sort when subarray small enough. */
        if (ir-l < M) {
            for (j=l+1; j<=ir; j++) {
                Key = ptrWorkKey[j];
                DataRec = ptrWorkRespondentData[j];
                Exclusion = ptrWorkExclusion[j];
                for (i=j-1; i>=l; i--) {
                    if (strcmp(ptrWorkKey[i], Key) <= 0) break;
                    ptrWorkKey[i+1] = ptrWorkKey[i];
                    ptrWorkRespondentData[i+1] = ptrWorkRespondentData[i];
                    ptrWorkExclusion[i+1] = ptrWorkExclusion[i];
                    if (strcmp(ptrWorkKey[i], Key) <= 0) break;
                }
                ptrWorkKey[i+1] = Key;
                ptrWorkRespondentData[i+1] = DataRec;
                ptrWorkExclusion[i+1] = Exclusion;
            }
            if (!jstack) {
                STC_FreeMemory (istack);
                return EIE_SORTRESPONDENTS_QUICKSORT_SUCCEED;
            }

            ir = istack[jstack];  /* Pop stack and begin a new round */
            l = istack[jstack-1]; /* of partitioning.                */
            jstack -= 2;
        }
        else {
            k = (l+ir) >> 1;      /* Choose median of left, center and  */
                                  /* right elements as partitioning     */
                                  /* element a. Also rearrange so that  */
                                  /* a[l] <= a[l+1] <= a[ir].           */
            SwapElements(ptrWorkKey, ptrWorkRespondentData,
                ptrWorkExclusion, k, l+1);
            if (strcmp(ptrWorkKey[l], ptrWorkKey[ir]) > 0)
                SwapElements(ptrWorkKey, ptrWorkRespondentData,
                    ptrWorkExclusion, l, ir);
            if (strcmp(ptrWorkKey[l+1], ptrWorkKey[ir]) > 0)
                SwapElements(ptrWorkKey, ptrWorkRespondentData,
                    ptrWorkExclusion, l+1, ir);
            if (strcmp(ptrWorkKey[l], ptrWorkKey[l+1]) > 0)
                SwapElements(ptrWorkKey, ptrWorkRespondentData,
                    ptrWorkExclusion, l, l+1);

            i = l+1; /* Initialize pointers for partitioning. */
            j = ir;

            Key = ptrWorkKey[l+1]; /* Partitioning element.*/
            DataRec = ptrWorkRespondentData[l+1];
            Exclusion = ptrWorkExclusion[l+1];
            for (;;) { /* Innermost loop. */
                /* scan up to find element > a */
                do i++; while (strcmp(ptrWorkKey[i], Key) < 0);
                /* scan down to find element < a */
                do j--; while (strcmp(ptrWorkKey[j], Key) > 0);

                if (j < i) break; /* Pointers crossed. Partitioning complete.*/
                SwapElements(ptrWorkKey, ptrWorkRespondentData,
                    ptrWorkExclusion, i, j); /* Exchange elements of "arrays".*/
            }

            /* Insert partitionning element in "arrays". */
            ptrWorkKey[l+1] = ptrWorkKey[j];
            ptrWorkKey[j] = Key;

            ptrWorkRespondentData[l+1] = ptrWorkRespondentData[j];
            ptrWorkRespondentData[j] = DataRec;

            ptrWorkExclusion[l+1] = ptrWorkExclusion[j];
            ptrWorkExclusion[j] = Exclusion;

            jstack += 2;
            /* Push pointers to larger subarray on stack,
               process smaller subarray immediately.*/
            if (jstack > AllRespondents->NumberofRespondents) {
                STC_FreeMemory (istack);
                return EIE_SORTRESPONDENTS_QUICKSORT_ERROR_STACK;
            }

            if (ir-i+1 >= j-l) {
                istack[jstack] = ir;
                istack[jstack-1] = i;
                ir = j-1;
            }
            else {
                istack[jstack] = j-1;
                istack[jstack-1] = l;
                l = i;
            }
        }
    }
}

/* SHELLSORT: sort "AllRespondents" into ascending order of "keys"
   and rearrange also structure elements "RespondentData" and
   "RespondentExclusion" */
void EI_SortRespondents_Shellsort (
    EIT_ALL_RESPONDENTS * AllRespondents)
{
    int gap;
    int i;
    int j;
    int k;
    char * TempKey;
    EIT_DATAREC TempDataRec;
    EIT_EXCLUSION TempRespondentExclusion;

    gap = 1;
    do
       (gap = 3*gap + 1);
    while (gap <= AllRespondents->NumberofRespondents);  /* Initialize "gap" */

    for (gap /= 3; gap > 0; gap /= 3)
        for (i = gap; i < AllRespondents->NumberofRespondents; i++)
            for (j = i - gap; j >= 0; j -= gap) {

                k = j + gap;
                if (strcmp(AllRespondents->Key[j], AllRespondents->Key[k]) < 0 )
                   break;

                TempKey = AllRespondents->Key[j];
                AllRespondents->Key[j] = AllRespondents->Key[k];
                AllRespondents->Key[k] = TempKey;

                TempDataRec = AllRespondents->RespondentData[j];
                AllRespondents->RespondentData[j] =
                    AllRespondents->RespondentData[k];
                AllRespondents->RespondentData[k] = TempDataRec;

                TempRespondentExclusion =
                    AllRespondents->RespondentExclusion[j];
                AllRespondents->RespondentExclusion[j] =
                    AllRespondents->RespondentExclusion[k];
                AllRespondents->RespondentExclusion[k] =
                    TempRespondentExclusion;
            }
}


/* Put in ascending order of "key" two respondents.
   (before "swap": arr[indice1] > arr[indice2]) */
static void SwapElements (
    char ** Key,
    EIT_DATAREC * RespondentData,
    EIT_EXCLUSION * RespondentExclusion,
    long indice1,
    long indice2)
{
    EIT_DATAREC TempDataRec;
    char * TempKey;
    EIT_EXCLUSION TempRespondentExclusion;

    TempKey = Key[indice1];
    Key[indice1] = Key[indice2];
    Key[indice2] = TempKey;

    TempDataRec = RespondentData[indice1];
    RespondentData[indice1] = RespondentData[indice2];
    RespondentData[indice2] = TempDataRec;

    TempRespondentExclusion = RespondentExclusion[indice1];
    RespondentExclusion[indice1] = RespondentExclusion[indice2];
    RespondentExclusion[indice2] = TempRespondentExclusion;

}
