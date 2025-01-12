#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
#include <string.h>
#include <time.h>

#include "EI_Common.h"
#include "EI_Message.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"

enum {
    HASH_MULTIPLIER = 31
};

static long mSeed = 1;
static time_t mTime = 11000000;

static double pythag(double a, double b);

/* --------------------------------------------------------------------

NAME         UTIL_Binary

FUNCTION     Do a binary search in an array of string
             sorted in ascending alphabetical order

PARAMETERS   1- search string
             2- array of pointers to character strings
             3- number of entries in the array of pointers

RETURN VALUE Int (if found,the position of the pointer pointing to
                              the located string
                  otherwise -1)

NOTES         See the book 'The C Programming Language', Kernighan and
              Ritchie, page 125.

----------------------------------------------------------------------*/

int UTIL_Binary (
    char *word,            /*search string*/
    char **tab,            /*array of pointers to character strings*/
    int number)            /*number of entries in the array of pointers*/
{
     int low, high, mid, cond;

     low = 0;
     high = number - 1;

     while (low <= high) {
        mid = (low + high) / 2;
        if ((cond = strcmp (word, tab[mid])) < 0)
            high = mid - 1;
        else if (cond > 0)
            low = mid + 1;
        else
            return (mid);
     }
     return (IS_NOT_FOUND);
}

/* --------------------------------------------------------------------

NAME         UTIL_Binary_Reverse

FUNCTION     Do a binary search in an array of string
             sorted in descending alphabetical order

PARAMETERS   1- search string
             2- array of pointers to character strings
             3- number of entries in the array of pointers

RETURN VALUE Int (if found,the position of the pointer pointing to
                              the located string
                  otherwise -1)
--------------------------------------------------------------------*/

int UTIL_Binary_Reverse (
    char *word,    /*search string*/
    char **tab,    /*array of pointers to character strings*/
    int number)    /*number of entries in the array of pointers*/
{
    int low, high, mid, cond;

    low = 0;

    high = number - 1;

    while (low <= high) {
        mid = (low + high) / 2;
        if ((cond = strcmp (word, tab[mid])) > 0)
            high = mid - 1;
        else if (cond < 0)
            low = mid + 1;
        else
            return (mid);
    }
    return (IS_NOT_FOUND);
}
/*------------------------------------------------------------------------------
skip comments in a string. a comment begins with '/' '*' and ends with '*' '/'.
if a comment is not terminated an error is triggered.
------------------------------------------------------------------------------*/
EIT_RETURNCODE UTIL_ConsumeComments (
	char * s,//string
	size_t * n,//number of consumed characters
	char ** Message)//message to print if 
{
	int NumberOfCommentsStarted;
	char * p = s;

	while (isspace(*p)) p++;/* skip spaces*/

	while (*p == '/' && *(p+1) == '*') {//slash star
		//found start of a comment
		p +=2;

		NumberOfCommentsStarted = 1;

		for (;;) {
			if (*p == '\0') {//end of string
				*Message = "End reached in the middle of a comment.";
				return EIE_FAIL;
			}
			if (*p == '/' && *(p+1) == '*') {//slash star, a comment starts
				p += 2;
				NumberOfCommentsStarted++;
			} else if (*p == '*' && *(p+1) == '/') {//star slash, a comment stops
				p += 2;
				NumberOfCommentsStarted--;
				if (NumberOfCommentsStarted == 0)
					break;//exit for
			}
			else
				p++;
		}

		while (isspace(*p)) p++;/* skip spaces*/
	}

	*n = p-s;//number of consumed characters

	return EIE_SUCCEED;
}


/* --------------------------------------------------------------------
NAME          UTIL_Decimal

FUNCTION      Check if a number has a number of decimals

PARAMETERS
              1- Real number (double)
              2- Decimal places wanted (int)

RETURN VALUE  EIT_UTIL_DECIMAL_RETURNCODE
                EIE_UTIL_DECIMAL_OK: The number has DecimalPlaces
                                          of decimals or more
                EIE_UTIL_DECIMAL_LOWER: Otherwise
----------------------------------------------------------------------*/
EIT_UTIL_DECIMAL_RETURNCODE UTIL_Decimal (
    double Number,     /* Real number           */
    int DecimalPlaces) /* Decimal places wanted */
{
    return EIM_DBL_EQ (Number-UTIL_Round (Number, DecimalPlaces), 0);
}

/*********************************************************************
*********************************************************************/
/*
Removes leading blanks/tab, trailing blanks/tab,
*/
char * UTIL_DropBlanks (
    char * String)
{
    char *Source;
    char *Destination;

    if (String[0] == '\0')
        return String;

    Destination = String;
    Source = String;

    while (isspace (*Source)) /* Remove all leading whitespace */
        Source++;

    while (*Source != '\0') {
        if (*Source == '\'') /* Leave quoted strings as they are */
            do {
                *Destination++ = *Source++;
            } while (*Source != '\'' && *Source != '\0');
        if (isspace (*Source)) {
            while (isspace (*(Source + 1))) /* Skip all but one space */
                Source++;
            *Source = ' '; /* Just Say No to tabs, linefeeds, etc */
        }
        *Destination++ = *Source++;
    }
    if (Destination != String && *(Destination - 1) == ' ')
        Destination--;
    *Destination = '\0';

    return (String);
}
/*
Removes leading blanks/tab, trailing blanks/tab.
This is a "safe" version, it does not call isspace() if the caracter is an accent.
*/
char * UTIL_DropBlanksSafeWithAccentuatedCharacters (
    char * String)
{
    char *Source;
    char *Destination;

    if (String[0] == '\0')
        return String;

    Destination = String;
    Source = String;

    while (*Source > 0 && isspace (*Source)) /* Remove all leading whitespace */
        Source++;

    while (*Source != '\0') {
        if (*Source == '\'') /* Leave quoted strings as they are */
            do {
                *Destination++ = *Source++;
            } while (*Source != '\'' && *Source != '\0');
        if (*Source > 0 && isspace (*Source)) {
            while (*(Source + 1) > 0 && isspace (*(Source + 1))) /* Skip all but one space */
                Source++;
            *Source = ' '; /* Just Say No to tabs, linefeeds, etc */
        }
        *Destination++ = *Source++;
    }
    if (Destination != String && *(Destination - 1) == ' ')
        Destination--;
    *Destination = '\0';

    return (String);
}

/* ---------------------------------------------------------------------
FUNCTION        Drop non-significant zeros.
PARAMETERS      1 - Pointer to the character string to drop zeros in.
RETURN VALUE    The parameter is updated and returned.
--------------------------------------------------------------------- */
char * UTIL_DropZeros (
    char *instring) /*  Incoming string to drop zeros in  */
{
    size_t i;
    size_t len;

    /*  Eliminate leading and trailing blanks  */
    UTIL_DropBlanks (instring);

    /*  Get the length of the incoming string  */
    len = strlen (instring);

    /*  Check to see if there is a dot  */
    for (i = 0; i < len; i++)
        if (instring[i] == '.')
            break;

    if (i == len)
        /* No dot in the incoming string  */
        return instring;

    /*  Eliminate non-significant zeros  */
    for (i = (len - 1); i > 0; i--)
        if (instring[i] != '0')
            break;

    /*  Eliminate non-significant dot  */
    if (instring[i] != '.')
        i++;

    instring[i] = '\0';

    return instring;
}

/*********************************************************************
* Free a unit-offset double matrix.
*********************************************************************/
void UTIL_FreeDouble1Matrix (
    double **M,
    unsigned int rows)
{
    unsigned int i;

    if (M == NULL) return;

    for (i = 1; i <= rows; i++)
        STC_FreeMemory(M[i]);
    STC_FreeMemory(M);
}

/*********************************************************************
* Free a zero-offset double matrix.
*********************************************************************/
void UTIL_FreeDouble0Matrix (
    double **M,
    unsigned int rows)
{
    unsigned int i;

    if (M == NULL) return;

    for (i = 0; i < rows; i++)
        STC_FreeMemory(M[i]);
    STC_FreeMemory(M);
}

/*********************************************************************
Hash a string.
From "The practice of programming." B.Kernighan and R. Pike.

This function is not to be used for security 
related cryptographic purposes.  
*********************************************************************/
EIT_HASH UTIL_Hash (
    char * s,
    unsigned int Size)
{
    EIT_HASH h;

    for (h = 0; *s != '\0'; s++)
        h = HASH_MULTIPLIER * h + *s;

    return h % Size;
}

/*--------------------------------------------------------
Checks if a string is an integer. An empty string is not an integer.
If it is, returns 1. Otherwise, returns 0.
----------------------------------------------------------*/
int UTIL_IsInteger (
    char * String)
{
    size_t l;

    l = strlen (String);

    return l != 0 && l == strspn (String, "1234567890");
}
/*------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
EIT_BOOLEAN UTIL_IsNumeric (
    char * s,
	int * n)
{
	int i;
	long l;
	char * s_ptr;
	for (i = 0; i < (int) strlen (s); i++)
		if (!isdigit (s[i])) {
			return EIE_FALSE;
		}
	l = strtol (s, &s_ptr, 10);
	if (l == LONG_MAX) {
        return EIE_FALSE;
	}

	*n = (int) l;

	return EIE_TRUE;
}




/*--------------------------------------------------------
For zero-offset matrices.
----------------------------------------------------------*/
double ** UTIL_MultiplyMatrices (
    double **x,
    double **y,
    int xrow,
    int xcol,
    int ycol)
{
    /*Supposedly this is
        (C) Copr. 1986-92 Numerical Recipes Software s@Ww#3#=!$!.
        although I couldn't find it anywhere. */

    int    i, j, k;
    double    **z;

    z = STC_AllocateMemory(xrow * sizeof *z);
    if (z == NULL)
        return NULL;

    for (i = 0; i < xrow; i++) {
        z[i] = STC_AllocateMemory(ycol * sizeof *z[i]);
        if (z[i] == NULL) {
            for (j = 0; j < i; j++)
                STC_FreeMemory(z[j]);
            STC_FreeMemory(z);
            return NULL;
        }
    }
    for (i = 0; i < xrow; i++)
        for (j = 0; j < ycol; j++) {
            z[i][j] = 0.0;
            for (k = 0; k < xcol; k++)
                z[i][j] += x[i][k] * y[k][j];
        }

    return z;
}



/*--------------------------------------------------------
For unit-offset matrices.
----------------------------------------------------------*/
double ** UTIL_MultiplyMatrices1 (
    double **x,
    double **y,
    int xrow,
    int xcol,
    int ycol)
{
    /*Supposedly this is
        (C) Copr. 1986-92 Numerical Recipes Software s@Ww#3#=!$!.
        although I couldn't find it anywhere (by this name anyway). */

    int    i, j, k;
    double    **z;

    z = STC_AllocateMemory((1 + xrow) * sizeof *z);
    if (z == NULL)
        return NULL;

    for (i = 1; i <= xrow; i++) {
        z[i] = STC_AllocateMemory((1 + ycol) * sizeof *z[i]);
        if (z[i] == NULL) {
            for (j = 1; j < i; j++)
                STC_FreeMemory(z[j]);
            STC_FreeMemory(z);
            return NULL;
        }
    }
    for (i = 1; i <= xrow; i++)
        for (j = 1; j <= ycol; j++) {
            z[i][j] = 0.0;
            for (k = 1; k <= xcol; k++)
                z[i][j] += x[i][k] * y[k][j];
        }
    return z;
}
/*--------------------------------------------------------
Prints a non null terminated string.
----------------------------------------------------------*/
void UTIL_PrintNNTString (
    char * String,
    int StringLength)
{
    EI_AddMessage ("", 4, "not useful since the use of EI_AddMessage (). try to print by other means.");
	while (StringLength-->0)
        EI_AddMessage ("", 4, "%c", *String++);
}
/*------------------------------------------------------
Print an array of strings
-------------------------------------------------------*/
void UTIL_PrintStrings (
    char ** String,
    int NumberofStrings)
{
    int i;

    for (i = 0; i < NumberofStrings; i++) {
        EI_AddMessage ("", 4, "%s", String[i]);
    }
}
/* --------------------------------------------------
case-insensitive version of strcmp(). returns -1 if s1 is greater than s2,
1 if s2 is greater than s1, and 0 if they are equal.
-----------------------------------------------------*/
int UTIL_StrICmp (
    const char * s1,
    const char * s2)
{
    for (; tolower (*s1) == tolower (*s2); ++s1, ++s2) {
        if (*s1 == '\0')
            return 0;
    }
    return tolower (*s1) < tolower (*s2) ? -1 : 1;
}

/* --------------------------------------------------
Converts string to lowercase string.
-----------------------------------------------------*/
char * UTIL_StrLower (
    char * Destination,
    char * Source)
{
    char * DestinationStart;

    DestinationStart = Destination;
    while (*Source) {
        *Destination++ = tolower (*Source++);
    }
    *Destination = *Source; /* add the EOL */
    return DestinationStart;
}

/* --------------------------------------------------
splits a string chunk of Width characters to create an array of strings.
returns the number of chunks, or -1 if allocation fails

example:
char ** v;
int n;
n = UTIL_StrSplit ("1234567890", 5, &v);

after the call v[0] is "12345" and v[1] is "67890"
it is the programmer's responsability to free the space.

for (i=0;i<n;i++)
    free (v[i]);
free (v);
-----------------------------------------------------*/
int UTIL_StrSplit (
    char * Source,
    int Width,
    char *** PtrArrayStrings)
{
    char ** ArrayStrings;
    size_t i;
    size_t l;
    size_t N;

    l = strlen (Source);
    N = (l==0 ? 1 : (l/Width + (l%Width ? 1 : 0)));

    ArrayStrings = STC_AllocateMemory (N * sizeof *ArrayStrings);
	if (ArrayStrings == NULL)
		return 0;

    for (i = 0; i < N-1; i++, Source += Width) {
        ArrayStrings[i] = STC_AllocateMemory (Width+1);
		if (ArrayStrings[i] == NULL)
			return 0;
        strncpy (ArrayStrings[i], Source, Width);
        ArrayStrings[i][Width] = '\0';
    }
    ArrayStrings[i] = STC_AllocateMemory (strlen (Source) + 1);
	if (ArrayStrings[i] == NULL)
		return 0;
    strcpy (ArrayStrings[i], Source);

    *PtrArrayStrings = ArrayStrings;

    return (int) N;
}

/*----------------------------------------------------
Removes spaces from a string. isspace() is used to find space characters.
------------------------------------------------------*/
char * UTIL_StrRemoveSpaces (
    char * Destination,
    char * Source)
{
    while (*Source != '\0') {
        if (!isspace (*Source))
            *Destination++ = *Source;
        Source++;
    }

    *Destination = '\0';

    return Destination;
}

/*----------------------------------------------------
Converts string to uppercase string.
------------------------------------------------------*/
char * UTIL_StrUpper (
    char * Destination,
    char * Source)
{
    char * DestinationStart;

    DestinationStart = Destination;
    while (*Source) {
        *Destination++ = toupper (*Source++);
    }
    *Destination = *Source; /* add the EOL */
    return DestinationStart;
}

/*********************************************************************
*********************************************************************/
/*
(C) Copr. 1986-92 Numerical Recipes Software 3$$.
returns a random number between 0.0 (inclusively) and 1.0 (exclusively)
with a period of 2.1 x 10 at the power of 9
*/
double UTIL_Rand (void) {
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

    long k;
    double ans;

    mSeed ^= MASK;
    k=(mSeed)/IQ;
    mSeed=IA*(mSeed-k*IQ)-IR*k;
    if (mSeed < 0) mSeed += IM;
    ans=AM*(mSeed);
    mSeed ^= MASK;
    return ans;
#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef MASK
}

/**************************************************
will return a long between Min(inclusively) and Max(exclusively)
**************************************************/
long UTIL_Random (
    long Min,
    long Max)
{
    return Min+(long)(UTIL_Rand ()*(Max-Min));
}
/*------------------------------------------------------------------------------
Adds an error message to the message list.
------------------------------------------------------------------------------*/
void UTIL_ReportError (
    char * Header1,
    char * Header2,
    char * ErrorMessage,
    char * ParseString,
    char * ParseStringPtr)
{
#define SIZEOFCHUNKS 72

    int CaretPosition;
    int i;
    int NumberOfSplitedStrings;
    char ** SplitedString;
    char * String;

    CaretPosition = (int) (ParseStringPtr-ParseString);

    /* String size must be big enough to hold:
       the edits +
       newline characters +
       the line of the caret + its newline characters +
       eol character */
    String = STC_AllocateMemory (
        strlen (ParseString) +
        strlen (ParseString)/SIZEOFCHUNKS + 1 +
        SIZEOFCHUNKS + 1 +
        1);
	if (String == NULL)
		return;
    String[0] = '\0';

    NumberOfSplitedStrings = UTIL_StrSplit (ParseString, SIZEOFCHUNKS,
        &SplitedString);

    for (i = 0; i < NumberOfSplitedStrings; i++) {
        strcat (String, SplitedString[i]);
        strcat (String, "\n");
        if (0 <= CaretPosition && CaretPosition < SIZEOFCHUNKS) {
            sprintf (String+strlen(String), "%*s^\n",
                CaretPosition, "");
        }
        CaretPosition -= SIZEOFCHUNKS;
    }

    EI_AddMessage (Header1, EIE_MESSAGESEVERITY_ERROR, "%s\n%s:\n%s",
        ErrorMessage, Header2, String);

    STC_FreeMemory (String);
    for (i = 0; i < NumberOfSplitedStrings; i++)
        STC_FreeMemory (SplitedString[i]);
    STC_FreeMemory (SplitedString);
}
/*********************************************************************
*********************************************************************/
#ifdef _WIN32
#include <Windows.h>
#endif
/**************************************************
**************************************************/
void UTIL_ResetTime (void) {
#ifdef _WIN32
    mTime = GetTickCount ();
#else
    time (&mTime);
#endif
}

/* -----------------------------------------------------------------------
NAME          UTIL_Round

FUNCTION      Round a number to a specific number of decimal places

PARAMETERS
              1- Real number before rounding (double)
              2- Decimal places wanted (int)

RETURN VALUE  Rounded number  (double)

--------------------------------------------------------------------------*/
double UTIL_Round (
    double Number,     /* Real number before rounding */
    int DecimalPlaces) /* Decimal places wanted       */
{
    double Factor;

    Factor = pow (10, (double) DecimalPlaces);

    return (Number < 0.0
        ? (ceil  (Number * Factor - 0.5) / Factor)
        : (floor (Number * Factor + 0.5) / Factor));
}
/*********************************************************************
prints time. help to analyse performance.
*********************************************************************/
void UTIL_ShowTime (
    char * Message)
{
#ifdef _WIN32
	EI_AddMessage ("", EIE_MESSAGESEVERITY_EMPTY, "Time: %8.2f %s\n",
		(GetTickCount () - mTime)/1000.0, Message);
    mTime = GetTickCount ();
#else
    EI_AddMessage ("", 4, "UTIL_ShowTime unavailable on this platform (%s)\n", Message);
    //static double StaticTime = 0.0;
	//double NewTime;
    //struct timeval PtrTime;
    //gettimeofday (&PtrTime, NULL);
    //NewTime = (double) PtrTime.tv_sec + (double)PtrTime.tv_usec/1000000.0;
    //EI_AddMessage ("", 4, "duree(sec)= %8.2f %s\n", NewTime - StaticTime, Message);
    //StaticTime = NewTime;
#endif
	EI_PrintMessages ();
}
/*------------------------------------------------------
Sort an array of strings in ascending alphabetical order
-------------------------------------------------------*/
void UTIL_SortStrings_A (
    char ** String,
    int NumberofStrings)
{
    int gap, j, k;
    char *tmpvalue;

    for (gap = NumberofStrings/2; gap > 0; gap /= 2)
        for (j = gap; j < NumberofStrings; j++)
            for (k = j - gap;
                k >= 0 && (strcmp (String[k], String[k+gap]) > 0); k -= gap)
                {
                    tmpvalue         = String[k];
                    String[k]        = String[k+gap];
                    String[k+gap]    = tmpvalue;
                }
}

/*-------------------------------------------------------
Sort an array of strings in descending alphabetical order
--------------------------------------------------------*/
void UTIL_SortStrings_D (
    char ** String,
    int NumberofStrings)
{
    int gap, j, k;
    char *tmpvalue;

    for (gap = NumberofStrings/2; gap > 0; gap /= 2)
        for (j = gap; j < NumberofStrings; j++)
            for (k = j - gap;
                k >= 0 && (strcmp (String[k], String[k+gap]) < 0); k -= gap)
                {
                    tmpvalue         = String[k];
                    String[k]        = String[k+gap];
                    String[k+gap]    = tmpvalue;
                }
}

/*********************************************************************
*********************************************************************/
void UTIL_SRand (
    long Seed)
{
    mSeed = Seed;
}

/*
(C) Copr. 1986-92 Numerical Recipes Software 3$$.*/
/*                                                                  */
/*Note: Three routines from the Numerical Recipies book are used    */
/*here. These routines are svdcmp, pythag, and svbksb. These form   */
/*a part of the solution for calculating betas in liner-regression  */
/*based estimator functions. Singular-value-decomposition is a      */
/*technique which is employed in this solution. Please see          */
/*Page 59 for details regarding the three routines used here.       */
/*------------------------------------------------------------------*/
/* The code is taken verbatum from the aforementioned book. The     */
/* following changes are made to the code in this file (as compared */
/* to the original given in the book) :-                            */
/*       - signature of svdcmp changed.  The return type is int     */
/*         it was void originally.  The return type indicates the   */
/*         success and failure of the routine. 1 indicates success  */
/*         and 0 indicates failure.                                 */
/*       - on failure the original svdcmp would call an internal    */
/*         print routine called nrerror; The line of code has       */
/*         been changed to return with a 0 indicating a failure to  */
/*         compute. Error handling is done in the calling routine.  */
/*------------------------------------------------------------------*/

/*   svdcmp fails if it does not converge                           */
/*   This number is set to 30 as given in the book, no              */
/*   rationale for the number 30 is given.                          */
/*   Our tests have all converged.(geis)                            */
/*   Return type of 0 indicates failure                             */
/*   Return type of 1 indicates no failure                          */
/*------------------------------------------------------------------*/

static double sqrarg;
#define SQR(a) ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg)

static double dsqrarg;
#define DSQR(a) ((dsqrarg=(a)) == 0.0 ? 0.0 : dsqrarg*dsqrarg)

static double dmaxarg1,dmaxarg2;
#define DMAX(a,b) (dmaxarg1=(a),dmaxarg2=(b),(dmaxarg1) > (dmaxarg2) ?\
        (dmaxarg1) : (dmaxarg2))

static double dminarg1,dminarg2;
#define DMIN(a,b) (dminarg1=(a),dminarg2=(b),(dminarg1) < (dminarg2) ?\
        (dminarg1) : (dminarg2))

static double maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static double minarg1,minarg2;
#define FMIN(a,b) (minarg1=(a),minarg2=(b),(minarg1) < (minarg2) ?\
        (minarg1) : (minarg2))

static long lmaxarg1,lmaxarg2;
#define LMAX(a,b) (lmaxarg1=(a),lmaxarg2=(b),(lmaxarg1) > (lmaxarg2) ?\
        (lmaxarg1) : (lmaxarg2))

static long lminarg1,lminarg2;
#define LMIN(a,b) (lminarg1=(a),lminarg2=(b),(lminarg1) < (lminarg2) ?\
        (lminarg1) : (lminarg2))

static int imaxarg1,imaxarg2;
#define IMAX(a,b) (imaxarg1=(a),imaxarg2=(b),(imaxarg1) > (imaxarg2) ?\
        (imaxarg1) : (imaxarg2))

static int iminarg1,iminarg2;
#define IMIN(a,b) (iminarg1=(a),iminarg2=(b),(iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))




/*********************************************************************
*********************************************************************/
static double pythag (
    double a,
    double b)
{
    double absa, absb;

    absa=fabs(a);
    absb=fabs(b);
    if (absa > absb)
        return absa*sqrt(1.0+SQR(absb/absa));
    else
        return (absb== 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}

/*********************************************************************
*********************************************************************/
EIT_BOOLEAN UTIL_Svbksb (
    double **u,
    double w[],
    double **v,
    int m,
    int n,
    double b[],
    double x[])
{
    int jj, j, i;
    double s, *tmp;

/*    tmp=vector(1,n);*/
    tmp = STC_AllocateMemory((n + 1) * sizeof *tmp);
    if (tmp == NULL) return EIE_FALSE;

    for (j=1;j<=n;j++) {
        s=0.0;
        if (w[j]) {
            for (i=1;i<=m;i++) s += u[i][j]*b[i];
            s /= w[j];
        }
        tmp[j]=s;
    }
    for (j=1;j<=n;j++) {
        s=0.0;
        for (jj=1;jj<=n;jj++) s += v[j][jj]*tmp[jj];
        x[j]=s;
    }
    STC_FreeMemory(tmp);
/*    free(tmp);*/
/*    freeV(tmp,1,n);*/

    return EIE_TRUE;
}


/********************************************************************
Does Singular Value Decomposition.
returns
1 on success
0 when no convergence
-1 on memory failure
********************************************************************/
int UTIL_Svdcmp (double **a, int m, int n, double w [], double **v)
{

     /*   double pythag(double a, double b); */
     /*   function protype declared.         */
        int flag,i,its,j,jj,k,l,nm;
    double anorm,c,f,g,h,s,scale,x,y,z,*rv1;

/*    rv1=vector(1,n);*/
    rv1 = STC_AllocateMemory((n + 3) * sizeof *rv1);
    if (rv1 == NULL) return -1;

    g=scale=anorm=0.0;
    for (i=1;i<=n;i++) {
        l=i+1;
        rv1[i]=scale*g;
        g=s=scale=0.0;
        if (i <= m) {
            for (k=i;k<=m;k++) scale += fabs(a[k][i]);
            if (scale) {
                for (k=i;k<=m;k++) {
                    a[k][i] /= scale;
                    s += a[k][i]*a[k][i];
                }
                f=a[i][i];
                g = -SIGN(sqrt(s),f);
                h=f*g-s;
                a[i][i]=f-g;
                for (j=l;j<=n;j++) {
                    for (s=0.0,k=i;k<=m;k++) s +=
                        a[k][i]*a[k][j];
                    f=s/h;
                    for (k=i;k<=m;k++) a[k][j] +=
                        f*a[k][i];
                }
                for (k=i;k<=m;k++) a[k][i] *= scale;
            }
        }
        w[i]=scale *g;
        g=s=scale=0.0;
        if (i <= m && i != n) {
            for (k=l;k<=n;k++) scale += fabs(a[i][k]);
            if (scale) {
                for (k=l;k<=n;k++) {
                    a[i][k] /= scale;
                    s += a[i][k]*a[i][k];
                }
                f=a[i][l];
                g = -SIGN(sqrt(s),f);
                h=f*g-s;
                a[i][l]=f-g;
                for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
                for (j=l;j<=m;j++) {
                    for (s=0.0,k=l;k<=n;k++) s +=
                        a[j][k]*a[i][k];
                    for (k=l;k<=n;k++) a[j][k] +=
                        s*rv1[k];
                }
                for (k=l;k<=n;k++) a[i][k] *= scale;
            }
        }
        anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
    }
    for (i=n;i>=1;i--) {
        if (i < n) {
            if (g) {
                for (j=l;j<=n;j++)
                    v[j][i]=(a[i][j]/a[i][l])/g;
                for (j=l;j<=n;j++) {
                    for (s=0.0,k=l;k<=n;k++) s +=
                        a[i][k]*v[k][j];
                    for (k=l;k<=n;k++) v[k][j] +=
                        s*v[k][i];
                }
            }
            for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
        }
        v[i][i]=1.0;
        g=rv1[i];
        l=i;
    }
    for (i=IMIN(m,n);i>=1;i--) {
        l=i+1;
        g=w[i];
        for (j=l;j<=n;j++) a[i][j]=0.0;
        if (g) {
            g=1.0/g;
            for (j=l;j<=n;j++) {
                for (s=0.0,k=l;k<=m;k++) s +=
                    a[k][i]*a[k][j];
                f=(s/a[i][i])*g;
                for (k=i;k<=m;k++) a[k][j] +=
                    f*a[k][i];
            }
            for (j=i;j<=m;j++) a[j][i] *= g;
        } else for (j=i;j<=m;j++) a[j][i]=0.0;
        ++a[i][i];
    }
    for (k=n;k>=1;k--) {
        for (its=1;its<=30;its++) {
            flag=1;
            for (l=k;l>=1;l--) {
                nm=l-1;
                if ((double)(fabs(rv1[l])+anorm) ==
                    anorm) {
                    flag=0;
                    break;
                }
                if ((double)(fabs(w[nm])+anorm) ==
                    anorm) break;
            }
            if (flag) {
                c=0.0;
                s=1.0;
                for (i=l;i<=k;i++) {
                    f=s*rv1[i];
                    rv1[i]=c*rv1[i];
                    if ((double)(fabs(f)+anorm) ==
                        anorm) break;
                    g=w[i];
                    h=pythag(f,g);
                    w[i]=h;
                    h=1.0/h;
                    c=g*h;
                    s = -f*h;
                    for (j=1;j<=m;j++) {
                        y=a[j][nm];
                        z=a[j][i];
                        a[j][nm]=y*c+z*s;
                        a[j][i]=z*c-y*s;
                    }
                }
            }
            z=w[k];
            if (l == k) {
                if (z < 0.0) {
                    w[k] = -z;
                    for (j=1;j<=n;j++) v[j][k] =
                        -v[j][k];
                }
                break;
            }
                /*if (its == 30) nrerror("no convergence in 30 */
              /*                         svdcmp iterations");*/
                                               /* above line changed */
            if (its == 30) return 0;
            x=w[l];
            nm=k-1;
            y=w[nm];
            g=rv1[nm];
            h=rv1[k];
            f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
            g=pythag(f,1.0);
            f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
            c=s=1.0;
            for (j=l;j<=nm;j++) {
                i=j+1;
                g=rv1[i];
                y=w[i];
                h=s*g;
                g=c*g;
                z=pythag(f,h);
                rv1[j]=z;
                c=f/z;
                s=h/z;
                f=x*c+g*s;
                g = g*c-x*s;
                h=y*s;
                y *= c;
                for (jj=1;jj<=n;jj++) {
                    x=v[jj][j];
                    z=v[jj][i];
                    v[jj][j]=x*c+z*s;
                    v[jj][i]=z*c-x*s;
                }
                z=pythag(f,h);
                w[j]=z;
                if (z) {
                    z=1.0/z;
                    c=f*z;
                    s=h*z;
                }
                f=c*g+s*y;
                x=c*y-s*g;
                for (jj=1;jj<=m;jj++) {
                    y=a[jj][j];
                    z=a[jj][i];
                    a[jj][j]=y*c+z*s;
                    a[jj][i]=z*c-y*s;
                }
            }
            rv1[l]=0.0;
            rv1[k]=f;
            w[k]=x;
        }
    }
    STC_FreeMemory(rv1);
/*    free(rv1);*/
/*    freeV(rv1,1,n);*/

    return 1; /*       return type added; see note in the */
              /*                beginning of the function */
}

/*--------------------------------------------------------------
 * Transpose a zero-offset matrix.
 ---------------------------------------------------------------*/
double ** UTIL_Transpose (
    double ** matrix,
    int NumRow,
    int NumCol)
{
    int i, j;
    double **trans;

    trans = STC_AllocateMemory (NumCol * sizeof *trans);
    if (trans == NULL) return NULL;

    for(i = 0; i < NumCol; i++){
        trans[i] = STC_AllocateMemory (NumRow * sizeof *trans[i]);
        if (trans[i] == NULL){
            for (j = 0; j < i; j++)
                STC_FreeMemory(trans[j]);
            STC_FreeMemory(trans);
            return NULL;
        }
    }

    for (i = 0; i < NumCol; i++)
        for (j = 0; j < NumRow; j++)
            trans[i][j] = matrix[j][i];

    return trans;
}
