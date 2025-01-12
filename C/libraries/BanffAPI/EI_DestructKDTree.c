/*****************************************************************************/
/* NAME:            EI_DestructKDTree                                        */
/*                                                                           */
/* DESCRIPTION:     Will Free the KDTree memory, one node at a time.         */
/*                  This is a recursive function which will call itself      */
/*                  on the leftson and on the rightson of the current node.  */
/*                  The recursion terminates at each node which is a         */
/*                  terminal node.                                           */
/*****************************************************************************/

/* ------------------------------------------------------------------ */
/*       Include Header Files                                         */
/* ------------------------------------------------------------------ */
#include "EI_Donor.h"

#include "STC_Memory.h"

void EI_DestructKDTree (
    EIT_KDTREE  *current_node)
{
    if (current_node->branchid != TERMNODE) {
        EI_DestructKDTree(current_node->leftson);
        EI_DestructKDTree(current_node->rightson);
    }
    STC_FreeMemory(current_node);
}
