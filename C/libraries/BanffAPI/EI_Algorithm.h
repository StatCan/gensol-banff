#ifndef EI_ALGORITHM_H
#define EI_ALGORITHM_H

/*!
\defgroup Algorithm Algorithm
@{
*/

#include "EI_Common.h"

/*! String representing the value of algorithm type EF. */
#define EIM_ALGORITHM_TYPE_EF "EF"
/*! String representing the value of algorithm type LR. */
#define EIM_ALGORITHM_TYPE_LR "LR"

/*! Type of algorithm. */
typedef enum EIT_ALGORITHM_TYPE {
        /*! Estimation function. */
    EIE_ALGORITHM_TYPE_EF,
        /*! Linear regression. */
    EIE_ALGORITHM_TYPE_LR,
        /*! Type is not valid. */
    EIE_ALGORITHM_TYPE_INVALID
} EIT_ALGORITHM_TYPE;

/*!
\brief Structure holding an algorithm.
*/
typedef struct EIT_ALGORITHM {
        /*! The name. */
    char * Name;
        /*! The type. */
    EIT_ALGORITHM_TYPE Type;
        /*! The status. */
    char * Status;
        /*! The formula. */
    char * Formula;
        /*! The description. */
    char * Description;
} EIT_ALGORITHM;

/*!
\brief Structure holding a list of algorithms.
*/
typedef struct EIT_ALGORITHM_LIST {
        /*! Number of allocated entries. */
    int NumberAllocatedEntries;
        /*! Number of entries. */
    int NumberEntries;
        /*! The list of algorithms. */
    EIT_ALGORITHM * Algorithm;
} EIT_ALGORITHM_LIST;

/*!
\defgroup EI_AlgorithmAdd EI_AlgorithmAdd()
\brief    Add a row at the end of the algorithm list structure.
@{
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_AlgorithmAdd (EIT_ALGORITHM_LIST *,
    char *, EIT_ALGORITHM_TYPE, char *, char *, char *);
/** @} **/

/*!
\defgroup EI_AlgorithmAddPredefined EI_AlgorithmAddPredefined()
\brief    Add the predefined algorithms at the end of the algorithm list structure.
@{
*/
CLASS_DECLSPEC EIT_RETURNCODE EI_AlgorithmAddPredefined (EIT_ALGORITHM_LIST *);
/** @} **/

/*!
\defgroup EI_AlgorithmAllocate EI_AlgorithmAllocate()
\brief    Allocate the algorithm list structure.
@{
*/
CLASS_DECLSPEC EIT_ALGORITHM_LIST * EI_AlgorithmAllocate (void);
/** @} **/

/*!
\defgroup EI_AlgorithmFind EI_AlgorithmFind()
\brief    Find the algorithm named name in the algorithm list structure.
@{
*/
CLASS_DECLSPEC int EI_AlgorithmFind (EIT_ALGORITHM_LIST *, char *);
/** @} **/

/*!
\defgroup EI_AlgorithmFree EI_AlgorithmFree()
\brief    Free the algorithm list structure.
@{
*/
CLASS_DECLSPEC void EI_AlgorithmFree (EIT_ALGORITHM_LIST *);
/** @} **/

/*!
\defgroup EI_AlgorithmPrint EI_AlgorithmPrint()
\brief    Print the contents of the algorithm list structure.
@{
*/
CLASS_DECLSPEC void EI_AlgorithmPrint (EIT_ALGORITHM_LIST *);
/** @} **/

/** @} **/

#endif
