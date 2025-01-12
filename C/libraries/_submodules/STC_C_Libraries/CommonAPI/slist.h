#ifndef SLIST_H
#define SLIST_H

#include "EI_Common.h"

typedef enum ttSListReturn {
    /* - Unrecoverable errors.
       - Erreur catastrophique. */
    eSListFail = (-1),
    /* - The function terminated with success.
       - La fonction a complété avec succès. */
    eSListSucceed = 0
} tSListReturn;

#define SLIST_NOTFOUND (-1)

typedef struct _tSList {
    int ne;   /* number of entries */
    int ae;   /* allocated entries */
    char **l; /* the list of Strings */
} tSList;

#define SList_TheList(list)     ((list)->l)
#define SList_NumEntries(list)  ((list)->ne)
#define SList_Entry(list,i)     ((list)->l[(i)])

typedef enum _tSListSortOrder {
    eSListSortAscending, /* sort from A to Z */
    eSListSortDescending /* sort from Z to A */
} tSListSortOrder;


/* Public functions */

/*
Add item at end of list / Ajoute un élément a la fin de la liste.

Add item at end of list.

Ajoute un élément a la fin de la liste.
*/
CLASS_DECLSPEC int SList_Add (
		/* A string.

		Une string. */
	char * Item,
        /* A list

        Une liste. */
	tSList * List
);

/*
Add items of a list to another one / Ajoute les éléments d'une liste a une autre.

Add the items of SourceList to the DestinationList.

Ajoute les éléments de SourceList à DestinationList.
*/
CLASS_DECLSPEC int SList_AddList (
        /* A list

        Une liste. */
	tSList * SourceList,
        /* A list

        Une liste. */
	tSList * DestinationList
);

/*
Add items of a list to another one no duplicate allowed / Ajoute les éléments d'une liste a une autre sans duplicat.

Add the items of SourceList to the DestinationList.

Ajoute les éléments de SourceList à DestinationList. Les éléments déjà présent ne sont pas ajoutés.
*/
CLASS_DECLSPEC int SList_AddListNoDup (
        /* A list

        Une liste. */
	tSList * SourceList,
        /* A list

        Une liste. */
	tSList * DestinationList
);

/*
Add item to list no duplicate allowed / Ajoute un élément a la liste sans duplicat.

Add item at end of list dulpicate are not added.

Ajoute un élément a la fin de la liste. Les éléments déjà présent ne sont pas ajoutés.
*/
CLASS_DECLSPEC int SList_AddNoDup (
		/* A string.

		Une string. */
	char * Item,
        /* A list

        Une liste. */
	tSList * List
);

/*
Duplicates a list / Crée une liste identique a la liste donnee.

Duplicates a list.

Crée une liste identique a la liste donnee.
*/
CLASS_DECLSPEC int SList_Dup (
        /* A list

        Une liste. */
	tSList * SourceList,
        /* A list

        Une liste. */
	tSList ** DestinationList
);

/*
Remove all items from list / Enleve tous les éléments de la listes.

Remove all items from list.

Enleve tous les éléments de la listes.
*/
CLASS_DECLSPEC void SList_Empty (
        /* A list

        Une liste. */
	tSList * List
);
CLASS_DECLSPEC int SList_Equal (tSList * l1, tSList * l2);
/*
Destroy a list / Detruit une liste.

Empty and deallocate the list.

Vide et libere la memoire de la liste.
*/
CLASS_DECLSPEC void SList_Free (
        /* A list

        Une liste. */
	tSList * List
);

/*
Print the list items / Affiche les elements de la liste.

Print the list items to stdout.

Affiche sur stdout les elements de la liste.
*/
CLASS_DECLSPEC void SList_Print (
        /* A list

        Une liste. */
	tSList * List
);

extern int DBG_SListPrint(char * dbgscrfn, char * dbgscratch, tSList * list);

/*
Checks if 2 lists share items.
returns the position of the first item of l1 in l2
otherwise, SLIST_NOTFOUND if the lists do not intersect.
*/
CLASS_DECLSPEC int SList_Intersect (
        /* A list

        Une liste. */
    tSList * l1,
        /* A second list

        Une seconde liste. */
    tSList * l2
);

CLASS_DECLSPEC int SList_IntersectBinary (tSList * l1, tSList * l2);

/*
Create a list / Crée une liste.

Allocate and initialize the list.

Alloue et initialise la liste.
*/
CLASS_DECLSPEC int SList_New (
        /* A list

        Une liste. */
	tSList ** List
);

/*
Remove item at a specified position from list / Enleve l'un élément de la position donnee de la liste.

Remove item at a specified position from list.

Enleve l'un élément de la position donnee de la liste.
*/
CLASS_DECLSPEC void SList_Remove (
        /* A list

        Une liste. */
	tSList * List,
		/* A position.

        Une position. */
	int Index
);

/*
Sort the list / Trier la liste.

Sort the list.

Trier la liste.
*/
CLASS_DECLSPEC void SList_Sort (
        /* A list

        Une liste. */
	tSList * List,
		/* Order of sort.

		Ordre du trie. */
	tSListSortOrder SortOrder
);

/*
Counts the number of Item matching a string in the list.
returns the number found.
*/
CLASS_DECLSPEC int SList_StringCount (
        /* A list

        Une liste. */
    tSList * List,
        /* A string
    
        Une chaine de caracteres. */
    char * String
);

/*
Search an item using a linear search / Recherche séquentielle d'un élément.

Search an item using a linear search. Returns the index of the item in the
list or SLIST_NOTFOUND if not found.

Recherche sequentielle d'un élément. La position de l'élément trouvé est
retourné ou SLIST_NOTFOUND sinon.
*/
CLASS_DECLSPEC int SList_StringSearch (
        /* A list

        Une liste. */
	tSList * List,
		/* A string.

		Une string. */
	char * Item
);

CLASS_DECLSPEC int SList_StringSearchBinary (tSList * l, char * listItem);

#endif
