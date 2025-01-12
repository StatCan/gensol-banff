/* contient toutes les fonctions reliees au type EIT_EDITS */
#ifndef EDITS_DISABLED

/*
edits parser
This is a list of the productions for the parser:

edits       -> edit
            |  edit edits
edit        -> components operator components ';'
            |  modifier ':' components operator components ';'
components  -> term
            |  term '+' components
            |  term '-' components
term        -> number
            |  number '*' variable
            |  variable
            |  variable '*' number
            |  '-' term
modifier    -> TOKEN_MODIFIER
number      -> TOKEN_NUMBER
operator    -> TOKEN_OPERATOR
variable    -> TOKEN_VARIABLE

TOKEN_MODIFIER {one of 'ACCEPTE'|'FAIL'|'PASS'|'REJET'}
TOKEN_NUMBER   {list of digits }
TOKEN_OPERATOR {one of '>'|'>='|'='|'!='|'<='|'<'}
TOKEN_VARIABLE {letter|underscore followed by list of letters|digits|underscore}
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Common.h"
#include "EI_Edits.h"
#include "EI_EditsConsistency.h"
#include "EI_Info.h"
#include "EI_Message.h"
#include "EIP_Common.h"
#include "STC_Memory.h"
#include "adtlist.h"
#include "slist.h"
#include "util.h"

#include "MessageCommonAPI.h"


#define CONSTANT ""
#define PARSER_MAX_ERROR_COUNT 7

enum ModifierIdEnum {
    eModifierIdFail,
    eModifierIdPass,
    eModifierIdRejet,
    eModifierIdAccepte
};
typedef enum ModifierIdEnum tModifierId;

enum OperatorIdEnum {
    eOperatorIdEqual,
    eOperatorIdGreater,
    eOperatorIdLess,
    eOperatorIdNotEqual
};
typedef enum OperatorIdEnum tOperatorId;

enum SideIdEnum {
    eSideIdLeft,
    eSideIdRight
};
typedef enum SideIdEnum tSideId;

enum TokenIdEnum {
    eTokenIdCoefficient,
    eTokenIdColon,
    eTokenIdDone,
    eTokenIdError,
    eTokenIdMinus,
    eTokenIdModifier,
    eTokenIdOperator,
    eTokenIdPlus,
    eTokenIdSemiColon,
    eTokenIdStar,
    eTokenIdUnknownCharacter,
    eTokenIdVariable
};
typedef enum TokenIdEnum tTokenId;


union ValueUnion {
    double Coefficient;
    tModifierId Modifier;
    tOperatorId Operator;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
};
typedef union ValueUnion tTokenValue;


struct TokenStructure {
    tTokenId Id;
    tTokenValue Value;
};
typedef struct TokenStructure tToken;

struct EditStructure {
    tModifierId Modifier;
    tOperatorId Operator;
    tADTList * TermList;
};
typedef struct EditStructure tEdit;

struct TermStructure {
    double Coefficient;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
};
typedef struct TermStructure tTerm;


static char * mValidModifier[] = {
    "FAIL", "PASS", "REJET", "ACCEPTE"
};


/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

/* contains info about the edit that the parser is currently working on. */
static tEdit mEdit;
/* contains the list of edits. */
static tADTList * mEditGroup;
/* points to the edits strings. */
static char * mEdits;
/* points somewhere in mEdits. the scanner moves it as it scans the string. */
static char * mEditsPtr;
/* for formatting edits */
static int mLength;
/* tell on which side (left or right) of the operator we are. terms on the
right side will be multiplied by -1 when inserted in the term list. */
static tSideId mSide;
/* for formatting edits */
static char * mSpace;
/* contains info about the term that the parser is currently working on. */
static tTerm mTerm;

static EIT_RETURNCODE AllocateEdits (EIT_EDITS * Edits, int NumberofEquations,
    int NumberofColumns);
static int CompareSearch (char * Variable, tTerm * Term);
static int CompareSort (tTerm * a, tTerm * b);
static void Concat (char *, char *, char *, char *, size_t *, int *);
static tOperatorId ConvertToPassOperator (tOperatorId Operator);
static void EditGroupFree (tADTList * EditGroup);
static void EditGroupInit (void);
static void EditGroupPrint (tADTList * EditGroup);
static void EditGroupTerminate (void);
static void EditInit (void);
static void EditPrint (tEdit * Edit);
static EIT_RETURNCODE EditTerminate (void);
static int FindNear0Coefficient (tADTList * TermList);
static EIT_RETURNCODE GenerateEdits (tADTList * EditGroup, EIT_EDITS * Edits);
static void InstallCoefficient (double Coefficient);
static EIT_RETURNCODE InstallEdit (void);
static void InstallMinus (void);
static void InstallModifier (tModifierId Modifier);
static void InstallOperator (tOperatorId Operator);
static void InstallTerm (void);
static void InstallVariable (char * Variable);
static EIT_RETURNCODE Match (tTokenId Id, tToken * Token);
static EIT_RETURNCODE MatchCoefficient (tToken * Token);
static EIT_RETURNCODE MatchComponents (tToken * Token);
static EIT_RETURNCODE MatchEdit (tToken * Token);
static EIT_RETURNCODE MatchEditGroup (tToken * Token);
static EIT_RETURNCODE MatchMinus (tToken * Token);
static EIT_RETURNCODE MatchModifier (tToken * Token);
static EIT_RETURNCODE MatchOperator (tToken * Token);
static EIT_RETURNCODE MatchTerm (tToken * Token);
static EIT_RETURNCODE MatchVariable (tToken * Token);
static char * ModifierIdTranslator (tModifierId ModifierId);
static void PrintLine (char *, char *, char *, int *);
static char * OperatorIdTranslator (tOperatorId OperatorId);
static void ReportError (char * String);
static void ReportWarning (char * String);
static EIT_RETURNCODE Scanner (tToken * Token);
static void TermInit (void);
static void TermPrint (tTerm * Term);
static void TermTerminate (void);
static char * TokenIdTranslator (tTokenId TokenId);
static void TokenPrint (tToken * Token);

/*
*/
EIT_EDITSAPPLY_RETURNCODE EI_EditsApply (
    EIT_EDITS * Edits, /* edits of an edit group                     */
    EIT_DATAREC * Data)/* recipient edit field values                */
{
    int i;

    /* record fail the edit if at least one missing or negative data */
    for (i = 0; i < Data->NumberofFields; i++)
        if (Data->StatusFlag[i] != FIELDOK)
            return EIE_EDITSAPPLY_FAIL;

    return EI_EditsApplyOnValues (Edits, Data->FieldValue);
}


/*
*/
EIT_EDITSAPPLY_RETURNCODE EI_EditsApplyOnValues (
    EIT_EDITS * Edits, /* edits of an edit group                     */
    double * Value)    /* recipient edit field values                */
{
    int ConstantIndex;
    int i;
    int j;
    double Sum;

    ConstantIndex = Edits->NumberofColumns - 1;

    for (i = 0; i < Edits->NumberofEquations; i++) {

        Sum = 0.0;
        for (j = 0; j < ConstantIndex; j++)
            Sum += (Edits->Coefficient[i][j] * Value[j]);

        if (i < Edits->NumberofInequalities) {
            /*
            If this is an inequality edit.
            Constant should be greater or equal to sum of columns.
            */
            if (EIM_DBL_LT (Edits->Coefficient[i][ConstantIndex], Sum)) {
                return EIE_EDITSAPPLY_FAIL;
            }
        }
        else {
            /*
            If this is an equality edit.
            Constant and sum of columns must be equal.
            */
            if (!EIM_DBL_EQ (Edits->Coefficient[i][ConstantIndex], Sum)) {
                return EIE_EDITSAPPLY_FAIL;
            }
        }
    } /* for each edit */
    return EIE_EDITSAPPLY_SUCCEED;
}

/*--------------------------------------------------------------------*/
/*Check that two edits group contain the exact same fields.           */
/*Return: -1 if they do not contain the same set of fields.           */
/*         0 otherwise.                                               */
/*--------------------------------------------------------------------*/
int EI_EditsCheckFieldsSet (
    EIT_EDITS * Edits1,
    EIT_EDITS * Edits2)
{
    int i;

    if (Edits1->NumberofColumns != Edits2->NumberofColumns)
        return -1;

    for (i = 0; i < Edits1->NumberofColumns-1; i++) {
        if (strcmp (Edits1->FieldName[i], Edits2->FieldName[i]) != 0)
            return -1;
    }

    return 0;
}

/******************************************************************************
changes edits  (see EI_EditsCleanOneEdit() for details)

Rules:
- only change equality rows
******************************************************************************/
void EI_EditsClean (
    EIT_EDITS * Edits)
{
    int i;

    for (i = Edits->NumberofInequalities; i < Edits->NumberofEquations; i++) {
        EI_EditsCleanOneEdit (i, Edits);
    }
}

/******************************************************************************
changes edit at index Row of the form
    - 2 * X = 0; and 5 * X = 0;      to X = 0;

Rules:
- only change row where constant is 0.0
- only change row with 1 column coefficient different then 0.0
******************************************************************************/
void EI_EditsCleanOneEdit (
    int Row,
    EIT_EDITS * Edits)
{
    int Column;
    int Count;
    int i;

    if (Edits->Coefficient[Row][Edits->NumberofColumns-1] == 0.0) {
        Count = 0;
        for (i = 0; i < Edits->NumberofColumns-1 && Count < 2; i++) {
            if (Edits->Coefficient[Row][i] != 0.0) {
                Count++;
                Column = i;
            }
        }
        if (Count == 1) {
            Edits->Coefficient[Row][Column] = 1.0;
        }
    }
}

/*
*/
void EI_EditsDropEquation (
    int Index,
    EIT_EDITS * Edits)
{
    int i;

    STC_FreeMemory (Edits->Coefficient[Index]);

    /* reduce equations count */
    Edits->NumberofEquations--;
    /* If equation is an inequality reduce inequality count */
    if (Index < Edits->NumberofInequalities)
        Edits->NumberofInequalities--;

    /* Move up the subsequent equations */
    for (i = Index; i < Edits->NumberofEquations ; i++) {
        Edits->Coefficient[i] = Edits->Coefficient[i+1];
        Edits->EditId[i] = Edits->EditId[i+1];
    }
}

/*
*/
EIT_EDITSDUPLICATE_RETURNCODE EI_EditsDuplicate (
    EIT_EDITS * ToEdits,
    EIT_EDITS * FromEdits)
{
    int i;
    int j;

    if (AllocateEdits (ToEdits, FromEdits->NumberofEquations,
            FromEdits->NumberofColumns) == EIE_FAIL)
        return EIE_EDITSDUPLICATE_FAIL;

    ToEdits->NumberofEquations    = FromEdits->NumberofEquations;
    ToEdits->NumberofInequalities = FromEdits->NumberofInequalities;

    for (i = 0; i < ToEdits->NumberofColumns; i++) {
        ToEdits->FieldName[i] = STC_StrDup (FromEdits->FieldName[i]);
        if (ToEdits->FieldName[i] == NULL)
            return EIE_EDITSDUPLICATE_FAIL;
    }

    for (i = 0; i < FromEdits->NumberofEquations; i++) {
        ToEdits->EditId[i] = FromEdits->EditId[i];
        for (j = 0; j < FromEdits->NumberofColumns; j++)
            ToEdits->Coefficient[i][j] = FromEdits->Coefficient[i][j];
    }

    return EIE_EDITSDUPLICATE_SUCCEED;
}

/*--------------------------------------------------------------------*/
/* Format an edit group for printing.                                 */
/*--------------------------------------------------------------------*/
void EI_EditsFormat (
    EIT_EDITS *Edits,
    char * TheEdits)
{
    EIT_FIELDNAMES Field;
    int i;
    char * Space;
    int TotalLength;

    TotalLength = 0;

    Field.FieldName = Edits->FieldName;
    Field.NumberofFields = Edits->NumberofColumns-1;

    Space = STC_AllocateMemory (
        (EI_InfoGetVariableNameMaxSize () + 10) * Edits->NumberofColumns);

    /*  Print the inequalities  */
    for (i = 0; i < Edits->NumberofInequalities; i++) {
        EI_EditsFormatEdit (Edits->EditId[i], Edits->Coefficient[i], &Field,
            "<=", Space);
        TotalLength += sprintf (TheEdits+TotalLength, "%s", Space);
    }

    /*  Print the equalities  */
    for (i = Edits->NumberofInequalities; i < Edits->NumberofEquations; i++) {
        EI_EditsFormatEdit (Edits->EditId[i], Edits->Coefficient[i], &Field,
            " =", Space);
        TotalLength += sprintf (TheEdits+TotalLength, "%s", Space);
    }

    STC_FreeMemory (Space);
}

/*--------------------------------------------------------------------*/
/* Format an edit for printing.                                       */
/* Use value EIM_EDITS_FAKEID to prevent printing the EditId          */
/*--------------------------------------------------------------------*/
void EI_EditsFormatEdit (
    int EditId,          /* Name of the edit                 */
    double * Coefficient,/* Coefficients of the edit         */
    EIT_FIELDNAMES * Field,         /* Array of variable names          */
    char * Relation,     /* Relation of the edit             */
    char * Space)
{
    char Constant[1001];   /* Coefficient of the constant      */
    char Coeff[1001];    /* Coefficient of a variable at the */
                         /* start of processing          */
    char * Coeffptr;
    size_t Count;           /* Number of characters on line     */
    int FirstElement;    /* Indicator of first element       */
    int FirstLine;       /* Indicator of first line          */
    int i;
    char Line[1001]; /* Line to be printed */

    mLength = 0;
    mSpace = Space;

    mLength += sprintf (mSpace+mLength, "\n");

    if (EditId != EIM_EDITS_FAKEID) {
        mLength += sprintf (mSpace+mLength, "%d\n", EditId);
    }

    /* Convert the coefficient of the constant  */
    /* from a double to a character string      */
    if (TOOEXTREME (Coefficient[Field->NumberofFields]))
        sprintf (Constant, "%e", Coefficient[Field->NumberofFields]);
    else {
        sprintf (Constant, "%.5f", Coefficient[Field->NumberofFields]);
        UTIL_DropZeros (Constant);
    }

    Count = 0;
    FirstElement = 1;
    FirstLine = 1;
    Line[0] = '\0';

    /* Get coefficient of each variable  */
    for (i = Field->NumberofFields-1; i >= 0; i--) {
        if (Coefficient[i] != 0.0) {
            /* Convert the coefficient of the variable */
            /* from a double to a character string     */
            if (TOOEXTREME (Coefficient[i]))
                sprintf (Coeff, "%+e", Coefficient[i]);
            else {
                sprintf (Coeff, "%+.5f", Coefficient[i]);
                UTIL_DropZeros (Coeff);
            }
            Coeffptr = Coeff;

            if (FirstElement != 0) {
                /* First element: print the minus sign but not the plus sign */
                if (Coeffptr[0] == '-')
                    Concat ("-", Line, Constant, Relation, &Count, &FirstLine);
                Coeffptr++;
                FirstElement = 0;
            }
            else {
                /* Not first element: print the minus sign or plus sign     */
                if (Coeffptr[0] == '-')
                    Concat ("-", Line, Constant, Relation, &Count, &FirstLine);
                else
                    Concat ("+", Line, Constant, Relation, &Count, &FirstLine);
                Coeffptr++;
            }

            if (Coefficient[i] != 1.0 && Coefficient[i] != -1.0) {
                /* Do not print a 1 coefficient    */
                Concat (Coeffptr, Line, Constant, Relation, &Count, &FirstLine);
                Concat ("*", Line, Constant, Relation, &Count, &FirstLine);
            }

            /* Print variable name */
            Concat (Field->FieldName[i], Line, Constant, Relation, &Count,
                &FirstLine);
        }
    }

    /* Print the line  */
    PrintLine (Line, Constant, Relation, &FirstLine);
}

/*--------------------------------------------------------------------*/
/* Calculate the required size to hold the result of EI_EditsFormat ()*/
/*--------------------------------------------------------------------*/
int EI_EditsFormatSize (
    EIT_EDITS * Edits)
{
    return Edits->NumberofEquations * Edits->NumberofColumns *
        (EI_InfoGetVariableNameMaxSize () + 20) + 1000;
}

/*--------------------------------------------------------------------*/
/* Free the edits                                                     */
/*--------------------------------------------------------------------*/
void EI_EditsFree (
    EIT_EDITS * Edits)
{
    int i;

    for (i = 0; i < Edits->NumberofColumns; i++) {
        /* free variable names */
        STC_FreeMemory (Edits->FieldName[i]);
    }
    /* free variable name array */
    STC_FreeMemory (Edits->FieldName);
    /* free edit id */
    STC_FreeMemory (Edits->EditId);

    for (i = 0; i < Edits->NumberofEquations; i++) {
        /* free coefficients */
        STC_FreeMemory (Edits->Coefficient[i]);
    }
    /* free coefficient array */
    STC_FreeMemory (Edits->Coefficient);
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
EIT_RETURNCODE EI_EditsParse (
    char * EditsString,
    EIT_EDITS * Edits)
{
    EIT_RETURNCODE crc; /* cumulative return code */
    int Done;
    int ErrorCount;
    EIT_RETURNCODE rc;
    tToken Token;

    mEdits = STC_StrDup (EditsString);
    UTIL_StrUpper (mEdits, mEdits);
    UTIL_DropBlanks (mEdits);
    mEditsPtr = mEdits;

    EditGroupInit ();

    crc = EIE_SUCCEED;
    Done = 0;
    ErrorCount = 0;
    while (!Done) {
        rc = Scanner (&Token);
        if (rc == EIE_SUCCEED) {
            rc = MatchEditGroup (&Token);
            if (rc == EIE_SUCCEED) {
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
                EI_AddMessage (M00001, EIE_ERROR, /* Edits parser */
                    M30003); /* Too many errors. Stop processing. */
            }
            else {
                /* advance to the next edits */
                mEditsPtr = strchr (mEditsPtr, ';');
                if (mEditsPtr == NULL)
                    Done = 1;
                else {
                    mEditsPtr++;
                    if (*mEditsPtr == '\0') {
                        Done = 1;
                    }
                }
            }

            /* global edit needs to be re-initialise */
            EditInit ();
        }
    }

    if (crc == EIE_SUCCEED) {
        crc = GenerateEdits (mEditGroup, Edits);
        if (crc == EIE_SUCCEED) {
            EI_EditsClean (Edits);
        }
    }

    EditGroupTerminate ();
    STC_FreeMemory (mEdits);

    return crc;
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
void EI_EditsPrint (
    EIT_EDITS * Edits)
{
    int i;
    int j;
	char s[1001];
    char Space[101];
    char Space1[101];

    strcpy (s, "ID   ");
    for (i = Edits->NumberofColumns-2; i >= 0; i--) {
        sprintf (Space, "\t%s ", Edits->FieldName[i]);
		strcat (s, Space);
    }
    sprintf (Space, "\t%s", "Constant");
	strcat (s, Space);
    EI_AddMessage ("", 4, s);

    for (i = 0; i < Edits->NumberofEquations; i++) {
        sprintf (Space, "%d ", Edits->EditId[i]);
        strcpy (s, Space);
        for (j = Edits->NumberofColumns-2; j >= 0; j--) {
            if (TOOEXTREME (Edits->Coefficient[i][j])) {
                sprintf (Space, "\t%e ", Edits->Coefficient[i][j]);
            }
            else {
                sprintf (Space1, "%.5f", Edits->Coefficient[i][j]);
                UTIL_DropZeros (Space1);
                sprintf (Space, "\t%s ", Space1);
            }
            strcat (s, Space);
        }
		if (i < Edits->NumberofInequalities) {
            sprintf (Space, "\t<= ");
		}
		else {
            sprintf (Space, "\t=  ");
		}
		strcat (s, Space);

        if (TOOEXTREME (Edits->Coefficient[i][Edits->NumberofColumns-1]))
            sprintf (Space, "%e", Edits->Coefficient[i][Edits->NumberofColumns-1]);
        else {
            sprintf (Space, "%.5f", Edits->Coefficient[i][Edits->NumberofColumns-1]);
            UTIL_DropZeros (Space);
        }
        strcat (s, Space);
		EI_AddMessage ("", 4, s);
    }
    EI_AddMessage ("", 4, "");

    EI_AddMessage ("", 4, "Col=%d, #Equation=%d #Inegalité=%d",
        Edits->NumberofColumns,
        Edits->NumberofEquations,
        Edits->NumberofInequalities);
}







/*
Allocates an EIT_EDITS
FieldName is allocated but non each FieldName[i] ones
*/
static EIT_RETURNCODE AllocateEdits (
    EIT_EDITS * Edits,
    int NumberofEquations,
    int NumberofColumns)
{
    int i;

    Edits->NumberofEquations    = NumberofEquations;
    Edits->NumberofColumns      = NumberofColumns;
    Edits->NumberofInequalities = 0;

    Edits->EditId = STC_AllocateMemory (
        NumberofEquations * sizeof *Edits->EditId);
    if (Edits->EditId == NULL)
        return EIE_FAIL;

    Edits->FieldName = STC_AllocateMemory (
        NumberofColumns * sizeof *Edits->FieldName);
    if (Edits->FieldName == NULL)
        return EIE_FAIL;

    Edits->Coefficient = STC_AllocateMemory (
        NumberofEquations * sizeof *Edits->Coefficient);
    if (Edits->Coefficient == NULL)
        return EIE_FAIL;

    for (i = 0; i < Edits->NumberofEquations; i++) {
        Edits->Coefficient[i] = STC_AllocateMemory (
            NumberofColumns * sizeof *Edits->Coefficient[i]);
        if (Edits->Coefficient[i] == NULL)
            return EIE_FAIL;
    }
    return EIE_SUCCEED;
}

/*
use to locate an entry in the Term list
*/
static int CompareSearch (
    char * Variable,
    tTerm * Term)
{
    return strcmp (Term->Variable, Variable);
}

/*
use to sort entries in the Term list
*/
static int CompareSort (
    tTerm * a,
    tTerm * b)
{
    return strcmp (b->Variable, a->Variable);
}

/*--------------------------------------------------------------------*/
/* Concatenate characters to be printed.                              */
/*--------------------------------------------------------------------*/
static void Concat (
    char *InString,     /* Incoming String to concatenate          */
    char *Line,         /* Line to be printed                      */
    char *Constant,     /* Coefficient of the constant             */
    char *Relation,     /* Relation of the edit to print           */
    size_t  *Count,        /* Number of characters on line            */
    int  *FirstLine)    /* Indicator of first line                 */
{
    size_t InLength;

    /* Evaluate the length of incoming string  */
    InLength = strlen (InString);

    if ((*Count + InLength) > 60) {
        /* Print line if it already has 60) characters  */
        PrintLine (Line, Constant, Relation, FirstLine);
        *Count = 0;
    }

    /* Concatenate incoming string to line  */
    strcpy ((Line + *Count), InString);

    /* Update the size (number of characters)  */
    *Count += InLength;

    /* Concatenate blank character to line  */
    strcpy ((Line + *Count), " ");

    /* Update the size for the space */
    *Count += 1;
}


/*
Converts the operator of a fail edit.
*/
static tOperatorId ConvertToPassOperator (
    tOperatorId Operator)
{
    switch (Operator) {
    case eOperatorIdGreater:  return eOperatorIdLess;
    case eOperatorIdLess:     return eOperatorIdGreater;
    case eOperatorIdNotEqual: return eOperatorIdEqual;
    }

    /* impossible to specify a FAIL edit with equal, has been trapped earlier */
    /* left for completeness and */
    /* will never happen, exclusively to shut up lint */
    return eOperatorIdNotEqual;
}

/*
Free an EditGroup
*/
static void EditGroupFree (
    tADTList * EditGroup)
{
    tEdit * Edit;
    int i;

    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEdit *) ADTList_Index (EditGroup, i);
        ADTList_Delete (Edit->TermList);
    }
    ADTList_Delete (EditGroup);
}

/*
Initialise the EditGroup.
Should be called once at the beginning of the parsing.
*/
static void EditGroupInit (void) {
    if (DEBUG) EI_AddMessage ("", 4, "EditGroupInit");

    mEditGroup = ADTList_Create (NULL, NULL, sizeof (tEdit), 20);

    EditInit ();
}

/*
Print an EditGroup
*/
static void EditGroupPrint (
    tADTList * EditGroup)
{
    tEdit * Edit;
    int i;

    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEdit *) ADTList_Index (EditGroup, i);
        EditPrint (Edit);
    }
}

/*
Free the EditGroup.
Should be called once at the end of the parsing.
*/
static void EditGroupTerminate (void) {
    if (DEBUG) {
        EI_AddMessage ("", 4, "EditGroupTerminate");
        EditGroupPrint (mEditGroup);
    }

    ADTList_Delete (mEdit.TermList);
    mEdit.TermList = NULL;
    EditGroupFree (mEditGroup);
    mEditGroup = NULL;
}

/*
Initialise the global edit.
Should be called every time a new edit starts.
*/
static void EditInit (void) {
    if (DEBUG) EI_AddMessage ("", 4, " EditInit");

    mEdit.Modifier = eModifierIdPass;
    mEdit.TermList = ADTList_Create ((tADTListCompareProc) CompareSort,
        (tADTListCompareProc) CompareSearch, sizeof (tTerm),
        20);

    mSide = eSideIdLeft;

    /* init the global term. */
    TermInit ();
}

/*
Print an Edit.
*/
static void EditPrint (
    tEdit * Edit)
{
    double Constant;
    int i;
    tTerm * Term;

    for (i = 0; i < ADTList_Length (Edit->TermList); i++) {
        Term = (tTerm *) ADTList_Index (Edit->TermList, i);
        if (strcmp (Term->Variable, CONSTANT) == 0)
            /* keep the constant number to print it at the end */
            Constant = Term->Coefficient;
        else
            TermPrint (Term);
    }
    EI_AddMessage ("", 4, "%s\n\t%e", OperatorIdTranslator (Edit->Operator), Constant);
}

/*
Validates the Edit.
Should be called as soon as the end of an edit is found.
*/
static EIT_RETURNCODE EditTerminate (void) {
    int Index;
    char Message[1001];
    tTerm * Term;

    if (DEBUG) EI_AddMessage ("", 4, " EditTerminate");

    /* add a 0.0 constant if needed */
    if (ADTList_Locate (mEdit.TermList, CONSTANT) == ADTLIST_NOTFOUND) {
        mTerm.Coefficient = 0.0;
        strcpy (mTerm.Variable, CONSTANT);
        InstallTerm ();
    }

    /* remove the term if coefficient is near 0.0 and it is not the constant */
    if (DEBUG) EditPrint (&mEdit);
    Index = FindNear0Coefficient (mEdit.TermList);
    while (Index != -1) {
        Term = (tTerm *) ADTList_Index (mEdit.TermList, Index);

        /* Removing variable '%s' because its coefficient (or the sum of its coefficients) is smaller than the
         smallest acceptable value %e. %e is a machine dependant value. */
        sprintf (Message, M20001 "\n",
            Term->Variable, EIM_DBL_EPSILON, EIM_DBL_EPSILON);
        ReportWarning (Message);
        ADTList_Remove (mEdit.TermList, Index);

        Index = FindNear0Coefficient (mEdit.TermList);
    }
    if (DEBUG) EditPrint (&mEdit);

    if (ADTList_Length (mEdit.TermList) == 1) {
        ReportError (M30004); /* No variable in edits */
        return EIE_FAIL;
    }

    /*
    check for a valid combinaison of modifier (PASS, FAIL)
    and operator (=, !=)
    */
    if (mEdit.Modifier == eModifierIdPass &&
            mEdit.Operator == eOperatorIdNotEqual) {
        ReportError (M30005); /* PASS edit can't be != */
        return EIE_FAIL;
    }
    if (mEdit.Modifier == eModifierIdFail &&
            mEdit.Operator == eOperatorIdEqual) {
        ReportError (M30006); /* FAIL edit can't be =" */
        return EIE_FAIL;
    }

    /* sort variables */
    ADTList_QSort (mEdit.TermList);

    /* at this point the constant is the last term */
    Term = (tTerm*) ADTList_Index (mEdit.TermList,
        ADTList_Length (mEdit.TermList)-1);
    if (Term->Coefficient == 0.0)
        /* NOTHING TO DO. 0.0 is good for the constant */;
    else if (EIM_DBL_EQ (Term->Coefficient, 0.0)) {
        /* Changing constant to 0.0 because it is smaller than the smallest acceptable value %e. %e is a machine
        dependant value.*/
        sprintf (Message, M20002 "\n",
            EIM_DBL_EPSILON, EIM_DBL_EPSILON);
        ReportWarning (Message);
        Term->Coefficient = 0.0;
    }
    else
        /* change sign of constant */
        Term->Coefficient *= -1.0;

    /* add the edit to the edit group */
    ADTList_Append (mEditGroup, (void *) &mEdit);

    return EIE_SUCCEED;
}

/*
scan all coefficients of the termlist, except the constant and
return index of the first ~0.0 coefficient, or -1 if none exists
*/
static int FindNear0Coefficient (
    tADTList * TermList)
{
    int i;
    tTerm * Term;

    for (i = 0; i < ADTList_Length (TermList); i++) {
        Term = (tTerm *) ADTList_Index (TermList, i);
        if (EIM_DBL_EQ (Term->Coefficient, 0.0) &&
                strcmp (Term->Variable, CONSTANT) != 0)
            return i;
    }
    return -1;
}

/*
move info the parser found to the edit array.
*/
static EIT_RETURNCODE GenerateEdits (
    tADTList * EditGroup,
    EIT_EDITS * Edits)
{
    int ChangeSign;
    tEdit * Edit;
    int i;
    int InsertIndex;
    int j;
    int NumberOfEqualities;
    tTerm * Term;
    int TermListIndex;
    tSList * VariableList;

    SList_New (&VariableList);

    /* create a sorted list containing the distinct variable names */
    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEdit *) ADTList_Index (EditGroup, i);
        for (j = 0; j < ADTList_Length (Edit->TermList); j++) {
            Term = (tTerm *) ADTList_Index (Edit->TermList, j);
            SList_AddNoDup (Term->Variable, VariableList);
        }
    }
    SList_Sort (VariableList, eSListSortDescending);

    /* change operators of fail edits making every edit a pass edit. */
    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEdit *) ADTList_Index (EditGroup, i);
        if (Edit->Modifier == eModifierIdFail) {
            Edit->Operator = ConvertToPassOperator (Edit->Operator);
        }
    }

    if (AllocateEdits (Edits, ADTList_Length (EditGroup),
            SList_NumEntries (VariableList)) == EIE_FAIL)
        return EIE_FAIL;

    /* allocate variable name array */
    for (i = 0; i < Edits->NumberofColumns; i++) {
        /* allocate variable names */
        Edits->FieldName[i] = STC_StrDup (SList_Entry (VariableList, i));
    }

    /* no need for it anymore */
    SList_Free (VariableList);

    NumberOfEqualities = 0;

    for (i = 0; i < Edits->NumberofEquations; i++) {
        Edit = (tEdit *) ADTList_Index (EditGroup, i);

        /* set InsertIndex to place equalities and inequalities in edit array */
        if (Edit->Operator == eOperatorIdEqual) {
            /* equalities are inserted at the end of the edit array */
            InsertIndex =
                Edits->NumberofEquations - NumberOfEqualities - 1;
            NumberOfEqualities++;
        }
        else {
            /* inequalties are inserted at the beginning of the edit array */
            InsertIndex = Edits->NumberofInequalities;
            Edits->NumberofInequalities++;
        }

        Edits->EditId[InsertIndex] = InsertIndex;

        /*
        The edit array must contains only 'less than' inequalities.
        When an edit is a 'greater than' or 'greater or equal than' edit,
        the sign of every term is changed, because we know that
        x - y > 1 is the same as -x + y < -1
        */
        ChangeSign = (Edit->Operator == eOperatorIdGreater ? -1 : 1);
        TermListIndex = 0;
        for (j = 0; j < Edits->NumberofColumns; j++) {
            Term = (tTerm *) ADTList_Index (Edit->TermList, TermListIndex);

            /* TermList contains only a subset of the fields in the edit array*/
            if (strcmp (Term->Variable, Edits->FieldName[j]) == 0) {
                if (Term->Coefficient == 0.0)
                    Edits->Coefficient[InsertIndex][j] = 0.0;
                else
                    Edits->Coefficient[InsertIndex][j] =
                        Term->Coefficient * ChangeSign;
                TermListIndex++;
            }
            else
                /* set Coefficient to 0.0 if FieldName is not in TermList */
                Edits->Coefficient[InsertIndex][j] = 0.0;
        }
    }
    return EIE_SUCCEED;
}

/*
A number was found.
*/
static void InstallCoefficient (
    double Coefficient)
{
    if (DEBUG) EI_AddMessage ("", 4, " InstallCoefficient");

    mTerm.Coefficient *= Coefficient;
}

/*
The end of an edit was found.
*/
static EIT_RETURNCODE InstallEdit (void) {
    EIT_RETURNCODE rc;

    if (DEBUG) EI_AddMessage ("", 4, " InstallEdit");

    rc = EditTerminate ();
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    EditInit ();

    return EIE_SUCCEED;
}

/*
A minus was found. Change sign of the coefficient.
*/
static void InstallMinus (void)
{
    if (DEBUG) EI_AddMessage ("", 4, " InstallMinus");

    mTerm.Coefficient *= -1.0;
}

/*
A modifier was found.
*/
static void InstallModifier (
    tModifierId Modifier)
{
    if (DEBUG) EI_AddMessage ("", 4, " InstallModifier");

    mEdit.Modifier = Modifier;
}

/*
An operator was found.
*/
static void InstallOperator (
    tOperatorId Operator)
{
    if (DEBUG) EI_AddMessage ("", 4, " InstallOperator");

    mSide = eSideIdRight;
    mEdit.Operator = Operator;
}

/*
The end of a term was found.
*/
static void InstallTerm (void) {
    if (DEBUG) EI_AddMessage ("", 4, "  InstallTerm");

    /* adjust for side of equation */
    if (mSide == eSideIdRight)
        mTerm.Coefficient *= -1.0;

    TermTerminate ();
    TermInit ();
}

/*
A variable was found.
*/
static void InstallVariable (
    char * Variable)
{
    if (DEBUG) EI_AddMessage ("", 4, " InstallVariable");

    strcpy (mTerm.Variable, Variable);
}

/*
Make sure the last token read match a specified token id.
If it does, call the scanner to find the next token.
If it does not, returns EIE_FAIL.
*/
static EIT_RETURNCODE Match (
    tTokenId Id,
    tToken * Token)
{
    if (Id != Token->Id) {
        char Message[101];

        sprintf (Message, M30007 "\n", /* Looking for '%s' but found '%s' instead. */
            TokenIdTranslator (Id), TokenIdTranslator (Token->Id));
        ReportError (Message);
        return EIE_FAIL;
    }

    return Scanner (Token);
}

/*
*/
static EIT_RETURNCODE MatchCoefficient (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdCoefficient)
        InstallCoefficient (Token->Value.Coefficient);

    rc = Match (eTokenIdCoefficient, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchComponents (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    rc = MatchTerm (Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    if (Token->Id == eTokenIdPlus) {
        rc = Match (eTokenIdPlus, Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchComponents (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    else if (Token->Id == eTokenIdMinus) {
        rc = MatchMinus (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        rc = MatchComponents (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchEdit (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdModifier) {
        rc = MatchModifier (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }
    rc = MatchComponents (Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    rc = MatchOperator (Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    rc = MatchComponents (Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    rc = Match (eTokenIdSemiColon, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    rc = InstallEdit ();
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchEditGroup (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    rc = MatchEdit (Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    while (Token->Id != eTokenIdDone) {
        rc = MatchEdit (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchMinus (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    InstallMinus ();

    rc = Match (eTokenIdMinus, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchModifier (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdModifier)
        InstallModifier (Token->Value.Modifier);

    rc = Match (eTokenIdModifier, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;
    rc = Match (eTokenIdColon, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchOperator (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdOperator)
        InstallOperator (Token->Value.Operator);

    rc = Match (eTokenIdOperator, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchTerm (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdMinus) {
        rc = MatchMinus (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
    }

    if (Token->Id == eTokenIdCoefficient) {
        rc = MatchCoefficient (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        if (Token->Id == eTokenIdStar) {
            rc = Match (eTokenIdStar, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
            if (Token->Id == eTokenIdMinus) {
                rc = MatchMinus (Token);
                if (rc != EIE_SUCCEED) return EIE_FAIL;
            }
            rc = MatchVariable (Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;

        }
    }
    else if (Token->Id == eTokenIdVariable) {
        rc = MatchVariable (Token);
        if (rc != EIE_SUCCEED) return EIE_FAIL;
        if (Token->Id == eTokenIdStar) {
            rc = Match (eTokenIdStar, Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
            if (Token->Id == eTokenIdMinus) {
                rc = MatchMinus (Token);
                if (rc != EIE_SUCCEED) return EIE_FAIL;
            }
            rc = MatchCoefficient (Token);
            if (rc != EIE_SUCCEED) return EIE_FAIL;
        }
    }
    else {
        char Message[1001];

        sprintf (Message, M30008 "\n", /* Looking for 'Coefficient' or 'Variable' but found '%s' instead. */
            TokenIdTranslator (Token->Id));
        ReportError (Message);
        return EIE_FAIL;
    }

    InstallTerm ();

    return EIE_SUCCEED;
}

/*
*/
static EIT_RETURNCODE MatchVariable (
    tToken * Token)
{
    EIT_RETURNCODE rc;

    if (Token->Id == eTokenIdVariable)
        InstallVariable (Token->Value.Variable);

    rc = Match (eTokenIdVariable, Token);
    if (rc != EIE_SUCCEED) return EIE_FAIL;

    return EIE_SUCCEED;
}

/*
returns a description for a modifier.
(no translation needed: used in debug statement only)
*/
static char * ModifierIdTranslator (
    tModifierId ModifierId)
{
    switch (ModifierId) {
    case eModifierIdFail: return ("Fail");
    case eModifierIdPass: return ("Pass");
    /* no default */
    }
    return ("Unknown modifier");
}

/*
returns a description for a operator.
(no translation needed: used in debug statement only)
*/
static char * OperatorIdTranslator (
    tOperatorId OperatorId)
{
    switch (OperatorId) {
    case eOperatorIdEqual:    return ("Equal");
    case eOperatorIdGreater:  return ("Greater");
    case eOperatorIdLess:     return ("Less");
    case eOperatorIdNotEqual: return ("Not equal");
    /* no default */
    }
    return ("Unknown operator");
}

/*--------------------------------------------------------------------*/
/* Print a line containing part of an edit.                           */
/*--------------------------------------------------------------------*/
static void PrintLine (
    char * Line,      /* Line to be printed                */
    char * Constant,  /* Coefficient of the Constant       */
    char * Relation,  /* Relation of the edit to print     */
    int * FirstLine)  /* Indicator or first line           */
{
    if (*FirstLine != 0) {
        *FirstLine = 0;

        /* Print part of the left side and the Relation of the edit */
        mLength += sprintf (mSpace+mLength, "%60s %2.2s", Line, Relation);

        /* Print the right side of the edit  */
        mLength += sprintf (mSpace+mLength, " %s\n", Constant);
    }
    else {
        mLength += sprintf (mSpace+mLength, "%60s\n", Line);
    }
}

/*
Adds a message to the message list.
The edits string is cut in pieces before printing and the caret point to
the line where the error occurs.
example:
1*x0001+2*x0002+3*x0@03+4*x0004+5*x0005+
                    ^
6*x0006+7*x0007+8*x0008+9*x0009+10*x0010>1000;
*/
static void ReportError (
    char * Message)
{
#define SIZEOFCHUNKS 72

    int CaretPosition;
    char * EditsString;
    int i;
    int NumberOfSplitedEdits;
    char ** SplitedEditsString;

    CaretPosition = (int) (mEditsPtr-mEdits);

    /* EditsString size must be big enough to hold:
       the edits +
       newline characters +
       the line of the caret + its newline characters +
       eol character */
    EditsString = STC_AllocateMemory (
        strlen (mEdits) +
        strlen (mEdits)/SIZEOFCHUNKS + 1 +
        SIZEOFCHUNKS + 1 +
        1);

    EditsString[0] = '\0';

    NumberOfSplitedEdits = UTIL_StrSplit (mEdits, SIZEOFCHUNKS,
        &SplitedEditsString);

    for (i = 0; i < NumberOfSplitedEdits; i++) {
        strcat (EditsString, SplitedEditsString[i]);
        strcat (EditsString, "\n");
        if (0 <= CaretPosition && CaretPosition < SIZEOFCHUNKS) {
            sprintf (EditsString+strlen(EditsString), "%*s^\n",
                CaretPosition, "");
        }
        CaretPosition -= SIZEOFCHUNKS;
    }

    EI_AddMessage (M00001, EIE_ERROR, "%s\n%s:\n%s", /* Edits parser */
        Message, M00002, EditsString);

    STC_FreeMemory (EditsString);
    for (i = 0; i < NumberOfSplitedEdits; i++)
        STC_FreeMemory (SplitedEditsString[i]);
    STC_FreeMemory (SplitedEditsString);
}

/*
Adds a warning message to the message list.
The edits string is cut in pieces before printing and the caret point to
the line where the warning occurs.
example:
1*x0001+2*x0002+3*x0@03+4*x0004+5*x0005+
                    ^
6*x0006+7*x0007+8*x0008+9*x0009+10*x0010>1000;
*/
static void ReportWarning (
    char * Message)
{
#define SIZEOFCHUNKS 72

    int CaretPosition;
    char * EditsString;
    int i;
    int NumberOfSplitedEdits;
    char ** SplitedEditsString;

    CaretPosition = (int) (mEditsPtr-mEdits);

    /* EditsString size must be big enough to hold:
       the edits +
       newline characters +
       the line of the caret + its newline characters +
       eol character */
    EditsString = STC_AllocateMemory (
        strlen (mEdits) +
        strlen (mEdits)/SIZEOFCHUNKS + 1 +
        SIZEOFCHUNKS + 1 +
        1);

    EditsString[0] = '\0';

    NumberOfSplitedEdits = UTIL_StrSplit (mEdits, SIZEOFCHUNKS,
        &SplitedEditsString);

    for (i = 0; i < NumberOfSplitedEdits; i++) {
        strcat (EditsString, SplitedEditsString[i]);
        strcat (EditsString, "\n");
        if (0 <= CaretPosition && CaretPosition < SIZEOFCHUNKS) {
            sprintf (EditsString+strlen(EditsString), "%*s^\n",
                CaretPosition, "");
        }
        CaretPosition -= SIZEOFCHUNKS;
    }

    EI_AddMessage (M00001, EIE_WARNING, "%s\n%s:\n%s", /* Edits parser */
        Message, M00024, EditsString);

    STC_FreeMemory (EditsString);
    for (i = 0; i < NumberOfSplitedEdits; i++)
        STC_FreeMemory (SplitedEditsString[i]);
    STC_FreeMemory (SplitedEditsString);
}

/*
lexical analyser.
reads the input and finds the next token.
set Token->Id to eTokenIdUnknownCharacter if the token is not recognise.
*/
static EIT_RETURNCODE Scanner (
    tToken * Token)
{
    char * p = mEditsPtr;

    /* skip spaces*/
    while (isspace(*p))
        p++;

    if (strspn (p, EI_InfoGetVariableNameFirstCharacterCharacterSet ()) > 0) {
        /* found a variable or a modifier */
        size_t n;

        Token->Value.Variable[0] = *p; /* save first character */
        p++;

        n = strspn (p, EI_InfoGetVariableNameCharacterSet ());

        if (n > (size_t) EI_InfoGetVariableNameMaxSize ()-1) {
            Token->Id = eTokenIdError;
            ReportError (M30009); /* Variable name too large! */
            p += n;
            mEditsPtr = p;
            return EIE_FAIL;
        }

        strncpy (Token->Value.Variable+1, p, n);
        Token->Value.Variable[n+1] = '\0';
        p += n;

        if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdFail]) == 0) {
            /* found fail modifier */
            Token->Id = eTokenIdModifier;
            Token->Value.Modifier = eModifierIdFail;
        }
        else if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdPass]) == 0) {
            /* found pass modifier */
            Token->Id = eTokenIdModifier;
            Token->Value.Modifier = eModifierIdPass;
        }
        else if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdRejet]) == 0) {
            /* found rejet modifier */
            Token->Id = eTokenIdModifier;
            Token->Value.Modifier = eModifierIdFail;
        }
        else if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdAccepte]) == 0) {
            /* found accepte modifier */
            Token->Id = eTokenIdModifier;
            Token->Value.Modifier = eModifierIdPass;
        }
        else
            /* found variable name */
            Token->Id = eTokenIdVariable;
    }
    else if (isdigit (*p) || (*p == '.' && isdigit (*(p+1)))) {
        /* found number */
        char * s;

        Token->Id = eTokenIdCoefficient;
        Token->Value.Coefficient = strtod (p, &s);
        p = s;
    }
    else if (*p == ':') {
        /* found colon */
        Token->Id = eTokenIdColon;
        p++;
    }
    else if (*p == '-') {
        /* found Minus */
        Token->Id = eTokenIdMinus;
        p++;
    }
    else if (*p == '+') {
        /* found Plus */
        Token->Id = eTokenIdPlus;
        p++;
    }
    else if (*p == ';') {
        /* found SemiColon */
        Token->Id = eTokenIdSemiColon;
        p++;
    }
    else if (*p == '*') {
        /* found star */
        Token->Id = eTokenIdStar;
        p++;
    }
    else if (*p == '>') {
        /* found greater than operator */
        Token->Id = eTokenIdOperator;
        Token->Value.Operator = eOperatorIdGreater;
        p++;

        /* skip spaces*/
        while (isspace(*p))
            p++;

        /* if next char is '=' skip it */
        if (*p == '=')
            p++;
    }
    else if (*p == '<') {
        /* found less than operator */
        Token->Id = eTokenIdOperator;
        Token->Value.Operator = eOperatorIdLess;
        p++;

        /* skip spaces*/
        while (isspace(*p))
            p++;

        /* if next char is '=' skip it */
        if (*p == '=')
            p++;
    }
    else if (*p == '=') {
        /* found equality operator */
        Token->Id = eTokenIdOperator;
        Token->Value.Operator = eOperatorIdEqual;
        p++;
    }
    else if (*p == '!') {
        p++;

        /* skip spaces*/
        while (isspace(*p))
            p++;

        if (*p == '=') {
            /* found inequality operator */
            Token->Id = eTokenIdOperator;
            Token->Value.Operator = eOperatorIdNotEqual;
            p++;
        }
        else {
            /* found an unknown character */
            Token->Id = eTokenIdUnknownCharacter;
            p++;
        }
    }
    else if (*p == '\0') {
        /* found the end */
        Token->Id = eTokenIdDone;
    }
    else {
        /* found an unknown character */
        Token->Id = eTokenIdUnknownCharacter;
        p++;
    }

    if (DEBUG) TokenPrint (Token);

    mEditsPtr = p;

    return EIE_SUCCEED;
}

/*
Initialise the global term.
Should be called every time a new term starts.
*/
static void TermInit (void) {
    if (DEBUG) EI_AddMessage ("", 4, "   TermInit");

    mTerm.Coefficient = 1.0;
    mTerm.Variable[0] = '\0';
}

/*
Print a term
*/
static void TermPrint (
    tTerm * Term)
{
    EI_AddMessage ("", 4, "%s\t%e", Term->Variable, Term->Coefficient);
}

/*
Put the term in the global edit.
If the variable name is not in the edit, the term is added.
If it's already present, the term is modified.
meaning if the edit is "x*1 + x*2 > 1000"
it is simplify to      "x*3       > 1000"
Should be called as soon as the end of a term is found.
*/
static void TermTerminate (void) {
    char Message[1001];
    tTerm * pTerm;
    int TermIndex;

    if (DEBUG) EI_AddMessage ("", 4, "   TermTerminate");

    /* variable with coefficient of 0.0 are dropped (really 0.0 not near 0.0) */
    if (mTerm.Coefficient == 0.0 && strcmp (mTerm.Variable, CONSTANT) != 0) {
        sprintf (Message, M20003 "\n", /* Removing variable '%s' because its coefficient is 0.0.*/ 
            mTerm.Variable);
        ReportWarning (Message);
        return;
    }

    TermIndex = ADTList_Locate (mEdit.TermList, mTerm.Variable);
    if (TermIndex == ADTLIST_NOTFOUND) {
        /* variable not present, add it */
        ADTList_Append (mEdit.TermList, (void *) &mTerm);
    }
    else {
        /* variable present, change it */
        pTerm = (tTerm *) ADTList_Index (mEdit.TermList, TermIndex);
        pTerm->Coefficient += mTerm.Coefficient;
        if (strcmp (pTerm->Variable, CONSTANT) == 0) {
            ReportWarning (M20004); /* Combining two constants. */
        }
        else {
            sprintf (Message, M20005 "\n", /* Combining variable '%s' because it was specified more than once. */
                pTerm->Variable);
            ReportWarning (Message);
        }

        /* remove the term if number is 0.0 (really 0.0 not near 0.0) and
           it is not the constant */
        if (pTerm->Coefficient == 0.0 &&
                strcmp (pTerm->Variable, CONSTANT) != 0) {
            sprintf (Message, M20006 "\n", /* Removing variable '%s' because the sum of its coefficients is 0.0. */
                pTerm->Variable);
            ReportWarning (Message);
            ADTList_Remove (mEdit.TermList, TermIndex);
        }
    }
}

/*
returns a description for a token.
*/
static char * TokenIdTranslator (
    tTokenId TokenId)
{
    switch (TokenId) {
    case eTokenIdCoefficient:      return (M00003); /* Coefficient */
    case eTokenIdColon:            return (M00004); /* Colon */
    case eTokenIdDone:             return (M00005); /* Done */
    case eTokenIdError:            return (M00006); /* Error */
    case eTokenIdMinus:            return (M00007); /* Minus */
    case eTokenIdModifier:         return (M00008); /* Modifier */
    case eTokenIdOperator:         return (M00009); /* Operator */
    case eTokenIdPlus:             return (M00010); /* Plus */
    case eTokenIdSemiColon:        return (M00011); /* SemiColon */
    case eTokenIdStar:             return (M00012); /* Asterix */
    case eTokenIdVariable:         return (M00013); /* Variable */
    case eTokenIdUnknownCharacter: return (M00014); /* Unknown character */
    /* no default */
    }
    return (M00014); /* Unknown character */
}

/*
print token and value if applicable.
*/
static void TokenPrint (
    tToken * Token)
{
    switch (Token->Id) {
    case eTokenIdColon:
    case eTokenIdDone:
    case eTokenIdError:
    case eTokenIdMinus:
    case eTokenIdPlus:
    case eTokenIdSemiColon:
    case eTokenIdStar:
    case eTokenIdUnknownCharacter:
    default:
        EI_AddMessage ("", 4, "%s", TokenIdTranslator (Token->Id));
        break;
    case eTokenIdCoefficient:
        EI_AddMessage ("", 4, "%s %.25f", TokenIdTranslator (Token->Id),
            Token->Value.Coefficient);
        break;
    case eTokenIdModifier:
        EI_AddMessage ("", 4, "%s %s", TokenIdTranslator (Token->Id),
            ModifierIdTranslator (Token->Value.Modifier));
        break;
    case eTokenIdOperator:
        EI_AddMessage ("", 4, "%s %s", TokenIdTranslator (Token->Id),
            OperatorIdTranslator (Token->Value.Operator));
        break;
    case eTokenIdVariable:
        EI_AddMessage ("", 4, "%s %s", TokenIdTranslator (Token->Id),
            Token->Value.Variable);
        break;
    }
}
#endif
