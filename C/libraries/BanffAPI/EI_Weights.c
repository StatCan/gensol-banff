/* contient toutes les fonctions reliees au type EIT_WEIGHTS */

/*
eight parser
This is a list of the productions for the parser:

weights   -> weight
          |  weight weights
weight    -> variable '=' number ';'
number    -> TOKEN_NUMBER
variable  -> TOKEN_VARIABLE

TOKEN_NUMBER   {list of digits}
TOKEN_VARIABLE {letter followed by list of letters|digits|underscore}
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_Info.h"
#include "EI_Message.h"
#include "EI_Weights.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "util.h"
#include "MessageBanffAPI.h"

#define PARSER_MAX_ERROR_COUNT 7

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

enum TokenIdEnum {
    eTokenIdDone,
    eTokenIdEqual,
    eTokenIdError,
    eTokenIdNumber,
    eTokenIdSemiColon,
    eTokenIdUnknownCharacter,
    eTokenIdVariable
};
typedef enum TokenIdEnum tTokenId;

union ValueUnion {
    double Number;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
};
typedef union ValueUnion tTokenValue;

struct TokenStructure {
    tTokenId Id;
    tTokenValue Value;
};
typedef struct TokenStructure tToken;

struct WeightStructure {
    double Number;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
};
typedef struct WeightStructure tTerm;


/* contains info about the Weight that the parser is currently working on. */
static tTerm mTerm;
/* points to the EIT_WEIGHTS receive by EI_WeightsParse() */
static EIT_WEIGHTS * mWeights;
/* points to the weights strings. */
static char * mWeightsString;
/* points somewhere in mWeightsString. the scanner moves it as it scans the string. */
static char * mWeightsStringPtr;

static int InstallNumber (double Number);
static void InstallVariable (char * Variable);
static void InstallWeight (void);
static int Match (tTokenId Id, tToken * Token);
static int MatchNumber (tToken * Token);
static int MatchVariable (tToken * Token);
static int MatchWeight (tToken * Token);
static int MatchWeights (tToken * Token);
static int Scanner (tToken * Token);
static void ReportError (char * String);
static void ReportWarning (char * String);
static char * TokenIdTranslator (tTokenId TokenId);
static void TokenPrint (tToken * Token);


int EI_WeightsAllocate (
    EIT_EDITS * Edits,
    EIT_WEIGHTS * Weights)
{
    int i;

    Weights->FieldName      = Edits->FieldName;
    Weights->NumberOfFields = Edits->NumberofColumns-1;

    Weights->Value = STC_AllocateMemory (
        Weights->NumberOfFields * sizeof *Weights->Value);
    if (Weights->Value == NULL) {
        return -1;
    }

    for (i = 0; i < Weights->NumberOfFields; i++)
        Weights->Value[i] = 1.0;

    return 0;
}

void EI_WeightsFree (
    EIT_WEIGHTS * Weights)
{
    STC_FreeMemory (Weights->Value);
}

int EI_WeightsParse (
    char * WeightsString,
    EIT_WEIGHTS * Weights)
{
    EIT_RETURNCODE crc; /* cumulative return code */
    int Done;
    int ErrorCount;
    int rc;
    tToken Token;

    mWeightsString = STC_StrDup (WeightsString);
    UTIL_StrUpper (mWeightsString, mWeightsString);
    UTIL_DropBlanks (mWeightsString);
    mWeightsStringPtr = mWeightsString;

    mWeights = Weights;

    crc = EIE_SUCCEED;
    Done = 0;
    ErrorCount = 0;
    while (!Done) {
        rc = Scanner (&Token);
        if (rc == 0) {
            rc = MatchWeights (&Token);
            if (rc == 0) {
                Done = 1;
            }
        }

        /* handle error if any of the previous function call failed */
        if (rc != EIE_SUCCEED) {
            crc = EIE_FAIL;
            ErrorCount++;
            if (ErrorCount >= PARSER_MAX_ERROR_COUNT) {
                /* quit when PARSER_MAX_ERROR_COUNT errors are found */
                Done = 1;
                EI_AddMessage (M00039, EIE_ERROR, /* Weights parser */
                    M30003); /* Too many errors. Stop processing. */
            }
            else {
                /* advance to the next edits */
                mWeightsStringPtr = strchr (mWeightsStringPtr, ';');
                if (mWeightsStringPtr == NULL)
                    Done = 1;
                else {
                    mWeightsStringPtr++;
                    if (*mWeightsStringPtr == '\0') {
                        Done = 1;
                    }
                }
            }
        }
    }

    STC_FreeMemory (mWeightsString);

    return crc;
}

/*
(debug function)
*/
void EI_WeightsPrint (
    EIT_WEIGHTS * Weights)
{
    int i;

    for (i = 0; i < Weights->NumberOfFields; i++) {
        printf ("%*s ", 8, Weights->FieldName[i]);
    }
    puts ("");

    for (i = 0; i < Weights->NumberOfFields; i++) {
        printf ("%8.2f ", Weights->Value[i]);
    }
    puts ("");

    printf ("Columns=%d\n", Weights->NumberOfFields);
}


/*
A number was found.
*/
static int InstallNumber (
    double Number)
{
    if (DEBUG) puts (" InstallNumber");

    if (EIM_DBL_EQ (Number, 0.0)) {
        ReportError (M30105); /* Number must be greater than 0. */

        return -1;
    }

    mTerm.Number = Number;

    return 0;
}

/*
A variable was found.
*/
static void InstallVariable (
    char * Variable)
{
    int Found;
    int i;

    if (DEBUG) puts (" InstallVariable");

    Found = 0;
    for (i = 0; i < mWeights->NumberOfFields; i++) {
        if (strcmp (Variable, mWeights->FieldName[i]) == 0) {
            Found = 1;
        }
    }

    if (!Found) {
        char Message[1001];

        sprintf (Message,
            M20027, /* Variable %s has been assigned a weight but is not in the
                       edits. */
            Variable);
        ReportWarning (Message);
    }

    strcpy (mTerm.Variable, Variable);
}

/*
A weight was found.
*/
static void InstallWeight (void) {
    int i;

    if (DEBUG) puts (" InstallWeight");

    for (i = 0; i < mWeights->NumberOfFields; i++) {
        if (strcmp (mTerm.Variable, mWeights->FieldName[i]) == 0) {
            mWeights->Value[i] = mTerm.Number;
            return;
        }
    }
}

/*
Make sure the last token read match a specified token id.
If it does, call the scanner to find the next token.
If it does not, returns -1.
*/
static int Match (
    tTokenId Id,
    tToken * Token)
{
    if (Id != Token->Id) {
        char Message[101];

        sprintf (Message,
            M30007 "\n", /* Looking for '%s' but found '%s' instead. */
            TokenIdTranslator (Id), TokenIdTranslator (Token->Id));
        ReportError (Message);
        return -1;
    }

    return Scanner (Token);
}

/*
*/
static int MatchNumber (
    tToken * Token)
{
    int rc;

    if (Token->Id == eTokenIdNumber) {
        rc = InstallNumber (Token->Value.Number);
        if (rc != 0) return rc;
    }

    rc = Match (eTokenIdNumber, Token);
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchVariable (
    tToken * Token)
{
    int rc;

    if (Token->Id == eTokenIdVariable)
        InstallVariable (Token->Value.Variable);

    rc = Match (eTokenIdVariable, Token);
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchWeight (
    tToken * Token)
{
    int rc;

    rc = MatchVariable (Token);
    if (rc != 0) return rc;
    rc = Match (eTokenIdEqual, Token);
    if (rc != 0) return rc;
    rc = MatchNumber (Token);
    if (rc != 0) return rc;
    rc = Match (eTokenIdSemiColon, Token);
    if (rc != 0) return rc;

    InstallWeight ();

    return 0;
}

/*
*/
static int MatchWeights (
    tToken * Token)
{
    int rc;

    rc = MatchWeight (Token);
    if (rc != 0) return rc;
    while (Token->Id != eTokenIdDone) {
        rc = MatchWeight (Token);
        if (rc != 0) return rc;
    }

    return 0;
}

/*
lexical analyser.
reads the input and finds the next token.
set Token->Id to eTokenIdUnknownCharacter if the token is not recognise.
*/
static int Scanner (
    tToken * Token)
{
    char * p = mWeightsStringPtr;

    /* skip spaces*/
    while (isspace(*p))
        p++;

    if (strspn (p, EI_InfoGetVariableNameFirstCharacterCharacterSet ()) > 0) {
        /* found variable name */
        size_t n;

        Token->Value.Variable[0] = *p; /* save first character */
        p++;

        n = strspn (p, EI_InfoGetVariableNameCharacterSet ());

        if (n > (size_t) EI_InfoGetVariableNameMaxSize ()-1) {
            Token->Id = eTokenIdError;
            ReportError (M30009); /* Variable name too large! */
            return -1;
        }

        Token->Id = eTokenIdVariable;
        strncpy (Token->Value.Variable+1, p, n);
        Token->Value.Variable[n+1] = '\0';
        p += n;
    }
    else if (isdigit (*p) || (*p == '.' && isdigit (*(p+1)))) {
        /* found number */
        char * s;

        Token->Id = eTokenIdNumber;
        Token->Value.Number = strtod (p, &s);
        p = s;
    }
    else if (*p == ';') {
        /* found SemiColon */
        Token->Id = eTokenIdSemiColon;
        p++;
    }
    else if (*p == '=') {
        /* found equality operator */
        Token->Id = eTokenIdEqual;
        p++;
    }
    else if (*p == '\0') {
        /* found the end */
        Token->Id = eTokenIdDone;
    }
    else {
        /* found an unknown character */
        Token->Id = eTokenIdUnknownCharacter;
    }

    if (DEBUG) TokenPrint (Token);

    mWeightsStringPtr = p;

    return 0;
}

/*
Adds a error message to the message list.
*/
static void ReportError (
    char * String)
{
    /* M00039 -> "Weights parser"
       M00002 -> "Error at or before" */
    EI_AddMessage (M00039, EIE_ERROR, "%s\n%s:\n%s\n%*s^\n",       
        String, M00002, mWeightsString, mWeightsStringPtr-mWeightsString, "");
}

/*
Adds a warning message to the message list.
*/
static void ReportWarning (
    char * String)
{
    /* M00039 -> "Weights parser" */
    EI_AddMessage (M00039, EIE_WARNING,
       "%s\n", String, "");
}

/*
returns a description for a token.
*/
static char * TokenIdTranslator (
    tTokenId TokenId)
{
    switch (TokenId) {
    case eTokenIdDone:             return (M00005); /* Done */
    case eTokenIdEqual:            return (M00040); /* Equal */
    case eTokenIdError:            return (M00006); /* Error */
    case eTokenIdNumber:           return (M00041); /* Number */
    case eTokenIdSemiColon:        return (M00011); /* SemiColon */
    case eTokenIdVariable:         return (M00013); /* Variable */
    case eTokenIdUnknownCharacter: return (M00014); /* Unknown Character */
    /* no default */
    }
    return (M00042); /* Unknown token */
}

/*
print token and value if applicable (debug function).
*/
static void TokenPrint (
    tToken * Token)
{
    switch (Token->Id) {
    case eTokenIdDone:
    case eTokenIdEqual:
    case eTokenIdError:
    case eTokenIdSemiColon:
    case eTokenIdUnknownCharacter:
    default:
        printf ("%s\n", TokenIdTranslator (Token->Id));
        break;
    case eTokenIdNumber:
        printf ("%s %f\n", TokenIdTranslator (Token->Id),
            Token->Value.Number);
        break;
    case eTokenIdVariable:
        printf ("%s %s\n", TokenIdTranslator (Token->Id),
            Token->Value.Variable);
        break;
    }
}
