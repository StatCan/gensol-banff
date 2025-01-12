/* ------------------------------------------------------------------ */
/*                                                                    */
/* NAME:        EI_Transform                                          */
/*                                                                    */
/* DESCRIPTION: Will transform the original values of one field       */
/*              to values in the range (0-1).                         */
/*                                                                    */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */

#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Donor.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "MatchingField.h"

/*--------------------------------------------------------------------*/
/* Local Functions                                                    */
/*--------------------------------------------------------------------*/
static int ReadOriginalValues (char *, EIT_MATCHVAL *, EIT_MFREADCALLBACK);
static int TransformData (EIT_MATCHVAL *, EIT_MATCHVAL *);

/*--------------------------------------------------------------------*/

EIT_TRF_RETURNCODE EI_Transform (
        /* Matching field Name / Nom du champ d'appariement */
    EIT_FIELDNAMES * MatchingFields,
        /* Callback function to read matching field data */
        /* This function will read the data for*/
        /* one field at a time*/
        /* Fonction de lecture des valeurs des champs d'appariement*/
        /* Cette fonction lit les données pour un champ à la fois*/
    EIT_MFREADCALLBACK ReadMFData,
        /* Structure to hold transformed data*/
        /* Structure de valeurs transformées*/
    EIT_TRANSFORMED * AllTransformed)
{
    int cbrc;
    int i;
    EIT_MATCHVAL * OriginalValues;
    EIT_MATCHVAL   TransformedValues;
    int trfrcode;

    AllTransformed->NumberofFields = MatchingFields->NumberofFields;
    AllTransformed->FieldName = MatchingFields->FieldName;
    AllTransformed->TransformedValues = STC_AllocateMemory (
        MatchingFields->NumberofFields *
            sizeof *AllTransformed->TransformedValues);
    if (AllTransformed->TransformedValues == NULL)
        return EIE_TRANSFORM_FAIL;

    for (i=0;i<MatchingFields->NumberofFields;i++)
    {
        OriginalValues = MF_FieldValuesAlloc ();
        if (OriginalValues == NULL)
            return EIE_TRANSFORM_FAIL;

        cbrc = ReadOriginalValues (MatchingFields->FieldName[i],
            OriginalValues, ReadMFData);
        if (cbrc == -1)
            return EIE_TRANSFORM_FAIL;

        trfrcode = TransformData (OriginalValues, &TransformedValues);
        if (trfrcode == -1)
            return EIE_TRANSFORM_FAIL;

        MF_FieldValuesFree (OriginalValues);
        AllTransformed->TransformedValues[i]=TransformedValues;

        MF_FieldValuesSort (&TransformedValues);
    }
    return EIE_TRANSFORM_SUCCEED;
}

/*--------------------------------------------------------------------*/
/* Read the original values of the current matching field             */
/*--------------------------------------------------------------------*/
static int ReadOriginalValues (
    char *FieldName,
    EIT_MATCHVAL * OriginalValues,
    EIT_MFREADCALLBACK ReadMFData)
{
    int Index;
    char *Key;
    MFT_FIELDVALUESADD_RETURNCODE mfrcode;
    EIT_READCALLBACK_RETURNCODE rcode;
    double Value;

    Index = 0;

    rcode = ReadMFData (FieldName, &Index, &Key, &Value);
    if (rcode == EIE_READCALLBACK_FAIL)
        return -1;

    while (rcode == EIE_READCALLBACK_FOUND)
    {
        mfrcode = MF_FieldValuesAdd (OriginalValues, Key, Value);
        if (mfrcode == MFE_FIELDVALUESADD_FAIL)
            return -1;

        rcode = ReadMFData (FieldName, &Index, &Key, &Value);
        if (rcode == EIE_READCALLBACK_FAIL)
            return -1;
    }
    return 1;
}


/*--------------------------------------------------------------------*/
/* Transform the matchable fields in each match column into the       */
/* interval: (0, 1).                                                   */
/*                                                                    */
/* Within the match column,  the transformation is done in 2 steps:   */
/* (1)  Sort the data in ascending order;                             */
/* (2)  Pass through the sorted array once and for each distinct      */
/*      value:  - determine its rank and the number of fields sharing */
/*                this same distinct value;                           */
/*              - calculate the corresponding transformed value       */
/*--------------------------------------------------------------------*/
static int TransformData (
    EIT_MATCHVAL * OriginalValues,
    EIT_MATCHVAL * TransformedValues)
{
    int      j, k;
    int      gap;          /* Used in sort.                           */
    int      lower;        /* Lowest index of the next distinct value */
    int      nb_distinct;  /* Number of distinct values */
    int      nb_rec;       /* Number of fields in match column        */
    int      upper;        /* Upper limit of the next distinct value  */

    double   count;        /* Number of fields sharing the next value */
    double   rank;         /* Rank of the next distinct value         */
    double   tempvalue;    /* Used in sort                            */
    char     *tempkey;     /* Used in sort                            */
    double   total_plus;   /* Total fields in a column, plus 1        */
    double   transformed;  /* Temporary transformed value             */
    double   *value_ptr;   /* Array of original column values.        */
    char     **key_ptr;    /* Array of original column values.        */


    /*DEBUG
    for (i=0;i<OriginalValues->NumberofValues;i++)
        printf ("Key is: %s\tOriginal Value is: %f\n",
            OriginalValues->Key[i], OriginalValues->Value[i]);
    END DEBUG */

    TransformedValues->NumberofValues = OriginalValues->NumberofValues;
    TransformedValues->NumberofAllocatedValues =
        OriginalValues->NumberofAllocatedValues;
    TransformedValues->Key = STC_AllocateMemory (
        TransformedValues->NumberofValues * sizeof *TransformedValues->Key);
    if (TransformedValues->Key == NULL)
        return -1;

    TransformedValues->Value = STC_AllocateMemory (
        TransformedValues->NumberofValues * sizeof *TransformedValues->Value);
    if (TransformedValues->Value == NULL)
        return -1;

    key_ptr = OriginalValues->Key;
    value_ptr = OriginalValues->Value;
    nb_rec    = TransformedValues->NumberofValues;

    /*** SORT THE ORIGINAL VALUES OF THE CURRENT MATCH COLUMN *****/
    for (gap = nb_rec/2; gap > 0; gap /= 2)
        for (j = gap; j < nb_rec; j++)
            for (k = j - gap;
                k >= 0 && value_ptr[k] > value_ptr[k+gap]; k -= gap)
                {
                    tempvalue        = value_ptr[k];
                    value_ptr[k]     = value_ptr[k+gap];
                    value_ptr[k+gap] = tempvalue;

                    tempkey          = key_ptr[k];
                    key_ptr[k]       = key_ptr[k+gap];
                    key_ptr[k+gap]   = tempkey;
                }


    /**************************************************************/
    /*** COLLAPSE ARRAY OF ORIGINAL VALUES INTO ARRAY OF        ***/
    /*** DISTINCT ORIGINAL VALUES AND PARALLEL ARRAY OF         ***/
    /*** CORRESPONDING TRANSFORMED VALUES                       ***/
    /**************************************************************/
    total_plus  = (double) nb_rec + 1;
    nb_distinct = 0;
    for (lower = 0; lower < nb_rec; )
    {
        for (upper = lower; (upper < nb_rec) &&
            (value_ptr[lower] == value_ptr[upper]); upper++)
        {
            ;
        }

        count = (double) upper - lower;
        rank  = (double) lower + 1;

        /* This line does the actual transformation of data */
        /* See Functional Description of GEIS for more info */
        transformed = (rank + (count - 1.0)/2.0)/total_plus;

        /*
        printf ("lower is: %d\tupper is: %d\n", lower, upper);
        printf ("count is: %f\trank is: %f\n", count, rank);
        printf ("transformed is: %f\n", transformed);
        */

        nb_distinct++;
        for (k = lower; k < count+lower; k++)
        {
            TransformedValues->Key[k] = OriginalValues->Key[k];
            TransformedValues->Value[k] = transformed;
            /*
            printf ("TransformedValues->Value[k] is: %f\tk is: %d\n",
                TransformedValues->Value[k], k);
            */
        }
        lower = upper;
    }
    /*DEBUG
    for (i=0;i<TransformedValues->NumberofValues;i++)
        printf ("Key is: %s\tOriginal Value is: %f\tTransformed Value is: %f\n",
            OriginalValues->Key[i], OriginalValues->Value[i],
            TransformedValues->Value[i]);
    END DEBUG*/

    return 1;
}
