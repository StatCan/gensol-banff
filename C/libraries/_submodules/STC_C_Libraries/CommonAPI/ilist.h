#ifndef _ILIST_H_
#define _ILIST_H_

typedef int tIListType;

typedef enum tIListReturn {
    eIListFail = (-1),
    eIListSucceed = 0
} tIListReturn;

#define ILIST_NOTFOUND (-1)

typedef struct tIList {
    int ne;   /* number of entries */
    int ae;   /* allocated entries */
    tIListType * l; /* the list of items */
} tIList;

#define IList_TheList(list)     ((list)->l)
#define IList_NumEntries(list)  ((list)->ne)
#define IList_Entry(list, i)    ((list)->l[(i)])

typedef enum _tIListSortOrder {
    eIListSortAscending, /* sort from 0 to 9 */
    eIListSortDescending /* sort from 9 to 0 */
} tIListSortOrder;

CLASS_DECLSPEC int IList_Add (tIListType Item, tIList * List);
extern int IList_AddList (tIList * SourceList, tIList * DestinationList);
extern int IList_AddListNoDup (tIList * SourceList, tIList * DestinationList);
extern int IList_AddNoDup (tIListType Item, tIList * List);
extern int IList_Dup (tIList * SourceList, tIList ** DestinationList);
extern void IList_Empty (tIList * List);
extern int IList_Equal (tIList * l1, tIList * l2);
CLASS_DECLSPEC void IList_Free (tIList * List);
extern void IList_Print (tIList * List);
extern int DBG_IListPrint(char * dbgscrfn, char * dbgscratch, tIList * list);
extern int IList_Intersect (tIList * l1, tIList * l2);
CLASS_DECLSPEC int IList_New (tIList ** List);
extern void IList_Remove (tIList * List, int Index);
extern void IList_Sort (tIList * List, tIListSortOrder SortOrder);
extern int IList_ItemCount (tIList * List, tIListType Item);
extern int IList_Search (tIList * List, tIListType Item);
extern int IList_SearchBinary (tIList * List, tIListType Item);
#endif
