#ifndef ADTLIST_H
#define ADTLIST_H

#define ADTLIST_NOTFOUND (-1)

typedef enum {
    eBasic,           /* simple items (default) */
    eRecyclic         /* complex items allocated and recycled by list */
} tADTListBaseType;

typedef int (*tADTListCompareProc)(const void *, const void *);

typedef struct _tADTList {
    int                itemCount;   /* # of items physically allocated */
    int                itemsUsed;   /* # of items in list */
    int                itemSize;    /* size of each item */
    int                allocSize;   /* # of items to add per reallocation */
    char              *listItems;   /* the allocated array of items */
    tADTListCompareProc itemCmp;    /* compare function used in sorting */
    tADTListCompareProc keyCmp;     /* compare function used in key searches */
    tADTListBaseType   type;        /* base list type */
    char              *opaque;      /* internal use by base list mechanism */
} tADTList;


/*
Add item at end of list / Ajoute un �l�ment a la fin de la liste.

Add item at end of list.

Ajoute un �l�ment a la fin de la liste.
*/
extern tADTList *ADTList_Append (
		/* A list

		Une liste. */
    tADTList * List,
		/* An item.

		Un �l�ment. */
	char * Item
);

/*
Search an item using a binary search / Recherche dichotomique d'un �l�ment.

Search an item using a binary search. The list must be sorted. It uses the
KeyCmp function to compare the key.  Returns the item or NULL if not found.

Recherche dichotomique d'un �l�ment. La liste doit etre trie. La fonction
KeyCmp est utilis� pour compar� la cl�. L'�l�ment trouv� est retourn� ou NULL
sinon.
*/
extern char *ADTList_BSearch (
		/* A list

		Une liste. */
    tADTList * List,
		/* Pointer to a key.

		Pointeur a une cl�. */
	char * Key
);

/*
Add items of a list to another one / Ajoute les �l�ments d'une liste a une autre.

Add the items of SourceList to the DestinationList.

Ajoute les �l�ments de SourceList � DestinationList.
*/
extern tADTList *ADTList_Concat (
		/* A list

		Une liste. */
	tADTList * DestinationList,
		/* A list

		Une liste. */
	tADTList * SourceList
);

/*
Copy items of a list to another list / Copie les �l�ments d'une liste a une autre.

Replace items of DestinationList by those of SourceList. After calling
ADTList_Copy() both lists are identical.

Remplace le contenue de DestinationList par celui de SourceList.
Les deux listes sont identiques apres l'appelle a ADTList_Copy().
*/
extern tADTList *ADTList_Copy (
		/* A list

		Une liste. */
	tADTList * DestinationList,
		/* A list

		Une liste. */
	tADTList * SourceList
);

/*
Create a list / Cr�e une liste.

Allocate and initialize the list.

Alloue et initialise la liste.
*/
extern tADTList *ADTList_Create (
		/* Sort function.

		fonction d'ordonancement */
	tADTListCompareProc ItemCmp,
		/* Search function.

		Fonction de recherche d'�l�ment. */
	tADTListCompareProc KeyCmp,
		/* List item size.

		Grosseur d'un �l�ment de la liste.*/
    int ItemSize,
		/* Size of (re)allocation of the list.

		Nombre d'�l�ment a (r�)allouer � la liste. */
    int AllocationSize
);

/* 
Destroy a list / Detruit une liste.

Empty and deallocate the list. If the items of the list points to memory
space that was allocate by the user, it's the user responsability to free
that space.

Vide et libere la memoire de la liste. Si les �l�ments de la liste refere
a de la memoire allou� dynamiquement par l'utilisateur, celle ci doit etre
libere manuellement.
*/
extern void ADTList_Delete (
		/* A list

		Une liste. */
    tADTList * List
);

/*
Duplicates a list / Cr�e une liste identique a la liste donnee.

Duplicates a list.

Cr�e une liste identique a la liste donnee.
*/
extern tADTList *ADTList_Dup (
		/* A list

		Une liste. */
    tADTList * List
);

/*
Remove all items from list / Enleve tous les �l�ments de la listes.

Remove all items from list.

Enleve tous les �l�ments de la listes.
*/
extern void ADTList_Empty (
		/* A list

		Une liste. */
    tADTList * List
);

/*
Return item at position index / Retourne l'�l�ment a la position demand�.

Return item at position index.

Retourne l'�l�ment a la position demand�.
*/
extern char *ADTList_Index (
		/* A list

		Une liste. */
    tADTList * List,
		/* A position.

		Une position. */
	int Index
);

/*
Add item in list at a specified position / Ajoute un �l�ment dans la liste a une position donnee.

Add item in list at a specified position.

Ajoute un �l�ment dans la liste a une position donnee.
*/
extern tADTList *ADTList_Insert (
		/* A list

		Une liste. */
    tADTList * List,
		/* An item.

		Un �l�ment. */
	char * Item,
		/* A position.

		Une position. */
	int Index
);

/*
Replace ItemCmp() function / Remplace la fonction ItemCmp()

Replace ItemCmp() function. Returns the old one.

Remplace la fonction ItemCmp(). Retourne l'ancienne.
*/
extern void *ADTList_ItemCmp (
		/* A list

		Une liste. */
	tADTList * List,
		/* Sort function.

		fonction d'ordonancement */
	tADTListCompareProc ItemCmp
);

/*
Replace KeyCmp() function / Remplace la fonction KeyCmp()

Replace KeyCmp() function. Returns the old one.

Remplace la fonction KeyCmp(). Retourne l'ancienne.
*/
extern void *ADTList_KeyCmp (
		/* A list

		Une liste. */
	tADTList * List,
		/* Search function.

		Fonction de recherche d'�l�ment. */
	tADTListCompareProc KeyCmp
);

/*
Return the number of items / Retourne le nombre d'�l�ments de la liste.

Return the number of items.

Retourne le nombre d'�l�ments de la liste.
*/
extern int ADTList_Length (
		/* A list

		Une liste. */
    tADTList * List
);

/*
Search an item using a linear search / Recherche s�quentielle d'un �l�ment.

Search an item using a linear search. It uses the KeyCmp() function to compare
the key.  Returns the index of the item in the list or ADTLIST_NOTFOUND if
not found.

Recherche sequentielle d'un �l�ment. La fonction KeyCmp() est utilis� pour compar�
la cl�. La position de l'�l�ment trouv� est retourn� ou ADTLIST_NOTFOUND sinon.
*/
extern int ADTList_Locate (
		/* A list

		Une liste. */
    tADTList * List,
		/* Pointer to a key.

		Pointeur a une cl�. */
	char * Key
);

/*
Search an item using a linear search / Recherche s�quentielle d'un �l�ment.

Search an item using a linear search. It uses the KeyCmp function to compare
the key.  Returns the item or NULL if not found.

Recherche s�quentielle d'un �l�ment. La fonction KeyCmp est utilis� pour
compar� la cl�. L'�l�ment trouv� est retourn� ou NULL sinon.
*/
extern char *ADTList_LSearch (
		/* A list

		Une liste. */
    tADTList * List,
        /* Pointer to a key.

        Pointeur a une cl�. */
	char * Key
);

/*
Apply a function to each item of the list / Applique une fonction a chaque �l�ment de la liste.

Apply a function to each item of the list.

Applique une fonction a chaque �l�ment de la liste.
*/
extern int ADTList_Massage (
		/* A list

		Une liste. */
	tADTList * List,
		/* A function. Must return 1.

		Une fonction. Cette fonction doit retourner 1. */
	int (*Function)(void *),
		/* Index of first item to be massaged.

		Position du premier �l�ment a traiter.*/
	int StartIndex,
		/* Massaged all items with index less than StopIndex

		Traiter les �l�ments dont la position est inf�rieure a StopIndex*/
	int StopIndex
);

/*
Add item at beginning of list / Ajoute un �l�ment au debut de la liste.

Add item at beginning of list.

Ajoute un �l�ment au debut de la liste.
*/
extern tADTList *ADTList_Prepend (
		/* A list

		Une liste. */
    tADTList * List,
		/* An item.

		Un �l�ment. */
	char * Item
);

/*
Sort the list / Trier la liste.

Sort the list(). ItemCmp() function will be use to compare list items.

Trier la liste en utilisant la fonction ItemCmp() pour comparer les �l�ments
entre eux.
*/
extern tADTList *ADTList_QSort (
		/* A list

		Une liste. */
	tADTList * List
);

/*
Remove item at a specified position from list / Enleve l'un �l�ment de la position donnee de la liste.

Remove item at a specified position from list.

Enleve l'un �l�ment de la position donnee de la liste.
*/
extern tADTList *ADTList_Remove (
		/* A list

		Une liste. */
    tADTList * List,
		/* A position.

		Une position. */
	int Index
);

#ifdef NEVERTOBEDEFINED
/* what the hey is this? */
extern tADTList *rclist_Initialize (
	tADTList *,
	int (*)(void *),
    int (*)(void *)
);
/* what the hey is this? */
extern char *rclist_GetUnusedItemStruct (
	tADTList *
);
#endif

#endif
