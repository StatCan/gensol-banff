#ifndef HT_H
#define HT_H

#include "EI_Common.h"

typedef struct HTT_HISTORICALTREND {
    char ** Id;
    double * Current;
    double * Previous;
	double * Weight; // allocated when necessary only
	double * Ratio; // weighted or unweighted
    double * Effect;
    int NumberAllocatedEntries;
    int NumberEntries;
    EIT_BOOLEAN IsWeighted;
} HTT_HISTORICALTREND;


/*
Add item at end of list / Ajoute un élément a la fin de la liste.

Add item at end of list.

Ajoute un élément a la fin de la liste.
*/
extern HTT_HISTORICALTREND * HT_Add (
		/* A list.

        Une liste. */
	HTT_HISTORICALTREND * HT,
        /* An id.

        Une clé. */
	char * Id,
        /* A value for the current period.

        Une valeur pour la période courante. */
    double CurrentValue,
        /* A value for the previous period.

        Une valeur pour la période précédente. */
	double PreviousValue,
	double Weight
);

/*
Create a list / Crée une liste.

Allocate and initialize the list.

Alloue et initialise la liste.
*/
extern HTT_HISTORICALTREND * HT_Alloc (EIT_BOOLEAN IsWeighted);

/*
Destroy a list / Detruit une liste.

Empty and deallocate the list.

Vide et libere la memoire de la liste.
*/
extern void HT_Free (
        /* A list.

        Une liste. */
	HTT_HISTORICALTREND * HT
);

extern void HT_Print (HTT_HISTORICALTREND * HT);
extern void HT_PrintInfo (HTT_HISTORICALTREND * HT);

/*
Sort the list by effect / Trier la liste par effet.

Sort the list by effect.

Trier la liste par effet.
*/
extern void HT_SortEffect (
        /* A list.

        Une liste. */
    HTT_HISTORICALTREND * HT
);

/*
Sort the list by ratio / Trier la liste par ratio.

Sort the list by ratio.

Trier la liste par ratio.
*/
extern void HT_SortRatio (
        /* A list.

        Une liste. */
	HTT_HISTORICALTREND * HT
);

#endif
