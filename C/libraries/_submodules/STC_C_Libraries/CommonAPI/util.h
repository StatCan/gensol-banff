#ifndef UTIL_H
#define UTIL_H

#include "EI_Common.h"
#include "EI_Edits.h"

/****************************************************************************/
/* This header file contains prototypes and definitions for functions       */
/* that are used by the API but that are not specific to a methodology.     */
/****************************************************************************/

#define IS_NOT_FOUND -1

typedef unsigned int EIT_HASH;

/* Return code for function UTIL_DECIMAL */
typedef enum{
     EIE_UTIL_DECIMAL_GREATER = 0,/* Number of decimal is equal or greater than specified*/
     EIE_UTIL_DECIMAL_OK    = 1   /* Number of decimal is lower than specified*/
} EIT_UTIL_DECIMAL_RETURNCODE;


/* SEARCH A STRING IN AN ARRAY OF STRINGS / CHERCHE UN MOT DANS UN TABLEAU DE MOTS*/
/* This function returns the position in the array or IS_NOT_FOUND */
/**/
/* Cette fonction cherche un mot dans un tableau de mots et retourne la*/
/* position dans le tableau ou IS_NOT_FOUND */
CLASS_DECLSPEC int UTIL_Binary (
        /* String to find / Mot recherché */
    char * Word,
        /* Array of strings / Tableau de mots */
    char **Tab,
        /* Number of strings in array / Nombre de mots dans le tableau*/
    int    Number
);

/* SEARCH A STRING IN AN ARRAY OF STRINGS SORTED IN DESCENDING ORDER/ CHERCHE UN MOT DANS UN TABLEAU DE MOTS TRIE EN ORDRE DESCENDANT*/
/* This function returns the position in the array or IS_NOT_FOUND */
/**/
/* Cette fonction cherche un mot dans un tableau de mots et retourne la*/
/* position dans le tableau ou IS_NOT_FOUND */
CLASS_DECLSPEC int  UTIL_Binary_Reverse (
    char *word,     /*search string*/
    char **tab,     /*array of pointers to character strings*/
    int  number     /*number of entries in the array of pointers*/
);

/*********************************************************************
skip comments in a string.
*********************************************************************/
CLASS_DECLSPEC EIT_RETURNCODE UTIL_ConsumeComments (
	char * s,        //string
	size_t * n,      //number of consumed characters
	char ** Message);//returned message, explaining the reason of the failure

/* CHECK THE NUMBER OF DECIMALS / VERIFIE LE NOMBRE DE DECIMALES */
/* This function verifies that the number contains the specified number */
/* of decimales.                                                        */
/**/
/* Cette fonction vérifie si un nombre a au moins le nombre de décimales*/
/* précisé.                                                             */
CLASS_DECLSPEC EIT_UTIL_DECIMAL_RETURNCODE UTIL_Decimal (
        /* Real Number / Nombre réel*/
    double Number,
        /* Number of decimals / Nombre de décimales*/
    int    DecimalPlaces
);

/*
Removes leading blanks/tab, trailing blanks/tab.
*/
CLASS_DECLSPEC char * UTIL_DropBlanks (char * String);
CLASS_DECLSPEC char * UTIL_DropBlanksSafeWithAccentuatedCharacters (char * String);

/*
Drop non-significant zeros.
*/
CLASS_DECLSPEC char * UTIL_DropZeros (char *instring);


/*********************************************************************
* Free a unit-offset double matrix.
*********************************************************************/
CLASS_DECLSPEC void UTIL_FreeDouble1Matrix(double **, unsigned int);

/*********************************************************************
* Free a zero-offset double matrix.
*********************************************************************/
CLASS_DECLSPEC void UTIL_FreeDouble0Matrix(double **, unsigned int);

/*********************************************************************
Hash a string.
*********************************************************************/
CLASS_DECLSPEC EIT_HASH UTIL_Hash (char *, unsigned int);

/*--------------------------------------------------------
Checks if a string is an integer. An empty string is not an integer.
If it is, returns 1. Otherwise, returns 0.
----------------------------------------------------------*/
CLASS_DECLSPEC int UTIL_IsInteger (char *);

CLASS_DECLSPEC EIT_BOOLEAN UTIL_IsNumeric (char * s, int * n);

/*--------------------------------------------------------
Multiplies zero-offset matrices
----------------------------------------------------------*/
CLASS_DECLSPEC double ** UTIL_MultiplyMatrices(double **x, double **y, 
			   int xrow, int xcol, int ycol);

/*--------------------------------------------------------
Multiplies unit-offset matrices
----------------------------------------------------------*/
CLASS_DECLSPEC double ** UTIL_MultiplyMatrices1(double **x, double **y, 
			   int xrow, int xcol, int ycol);

/*
Prints a non null terminated string.
Imprime une chaine de caractere qui n'est pas terminé par un caractere null.
*/
CLASS_DECLSPEC void UTIL_PrintNNTString (
        /* A not null terminated string.

        Une chaine de caracteres non terminé par un caractere null. */
    char * String,
        /*  String length.

        Grandeur de la chaine. */
    int StringLength
);

/*--------------------------------------------------------
Prints an array of strings.
----------------------------------------------------------*/
CLASS_DECLSPEC void UTIL_PrintStrings (char **, int);

/*--------------------------------------------------------
return a random number between 0 and 0.99999999999
----------------------------------------------------------*/
CLASS_DECLSPEC double UTIL_Rand (void);

/*--------------------------------------------------------
return a random number between Min and Max
----------------------------------------------------------*/
CLASS_DECLSPEC long UTIL_Random (long Min, long Max);

CLASS_DECLSPEC void UTIL_ReportError (char * Header1, char * Header2, char * ErrorMessage, char * ParseString, char * ParseStringPtr);

CLASS_DECLSPEC void UTIL_ResetTime (void);

/* ROUND A REAL NUMBER / ARRONDIR UN NOMBRE REEL */
/* This function will round a real number to the specified number of decimals*/
/**/
/* Cette fonction arrondit un nombre réel au nombre de décimales spécifiées*/
CLASS_DECLSPEC double UTIL_Round (
        /* Real number before rounding/Nombre réel avant arrondissement*/
    double Number,
        /* Number of decimals / Nombre de décimales*/
    int    DecimalPlaces
);

CLASS_DECLSPEC void UTIL_ShowTime (char * Message);

/*
Sorts an array of strings in ascending alphabetical order/Trie un tableau de chaines de caracteres en order alphabetique ascendant.
*/
CLASS_DECLSPEC void UTIL_SortStrings_A (
char ** String,
int     NumberofStrings
);

/*
Sorts an array of strings in descending alphabetical order/Trie un tableau de chaines de caracteres en order alphabetique descendant.
*/
CLASS_DECLSPEC void UTIL_SortStrings_D (
char ** String,
int     NumberofStrings
);

/* --------------------------------------------------
set the seed of the random number generator
----------------------------------------------------- */
CLASS_DECLSPEC void UTIL_SRand (long Seed);

/* --------------------------------------------------
case-insensitive version of strcmp (). returns -1 if s1 is greater than s2,
1 if s2 is greater than s1, and 0 if they are equal.
----------------------------------------------------- */
CLASS_DECLSPEC int UTIL_StrICmp (const char * s1, const char * s2);

/*
Copies a string to another string changing it to lowercase / Copie une chaine de caracteres et la change en lettres minuscules.

Copies a string to another string changing it to lowercase. The destination string is returned.
To modify a string in place use as follow: UTIL_StrLower (s, s);

Copie une chaine de caracteres et la change en lettres minuscules. La chaine Destination est retournée.
Si Destination et Source est le meme variable {UTIL_StrLower (s, s);} alors la chaine elle meme sera changé.
*/
CLASS_DECLSPEC char * UTIL_StrLower (
        /*  Destination string.

        Chaine de caracteres de destination. */
    char * Destination,
        /* Source string.

        Chaine de caracteres source. */
    char * Source
);

/*
splits a string chunk of Width characters to create an array of strings.
returns the number of chunks, or
-1 if allocation fails
example:
char ** v;
int n;
n = UTIL_StrSplit ("1234567890", 5, &v);
after the call v[0] is "12345" and v[1] is "67890"
it is the programmer's responsability to free the space.
for (i=0;i<n;i++)
    free (v[i]);
free (v);
*/
CLASS_DECLSPEC int UTIL_StrSplit (char * Source, int Width,
    char *** PtrArrayStrings);

/*
Removes spaces from a string. isspace () is used to find space characters.
*/
CLASS_DECLSPEC char * UTIL_StrRemoveSpaces (char * Dest, char * Source);

/*
Copies a string to another string changing it to uppercase / Copie une chaine de caracteres et la change en lettres majuscules.

Copies a string to another string changing it to uppercase. The destination string is returned.
To modify a string in place use as follow: UTIL_StrUpper (s, s);

Copie une chaine de caracteres et la change en lettres majuscules. La chaine Destination est retournée.
Si Destination et Source est le meme variable {UTIL_StrUpper (s, s);} alors la chaine elle meme sera changé.
*/
CLASS_DECLSPEC char * UTIL_StrUpper (
        /*  Destination string.

        Chaine de caracteres de destination. */
    char * Destination,
        /* Source string.

        Chaine de caracteres source. */
    char * Source
);

/*
Does Singular Value Decomposition.
See Numerical Recipes and util.c
*/
CLASS_DECLSPEC int UTIL_Svdcmp (double **U, int m, int n, double w[], double **V);

CLASS_DECLSPEC EIT_BOOLEAN UTIL_Svbksb(double **u, double w[], double **v, 
			    int m, int n, double b[], double x[]);

CLASS_DECLSPEC double **UTIL_Transpose (double ** matrix, int NumRow, int NumCol);

#endif
