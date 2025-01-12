#ifndef CD_H
#define CD_H

#include "EI_Common.h"

typedef struct CDT_CURRENTDATA {
    char ** Id;
    double * Current; // weighted or unweighted
    double * Weight; // allocated when necessary only
	double * UnweightedCurrent; // allocated when necessary only
    int NumberAllocatedEntries;
    int NumberEntries;
    EIT_BOOLEAN IsWeighted;
} CDT_CURRENTDATA;


/*
Add item at end of list / Ajoute un élément a la fin de la liste.

Add item at end of list.

Ajoute un élément a la fin de la liste.
*/
extern CDT_CURRENTDATA * CD_Add (
		/* A list.

        Une liste. */
	CDT_CURRENTDATA * CD,
		/* An id.

        Une clé. */
	char * Id,
		/* A value.

        Une valeur. */
    double Value,
	double Weight
);

/*
Create a list / Crée une liste.

Allocate and initialize the list.

Alloue et initialise la liste.
*/
extern CDT_CURRENTDATA * CD_Alloc (EIT_BOOLEAN IsWeighted);

/*
Destroy a list / Detruit une liste.

Empty and deallocate the list.

Vide et libere la memoire de la liste.
*/
extern void CD_Free (
		/* A list.

        Une liste. */
	CDT_CURRENTDATA * CD
);

extern void CD_Print (CDT_CURRENTDATA * CD);
extern void CD_PrintInfo (CDT_CURRENTDATA * CD);

/*
Sort the list / Trier la liste.

Sort the list.

Trier la liste.
*/
extern void CD_Sort (
		/* A list.

        Une liste. */
	CDT_CURRENTDATA * CD
);

#endif
