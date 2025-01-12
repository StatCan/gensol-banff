/*
adtlist.c

Abstract Data Type LIST.

These functions helps to manage a list of anything (list of structure, pointer to structure, list of lists).

STOLEN FROM DC2.
and then
STOLEN FROM SICCARR.
do you see a pattern?
*/

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>

#include "adtlist.h"
#include "EI_Message.h"
#include "STC_Memory.h"

#define INDEX_LIST(l,index) (((l)->listItems) + ((index) * (l)->itemSize))

typedef struct {
    int  (*onAllocItem)(void *);
    int  (*onFreeItem)(void *);
    char  *insert_item;
    char  *scratch_item;
} tRecyclicInfo;


/*
 ***======================***
 ***  PRIVATE PROTOTYPES: ***
 ***======================***
 */

                        static tADTList *
BasicInsertListItem     (tADTList *, char *, int);

						static tADTList *
BasicRemoveListItem		(tADTList *, int);

						static int
CmpStub					(const void *, const void *);

						static tADTList *
InsertListItem			(tADTList *, char *, int);

						static void
RecyclicFreeListItems	(tADTList *);

						static tADTList *
RecyclicInsertListItem	(tADTList *, char *, int);

						static tADTList *
RecyclicRemoveListItem	(tADTList *, int);

						static tADTList *
ResizeListArray			(tADTList *);




/*
 ***==========================***
 ***  PUBLIC FUNCTIONS: list: ***
 ***==========================***
 */

/*************************************************************************
 *  Description: add "item" to end of "list".
 *************************************************************************/

tADTList *
ADTList_Append (tADTList *list, char *item)
{
    return InsertListItem (list, item, list->itemsUsed);
}


/*************************************************************************
 *  Description: use BINARY search to find item in "list" which matches
 *               "key" using the keyCmp function.
 *************************************************************************/

char *
ADTList_BSearch (tADTList *list, char *key)
{
    return ((char *) bsearch (key, list->listItems,
        list->itemsUsed, list->itemSize, list->keyCmp));
}


/*************************************************************************
 *  Description: concatenate "list2" to the end of "list1"
 *************************************************************************/

tADTList *
ADTList_Concat (tADTList *list1, tADTList *list2)
{
    int i;

    for (i = 0; i < ADTList_Length (list2); i++)
        ADTList_Append (list1, ADTList_Index (list2, i));

    return list1;
}


/*************************************************************************
 *  Description: copy "list2" over "list1" (which must exist).
 *************************************************************************/

tADTList *
ADTList_Copy (tADTList *list1, tADTList *list2)
{
    list1->itemsUsed = 0;

    return ADTList_Concat (list1, list2);
}


/*************************************************************************
 *  Description: create (allocate and initialize) a list.
 *************************************************************************/

tADTList *
ADTList_Create (
    tADTListCompareProc itemCmp, /* Function used by sort the list */
    tADTListCompareProc keyCmp,  /* Function used by search for an item of the list */
    int itemSize,
    int allocSize)
{
    tADTList *newList;

    newList = STC_AllocateMemory (sizeof *newList);

    if (newList != NULL) {
        if (itemCmp != NULL)
            newList->itemCmp = itemCmp;
        else
            newList->itemCmp = CmpStub;

        if (keyCmp != NULL)
            newList->keyCmp  = keyCmp;
        else
            newList->keyCmp  = CmpStub;

        newList->itemSize  = itemSize;
        newList->itemsUsed = 0;

		if (allocSize > 0)
	    	newList->allocSize = allocSize;
		else
	    	newList->allocSize = 1;

        newList->listItems = NULL;
        newList->itemCount = 0;
		newList->type      = eBasic;
		newList->opaque    = NULL;
    }

    return newList;
}


/*************************************************************************
 *  Description: delete (destroy, free) "list".
 *************************************************************************/

void
ADTList_Delete (tADTList *list)
{

	switch (list->type) {
	case eRecyclic:
		RecyclicFreeListItems (list);
		break;

	case eBasic:
	default:
    	if (list->itemCount > 0)
        	STC_FreeMemory (list->listItems);
		break;
	}

    list->itemCmp   = NULL;
    list->keyCmp    = NULL;
    list->listItems = NULL;
    list->itemSize  = 0;
    list->itemCount = 0;
    list->itemsUsed = 0;

    STC_FreeMemory (list);
}


/*************************************************************************
 *  Description: return a duplicate (newly created) copy of list "l".
 *************************************************************************/

tADTList *
ADTList_Dup (tADTList *l)
{
	tADTList *newList;

	newList = ADTList_Create (l->itemCmp, l->keyCmp, l->itemSize, l->itemsUsed);
	if (newList == NULL)
		return NULL;

    return ADTList_Copy (newList, l);
}


/*************************************************************************
 *  Description: remove all items from "list" (effectively but not actually).
 *************************************************************************/

void
ADTList_Empty (tADTList *list)
{
    list->itemsUsed = 0;
}


/*************************************************************************
 *  Description: return the list item at position "index" in "list".
 *************************************************************************/

char *
ADTList_Index (tADTList *list, int index)
{
    if ((index >= 0) && (index < list->itemsUsed))
        return (char *) INDEX_LIST (list, index);
    else
        return NULL;
}


/*************************************************************************
 *  Description: insert "item" into "list" in position "index".
 *
 *  Note: when index == 0,               this is identical to ADTList_prepend;
 *        when index == list->itemsUsed, this is identical to ADTList_append.
 *************************************************************************/

tADTList *
ADTList_Insert (tADTList *list, char *item, int index)
{
	/* Only insert if index is valid */
	if ((index >= 0) && (index <= list->itemsUsed))
		list = InsertListItem (list, item, index);

    return list;
}


/*************************************************************************
 *  Description: set "itemCmp" as the itemCmp function for "list".
 *************************************************************************/

void *
ADTList_ItemCmp (tADTList *list, tADTListCompareProc itemCmp)
{
	tADTListCompareProc oldCmp;
    oldCmp = list->itemCmp;
    list->itemCmp = itemCmp;
    return (void *) oldCmp;
}


/*************************************************************************
 *  Description: set "keyCmp" as the keyCmp function for "list".
 *************************************************************************/

void *
ADTList_KeyCmp (tADTList *list, tADTListCompareProc keyCmp)
{
	tADTListCompareProc	oldCmp;
	oldCmp = list->keyCmp;
	list->keyCmp = keyCmp;
	return (void *) oldCmp;
}


/*************************************************************************
 *  Description: return the number of items in "list".
 *************************************************************************/

int
ADTList_Length (tADTList *list)
{
    return list->itemsUsed;
}


/*************************************************************************
 *  Description: use LINEAR search to find the index of the ITEM in "list"
 *               matching "key" using the keyCmp function.
 *************************************************************************/

int
ADTList_Locate (tADTList *list, char *key)
{
    int i;

    for (i = 0; i < list->itemsUsed; i++)
        if ((list->keyCmp) (key, INDEX_LIST (list, i)) == 0)
            return i;

    return ADTLIST_NOTFOUND;
}


/*************************************************************************
 *  Description: use LINEAR search to find the INDEX of the item in "list"
 *               matching "key" using the keyCmp function.
 *************************************************************************/

char *
ADTList_LSearch (tADTList *list, char *key)
{
    int i;

    for (i = 0; i < list->itemsUsed; i++)
        if ((list->keyCmp) (key, INDEX_LIST (list, i)) == 0)
            return INDEX_LIST (list, i);

    return NULL;
}


/*************************************************************************
 *  Description: apply foo to each item of the list, starting with the item
 *               at "first_index" and including all items whose index is less
 *               than "limit".  foo must return 1.
 *
 *               Note that any physically allocated item is accessible.
 *
 *  Result:  1 on success or -1 on failure
 *************************************************************************/
int
ADTList_Massage (tADTList *list, int (*foo)(void *), int first_index, int limit)
{
    int i;

	if (first_index >= 0 && limit <= list->itemCount)
	    for (i = first_index; i < limit; i++)
	        if ((foo)(INDEX_LIST (list, i)) != 1)
	        	return -1;

    return 1;
}


/*************************************************************************
 *  Description: add "item" to the beginning of "list".
 *************************************************************************/

tADTList *
ADTList_Prepend (tADTList *list, char *item)
{
	return InsertListItem (list, item, 0);
}


/*************************************************************************
 *  Description: sort "list" using itemCmp function.
 *************************************************************************/

tADTList *
ADTList_QSort (tADTList *list)
{
    qsort (list->listItems, list->itemsUsed, list->itemSize, list->itemCmp);
    return list;
}


/*************************************************************************
 *  Description: remove the item in the "index" position from "list".
 *************************************************************************/
tADTList *
ADTList_Remove (tADTList *list, int index)
{
	tADTList *t;

	switch (list->type) {
	case eRecyclic:
		t =  RecyclicRemoveListItem (list, index);
		break;

	case eBasic:
	default:
		t = BasicRemoveListItem (list, index);
		break;
	}

	return (t);
}




/*
 ***=============================***
 ***  PUBLIC FUNCTIONS: rclist:  ***
 ***=============================***
 */

/*************************************************************************
 *  PURPOSE:
 *      Convert a basic list into a recyclic list.
 *      Must be called immediately after the list was created.
 *      The onAllocItem is a function which will be called on each item
 *      immediately after it has been allocated (before it is used).
 *      The onFreeItem is a function which is called on each item immediately
 *      before the itemList is freed.
 *
 *      onAllocItem and onFreeItem must return 1.
 *
 *  RESULT:
 *      Returns the input list on succees, NULL on failure.
 *************************************************************************/
tADTList *
rclist_Initialize (tADTList *list, int (*onAllocItem) (void *),
	 int (*onFreeItem) (void *))
{
	tRecyclicInfo  *work_area;

	if (list->itemCount != 0 || list->opaque != NULL || onAllocItem == NULL ||
            onFreeItem == NULL)
		return NULL;

    work_area = STC_AllocateMemory (sizeof *work_area);
    if (work_area == NULL)
		return NULL;

	work_area->onAllocItem  = onAllocItem;
	work_area->onFreeItem   = onFreeItem;
	work_area->insert_item  = STC_AllocateMemory (list->itemSize);
	work_area->scratch_item = STC_AllocateMemory (list->itemSize);

	if (work_area->insert_item == NULL || work_area->scratch_item == NULL)
		return NULL;

	if ((work_area->onAllocItem) (work_area->insert_item) != 1)
		return NULL;

	list->type   = eRecyclic;
	list->opaque = (char *) work_area;

	return list;
}


/*******************************************************************************
 *  PURPOSE:
 *      Return the location of a fully allocated list item which must be used
 *      when inserting into the list.
 ******************************************************************************/
char *
rclist_GetUnusedItemStruct (tADTList *list)
{
	return (((tRecyclicInfo *) (list->opaque))->insert_item);
}





/*
 ***===============================***
 ***  PRIVATE FUNCTIONS: General:  ***
 ***===============================***
 */

/*************************************************************************
 *  Description: insert "item" into "list" in the "index" position.
 *************************************************************************/
static tADTList *
InsertListItem (tADTList *list, char *item, int index)
{
	tADTList *t;

	switch (list->type) {
	case eRecyclic:
		t = RecyclicInsertListItem (list, item, index);
		break;

	case eBasic:
	default:
		t = BasicInsertListItem (list, item, index);
		break;
	}

	return t;
}


/*************************************************************************
 *  Description: If no compare functions are specified this stub is used.
 *************************************************************************/
static int
CmpStub (const void *a, const void *b)
{
	EI_AddMessage ("", 4, "Hey Man!!! Define a compare function in the ADTList_Create call!\n");
	return 0;
}


/*************************************************************************
 *  Description:
 *************************************************************************/
static tADTList *
ResizeListArray (tADTList * list)
{
    void * ptr;

	list->itemCount += list->allocSize;

    ptr = STC_ReallocateMemory (
        list->itemSize * (list->itemCount-list->allocSize),
        list->itemSize * list->itemCount,
        list->listItems);
	if (ptr == NULL) {
        ADTList_Delete (list);
        return NULL;
    }
    list->listItems = ptr;

	return list;
}




/*
 ***===========================================***
 ***  PRIVATE FUNCTIONS: Basic List Mechanism: ***
 ***===========================================***
 */


/*************************************************************************
 *  Description: insert "item" into "list" in the "index" position.
 *************************************************************************/
static tADTList *
BasicInsertListItem (tADTList *list, char *item, int index)
{
    int i;

    if (list->itemsUsed == list->itemCount)
       if (ResizeListArray (list) == NULL)
           return NULL;

  	/* Shift down elements to make room for new element */
	for (i = list->itemsUsed; i > index; i--) {
		memcpy ((char *) INDEX_LIST (list, i), (char *) INDEX_LIST (list, i-1),
			list->itemSize);
	}

    /* Add new element */
    memcpy ((char *) INDEX_LIST (list, index), (char *) item, list->itemSize);

    list->itemsUsed++;

    return list;
}


/*************************************************************************
 *  Description: remove the item in the "index" position from "list".
 *************************************************************************/
static tADTList *
BasicRemoveListItem (tADTList *list, int index)
{
	int i;

    if ((index >= 0) && (index < list->itemsUsed)) {

		list->itemsUsed--;

    	/* Shift up elements */
    	for (i = index + 1; i <= list->itemsUsed; i++) {
        	memcpy ((char *) INDEX_LIST (list, i-1),
                (char *) INDEX_LIST (list, i),
                list->itemSize);
    	}
	}
    return list;
}




/*
 ***==============================================***
 ***  PRIVATE FUNCTIONS: Recyclic List Mechanism: ***
 ***==============================================***
 */

/*************************************************************************
 *  PURPOSE:
 *      To free the listItems array.
 *************************************************************************/
static void
RecyclicFreeListItems (tADTList *list)
{
	tRecyclicInfo *work_area;


	/*** Free the recyclic stuff:
	 *** - free extra allocations that were done in each item;
	 *** - free the recyclic work area.
	 ***------------------------------------------------------*/
	work_area = (tRecyclicInfo *) list->opaque;

	ADTList_Massage (list, work_area->onFreeItem, 0, list->itemCount);
	(work_area->onFreeItem) (work_area->insert_item);

	STC_FreeMemory (work_area->insert_item);
	STC_FreeMemory (work_area->scratch_item);
	work_area->onAllocItem = NULL;
	work_area->onFreeItem  = NULL;
	STC_FreeMemory (list->opaque);


	/*** Free the regular list items array.
	 ***-----------------------------------*/
	if (list->itemCount > 0)
  		STC_FreeMemory (list->listItems);
}


/*************************************************************************
 *  PURPOSE:
 *      Insert an item into the "index" position in the list.
 *      The item must actually be the opaque->insert_item.
 *
 *  RESULT:
 *      The list is returned on success, NULL on failure.
 *************************************************************************/
static tADTList *
RecyclicInsertListItem (tADTList *list, char *item, int index)
{
    int            i;
	tRecyclicInfo *work_area;


	work_area = (tRecyclicInfo *) list->opaque;

	if (item != work_area->insert_item)
		return NULL;

    if (list->itemsUsed == list->itemCount) {
		if (ResizeListArray (list) == NULL)
            return NULL;

		if (ADTList_Massage (list, work_area->onAllocItem, list->itemsUsed,
			 list->itemCount) != 1)
			return NULL;
    }


	/*** Preserve the item about to be overwritten;
     *** shift items down, as required to make room for the new item;
     *** move the item (= opaque->insert_item) into the slot;
	 *** move the preserved item to opaque->insert_item.
	 ***-------------------------------------------------------------*/
	memcpy (work_area->scratch_item, (char *) INDEX_LIST (list, list->itemsUsed),
           list->itemSize);

	for (i = list->itemsUsed; i > index; i--) {
		memcpy ((char *) INDEX_LIST (list, i), (char *) INDEX_LIST (list, i-1),
				list->itemSize);
	}

    memcpy ((char *) INDEX_LIST (list, index), (char *) item, list->itemSize);
	memcpy (work_area->insert_item, work_area->scratch_item, list->itemSize);


    list->itemsUsed++;

    return list;
}


/*************************************************************************
 *  PURPOSE:
 *      Remove an item from the "index" position of the list.
 *
 *  RESULT:
 *      The list is returned on success, NULL on failure.
 *************************************************************************/
static tADTList *
RecyclicRemoveListItem (tADTList *list, int index)
{
	int            i;
	tRecyclicInfo *work_area;

    if ((index < 0) || (index >= list->itemsUsed))
		return list;

	work_area = (tRecyclicInfo *) list->opaque;

	list->itemsUsed--;

	/*** Preserve the item about to be removed;
	 *** shift items up, as required;
	 *** move the preserved item to the slot vacated by the shift.
	 ***----------------------------------------------------------*/
	memcpy (work_area->scratch_item, (char *) INDEX_LIST (list, index),
	       list->itemSize);

   	for (i = index + 1; i <= list->itemsUsed; i++) {
       	memcpy ((char *) INDEX_LIST (list, i-1), (char *) INDEX_LIST (list, i),
                list->itemSize);
   	}

	memcpy ((char *) INDEX_LIST (list, list->itemsUsed),
	        work_area->scratch_item, list->itemSize);

    return list;
}
