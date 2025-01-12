/* contient le parseur de formule lr */

/*
This is a list of the productions for the parser:

lrformula   -> expression
expression  -> term moreterms
moreterms   -> ',' term moreterms
            |  NOTHING
term        -> intercept
            |  power
intercept   -> TOKEN_INTERCEPT
power       -> variable exponent
exponent    -> '^' number
            |  '^' '-' number
            |  NOTHING
number      -> TOKEN_NUMBER
variable    -> TOKEN_FIELDID attributes
            |  TOKEN_AUX attributes
attributes  -> '(' TOKEN_PERIOD ')'
            |  NOTHING

TOKEN_AUX { 'aux' followed by integer }
TOKEN_FIELDID { 'fieldid' }
TOKEN_INTERCEPT { 'intercept' }
TOKEN_NUMBER { list of digits }
TOKEN_PERIOD { one of 'c', 'h' }
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
#include "EI_Estimator.h"
#include "EI_Message.h"
#include "EI_Stack.h"
#include "EI_Symbol.h"

#include "EIP_Common.h"
#include "STC_Memory.h"

#include "util.h"

#include "MessageBanffAPI.h"

/*
too big or too small!
use this to format double value
example:
double d;
d=999999999999;
if (TOOEXTREME (d))
    printf ("%e", d);
else
    printf ("%f", d);
*/
#define TOOEXTREME(d) ((d)!=0.0&&(((d)>=100000||(d)<=-100000)||((d)<.00001&&(d)>-.00001)))

enum TokenId {
    eTokenIdBeta,
    eTokenIdCaret,
    eTokenIdComma,
    eTokenIdDone,
    eTokenIdError,
    eTokenIdIntercept,
    eTokenIdMinus,
    eTokenIdMultiply,
    eTokenIdNumber,
    eTokenIdParenthesisLeft,
    eTokenIdParenthesisRight,
    eTokenIdPeriod,
    eTokenIdPlaceHolder,
    eTokenIdInvalid
};
typedef enum TokenId tTokenId;

/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

#undef TOKEN_SIZE
#define TOKEN_SIZE 100

#undef CASE
#define CASE(X) case(X):return(#X);

struct Variable {
    char Name[TOKEN_SIZE+1];
    EIT_PERIOD Period;
    double Exponent;
};
typedef struct Variable tVariable;

union TokenValue {
    double Number;
    tVariable Variable;
};
typedef union TokenValue tTokenValue;

struct Token {
    tTokenId Id;
    tTokenValue Value;
};
typedef struct Token tToken;


/* points to the beginning of the formula string. */
static char * mFormula;
/* points somewhere in mFormula. the scanner moves it as it scans the string. */
static char * mFormulaPtr;
/* Next beta number to add to stack */
static int mBetaNumber;


static EIT_RETURNCODE Emit (tTokenId, tTokenValue *, EIT_STACK *);
static EIT_RETURNCODE Match (tTokenId, tToken *);
static EIT_RETURNCODE MatchExponent (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchExpression (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchPower (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchTerm (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchVariable (tToken *, EIT_STACK *);
static char * PeriodTranslator (EIT_PERIOD);
static void ReportError (char *);
static EIT_RETURNCODE Scanner (tToken *);
static char * TokenIdTranslator (tTokenId);
static void TokenPrint (tToken *);
static void TokenIdPrint (tTokenId);
static EIT_RETURNCODE VerifyAuxNumbering (EIT_STACK *);
static EIT_RETURNCODE VerifyFieldIdCurrent (EIT_STACK *);
static EIT_RETURNCODE VerifyFormula (EIT_STACK *);
static EIT_RETURNCODE VerifyPlaceHolderUniqueness (EIT_STACK *);



/*********************************************************************
*********************************************************************/
EIT_RETURNCODE EI_ParseLRFormula (
    char * LRFormulaString,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;
    tToken Token;

    mFormula = STC_StrDup (LRFormulaString);
    UTIL_StrUpper (mFormula, mFormula);
    UTIL_StrRemoveSpaces (mFormula, mFormula);
    mFormulaPtr = mFormula;
    mBetaNumber = 1;

    if (DEBUG) puts (mFormula);

    rc = Scanner (&Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    rc = MatchExpression (&Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    rc = Match (eTokenIdDone, &Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    rc = VerifyFormula (Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    STC_FreeMemory (mFormula);

    return EIE_SUCCEED;
}




/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE Emit (
    tTokenId TokenId,
    tTokenValue * TokenValue,
    EIT_STACK * Stack)
{
    char BetaStr[101];

    EIT_STACK_ENTRY Entry;

    if (DEBUG) TokenIdPrint (TokenId);

    /* rene met ca dans EI_StackInitEntry()? */
    Entry.Operation = EIE_OPERATION_INVALID;
    Entry.Value = 0xdeadbeef;
    Entry.PlaceHolder = NULL;
    Entry.Period = EIE_PERIOD_CURRENT; /* default */
    Entry.Aggregate = EIE_AGGREGATE_VALUE; /* default */
    Entry.Symbol = NULL;

    switch (TokenId) {
    case eTokenIdBeta:
        Entry.Operation = EIE_OPERATION_PUSHSYMBOL;
        sprintf (BetaStr, EIM_BETA_PREFIX "%d", mBetaNumber++);
        Entry.PlaceHolder = STC_StrDup (BetaStr);
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdCaret:
        Entry.Operation = EIE_OPERATION_POWER;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdComma:
        Entry.Operation = EIE_OPERATION_ADD;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdIntercept:
        Entry.Operation = EIE_OPERATION_PUSHSYMBOL;
        Entry.PlaceHolder = STC_StrDup (EIM_BETA_PREFIX "0");
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdMultiply:
        Entry.Operation = EIE_OPERATION_MULTIPLY;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdNumber:
        if (DEBUG) printf ("\t%f", TokenValue->Number);
        Entry.Operation = EIE_OPERATION_PUSH;
        Entry.Value = TokenValue->Number;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdPlaceHolder:
        if (DEBUG)
            printf ("\t%s %s",
                TokenValue->Variable.Name,
                PeriodTranslator (TokenValue->Variable.Period));
        Entry.Operation = EIE_OPERATION_PUSHSYMBOL;
        Entry.PlaceHolder = STC_StrDup (TokenValue->Variable.Name);
        Entry.Period = TokenValue->Variable.Period;
        Entry.Aggregate = EIE_AGGREGATE_VALUE;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    default:
        if (DEBUG) {
            printf ("Emit(): Unknown token <%d>\n", TokenId);
            fflush (NULL);
        }
        ReportError (M30081); /* Emit(): Unknown token */
        return EIE_FAIL;
    }

    if (DEBUG) assert (Entry.Operation != EIE_OPERATION_INVALID);

    return EIE_SUCCEED;
}
/*********************************************************************
Make sure the last token read match a specified token id.
If it does, call the scanner to find the next token.
If it does not, returns -1.
*********************************************************************/
static EIT_RETURNCODE Match (
    tTokenId Id,
    tToken * Token)
{
    if (Id != Token->Id) {
        char Message[101];

        sprintf (Message,
            /* Looking for '%s' but found '%s' instead. */
            M30007 "\n", 
            TokenIdTranslator (Id) , TokenIdTranslator (Token->Id));
        ReportError (Message);
        return EIE_FAIL;
    }

    return Scanner (Token);
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchExponent (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;
    double sign = 1;

    if (DEBUG) puts ("MatchExponent");

    if (Token->Id == eTokenIdMinus) {
        sign = -1;
        rc = Match (Token->Id, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    if (Token->Id == eTokenIdNumber) {
        Token->Value.Number *= sign;
        rc = Emit (eTokenIdNumber, &Token->Value, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Match (eTokenIdNumber, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else {
        ReportError (M30083); /* Exponent(): '^' must be followed by a number */
        return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchExpression (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchExpression");

    rc = MatchTerm (Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    while (Token->Id == eTokenIdComma) {
        rc = Match (eTokenIdComma, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchTerm (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdComma, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchPower (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchPower");

    rc = MatchVariable (Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    if (Token->Id == eTokenIdCaret) {
        rc = Match (eTokenIdCaret, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchExponent (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdCaret, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    } else {
        /* force default exponent (1) */
        Token->Value.Number = 1.0;
        rc = Emit (eTokenIdNumber, &Token->Value, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdCaret, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchTerm (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchTerm");

    if (Token->Id == eTokenIdIntercept) {
        rc = Match (eTokenIdIntercept, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdIntercept, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else if (Token->Id == eTokenIdPlaceHolder) {
        rc = Emit (eTokenIdBeta, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchPower (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdMultiply, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchVariable (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchVariable");

    Token->Value.Variable.Exponent = 1;
    Token->Value.Variable.Period = EIE_PERIOD_CURRENT;

    if (Token->Id == eTokenIdPlaceHolder) {
        rc = Match (eTokenIdPlaceHolder, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        if (Token->Id == eTokenIdParenthesisLeft) {
            rc = Match (eTokenIdParenthesisLeft, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
            rc = Match (eTokenIdPeriod, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
            rc = Match (eTokenIdParenthesisRight, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
        }
        rc = Emit (eTokenIdPlaceHolder, &Token->Value, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else {
        char Message[1001];

        sprintf (Message,
            M30084 "\n", /* Looking for 'Aux' or 'FieldId' 
                            but found '%s' instead.*/
            TokenIdTranslator (Token->Id));
        ReportError (Message);
        return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
Adds a message to the message list.
The string is cut in pieces before printing and the caret point to
the line where the error occurs.
*********************************************************************/
static void ReportError (
    char * ErrorMessage)
{
#define SIZEOFCHUNKS 72

    int CaretPosition;
    int i;
    int NumberOfSplitedString;
    char ** SplitedString;
    char * String;

    CaretPosition = (int) (mFormulaPtr-mFormula);

    /* String size must be big enough to hold:
       the edits +
       newline characters +
       the line of the caret + its newline characters +
       eol character */
    String = STC_AllocateMemory (
        strlen (mFormula) +
        strlen (mFormula)/SIZEOFCHUNKS + 1 +
        SIZEOFCHUNKS + 1 +
        1);

    String[0] = '\0';

    NumberOfSplitedString = UTIL_StrSplit (mFormula, SIZEOFCHUNKS,
        &SplitedString);

    for (i = 0; i < NumberOfSplitedString; i++) {
        strcat (String, SplitedString[i]);
        strcat (String, "\n");
        if (0 <= CaretPosition && CaretPosition < SIZEOFCHUNKS) {
            sprintf (String+strlen(String), "%*s^\n",
                CaretPosition, "");
        }
        CaretPosition -= SIZEOFCHUNKS;
    }

    /* M00036 -> "Linear regression parser"
       M00002 -> "Error at or before" */
    EI_AddMessage (M00036, EIE_ERROR, "%s\n%s:\n%s",
        ErrorMessage, M00002, String);

    STC_FreeMemory (String);
    for (i = 0; i < NumberOfSplitedString; i++)
        STC_FreeMemory (SplitedString[i]);
    STC_FreeMemory (SplitedString);
}
/*********************************************************************
lexical analyser.
reads the input and finds the next token.
set Token->Id to eTokenIdError if the token is not recognise.
*********************************************************************/
static EIT_RETURNCODE Scanner (
    tToken * Token)
{
    char String[TOKEN_SIZE+1];
    char * p = mFormulaPtr;

    /* skip spaces*/
    while (isspace (*p))
        p++;

    if (isalpha (*p)) {
        /* found aux, fieldid or period */
        size_t n;

        n = strspn (p, "AUXCFIELDIDHINTERCEPT1234567890");

        if (n > TOKEN_SIZE) {
            Token->Id = eTokenIdError;
            ReportError (M30085); /* Token too large. */
            return EIE_FAIL;
        }

        strncpy (String, p, n);
        String[n] = '\0';
        p += n;

        if (strncmp (String, "AUX", 3) == 0) {
            Token->Id = eTokenIdPlaceHolder;
            if (!UTIL_IsInteger (String+3)) {
                Token->Id = eTokenIdError;
                ReportError (M30086); /* AUX not followed by integer. */
                return EIE_FAIL;
            }
            strcpy (Token->Value.Variable.Name, String);
        }
        else if (strcmp (String, "C") == 0) {
            Token->Id = eTokenIdPeriod;
            Token->Value.Variable.Period = EIE_PERIOD_CURRENT;
        }
        else if (strcmp (String, "FIELDID") == 0) {
            Token->Id = eTokenIdPlaceHolder;
            strcpy (Token->Value.Variable.Name, String);
        }
        else if (strcmp (String, "H") == 0) {
            Token->Id = eTokenIdPeriod;
            Token->Value.Variable.Period = EIE_PERIOD_HISTORICAL;
        }
        else if (strcmp (String, "INTERCEPT") == 0) {
            Token->Id = eTokenIdIntercept;
        }
        else {
            Token->Id = eTokenIdError;
            ReportError (M30087); /* Syntax error. */
            return EIE_FAIL;
        }
    }
    else if (isdigit (*p) || (*p == '.' && isdigit (*(p+1)))) {
        /* found number */
        char * s;

        Token->Id = eTokenIdNumber;
        Token->Value.Number = strtod (p, &s);
        /* when the Number is close to zero report an error */
        if (EIM_DBL_EQ (Token->Value.Number, 0.0)) {
            Token->Id = eTokenIdError;
            ReportError (M30088); /* Zero constant is illegal. */
            return EIE_FAIL;
        }
        p = s;
    }
    else if (*p == '(') {
        Token->Id = eTokenIdParenthesisLeft;
        p++;
    }
    else if (*p == ')') {
        Token->Id = eTokenIdParenthesisRight;
        p++;
    }
    else if (*p == '^') {
        Token->Id = eTokenIdCaret;
        p++;
    }
    else if (*p == '-') {
        Token->Id = eTokenIdMinus;
        p++;
    }
    else if (*p == ',') {
        Token->Id = eTokenIdComma;
        p++;
    }
    else if (*p == '\0') {
        Token->Id = eTokenIdDone;
    }
    else {
        Token->Id = eTokenIdError;
        ReportError (M30087); /* Syntax error. */
        return EIE_FAIL;
    }

    if (DEBUG) TokenPrint (Token);

    mFormulaPtr = p;

    return EIE_SUCCEED;
}
/*********************************************************************
print token id
*********************************************************************/
static void TokenIdPrint (
    tTokenId TokenId)
{
    printf ("%s\n", TokenIdTranslator (TokenId));
}
/*********************************************************************
returns a description for a token.
*********************************************************************/
static char * TokenIdTranslator (
    tTokenId TokenId)
{
    if (DEBUG) assert (0 <= TokenId && TokenId < eTokenIdInvalid);

    switch (TokenId) {
    CASE (eTokenIdCaret)
    CASE (eTokenIdComma)
    CASE (eTokenIdDone)
    CASE (eTokenIdError)
    CASE (eTokenIdIntercept)
    CASE (eTokenIdMinus)
    CASE (eTokenIdNumber)
    CASE (eTokenIdParenthesisLeft)
    CASE (eTokenIdParenthesisRight)
    CASE (eTokenIdPeriod)
    CASE (eTokenIdPlaceHolder)
    default:
        puts ("TokenIdTranslator (): Unknown TokenId"); fflush (NULL);
        exit (-1);
    }
    return "Will never be here!";
}
/*********************************************************************
print token and value if applicable (debug function).
*********************************************************************/
static void TokenPrint (
    tToken * Token)
{
    assert (0 <= Token->Id && Token->Id < eTokenIdInvalid);

    TokenIdPrint (Token->Id);

    switch (Token->Id) {
    case eTokenIdPlaceHolder:
        printf (" %s %f %s\n",
            Token->Value.Variable.Name,
            Token->Value.Variable.Exponent,
            PeriodTranslator (Token->Value.Variable.Period));

        break;

    case eTokenIdNumber:
        printf (" %f\n", Token->Value.Number);
        break;
    }
}
/*********************************************************************
returns a description for an period (debug function).
*********************************************************************/
static char * PeriodTranslator (
    EIT_PERIOD Period)
{
    assert (0 <= Period && Period < EIE_PERIOD_INVALID);

    switch (Period) {
    CASE (EIE_PERIOD_CURRENT)
    CASE (EIE_PERIOD_HISTORICAL)
    default:
        puts ("PeriodTranslator (): Unknown Period"); fflush (NULL);
        exit (-1);
    }
    return "Will never be here!";
}
/*********************************************************************
Verify that aux place holder are numbered from 1 to N.
if auxN exists, auxN-1 must exists.
*********************************************************************/
static EIT_RETURNCODE VerifyAuxNumbering (
    EIT_STACK * Stack)
{
    int i;
    int AuxNumbers[100]; /* rene */
    int Number;
    int MaxNumber;
    EIT_RETURNCODE rc;
    char Message[1001];

    rc = EIE_SUCCEED;
    MaxNumber = -1;

    /* initialize */
    for (i=0; i<100; i++)
        AuxNumbers[i] = 0;

    /* set */
    for (i = 0; i < Stack->NumberEntries; i++) {
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL) {
            if (strncmp (Stack->Entry[i].PlaceHolder, "AUX", 3) == 0) {
                Number = atoi (Stack->Entry[i].PlaceHolder+3);  // nosemgrep  // not seeing this as an attack vector, ignore
                if (Number == 0) {
                    /* 0 is not a valid AUX number. */
                    sprintf (Message, M30089 "\n");
                    ReportError (Message);
                    rc = EIE_FAIL;
                }
                else if (Number > 100) {
                    sprintf (Message, M30090"\n", /* AUX%d is too big. */
                        Number);
                    ReportError (Message);
                    rc = EIE_FAIL;
                }
                else {
                    AuxNumbers[Number]++;
                    if (Number > MaxNumber) MaxNumber = Number;
                }
            }
        }
    }

    /* validate */
    for (i=1; i<MaxNumber; i++) {
        if (AuxNumbers[i] == 0) {
            sprintf (Message, M30091 "\n", /* Missing AUX%d. */
                i);
            ReportError (Message);
            rc = EIE_FAIL;
        }
    }

    return rc;
}
/*********************************************************************
Verify that fieldid current was NOT specified.
*********************************************************************/
static EIT_RETURNCODE VerifyFieldIdCurrent (
    EIT_STACK * Stack)
{
    int i;
    char Message[1001];

    for (i = 0; i < Stack->NumberEntries; i++) {
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL &&
                strcmp (Stack->Entry[i].PlaceHolder, "FIELDID") == 0 &&
                Stack->Entry[i].Period == EIE_PERIOD_CURRENT) {
                sprintf (
                    /* FIELDID cannot be specified for the current period. */
                    Message, M30092 "\n");
            ReportError (Message);
            return EIE_FAIL;
        }
    }
    return EIE_SUCCEED;
}
/*********************************************************************
Verify formula.
Make all verifications before quitting.
*********************************************************************/
static EIT_RETURNCODE VerifyFormula (
    EIT_STACK * Stack)
{
    EIT_RETURNCODE crc; /* cumulative return code */
    EIT_RETURNCODE rc;

    crc = EIE_SUCCEED;

    rc = VerifyAuxNumbering (Stack);
    if (rc != EIE_SUCCEED) crc = EIE_FAIL;

    rc = VerifyFieldIdCurrent (Stack);
    if (rc != EIE_SUCCEED) crc = EIE_FAIL;

    rc = VerifyPlaceHolderUniqueness (Stack);
    if (rc != EIE_SUCCEED) crc = EIE_FAIL;

    return crc;
}
/*********************************************************************
Verify that no place holder have the same Name + Period + Exponent
combination
*********************************************************************/
static EIT_RETURNCODE VerifyPlaceHolderUniqueness (
    EIT_STACK * Stack)
{
    EIT_STACK_ENTRY * EntryI;
    EIT_STACK_ENTRY * EntryIPlus1;
    EIT_STACK_ENTRY * EntryJ;
    EIT_STACK_ENTRY * EntryJPlus1;
    int i;
    int j;
    char Message[256];

/*puts ("VerifyPlaceHolderUniqueness()....");*/

    for (i = 0; i < Stack->NumberEntries-1; i++) {
        EntryI = &Stack->Entry[i];
        EntryIPlus1 = &Stack->Entry[i+1];
        if (EntryI->Operation == EIE_OPERATION_PUSHSYMBOL &&
            ((strncmp (EntryI->PlaceHolder, "AUX", 3) == 0) ||
             (strcmp (EntryI->PlaceHolder, "FIELDID") == 0) ||
             (strcmp (EntryI->PlaceHolder, EIM_BETA_PREFIX "0") == 0))) {
            for (j = i + 1; j < Stack->NumberEntries; j++) {
                EntryJ = &Stack->Entry[j];
                EntryJPlus1 = &Stack->Entry[j+1];
                if (EntryJ->Operation == EIE_OPERATION_PUSHSYMBOL &&
                    ((strncmp (EntryJ->PlaceHolder, "AUX", 3) == 0) ||
                     (strcmp (EntryJ->PlaceHolder, "FIELDID") == 0) ||
                     (strcmp (EntryJ->PlaceHolder, EIM_BETA_PREFIX "0") == 0))) {

/*printf ("i=%d %s %d %f\nj=%d %s %d %f\n\n",*/
/*    i, EntryI->PlaceHolder, EntryI->Period, EntryIPlus1->Value,*/
/*    j, EntryJ->PlaceHolder, EntryJ->Period, EntryJPlus1->Value);*/

                    if (strcmp (EntryI->PlaceHolder, EntryJ->PlaceHolder)==0 &&
                            EntryI->Period == EntryJ->Period &&
                            EntryIPlus1->Value == EntryJPlus1->Value) {

                        sprintf (
                             /* At least two regressors are identical including
                                intercept. They have the same name, period and
                                exponent combination. */
                            Message, M30093 "\n");
                        ReportError (Message);
                        return EIE_FAIL;
                    }
                }
            }
        }
    }
/*printf ("....VerifyPlaceHolderUniqueness()\n");*/

    return EIE_SUCCEED;
}
