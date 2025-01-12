/*
slist.c

String list.

These functions helps to manage a list of strings.

STOLEN FROM DC2.
and then
STOLEN FROM SICCARR.
do you see a pattern?
*/

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Message.h"
#include "STC_Memory.h"
#include "slist.h"

/**********************************************************************/
/*                     DEFINE                                         */
/**********************************************************************/
#define ALLOCINCREMENT 20

/**********************************************************************/
/*                     PUBLIC FUNCTIONS                               */
/**********************************************************************/

/**********************************************************************/
/*                     PRIVATE FUNCTIONS                              */
/**********************************************************************/
static int SortAscending  (const void *a, const void *b);
static int SortDescending (const void *a, const void *b);

/*********************************************************************
 * Description: Adds item to list
 ********************************************************************/
int SList_Add (
    char *item,
    tSList *list)
{
    void * ptr;

    if (list->ne == list->ae) {   /* need more space ... */
        list->ae += ALLOCINCREMENT;
        ptr = STC_ReallocateMemory (
            sizeof *list->l * (list->ae-ALLOCINCREMENT+1),
            sizeof *list->l * (list->ae + 1),
            list->l);
        if (ptr == NULL) {
            SList_Free (list);
            return eSListFail;
        }
        list->l = ptr;
    }

    list->l[list->ne] = STC_StrDup (item);
	if (list->l[list->ne] == NULL) {
		return eSListFail;
	}

    list->ne = list->ne + 1;
    list->l[list->ne] = NULL;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Adds all items of a source list to a destination list
 ********************************************************************/
int SList_AddList (
    tSList *s,
    tSList *d)
{
    int i;

    for (i = 0; i < SList_NumEntries (s); i++)
        if (SList_Add (SList_Entry (s, i), d) == eSListFail)
            return eSListFail;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Adds items, if not already in, of a source list to a destination list
 ********************************************************************/
int SList_AddListNoDup (
    tSList *s,
    tSList *d)
{
    int i;

    for (i = 0; i < SList_NumEntries (s); i++)
        if (SList_AddNoDup (SList_Entry (s, i), d) == eSListFail)
            return eSListFail;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Adds item to list if not already in
 ********************************************************************/
int SList_AddNoDup (
    char *item,
    tSList *list)
{
    /* We do not add the same element twice */
    if (SList_StringSearch (list, item) == SLIST_NOTFOUND)
        if (SList_Add (item, list) == eSListFail)
            return eSListFail;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Creates a copy of the list
 ********************************************************************/
int SList_Dup (
    tSList * src,
    tSList ** dest)
{
    int i;

    if (SList_New (dest) == eSListFail)
        return eSListFail;

    for (i = 0; i < SList_NumEntries (src); i++)
        if (SList_Add (SList_Entry (src, i), *dest) == eSListFail)
            return eSListFail;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Empties the list
 ********************************************************************/
void SList_Empty (
    tSList *list)
{
    int i;

    for (i = 0; i < SList_NumEntries(list); i++)
        STC_FreeMemory (SList_Entry(list, i));

    list->l[0] = NULL;
    SList_NumEntries(list) = 0;
    return;
}
/*********************************************************************
Checks if 2 lists are the same.
returns 1 if they are equal
otherwise, 0.
*********************************************************************/
int SList_Equal (
    tSList * l1,
    tSList * l2)
{
    int i;

	if (l1->ne != l2->ne) return 0;

    for (i = 0; i < l1->ne; i++) {
		if (strcmp (SList_Entry(l1, i), SList_Entry(l2, i)) != 0) {
            return 0;
        }
    }
    return 1;
}
/*********************************************************************
 * Description: Frees the list
 ********************************************************************/
void SList_Free (
    tSList *list)
{
    SList_Empty (list);
    STC_FreeMemory (list->l);
    STC_FreeMemory (list);
    return;
}
/*********************************************************************
Checks if 2 lists share items.
returns the position of the first item of l1 in l2
otherwise, SLIST_NOTFOUND if the lists do not intersect.
*********************************************************************/
int SList_Intersect (
    tSList * l1,
    tSList * l2)
{
    int i;
    char ** list;

    list = SList_TheList (l1);

    if (list != NULL) {
        for (i = 0; list[i]; i++) {
            if (SList_StringSearch (l2, list[i]) != SLIST_NOTFOUND)
                return i;
        }
    }
    return SLIST_NOTFOUND;
}
/*********************************************************************
Checks if 2 lists share items.
returns the position of an item of l1 in l2
otherwise, SLIST_NOTFOUND if the lists do not intersect.
*********************************************************************/
int SList_IntersectBinary (
    tSList * l1,
    tSList * l2)
{
    int i;
    char ** list;

    list = SList_TheList (l1);

    if (list != NULL) {
        for (i = 0; list[i]; i++) {
            if (SList_StringSearchBinary (l2, list[i]) != SLIST_NOTFOUND)
                return i;
        }
    }
    return SLIST_NOTFOUND;
}
/*********************************************************************
 * Description: Allocates a list
 ********************************************************************/
int SList_New (
    tSList ** NewList)
{
    tSList *list;

    *NewList = NULL;
    list = STC_AllocateMemory (sizeof *list);
    if (list == NULL)
        return eSListFail;

    list->l = STC_AllocateMemory (sizeof *list->l * (ALLOCINCREMENT+1));
    if (list->l == NULL) {
        STC_FreeMemory (list);
        return eSListFail;
    }
    *NewList = list;

    list->ne = 0;
    list->ae = ALLOCINCREMENT;
    list->l[0] = NULL;

    return eSListSucceed;
}

/*********************************************************************
 * Description: Prints all items of the list
 ********************************************************************/
void SList_Print (
    tSList * list)
{
    int i;

    for (i = 0; i < SList_NumEntries (list); i++)
        EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY , "%s\n", SList_Entry(list, i));
}
#ifdef DBG_ON
int DBG_SListPrint(char * dbgscrfn, char * dbgscratch, tSList * list)
{
    /*-members of instances of "tSList" are:
       ne  (number of entries--type is                     int   )
       ae  (allocated entries--type is                     int   )
       l   (the list of items--type is ne-element array of char *)
    */
    int i;
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "{");
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'ne': %d, ", list->ne);
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'ae': %d, ", list->ae);
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'l': ["                                                        );
    for (i=0;i<list->ne;i=i+1)
    {
        /* if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "(extra for debugging: list, (list->l), i, (list->l)[i]: '%d, %d, %d, \"%s\"')", list, (list->l), i, (list->l)[i]); */
        if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "'%s', ", list->l[i]);
    }
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "],"                                                              );
    if(strnlen(dbgscratch,DBGBUFSIZE) > DBGBUFHALF){DBG_dtf(dbgscrfn, dbgscratch);DBG_cdbs(dbgscratch);};sprintf((dbgscratch+strnlen(dbgscratch,DBGBUFSIZE)), "}");
    
    return 0;
};
#endif
/*********************************************************************
 * Description: Removes an item at position 'position' from the list
 ********************************************************************/
void SList_Remove (
    tSList *list,
    int position)
{
    int i;

    STC_FreeMemory (list->l[position]);
    for (i = position; i < SList_NumEntries(list); i++)
        list->l[i] = list->l[i+1];

    list->ne--;
}

/*********************************************************************
Counts the number of Item matching a string in the list.
returns the number found
*********************************************************************/
int SList_StringCount (
    tSList * l,
    char * String)
{
    int Count = 0;
    int i;
    char ** list;

    list = SList_TheList (l);

    if (list != NULL) {
        for (i = 0; list[i]; i++) {
            if (strcmp(list[i], String) == 0)  /* match */
                Count++;
        }
    }
    return Count;
}

/*****************************************************************
 * Description:    Searches the NULL terminated list of strings for listItem.
 * SearchList returns the index where it is found or SLIST_NOTFOUND.
 *****************************************************************/
int SList_StringSearch (
    tSList * l,
    char * listItem)
{
    int i = 0;
    char ** list;

    list = SList_TheList (l);

    if (list == NULL)
        return SLIST_NOTFOUND;

    while (list[i]) {
        if (strcmp(list[i], listItem) == 0)  /* match */
            return i;
        i++;
    }

    return SLIST_NOTFOUND;
}

/*****************************************************************
 * Description:    Searches the NULL terminated list of strings for listItem.
 * SearchList returns the index where it is found or SLIST_NOTFOUND.
 *****************************************************************/
int SList_StringSearchBinary (
    tSList * l,
    char * listItem)
{
    char ** list;
	int low, high, mid;
	int rc;

    list = SList_TheList (l);

    if (list == NULL)
        return SLIST_NOTFOUND;

	//EI_AddMessage ("", 4, "looking for %d", listItem);
	low = 0;
	high = l->ne-1;
	while (low <= high) {
		mid = (low + high) / 2;
		//EI_AddMessage ("", 4, "  compare with looking for %d", list[mid]);
		rc = strcmp (listItem, list[mid]);
		if (rc == 0)
			return mid;
		else if (rc > 0)
			low = mid + 1;
		else
			high = mid - 1;
	}

    return SLIST_NOTFOUND;
}
/*****************************************************************
 * Description:    Sorts the list alphabetically.
 *****************************************************************/
void SList_Sort (
    tSList * l,
    tSListSortOrder SortOrder)
{
/*    int DebugPrint = 0;*/
/*    char * prevlocalenameptr;*/
/*    char * localenameptr;*/

/*    prevlocalenameptr = setlocale (LC_COLLATE, NULL);*/
/*    if (DebugPrint) fprintf (TRACE_FILE, "prev=%s\n", prevlocalenameptr);*/

    /*
    Use french collating order. An accented characters is considered
    equal to the same character without accent
    */
/*    localenameptr = setlocale (LC_COLLATE, "french_canada");*/
/*    if (DebugPrint) fprintf (TRACE_FILE, "new=%s\n", localenameptr);*/

    qsort (SList_TheList(l), SList_NumEntries(l), sizeof ((l->l)[0]),
        ((SortOrder == eSListSortAscending)
            ? (SortAscending)
            : (SortDescending)));

/*    localenameptr = setlocale (LC_COLLATE, prevlocalenameptr);*/
/*    if (DebugPrint) fprintf (TRACE_FILE, "reset=%s\n", localenameptr);*/
}

/*
Locale function
*/

/*
Compares two strings
*/
static int SortAscending (
    const void *a,
    const void *b)
{
    /* strcoll() is like strcmp(), but it uses locale information */
/*    return strcoll (*(char **)a, *(char **)b);*/
    return strcmp (*(char **)a, *(char **)b);
}

/*
Compares two strings
*/
static int SortDescending (
    const void *a,
    const void *b)
{
    /* strcoll() is like strcmp(), but it uses locale information */
/*    return strcoll (*(char **)b, *(char **)a);*/
    return strcmp (*(char **)b, *(char **)a);
}
