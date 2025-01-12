#ifndef EI_AVERAGE_H
#define EI_AVERAGE_H

/*!
\defgroup Average Average
@{
*/

#include "EI_Common.h"

/*!
\brief Structure holding an entry of the average link list.
*/
typedef struct EIT_AVERAGE {
        /*! The name. */
    char * Name;
        /*! The period. */
    EIT_PERIOD Period;
        /*! Index of Name in DataRec[Period]. */
    int VariableIndex;
        /*! Index of Weight in DataRec[Period]. */
    int WeightIndex;
        /*! SUM field or SUM (field * weigth). */
    double Numerator;
        /*! count or SUM weight. */
    double Denominator;
        /*! The average value. */
    double Average;
        /*! # of acceptable records. */
    int Count;
        /*! Pointer to the next entry in the link list. */
    struct EIT_AVERAGE * Next;
} EIT_AVERAGE;

/*!
\defgroup EI_AverageAdd EI_AverageAdd()
\brief    Add a row at the end of the average list structure.
@{
*/
CLASS_DECLSPEC EIT_AVERAGE * EI_AverageAdd (EIT_AVERAGE *, char *, EIT_PERIOD);
/** @} **/

/*!
\defgroup EI_AverageFree EI_AverageFree()
\brief    Free the average list structure.
@{
*/
CLASS_DECLSPEC void EI_AverageFree (EIT_AVERAGE *);
/** @} **/

/*!
\defgroup EI_AverageFind EI_AverageFind()
\brief    Find the average named name in the average list structure.
@{
*/
CLASS_DECLSPEC EIT_AVERAGE * EI_AverageLookup (EIT_AVERAGE *, char *, EIT_PERIOD);
/** @} **/

/*!
\defgroup EI_AveragePrint EI_AveragePrint()
\brief    Print the contents of the average list structure.
@{
*/
CLASS_DECLSPEC void EI_AveragePrint (EIT_AVERAGE *);
/** @} **/

/** @} **/

#endif
