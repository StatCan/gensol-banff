/*
prorating edits parser
This is a list of the productions for the parser:

edits       -> edit
            |  edit edits
edit        -> components '=' total ';'
components  -> term
            |  term '+' components
term        -> variable
            |  variable ':' modifier
            |  weight variable
            |  weight variable ':' modifier
modifier    -> TOKEN_MODIFIER
total       -> TOKEN_VARIABLE
variable    -> TOKEN_VARIABLE
weight      -> TOKEN_NUMBER

TOKEN_MODIFIER {one of 'A'|'I'|'N'|'O'}
TOKEN_NUMBER   {list of digits}
TOKEN_VARIABLE {letter followed by list of letters|digits|underscore}
*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adtlist.h"
#include "EI_Common.h"
#include "EI_Info.h"
#include "EI_Message.h"
#include "EI_ProratingEdits.h"
#include "EI_Prorating.h"
#include "STC_Memory.h"
#include "slist.h"
#include "util.h"
#include "MessageBanffAPI.h"

#define PARSER_MAX_ERROR_COUNT 7

enum ModifierIdEnum {
    eModifierIdAlways = 0,
    eModifierIdImputed = 1,
    eModifierIdNever = 2,
    eModifierIdOriginal = 3
};
typedef enum ModifierIdEnum tModifierId;

enum TokenIdEnum {
    eTokenIdColon,
    eTokenIdDone,
    eTokenIdError,
    eTokenIdModifier,
    eTokenIdOperator,
    eTokenIdPlus,
    eTokenIdSemiColon,
    eTokenIdUnknownCharacter,
    eTokenIdVariable,
    eTokenIdWeight
};
typedef enum TokenIdEnum tTokenId;


union ValueUnion {
    tModifierId Modifier;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
    double Weight;
};
typedef union ValueUnion tTokenValue;


struct TokenStructure {
    tTokenId Id;
    tTokenValue Value;
};
typedef struct TokenStructure tToken;


struct EditNodeStructure {
    EIT_PRORATING_MODIFIER Modifier;
    char Variable[EIM_VARIABLE_NAME_MAX_SIZE+1];
    double Weight;
    tADTList * TermList;
};
typedef struct EditNodeStructure tEditNode;


/*
*/
static char * mValidModifier[] = {
    "A", "I", "N", "O"
};


/*
set DEBUG to 1 to activate the debugging print statements.
set DEBUG to 0 to deactivate the debugging print statements.
If DEBUG is zero, most compilers won't generate any code for the debugging
statements.
*/
enum {DEBUG = 0};

/* contains info about the edit that the parser is currently working on. */
static tEditNode mEdit;
/* points to the edits strings. */
static char * mEdits;
/* points somewhere in mEdits. the scanner moves it as it scans the string. */
static char * mEditsPtr;
/* contains info about the term that the parser is currently working on. */
static tEditNode mTerm;
/* root terms containing the tree structure of the edits. */
static tADTList * mEditGroup;

static int CompareSearch (char * Variable, tEditNode * Term);
static int EditGroupFixHierarchy (tADTList * EditGroup);
static int EditGroupFindDuplicate (tADTList * EditGroup);
static void EditGroupFree (tADTList * EditGroup);
static void EditGroupInit (void);
static void EditGroupPrint (tADTList * EditGroup, int Level);
static void EditGroupTerminate (void);
static void EditInit (void);
static int EditTerminate (void);
static void FindDuplicate (tADTList * Branch, tSList * ListOfDistinctVariables,
    tSList * ListOfRepeatingVariables);
static void GenerateProratingEdit (tEditNode * Edit,
    EIT_PRORATING_EDIT * ProratingEdit, int Level);
static int GenerateProratingEdits (tADTList * EditGroup,
    EIT_PRORATING_GROUP * ProratingEdits);
static void GetNodes (tADTList * Branch, int RequestedLevel, int CurrentLevel,
    tEditNode ** EditNodeListPtr, int * NumberOfNodes);
static int InstallEdit (void);
static void InstallModifier (tModifierId Modifier);
static int InstallWeight (double Weight);
static int InstallTerm (void);
static void InstallTotal (char * Total);
static void InstallVariable (char * Variable);
static tEditNode * LocateVariable (tADTList * Branch, char * Variable);
static int Match (tTokenId Id, tToken * Token);
static int MatchComponents (tToken * Token);
static int MatchEdit (tToken * Token);
static int MatchEditGroup (tToken * Token);
static int MatchModifier (tToken * Token);
static int MatchOperator (tToken * Token);
static int MatchTerm (tToken * Token);
static int MatchTotal (tToken * Token);
static int MatchVariable (tToken * Token);
static int MatchWeight (tToken * Token);
static void ReportError (char * String);
static int Scanner (tToken * Token);
static void TermInit (tEditNode * Term);
static void TermPrint (tEditNode * Term, int Level);
static int TermTerminate (void);
static void TokenPrint (tToken * Token);
static char * TokenTranslator (tTokenId TokenId);
static void TreeStatistics (tADTList * Branch, int * NumberOfEdits);


/*
*/
void EI_ProratingEditsFree (
    EIT_PRORATING_GROUP * ProratingEdits)
{
    int i, j;

    for (i = 0; i < ProratingEdits->NumberOfEdits; i++) {
        for (j = 0; j < ProratingEdits->Edit[i].NumberOfFields; j++) {
            STC_FreeMemory (ProratingEdits->Edit[i].FieldName[j]);
        }
        STC_FreeMemory (ProratingEdits->Edit[i].FieldName);
        STC_FreeMemory (ProratingEdits->Edit[i].TotalName);
        STC_FreeMemory (ProratingEdits->Edit[i].Weight);
        STC_FreeMemory (ProratingEdits->Edit[i].ProratingFlag);
    }
    STC_FreeMemory (ProratingEdits->Edit);
}

/*
Only one edit group allowed
*/
EIT_PRORATING_EDITS_PARSE_RETURNCODE EI_ProratingEditsParse (
    char * Edits,
    EIT_PRORATING_GROUP * ProratingEdits)
{
    EIT_BOOLEAN AreThereMultipleGroups = EIE_FALSE;
    EIT_RETURNCODE crc; /* cumulative return code */
    int Done;
    int ErrorCount;
    int rc;
    tToken Token;

    mEdits = STC_StrDup (Edits);
    UTIL_StrUpper (mEdits, mEdits);
    UTIL_DropBlanks (mEdits);
    mEditsPtr = mEdits;

    EditGroupInit ();

    crc = EIE_SUCCEED;
    Done = 0;
    ErrorCount = 0;
    while (!Done) {
        rc = Scanner (&Token);
        if (rc == 0) {
            rc = MatchEditGroup (&Token);
            if (rc == 0) {
                Done = 1;
            }
        }

        /* handle error if any of the previous function call failed */
        if (rc != 0) {
            crc = EIE_FAIL;
            ErrorCount++;
            if (ErrorCount >= PARSER_MAX_ERROR_COUNT) {
                /* quit when PARSER_MAX_ERROR_COUNT errors are found */
                Done = 1;
                EI_AddMessage (M00037, EIE_ERROR, /* Prorating edits parser */
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
        rc = EditGroupFixHierarchy (mEditGroup);
        if (rc == 0) {
            if (ADTList_Length (mEditGroup) > 1) {
                AreThereMultipleGroups = EIE_TRUE;

                EI_AddMessage (M00037, EIE_ERROR, /* Prorating edits parser */
                    M30098); /* Edits contain multiple 'prorating groups' */

                EditGroupTerminate ();
                STC_FreeMemory (mEdits);
                return EIE_PRORATING_EDITS_PARSE_FAIL_MULTIPLE_GROUP;
            }
            rc = EditGroupFindDuplicate (mEditGroup);
            if (rc == 0) {
                rc = GenerateProratingEdits (mEditGroup, ProratingEdits);
                if (rc != 0) {
                    EditGroupTerminate ();
                    STC_FreeMemory (mEdits);
                    return EIE_PRORATING_EDITS_PARSE_FAIL;                
                }
            }
            else {
                EditGroupTerminate ();
                STC_FreeMemory (mEdits);
                return EIE_PRORATING_EDITS_PARSE_FAIL;
            }
        }
        else {
            EditGroupTerminate ();
            STC_FreeMemory (mEdits);
            return EIE_PRORATING_EDITS_PARSE_FAIL;
        }
    }
    else {
        EditGroupTerminate ();
        STC_FreeMemory (mEdits);
        return EIE_PRORATING_EDITS_PARSE_FAIL;
    }

    EditGroupTerminate ();
    STC_FreeMemory (mEdits);

    return EIE_PRORATING_EDITS_PARSE_SUCCEED;
}

/*
*/
void EI_ProratingEditsPrint (
    EIT_PRORATING_GROUP * ProratingEdits)
{
    int i, j;

    for (i = 0; i < ProratingEdits->NumberOfEdits; i++) {
        printf ("%*sTotalName=%s Level=%d\n",
            ProratingEdits->Edit[i].Level, "",
            ProratingEdits->Edit[i].TotalName,
            ProratingEdits->Edit[i].Level);
        for (j = 0; j < ProratingEdits->Edit[i].NumberOfFields; j++) {
            printf ("%*sFieldName=%s W=%f PF=%d\n",
                ProratingEdits->Edit[i].Level, "",
                ProratingEdits->Edit[i].FieldName[j],
                ProratingEdits->Edit[i].Weight[j],
                ProratingEdits->Edit[i].ProratingFlag[j]
            );
        }
    }
}

/*
use to locate an entry in the Term list
*/
static int CompareSearch (
    char * Variable,
    tEditNode * Term)
{
    return strcmp (Term->Variable, Variable);
}

/*
traverse the trees to find if the root of a tree matches a leaf of another tree
if one is found, the root term list is moved to the leaf term list and
the root node is removed.

root nodes are:
year          and q1            and q2
|                 |                 |
q1+q2+q3+q4       m1+m2+m3+m4       m5+m6+m7+m8

after this function is called the tree will look like

root node is:
year
|
q1+q2+q3+q4
|  |_________
|            |
m1+m2+m3+m4  m5+m6+m7+m8
*/
static int EditGroupFixHierarchy (
    tADTList * EditGroup)
{
    int i, j;
    tEditNode * EditI; /* always point to a root node */
    tEditNode * EditJ; /* always point to a root node */
    tEditNode * Edit;

/* GOTO ALERT!!!!!!*/
/* as soon as a node is moved we want to start over */
/* because the tree changed and something might be missed */
STARTOVER:
    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        EditI = (tEditNode *) ADTList_Index (EditGroup, i);
        for (j = 0; j < ADTList_Length (EditGroup); j++) {
            if (i != j) {
                EditJ = (tEditNode *) ADTList_Index (EditGroup, j);

                /*
                find root variable of I Branch in brach J
                */
                Edit = LocateVariable (EditJ->TermList, EditI->Variable);

                if (Edit != NULL) {
                    if (Edit->TermList == NULL) {
                        /*
                        Edit is a leaf
                        move the tree root to the leaf node
                        */
                        Edit->TermList = EditI->TermList;
                        EditI->TermList = NULL;
                        ADTList_Remove (EditGroup, i);
                        goto STARTOVER;
                    }
                    else {
                        /*
                        Edit is NOT a leaf
                        that means the variable appear twice in the edits
                        */
                        ReportError (M30099); /* EditGroupFixHierarchy(): Can't 
                                                 have same variable twice! */
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}

/*
find variable that appears more that once in edit group
*/
static int EditGroupFindDuplicate (
    tADTList * EditGroup)
{
    tSList * ListOfDistinctVariables;
    tSList * ListOfRepeatingVariables;

    SList_New (&ListOfDistinctVariables);
    SList_New (&ListOfRepeatingVariables);

    FindDuplicate (EditGroup, ListOfDistinctVariables,
        ListOfRepeatingVariables);

    /* duplicate are found! report them */
    if (SList_NumEntries (ListOfRepeatingVariables) != 0) {
        int i;
        char * Ptr;
        int Size;
        int ReallocationSize = 100;
        char * RepeatingVariables;

        Size = 50 + 1;
        RepeatingVariables = STC_AllocateMemory (Size);
        if (RepeatingVariables == NULL)
            return -1;

        strcpy (RepeatingVariables, M30100); /* The following variable(s) 
                                                is/are repeated: */
        for (i = 0; i < SList_NumEntries (ListOfRepeatingVariables); i++) {
            Size += ReallocationSize;
            Ptr = STC_ReallocateMemory (
                Size-ReallocationSize,
                Size,
                RepeatingVariables);
            if (Ptr == NULL)
                return -1;

            RepeatingVariables = Ptr;

            strcat (RepeatingVariables,
                SList_Entry (ListOfRepeatingVariables, i));
            strcat (RepeatingVariables, ",");
        }
        RepeatingVariables[strlen (RepeatingVariables)-1] = '.';
        ReportError (RepeatingVariables);

        STC_FreeMemory (RepeatingVariables);
        SList_Free (ListOfDistinctVariables);
        SList_Free (ListOfRepeatingVariables);

        return -1;
    }

    SList_Free (ListOfDistinctVariables);
    SList_Free (ListOfRepeatingVariables);

    return 0;
}

/*
free an EditGroup
*/
static void EditGroupFree (
    tADTList * EditGroup)
{
    int i;
    tEditNode * Edit;

    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEditNode *) ADTList_Index (EditGroup, i);
        if (Edit->TermList != NULL)
            EditGroupFree (Edit->TermList);
    }
    ADTList_Delete (EditGroup);
}

/*
Initialise the EditGroup.
Should be called once at the beginning of the parsing.
*/
static void EditGroupInit (void) {
    if (DEBUG) puts ("EditGroupInit");

    mEditGroup = ADTList_Create (NULL, NULL, sizeof (tEditNode), 20);

    EditInit ();
}

/*
Print an EditGroup (debug function)
*/
static void EditGroupPrint (
    tADTList * EditGroup,
    int Level)
{
    int i, j;
    tEditNode * Edit1;
    tEditNode * Edit2;

    if (EditGroup == NULL)
        return;

    Level++;

    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit1 = (tEditNode *) ADTList_Index (EditGroup, i);

        if (Edit1->TermList != NULL) {
            printf ("%*sTotal:(Level=%d) %s\n",
                Level, "", Level, Edit1->Variable);
            for (j = 0; j < ADTList_Length (Edit1->TermList); j++) {
                Edit2 = (tEditNode *) ADTList_Index (Edit1->TermList, j);
                TermPrint (Edit2, Level);
            }
            EditGroupPrint (Edit1->TermList, Level);
        }
    }
}

/*
Free the EditGroup.
Should be called once at the end of the parsing.
*/
static void EditGroupTerminate (void) {
    if (DEBUG) puts ("EditGroupTerminate");
    if (DEBUG) EditGroupPrint (mEditGroup, -1);

    EditGroupFree (mEditGroup);
    mEditGroup = NULL;
    EditGroupFree (mEdit.TermList);
    mEdit.TermList = NULL;
}

/*
Initialise the global edit and term.
Should be called every time a new edit starts.
*/
static void EditInit (void) {
    if (DEBUG) puts (" EditInit");

    TermInit (&mTerm);
    TermInit (&mEdit);
    mEdit.TermList = ADTList_Create (NULL, (tADTListCompareProc) CompareSearch,
        sizeof (tEditNode), 20);
}

/*
Validates the Edit.
Should be called as soon as the end of an edit is found.
*/
static int EditTerminate (void) {
    char Message[150]; /* a variable can be 100 long */
    tEditNode * Edit;

    if (DEBUG) puts (" EditTerminate");

    /* find variable in the edit group tree */
    Edit = LocateVariable (mEditGroup, mEdit.Variable);
    if (Edit == NULL) {
        /* variable not present, add it to root */
        ADTList_Append (mEditGroup, (void *) &mEdit);
    }
    else {
        if (Edit->TermList == NULL) {
            /* variable present, add it to node */
            Edit->TermList = mEdit.TermList;
            mEdit.TermList = NULL;
        }
        else {
            sprintf (Message, M30101, mEdit.Variable); /* Variable '%s' is used
                                                          twice as a total! */                
            ReportError (Message);
            return -1;
        }
    }

    return 0;
}

/*
find variable that appears more that once in edit group
returns a list of duplicate variables if
*/
static void FindDuplicate (
    tADTList * Branch,
    tSList * ListOfDistinctVariables,
    tSList * ListOfRepeatingVariables)
{
    int i;
    tEditNode * Edit;
    int SizeBefore, SizeAfter;

    if (Branch == NULL)
        return;

    for (i = 0; i < ADTList_Length (Branch); i++)  {
        Edit = (tEditNode *) ADTList_Index (Branch, i);
        SizeBefore = SList_NumEntries (ListOfDistinctVariables);
        SList_AddNoDup (Edit->Variable, ListOfDistinctVariables);
        SizeAfter = SList_NumEntries (ListOfDistinctVariables);
        if (SizeBefore == SizeAfter) {
            SList_Add (Edit->Variable, ListOfRepeatingVariables);
        }
        if (Edit->TermList != NULL)
            FindDuplicate (Edit->TermList, ListOfDistinctVariables,
                ListOfRepeatingVariables);
    }
}

/*
allocate and fill EIT_PRORATING_EDIT edit
*/
static void GenerateProratingEdit (
    tEditNode * Edit,
    EIT_PRORATING_EDIT * ProratingEdit,
    int Level)
{
    int i;
    tEditNode * LocalEdit;

    ProratingEdit->NumberOfFields = ADTList_Length (Edit->TermList);

    ProratingEdit->Level = Level;
    ProratingEdit->TotalName = STC_StrDup (Edit->Variable);
    ProratingEdit->FieldName = STC_AllocateMemory (
        ProratingEdit->NumberOfFields * sizeof *ProratingEdit->FieldName);
    ProratingEdit->Weight = STC_AllocateMemory (
        ProratingEdit->NumberOfFields * sizeof *ProratingEdit->Weight);
    ProratingEdit->ProratingFlag = STC_AllocateMemory (
        ProratingEdit->NumberOfFields * sizeof *ProratingEdit->ProratingFlag);

    for (i = 0; i < ProratingEdit->NumberOfFields; i++) {
        LocalEdit = (tEditNode *) ADTList_Index (Edit->TermList, i);
        ProratingEdit->FieldName[i]    = STC_StrDup (LocalEdit->Variable);
        ProratingEdit->Weight[i]        = LocalEdit->Weight;
        ProratingEdit->ProratingFlag[i] = LocalEdit->Modifier;
    }
}

/*
move info the parser found to the EIT_PRORATING_GROUP edits.
move related the edits in groups.
*/
static int GenerateProratingEdits (
    tADTList * EditGroup,
    EIT_PRORATING_GROUP * ProratingEdits)
{
    tEditNode ** EditList;
    int i, j;
    int Index;
    int RequestedLevel;
    int NumberOfNodes;
    tEditNode * Edit;

    ProratingEdits->NumberOfEdits = 0;

    TreeStatistics (EditGroup, &ProratingEdits->NumberOfEdits);

    if (DEBUG)
        printf ("statistique NumberOfEdits=%d\n",
            ProratingEdits->NumberOfEdits);

    ProratingEdits->Edit = STC_AllocateMemory (
        ProratingEdits->NumberOfEdits * sizeof *ProratingEdits->Edit);
    if (ProratingEdits->Edit == NULL)
        return -1;

    EditList = STC_AllocateMemory (
        ProratingEdits->NumberOfEdits * sizeof *EditList);
    if (EditList == NULL)
        return -1;

    Index = 0;
    for (i = 0; i < ADTList_Length (EditGroup); i++) {
        Edit = (tEditNode *) ADTList_Index (EditGroup, i);
        GenerateProratingEdit (Edit, &ProratingEdits->Edit[Index++], 0);
        RequestedLevel = 1;
        NumberOfNodes = 0;
        GetNodes (Edit->TermList, RequestedLevel, 0, EditList, &NumberOfNodes);
        while (NumberOfNodes != 0) {
            for (j = 0; j < NumberOfNodes; j++) {
                GenerateProratingEdit (EditList[j],
                    &ProratingEdits->Edit[Index++], RequestedLevel);
            }
            NumberOfNodes = 0;
            RequestedLevel++;
            GetNodes (Edit->TermList, RequestedLevel, 0, EditList,
                &NumberOfNodes);
        }
    }
    STC_FreeMemory (EditList);

    return 0;
}

/*
return all nodes of requested level
*/
static void GetNodes (
    tADTList * Branch, /*in*/
    int RequestedLevel, /*in*/
    int CurrentLevel, /*in*/
    tEditNode ** EditNodeListPtr, /*out*/
    int * NumberOfNodes) /*out*/
{
    int i;
    tEditNode * Edit;

    if (Branch == NULL)
        return;

    CurrentLevel++;

    if (RequestedLevel == CurrentLevel) {
        /* the Current Level is right, copy the nodes ptr */
        for (i = 0; i < ADTList_Length (Branch); i++)  {
            Edit = (tEditNode *) ADTList_Index (Branch, i);
            if (Edit->TermList != NULL) {
                EditNodeListPtr[*NumberOfNodes] = Edit;
                (*NumberOfNodes)++;
            }
        }
    }
    else {
        /* the Current Level is too low, go deeper in all nodes */
        for (i = 0; i < ADTList_Length (Branch); i++)  {
            Edit = (tEditNode *) ADTList_Index (Branch, i);
            if (Edit->TermList != NULL)
                GetNodes (Edit->TermList, RequestedLevel, CurrentLevel,
                    EditNodeListPtr, NumberOfNodes);
        }
    }
}

/*
The end of an edit was found.
*/
static int InstallEdit (void) {
    int rc;

    if (DEBUG) puts (" InstallEdit");

    rc = EditTerminate ();
    if (rc != 0) return rc;

    EditInit ();

    return 0;
}

/*
A modifier was found.
*/
static void InstallModifier (
    tModifierId Modifier)
{
    if (DEBUG) puts ("   InstallModifier");

    switch (Modifier) {
    case eModifierIdAlways:
        mTerm.Modifier = EIE_PRORATING_MODIFIER_ALWAYS;
        break;
    case eModifierIdImputed:
        mTerm.Modifier = EIE_PRORATING_MODIFIER_IMPUTED;
        break;
    case eModifierIdNever:
        mTerm.Modifier = EIE_PRORATING_MODIFIER_NEVER;
        break;
    default: /* eModifierIdOriginal */
        mTerm.Modifier = EIE_PRORATING_MODIFIER_ORIGINAL;
        break;
    }
}

/*
A weight was found.
*/
static int InstallWeight (
    double Weight)
{
    if (DEBUG) puts ("   InstallWeight");

    if (Weight <= 0) {
        ReportError (M30102); /* The weight must be positive! */
        return -1;
    }
    mTerm.Weight = Weight;

    return 0;
}

/*
The end of a term was found.
*/
static int InstallTerm (void) {
    int rc;

    if (DEBUG) puts ("  InstallTerm");

    rc = TermTerminate ();
    if (rc != 0) return rc;
    TermInit (&mTerm);

    return 0;
}

/*
A total was found.
*/
static void InstallTotal (
    char * Total)
{
    if (DEBUG) puts ("  InstallTotal");

    UTIL_StrUpper (mEdit.Variable, Total);
}

/*
A variable was found.
*/
static void InstallVariable (
    char * Variable)
{
    if (DEBUG) puts ("   InstallVariable");

    UTIL_StrUpper (mTerm.Variable, Variable);
}

/*
traverse the tree to find a variable
*/
static tEditNode * LocateVariable (
    tADTList * Branch,
    char * Variable)
{
    int i;
    tEditNode * Edit;

    if (Branch == NULL)
        return NULL;

    for (i = 0; i < ADTList_Length (Branch); i++)  {
        /* is Variable a total? */
        Edit = (tEditNode *) ADTList_Index (Branch, i);
        if (strcmp (Edit->Variable, Variable) == 0) {
            return Edit;
        }
        /* is Variable a term? */
        Edit = LocateVariable (Edit->TermList, Variable);
        if (Edit != NULL)
            return Edit;
    }
    return NULL;
}

/*
Make sure the last token read match a specified token id.
If it does, call the scanner to find the next token.
If it does not, returns -1
*/
static int Match (
    tTokenId Id,
    tToken * Token)
{
    if (Id != Token->Id) {
        char Message[101];

        sprintf (Message,
            /* Looking for '%s' but found '%s' instead. */
            M30007 "\n", 
            TokenTranslator (Id), TokenTranslator (Token->Id));
        ReportError (Message);
        return -1;
    }

    return Scanner (Token);
}

/*
*/
static int MatchComponents (
    tToken * Token)
{
    int rc;

    rc = MatchTerm (Token);
    if (rc != 0) return rc;
    if (Token->Id == eTokenIdPlus) {
        rc = Match (eTokenIdPlus, Token);
        if (rc != 0) return rc;
        rc = MatchComponents (Token);
        if (rc != 0) return rc;
    }

    return 0;
}

/*
*/
static int MatchEdit (
    tToken * Token)
{
    int rc;

    rc = MatchComponents (Token);
    if (rc != 0) return rc;
    rc = MatchOperator (Token);
    if (rc != 0) return rc;
    rc = MatchTotal (Token);
    if (rc != 0) return rc;
    rc = Match (eTokenIdSemiColon, Token);
    if (rc != 0) return rc;

    rc = InstallEdit ();
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchEditGroup (
    tToken * Token)
{
    int rc;

    rc = MatchEdit (Token);
    if (rc != 0) return rc;
    while (Token->Id != eTokenIdDone) {
        rc = MatchEdit (Token);
        if (rc != 0) return rc;
    }

    return 0;
}

/*
*/
static int MatchModifier (
    tToken * Token)
{
    int rc;

    if (Token->Id == eTokenIdModifier)
        InstallModifier (Token->Value.Modifier);

    rc = Match (eTokenIdModifier, Token);
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchOperator (
    tToken * Token)
{
    int rc;

    rc = Match (eTokenIdOperator, Token);
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchTerm (
    tToken * Token)
{
    int rc;

    if (Token->Id == eTokenIdWeight) {
        rc = MatchWeight (Token);
        if (rc != 0) return rc;
    }
    rc = MatchVariable (Token);
    if (rc != 0) return rc;
    if (Token->Id == eTokenIdColon) {
        rc = Match (eTokenIdColon, Token);
        if (rc != 0) return rc;
        rc = MatchModifier (Token);
        if (rc != 0) return rc;
    }

    rc = InstallTerm ();
    if (rc != 0) return rc;

    return 0;
}

/*
*/
static int MatchTotal (
    tToken * Token)
{
    int rc;

    if (Token->Id == eTokenIdVariable)
        InstallTotal (Token->Value.Variable);

    rc = Match (eTokenIdVariable, Token);
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

    rc = InstallWeight (Token->Value.Weight);
    if (rc != 0) return rc;

    rc = Match (eTokenIdWeight, Token);
    if (rc != 0) return rc;

    return 0;
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
    char * ErrorMessage)
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
            sprintf (EditsString+strlen (EditsString), "%*s^\n",
                CaretPosition, "");
        }
        CaretPosition -= SIZEOFCHUNKS;
    }

    /* M00037 -> "Prorating edits parser"
       M00002 -> "Error at or before" */
    EI_AddMessage (M00037, EIE_ERROR, "%s\n%s:\n%s",
        ErrorMessage, M00002, EditsString);

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
static int Scanner (
    tToken * Token)
{
    char * p = mEditsPtr;

    /* skip leading spaces*/
    while (isspace (*p))
        p++;

    if (strspn (p, EI_InfoGetVariableNameFirstCharacterCharacterSet ()) > 0) {
        /* found a variable or a modifier */
        size_t n;

        Token->Value.Variable[0] = *p; /* save first character */
        p++;

        n = strspn (p, EI_InfoGetVariableNameCharacterSet ());

        if (n > (size_t) EI_InfoGetVariableNameMaxSize ()-1) {
            Token->Id = eTokenIdError;
            ReportError ("Variable name too large!");
            return -1;
        }

        strncpy (Token->Value.Variable+1, p, n);
        Token->Value.Variable[n+1] = '\0';
        p += n;

        if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdAlways]) == 0 ||
            strcmp (Token->Value.Variable, mValidModifier[eModifierIdImputed]) == 0 ||
            strcmp (Token->Value.Variable, mValidModifier[eModifierIdNever]) == 0 ||
            strcmp (Token->Value.Variable, mValidModifier[eModifierIdOriginal]) == 0) {
            Token->Id = eTokenIdModifier;
            if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdAlways]) == 0 )
                Token->Value.Modifier = eModifierIdAlways;
            else if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdImputed]) == 0)
                Token->Value.Modifier = eModifierIdImputed;
            else if (strcmp (Token->Value.Variable, mValidModifier[eModifierIdNever]) == 0)
                Token->Value.Modifier = eModifierIdNever;
            else
                Token->Value.Modifier = eModifierIdOriginal;
        }
        else
            Token->Id = eTokenIdVariable;
    }
    else if (isdigit (*p) || (*p == '.' && isdigit (*(p+1)))) {
        /* found Weight */
        char * s;

        Token->Id = eTokenIdWeight;
        Token->Value.Weight = strtod (p, &s);
        p = s;
    }
    else if (*p == '+') {
        /* found Plus */
        Token->Id = eTokenIdPlus;
        p++;
    }
    else if (*p == ':') {
        /* found colon */
        Token->Id = eTokenIdColon;
        p++;
    }
    else if (*p == ';') {
        /* found SemiColon */
        Token->Id = eTokenIdSemiColon;
        p++;
    }
    else if (*p == '=') {
        /* found operator */
        Token->Id = eTokenIdOperator;
        p++;
    }
    else if (*p == '\0') {
        /* found the end */
        Token->Id = eTokenIdDone;
    }
    else {
        /* found and unknown character */
        Token->Id = eTokenIdError;
    }

    if (DEBUG) TokenPrint (Token);

    mEditsPtr = p;

    return 0;
}

/*
Initialise a term.
*/
static void TermInit (
    tEditNode * Term)
{
    if (DEBUG) puts ("  TermInit");

    Term->Variable[0]='\0';
    Term->Weight=1;
    Term->Modifier = EIE_PRORATING_MODIFIER_NOTSET;
    Term->TermList = NULL;
}

/*
Print a term (debug function)
*/
static void TermPrint (
    tEditNode * Term,
    int Level)
{
    char Modifier[9];

    switch (Term->Modifier) {
    case (EIE_PRORATING_MODIFIER_ALWAYS):
        strcpy (Modifier, "Always  ");
        break;
    case (EIE_PRORATING_MODIFIER_IMPUTED):
        strcpy (Modifier, "Imputed ");
        break;
    case (EIE_PRORATING_MODIFIER_NEVER):
        strcpy (Modifier, "Never   ");
        break;
    case (EIE_PRORATING_MODIFIER_ORIGINAL):
        strcpy (Modifier, "Original");
        break;
    case (EIE_PRORATING_MODIFIER_NOTSET):
        strcpy (Modifier, "Not Set ");
        break;
    default:
        printf ("Modifier is :%d\n", Term->Modifier);
        exit (-1);
        break;
    };
    printf ("%*sCompo:(Level=%d) (%1.1f)%s:%s\n",
        Level, "", Level, Term->Weight, Term->Variable, Modifier);
}

/*
Put the term in the global edit.
If the variable name is already in the edit write error message
*/
static int TermTerminate (void) {
    int TermIndex;

    if (DEBUG) puts ("  TermTerminate");
    if (DEBUG) TermPrint (&mTerm, -1);

    TermIndex = ADTList_Locate (mEdit.TermList, mTerm.Variable);
    if (TermIndex == ADTLIST_NOTFOUND) {
        /* variable not present in edit, add it */
        ADTList_Append (mEdit.TermList, (void *) &mTerm);
    }
    else {
        /* variable already in edit, ERROR */
        ReportError (M30103); /* Can't have same variable twice in an edit! */
        return -1;
    }

    return 0;
}

/*
TokenPrint (debug function).
*/
static void TokenPrint (
    tToken * Token)
{
    switch (Token->Id) {
    case eTokenIdColon:
    case eTokenIdDone:
    case eTokenIdError:
    case eTokenIdOperator:
    case eTokenIdPlus:
    case eTokenIdSemiColon:
        printf ("%s\n", TokenTranslator (Token->Id));
        break;
    case eTokenIdWeight:
        printf ("%s %f\n", TokenTranslator (Token->Id), Token->Value.Weight);
        break;
    case eTokenIdModifier:
    case eTokenIdVariable:
        printf ("%s %s\n", TokenTranslator (Token->Id), Token->Value.Variable);
        break;
    default:
        printf ("%s\n", TokenTranslator (Token->Id));
        exit (-1);
    }
}

/*
*/
static char * TokenTranslator (
    tTokenId TokenId)
{
    switch (TokenId) {
    case eTokenIdColon:     return (M00004); /* Colon     */
    case eTokenIdDone:      return (M00005); /* Done      */ 
    case eTokenIdError:     return (M00006); /* Error     */
    case eTokenIdModifier:  return (M00008); /* Modifier  */
    case eTokenIdWeight:    return (M00038); /* Weight    */
    case eTokenIdOperator:  return (M00009); /* Operator  */
    case eTokenIdPlus:      return (M00010); /* Plus      */
    case eTokenIdSemiColon: return (M00011); /* SemiColon */
    case eTokenIdVariable:  return (M00013); /* Variable  */
    /* no default */
    }
    return (M00014); /* Unknown character */
}

/*
calculate the number of edits in the tree.
*/
static void TreeStatistics (
    tADTList * Branch,
    int * NumberOfEdits)
{
    int i;
    tEditNode * Edit;

    if (Branch == NULL)
        return;

    for (i = 0; i < ADTList_Length (Branch); i++)  {
        Edit = (tEditNode *) ADTList_Index (Branch, i);
        if (Edit->TermList != NULL) {
            (*NumberOfEdits)++;
            TreeStatistics (Edit->TermList, NumberOfEdits);
        }
    }
}
