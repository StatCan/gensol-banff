#ifndef EI_ACCEPTABLEKEYSRESULT_H
#define EI_ACCEPTABLEKEYSRESULT_H

#include "EI_Common.h"

/*!
\defgroup AcceptableKeysResult AcceptableKeysResult
@{
*/

/*!
\brief Structure holding the list of acceptable record keys.
*/
typedef struct EIT_ACCEPTABLEKEYSRESULT_LIST {
        /*! Number of allocated entries. */
    int NumberAllocatedEntries;
        /*! Number of entries. */
    int NumberEntries;
        /*! The acceptable keys. */
    char ** AcceptableKeys;
} EIT_ACCEPTABLEKEYSRESULT_LIST;

/*!
\defgroup EI_AcceptableKeysResultAdd EI_AcceptableKeysResultAdd()
\brief    Add a row at the end of the acceptable keys structure.
@{
*/
CLASS_DECLSPEC EIT_ACCEPTABLEKEYSRESULT_LIST * EI_AcceptableKeysResultAdd (
    EIT_ACCEPTABLEKEYSRESULT_LIST *, char *);
/** @} **/

/*!
\defgroup EI_AcceptableKeysResultAllocate EI_AcceptableKeysResultAllocate()
\brief    Allocate the acceptable keys structure.
@{
*/
CLASS_DECLSPEC EIT_ACCEPTABLEKEYSRESULT_LIST * EI_AcceptableKeysResultAllocate (void);
/** @} **/

/*!
\defgroup EI_AcceptableKeysResultEmpty EI_AcceptableKeysResultEmpty()
\brief    Delete all antries in the acceptable keys structure. 
@{
*/
CLASS_DECLSPEC void EI_AcceptableKeysResultEmpty (EIT_ACCEPTABLEKEYSRESULT_LIST *);
/** @} **/

/*!
\defgroup EI_AcceptableKeysResultFree EI_AcceptableKeysResultFree()
\brief    free the acceptable keys structure. 
@{
*/
CLASS_DECLSPEC void EI_AcceptableKeysResultFree (EIT_ACCEPTABLEKEYSRESULT_LIST *);
/** @} **/

/*!
\defgroup EI_AcceptableKeysResultPrint EI_AcceptableKeysResultPrint()
\brief    Print the contents of the acceptable keys structure. 
@{
*/
CLASS_DECLSPEC void EI_AcceptableKeysResultPrint (EIT_ACCEPTABLEKEYSRESULT_LIST *);
/** @} **/

/** @} **/

#endif
