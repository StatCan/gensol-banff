/*
ilist.c
*/
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Message.h"
#include "STC_Memory.h"
#include "ilist.h"

/**********************************************************************/
/*                     DEFINE                                         */
/**********************************************************************/
#define ALLOCINCREMENT 20
#define END_MARKER     INT_MIN

/**********************************************************************/
/*                     PUBLIC FUNCTIONS                               */
/**********************************************************************/

/**********************************************************************/
/*                     PRIVATE FUNCTIONS                              */
/**********************************************************************/
static int ISortAscending  (const void *a, const void *b);
static int ISortDescending (const void *a, const void *b);

/*********************************************************************
 * Description: Adds item to list
 ********************************************************************/
int IList_Add (
    tIListType item,
    tIList * list)
{
    void * ptr;

    if (list->ne == list->ae) {   /* need more space ... */
        list->ae += ALLOCINCREMENT;
        ptr = STC_ReallocateMemory (
            sizeof *list->l * (list->ne + 1),
            sizeof *list->l * (list->ae + 1),
            list->l);
        if (ptr == NULL) {
            IList_Free (list);
            return eIListFail;
        }
        list->l = ptr;
    }

    list->l[list->ne] = item;
    list->ne++;
    list->l[list->ne] = END_MARKER;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Adds all items of a source list to a destination list
 ********************************************************************/
int IList_AddList (
    tIList *s,
    tIList *d)
{
    int i;

    for (i = 0; i < IList_NumEntries (s); i++)
        if (IList_Add (IList_Entry (s, i), d) == eIListFail)
            return eIListFail;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Adds items, if not already in, of a source list to a destination list
 ********************************************************************/
int IList_AddListNoDup (
    tIList *s,
    tIList *d)
{
    int i;

    for (i = 0; i < IList_NumEntries (s); i++)
        if (IList_AddNoDup (IList_Entry (s, i), d) == eIListFail)
            return eIListFail;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Adds item to list if not already in
 ********************************************************************/
int IList_AddNoDup (
    tIListType item,
    tIList *list)
{
    /* We do not add the same element twice */
    if (IList_Search (list, item) == ILIST_NOTFOUND)
        if (IList_Add (item, list) == eIListFail)
            return eIListFail;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Creates a copy of the list
 ********************************************************************/
int IList_Dup (
    tIList * src,
    tIList ** dest)
{
    int i;

    if (IList_New (dest) == eIListFail)
        return eIListFail;

    for (i = 0; i < IList_NumEntries (src); i++)
        if (IList_Add (IList_Entry (src, i), *dest) == eIListFail)
            return eIListFail;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Empties the list
 ********************************************************************/
void IList_Empty (
    tIList *list)
{
    list->l[0] = END_MARKER;
    IList_NumEntries (list) = 0;
}
/*********************************************************************
Checks if 2 lists are the same.
returns 1 if they are equal
otherwise, 0.
*********************************************************************/
int IList_Equal (
    tIList * l1,
    tIList * l2)
{
    int i;

	if (l1->ne != l2->ne) return 0;

    for (i = 0; l1->ne; i++) {
		if (IList_Entry(l1, i) != IList_Entry(l2, i)) {
            return 0;
        }
    }
    return 1;
}
/*********************************************************************
 * Description: Frees the list
 ********************************************************************/
void IList_Free (
    tIList *list)
{
    IList_Empty (list);
    STC_FreeMemory (list->l);
    STC_FreeMemory (list);
    return;
}
/*********************************************************************
Checks if 2 lists share items.
returns the position of the first item of l1 in l2
otherwise, ILIST_NOTFOUND if the lists do not intersect.
*********************************************************************/
int IList_Intersect (
    tIList * l1,
    tIList * l2)
{
    int i;
    tIListType * list;

    list = IList_TheList (l1);

    if (list != NULL) {
        for (i = 0; list[i]; i++) {
            if (IList_Search (l2, list[i]) != ILIST_NOTFOUND)
                return i;
        }
    }
    return ILIST_NOTFOUND;
}
/*********************************************************************
 * Description: Allocates a list
 ********************************************************************/
int IList_New (
    tIList ** NewList)
{
    tIList *list;

    *NewList = NULL;
    list = STC_AllocateMemory (sizeof *list);
    if (list == NULL)
        return eIListFail;

    list->l = STC_AllocateMemory (sizeof *list->l * (ALLOCINCREMENT+1));
    if (list->l == NULL) {
        STC_FreeMemory (list);
        return eIListFail;
    }
    *NewList = list;

    list->ne = 0;
    list->ae = ALLOCINCREMENT;
    list->l[0] = END_MARKER;

    return eIListSucceed;
}

/*********************************************************************
 * Description: Prints all items of the list
 ********************************************************************/
void IList_Print (
    tIList * list)
{
    int i;

    for (i = 0; i < IList_NumEntries (list); i++)
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "%d\n", IList_Entry(list, i));
}
#ifdef DBG_ON
int DBG_IListPrint(char * dbgscrfn, char * dbgscratch, tIList * list)
{
    /*-members of instances of "tIList" are:
       ne  (number of entries--type is                     int                                        )
       ae  (allocated entries--type is                     int                                        )
       l   (the list of items--type is ne-element array of tIListType (which is just an alias for int))
    */
    int i;
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "{");
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'ne': %d, ", list->ne);
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'ae': %d, ", list->ae);
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'l': ["                                                        );
    for (i=0;i<list->ne;i=i+1)
    {
        /* if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "(extra for debugging: list, (list->l), i, (list->l)[i]: '%d, %d, %d, %d')", list, (list->l), i, (list->l)[i]); */
        if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "%d, ", list->l[i]);
    }
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "],"                                                              );
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "}");
    
    return 0;
};
#endif
/*********************************************************************
 * Description: Removes an item at position 'position' from the list
 ********************************************************************/
void IList_Remove (
    tIList *list,
    int position)
{
    int i;

    for (i = position; i < IList_NumEntries(list); i++)
        list->l[i] = list->l[i+1];

    list->ne--;
}

/*********************************************************************
Counts the number of Item matching a string in the list.
returns the number found
*********************************************************************/
int IList_ItemCount (
    tIList * l,
    tIListType Item)
{
    int Count = 0;
    int i;
    tIListType * list;

    list = IList_TheList (l);

    if (list != NULL) {
        for (i = 0; list[i]; i++) {
            if (list[i] == Item)  /* match */
                Count++;
        }
    }
    return Count;
}

/*****************************************************************
 * Description:    Searches the END_MARKER terminated list of strings for listItem.
 * SearchList returns the index where it is found or ILIST_NOTFOUND.
 *****************************************************************/
int IList_Search (
    tIList * l,
    tIListType listItem)
{
    int i = 0;
    tIListType * list;

    list = IList_TheList (l);

    if (list == NULL)
        return ILIST_NOTFOUND;

    while (list[i] != END_MARKER) {
        if (list[i] == listItem)  /* match */
            return i;
        i++;
    }

    return ILIST_NOTFOUND;
}
/*****************************************************************
 * Description:    Searches the NULL terminated list of strings for listItem.
 * SearchList returns the index where it is found or ILIST_NOTFOUND.
 *****************************************************************/
int IList_SearchBinary (
    tIList * l,
    tIListType listItem)
{
    tIListType * list;
	int low, high, mid;

    list = IList_TheList (l);

    if (list == NULL)
        return ILIST_NOTFOUND;

	//EI_AddMessage ("", 4, "looking for %d", listItem);
	low = 0;
	high = l->ne-1;
	while (low <= high) {
		mid = (low + high) / 2;
		//EI_AddMessage ("", 4, "  compare with looking for %d", list[mid]);
		if (listItem == list[mid])
			return mid;
		else if (listItem > list[mid])
			low = mid + 1;
		else
			high = mid - 1;
	}

    return ILIST_NOTFOUND;
}
/*****************************************************************
 * Description:    Sorts the list alphabetically.
 *****************************************************************/
void IList_Sort (
    tIList * l,
    tIListSortOrder SortOrder)
{
	qsort (IList_TheList(l), IList_NumEntries(l), sizeof (l->l[0]),
        ((SortOrder == eIListSortAscending)
            ? (ISortAscending)
            : (ISortDescending)));
}
/*
Compares two items
*/
static int ISortAscending (
    const void *pa,
    const void *pb)
{
	int a = *(tIListType *)pa;
	int b = *(tIListType *)pb;
	if (a < b)
		return -1;
	else if (a > b)
		return 1;
	return 0;
}
/*
Compares two items
*/
static int ISortDescending (
    const void *pa,
    const void *pb)
{
	int a = *(tIListType *)pa;
	int b = *(tIListType *)pb;
	if (a < b)
		return 1;
	else if (a > b)
		return -1;
	return 0;
}
