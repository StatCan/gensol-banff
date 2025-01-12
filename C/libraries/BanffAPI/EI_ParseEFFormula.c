/* contient le parseur de formule ef */

/*
This is a list of the productions for the parser:

efformula   -> expression
expression  -> term moreterms
moreterms   -> '+' term moreterms
            |  '-' term moreterms
            |  NOTHING
term        -> power morepowers
morepowers  -> '*' power morepowers
            |  '/' power morepowers
            |  NOTHING
power       -> factor exponents
exponents   -> '^' number exponents
            |  '^' '-' number exponents
            |  NOTHING
factor      -> operand
            |  '(' expression ')'
            |  '-' factor
operand     -> number
            |  variable
number      -> TOKEN_NUMBER
variable    -> TOKEN_FIELDID attributes
            |  TOKEN_AUX attributes
attributes  -> '(' TOKEN_PERIOD ',' TOKEN_AGGREGATION ')'
            |  '(' TOKEN_PERIOD ')'
            |  '(' TOKEN_AGGREGATION ',' TOKEN_PERIOD ')'
            |  '(' TOKEN_AGGREGATION ')'
            |  NOTHING

TOKEN_AUX { 'aux' followed by integer }
TOKEN_AGGREGATION { one of 'a', 'v' }
TOKEN_FIELDID { 'fieldid' }
TOKEN_NUMBER { list of digits }
TOKEN_PERIOD { one of 'c', 'h' }
*/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EI_Common.h"
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
    eTokenIdAggregate,
    eTokenIdCaret,
    eTokenIdComma,
    eTokenIdDone,
    eTokenIdError,
    eTokenIdMinus,
    eTokenIdNumber,
    eTokenIdOperand,
    eTokenIdParenthesisRight,
    eTokenIdParenthesisLeft,
    eTokenIdPeriod,
    eTokenIdPlaceHolder,
    eTokenIdPlus,
    eTokenIdSlash,
    eTokenIdStar,
    eTokenIdUnaryMinus,
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
    EIT_AGGREGATE Aggregate;
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


static char * AggregateTranslator (EIT_AGGREGATE);
static EIT_RETURNCODE Emit (tTokenId, tTokenValue *, EIT_STACK *);
static EIT_RETURNCODE Match (tTokenId, tToken *);
static EIT_RETURNCODE MatchExponent (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchExpression (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchFactor (tToken *, EIT_STACK *);
static EIT_RETURNCODE MatchOperand (tToken *, EIT_STACK *);
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



/*********************************************************************
*********************************************************************/
EIT_RETURNCODE EI_ParseEFFormula (
    char * EFFormulaString,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;
    tToken Token;

    mFormula = STC_StrDup (EFFormulaString);
    UTIL_StrUpper (mFormula, mFormula);
    UTIL_StrRemoveSpaces (mFormula, mFormula);
    mFormulaPtr = mFormula;

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
returns a description for an aggregate (debug function).
*********************************************************************/
static char * AggregateTranslator (
    EIT_AGGREGATE Aggregate)
{
    assert (0 <= Aggregate && Aggregate < EIE_AGGREGATE_INVALID);

    switch (Aggregate) {
    CASE (EIE_AGGREGATE_AVERAGE)
    CASE (EIE_AGGREGATE_VALUE)
    default:
        puts ("AggregateTranslator (): Unknown Aggregate"); fflush (NULL);
        exit (-1);
    }
    return "Will never be here!";
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE Emit (
    tTokenId TokenId,
    tTokenValue * TokenValue,
    EIT_STACK * Stack)
{
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
    case eTokenIdCaret:
        Entry.Operation = EIE_OPERATION_POWER;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdMinus:
        Entry.Operation = EIE_OPERATION_SUBTRACT;
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
            printf ("\t%s %s %s",
                TokenValue->Variable.Name,
                PeriodTranslator (TokenValue->Variable.Period),
                AggregateTranslator (TokenValue->Variable.Aggregate));
        Entry.Operation = EIE_OPERATION_PUSHSYMBOL;
        Entry.PlaceHolder = STC_StrDup (TokenValue->Variable.Name);
        Entry.Period = TokenValue->Variable.Period;
        Entry.Aggregate = TokenValue->Variable.Aggregate;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdPlus:
        Entry.Operation = EIE_OPERATION_ADD;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdSlash:
        Entry.Operation = EIE_OPERATION_DIVIDE;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdStar:
        Entry.Operation = EIE_OPERATION_MULTIPLY;
        if (EI_StackPush (Stack, &Entry) == EIE_FAIL) return EIE_FAIL;
        break;

    case eTokenIdUnaryMinus:
        Entry.Operation = EIE_OPERATION_NEGATE;
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
        rc = Emit (Token->Id, &Token->Value, Stack);
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
    tTokenId TokenId;

    if (DEBUG) puts ("MatchExpression");

    rc = MatchTerm (Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    while (Token->Id == eTokenIdMinus || Token->Id == eTokenIdPlus) {
        TokenId = Token->Id;
        rc = Match (Token->Id, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchTerm (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (TokenId, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchFactor (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchFactor");

    if (Token->Id == eTokenIdMinus) {
        Token->Id = eTokenIdUnaryMinus;
        rc = Match (eTokenIdUnaryMinus, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchFactor (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (eTokenIdUnaryMinus, NULL, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else if (Token->Id == eTokenIdParenthesisLeft) {
        rc = Match (eTokenIdParenthesisLeft, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchExpression (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Match (eTokenIdParenthesisRight, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else {
        rc = MatchOperand (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    return EIE_SUCCEED;
}
/*********************************************************************
*********************************************************************/
static EIT_RETURNCODE MatchOperand (
    tToken * Token,
    EIT_STACK * Stack)
{
    EIT_RETURNCODE rc;

    if (DEBUG) puts ("MatchOperand");

    if (Token->Id == eTokenIdNumber) {
        rc = Emit (eTokenIdNumber, &Token->Value, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Match (eTokenIdNumber, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else {
        rc = MatchVariable (Token, Stack);
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

    rc = MatchFactor (Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    while (Token->Id == eTokenIdCaret) {
        rc = Match (eTokenIdCaret, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchExponent (Token, Stack);
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
    tTokenId TokenId;

    if (DEBUG) puts ("MatchTerm");

    rc = MatchPower (Token, Stack);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    while (Token->Id == eTokenIdSlash || Token->Id == eTokenIdStar) {
        TokenId = Token->Id;
        rc = Match (TokenId, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchPower (Token, Stack);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = Emit (TokenId, NULL, Stack);
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

    Token->Value.Variable.Period = EIE_PERIOD_CURRENT;
    Token->Value.Variable.Aggregate = EIE_AGGREGATE_VALUE;

    if (Token->Id == eTokenIdPlaceHolder) {
        rc = Match (eTokenIdPlaceHolder, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        if (Token->Id == eTokenIdParenthesisLeft) {
            rc = Match (eTokenIdParenthesisLeft, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
            if (Token->Id == eTokenIdPeriod) {
                rc = Match (eTokenIdPeriod, Token);
                if (rc != EIE_SUCCEED) return EIE_FAIL;
                if (Token->Id == eTokenIdComma) {
                    rc = Match (eTokenIdComma, Token);
                    if (rc != EIE_SUCCEED) return EIE_FAIL;
                    rc = Match (eTokenIdAggregate, Token);
                    if (rc != EIE_SUCCEED) return EIE_FAIL;
                }
            }
            else {
                rc = Match (eTokenIdAggregate, Token);
                if (rc != EIE_SUCCEED) return EIE_FAIL;
                if (Token->Id == eTokenIdComma) {
                    rc = Match (eTokenIdComma, Token);
                    if (rc != EIE_SUCCEED) return EIE_FAIL;
                    rc = Match (eTokenIdPeriod, Token);
                    if (rc != EIE_SUCCEED) return EIE_FAIL;
                }
            }
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
                            but found '%s' instead. */
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

    /* M00035 -> "Estimator function parser"
       M00002 -> "Error at or before" */
    EI_AddMessage (M00035, EIE_ERROR, "%s\n%s:\n%s",
        ErrorMessage, M00002, String);

    STC_FreeMemory (String);
    for (i = 0; i < NumberOfSplitedString; i++)
        STC_FreeMemory (SplitedString[i]);
    STC_FreeMemory (SplitedString);
}
/*********************************************************************
lexical analyser.
reads the input and finds the next token.
set Token->Id to eTokenIdUnknownCharacter if the token is not recognise.
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
        /* found aux, aggr, fieldid or period */
        size_t n;

        n = strspn (p, "AUXAVFIELDIDCH1234567890");

        if (n > TOKEN_SIZE) {
            Token->Id = eTokenIdError;
            ReportError (M30085); /* Token too large. */
            return EIE_FAIL;
        }

        strncpy (String, p, n);
        String[n] = '\0';
        p += n;

        if (strncmp (String, "AUX", 3) == 0) {
            if (!UTIL_IsInteger (String+3)) {
                Token->Id = eTokenIdError;
                ReportError (M30086); /* AUX not followed by integer. */
                return EIE_FAIL;
            }
            Token->Id = eTokenIdPlaceHolder;
            /* clean placeholder: replace AUX0003 by AUX3 */
            sprintf (Token->Value.Variable.Name, "AUX%d", atoi (String+3));  // nosemgrep  // ignore non-critical logging code
        }
        else if (strcmp (String, "A") == 0) {
            Token->Id = eTokenIdAggregate;
            Token->Value.Variable.Aggregate = EIE_AGGREGATE_AVERAGE;
        }
        else if (strcmp (String, "C") == 0) {
            Token->Id = eTokenIdPeriod;
            Token->Value.Variable.Period = EIE_PERIOD_CURRENT;
        }
        else if (strcmp (String, "FIELDID") == 0) {
            Token->Id = eTokenIdPlaceHolder;
            strcpy (Token->Value.Variable.Name, "FIELDID");
        }
        else if (strcmp (String, "H") == 0) {
            Token->Id = eTokenIdPeriod;
            Token->Value.Variable.Period = EIE_PERIOD_HISTORICAL;
        }
        else if (strcmp (String, "V") == 0) {
            Token->Id = eTokenIdAggregate;
            Token->Value.Variable.Aggregate = EIE_AGGREGATE_VALUE;
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
    else if (*p == '+') {
        Token->Id = eTokenIdPlus;
        p++;
    }
    else if (*p == '*') {
        Token->Id = eTokenIdStar;
        p++;
    }
    else if (*p == '/') {
        Token->Id = eTokenIdSlash;
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
    printf ("%s", TokenIdTranslator (TokenId));
}
/*********************************************************************
returns a description for a token.
*********************************************************************/
static char * TokenIdTranslator (
    tTokenId TokenId)
{
    if (DEBUG) assert (0 <= TokenId && TokenId < eTokenIdInvalid);

    switch (TokenId) {
    CASE (eTokenIdAggregate)
    CASE (eTokenIdCaret)
    CASE (eTokenIdComma)
    CASE (eTokenIdDone)
    CASE (eTokenIdError)
    CASE (eTokenIdMinus)
    CASE (eTokenIdNumber)
    CASE (eTokenIdOperand)
    CASE (eTokenIdParenthesisLeft)
    CASE (eTokenIdParenthesisRight)
    CASE (eTokenIdPeriod)
    CASE (eTokenIdPlaceHolder)
    CASE (eTokenIdPlus)
    CASE (eTokenIdSlash)
    CASE (eTokenIdStar)
    CASE (eTokenIdUnaryMinus)
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
        printf (" %s %s %s\n",
            Token->Value.Variable.Name,
            PeriodTranslator (Token->Value.Variable.Period),
            AggregateTranslator (Token->Value.Variable.Aggregate));

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
    char Message[1001];
    EIT_RETURNCODE rc;

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
                /*
                Kludge: do not remove the Number < 0 test.
                When the string is very long, atoi return a negative value.
                */
                else if (Number < 0 || Number > 100) {
                    sprintf (Message, M30090 "\n", /* AUX%d is too big. */
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
    for (i = 1; i < MaxNumber; i++) {
        if (AuxNumbers[i] == 0) {
            sprintf (Message, M30091 "\n", /* "Missing AUX%d. */
                i);
            ReportError (Message);
            rc = EIE_FAIL;
        }
    }

    return rc;
}
/*********************************************************************
Verify that current value of fieldid was NOT specified.
*********************************************************************/
static EIT_RETURNCODE VerifyFieldIdCurrent (
    EIT_STACK * Stack)
{
    int i;
    char Message[1001];

    for (i = 0; i < Stack->NumberEntries; i++) {
        if (Stack->Entry[i].Operation == EIE_OPERATION_PUSHSYMBOL &&
                Stack->Entry[i].Period == EIE_PERIOD_CURRENT &&
                Stack->Entry[i].Aggregate == EIE_AGGREGATE_VALUE &&
                strcmp (Stack->Entry[i].PlaceHolder, "FIELDID") == 0) {
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

    return crc;
}
