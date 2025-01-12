#include "SUtil.h"

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "IO_Messages.h"

#include "EI_Message.h"
#include "STC_Memory.h"

#include <ctype.h>  // for 'isspace()'

#ifdef _DEBUG
#include <assert.h>
#endif

/* theses are used by Printf () functions. */
static size_t mPrintfAllocatedSize;
static char* mPrintfString;
static char* mPrintfStringNewLine;

#ifndef EDITS_DISABLED
static char* EditsFormat(EIT_EDITS*);
#endif
static int CountCharacter(char*, char);

/* Define either the macro _en or _fr in makefile */
#if defined (_en)
#define MsgOutOfMemory "Out of memory."
#elif defined (_fr)
#define MsgOutOfMemory "Manque de mémoire."
#else
#error "Define either the macro _en or _fr"
#endif

/* SUtil_AreAllByVariablesInDataSet
    based on `SASUtil_AreAllByVariablesInDataSet`
    */
IO_BOOLEAN SUtil_AreAllByVariablesInDataSet(
    DSR_generic* LookUp,
    DSR_generic* DataSet)
{
    int i;
    int NumberOfByVariables;
    int Position;
    int* PositionList;
    EIT_RETURNCODE rcAABVID;

    NumberOfByVariables = LookUp->VL_by_var.count;

    if (NumberOfByVariables == 0) {
        return EIE_SUCCEED; /* no by variable specified */
    }

    PositionList = LookUp->VL_by_var.positions;

    rcAABVID = EIE_SUCCEED;
    for (i = 0; i < NumberOfByVariables; i++) {

        IO_RETURN_CODE rc_io = IORC_SUCCESS;
        rc_io = DSR_get_pos_from_names(DataSet, 1, &LookUp->VL_by_var.names[i], &Position);

        if (rc_io == IORC_SUCCESS) {
            IO_VAR_TYPE var_type_lookup = DSR_get_col_type(LookUp, PositionList[i]);
            IO_VAR_TYPE var_type_dataset = DSR_get_col_type(DataSet, Position);
            if (var_type_lookup != var_type_dataset) {
                //the BY variable is not the same type in both data set
                printf(SAS_MESSAGE_PREFIX_WARNING MsgByVariableNotSameType,
                    (int) strlen(LookUp->VL_by_var.names[i]), LookUp->VL_by_var.names[i]);
                rcAABVID = EIE_FAIL;
            }
        }
        else {
            //the BY variable is not in the secondary data set
            printf(SAS_MESSAGE_PREFIX_WARNING MsgByVariableNotInDataSet "\n",
                (int) strlen(LookUp->VL_by_var.names[i]), LookUp->VL_by_var.names[i], (int) strlen(DataSet->dataset_name), DataSet->dataset_name);
            rcAABVID = EIE_FAIL;
        }
    }

    return rcAABVID;
}

IO_BOOLEAN SUtil_GetOriginalName(DSR_generic* dsr, char* var_name) {
    int num_vars = (int) dsr->num_columns;

    for (int i = 0; i < num_vars; i++) {
        const char* column_name = dsr->col_names[i];
        if (IOUtil_stricmp_is_equal(var_name, column_name)) {
            IOUtil_strcpy(var_name, column_name);  // safe to use `strcpy`, src and dest are equal length
            return IOB_TRUE;
        }
    }
    return IOB_FALSE;
}

/*
Returns the position of variable in a SAS data set.
-1 if not found.
*/
int SUtil_GetVariablePosition(
    DSR_generic* dsr,
    char* VariableName)
{
    int i;
    int nvars;
    char nnameUppercase[IO_COL_NAME_LEN + 1];
    char VariableNameUppercase[IO_COL_NAME_LEN + 1];

    /* find the number of variables that are on the data set */
    nvars = dsr->num_columns;

    IOUtil_copy_varname(VariableNameUppercase, VariableName);
    IOUtil_str_upcase(VariableNameUppercase);

    for (i = 0; i < nvars; i++) {
        IOUtil_copy_varname(nnameUppercase, dsr->col_names[i]);
        IOUtil_str_upcase(nnameUppercase);

        if (!strcmp(nnameUppercase, VariableNameUppercase))
            return i;
    }
    return -1; /* Not found! */
}


/* Prints dataset name, num rows, num columns
    If not specified, prints nothing */
void SUtil_PrintInputDataSetInfo(DSR_generic* dsr) {
    if (dsr->is_specified != IOSV_SPECIFIED) {
        printf(MSG_PREFIX_NOTE JUM_DATASET_NOT_SPECIFIED, dsr->dataset_name);
        return;
    }

    printf(MSG_PREFIX_NOTE JUM_PRINT_INPUT_DATASET_METADATA,
        dsr->dataset_name,
        (int) dsr->num_columns,
        (int) dsr->num_records
    );
}

/* Prints whether dataset is enabled (requested) or not */
void SUtil_PrintOutputDataSetInfo(DSW_generic* dsw) {
    if (dsw->is_requested == IOB_TRUE) {
        printf(MSG_PREFIX_NOTE JUM_DATASET_ENABLED, dsw->dataset_name);
    }
    else {
        printf(MSG_PREFIX_NOTE JUM_DATASET_DISABLED, dsw->dataset_name);
    }

    return;
}

/*
Allocates memory for SUtil_PrintfToOutput ().
This function must be called before calling
SUtil_PrintfToOutput() or SUtil_PrintfToLog().
*/
void SUtil_PrintfAllocate(void) {
    mPrintfAllocatedSize = 10000;
    mPrintfString = STC_AllocateMemory(mPrintfAllocatedSize);
}

/*
Frees memory used by SUtil_PrintfToOutput ().
It is the programmers responsability to call this function once
the memory is not needed.
*/
void SUtil_PrintfFree(void) {
    STC_FreeMemory(mPrintfString);
    mPrintfAllocatedSize = 0;
    mPrintfString = NULL;
}

/*
Prints to log.
*/
int SUtil_PrintfToLog(
    const char* Format,
    ...)
{
    va_list ap;
    size_t ReasonableSize;
    size_t ActualSize;

    ReasonableSize = 100 * strlen(Format);

    /*    printf ("ReasonableSize=%d mPrintfAllocatedSize=%d\n",*/
    /*        ReasonableSize, mPrintfAllocatedSize);*/

    if (mPrintfAllocatedSize < ReasonableSize) {
        mPrintfString = STC_ReallocateMemory(mPrintfAllocatedSize, ReasonableSize, mPrintfString);
        mPrintfAllocatedSize = ReasonableSize;
    }

    va_start(ap, Format);
    ActualSize = vsnprintf(mPrintfString, mPrintfAllocatedSize, Format, ap);
#ifdef _DEBUG
    assert(ActualSize < mPrintfAllocatedSize);
#endif
    va_end(ap);

    printf(mPrintfString);

    /*    printf ("ActualSize=%d Allocated size=%d string WITH SAS newline=%d\n",*/
    /*        ActualSize, mPrintfAllocatedSize,*/
    /*        strlen (mPrintfStringNewLine)-ActualSize);*/

    return (int)ActualSize;
}

/*
Print the API messages.
FUTURE IMPROVEMENTS: SAS justified multi-line ERROR, WARNING, NOTE messages
    SAS used non-standard "%m" and "%-2n" format specifiers.
    These allowed justification as shown here
      ERROR: first line
             second line magically justified correctly
    Following SAS' removal this non-standard feature is not implemented
    To reproduce this outside of SAS
    - `SUtil_ReplaceBackslashN()` will need to replace '\n' with some non-standard specifier
    - re-add %m (i.e. another non-standard specifier) into code below 
      - (removed at commit c29931752b1055aabe0f54017206ac5ce2929c49)
    - scan message and find %m's location
        - add prepadding to messages following special newline
*/
void SUtil_PrintMessages(void)
{
    int i;
    EIT_MESSAGE* Message;
    char* Text;
    size_t Size;

    for (i = 0; i < EI_GetNumberOfMessages(); i++) {
        Message = EI_GetMessage(i);

        /* 32 additional characters for prefix, any separators in format specifier */
        Size = strlen(Message->MessageText);
        size_t max_size = Size + 32;
        Text = STC_AllocateMemory(max_size);

        switch (Message->Severity) {
        case EIE_MESSAGESEVERITY_INFORMATION:
            snprintf(Text, max_size, "%s%s", SAS_MESSAGE_PREFIX_NOTE, Message->MessageText); // removed %m
            break;
        case EIE_MESSAGESEVERITY_WARNING:
            if (Message->CallingProgram[0] == '\0')
                snprintf(Text, max_size, "%s%s", SAS_MESSAGE_PREFIX_WARNING, Message->MessageText); // removed %m
            else
                snprintf(Text, max_size, "%s%s: %s", SAS_MESSAGE_PREFIX_WARNING, Message->CallingProgram, Message->MessageText); // removed %m
            break;
        case EIE_MESSAGESEVERITY_ERROR:
        default:
            if (Message->CallingProgram[0] == '\0')
                snprintf(Text, max_size, "%s%s", SAS_MESSAGE_PREFIX_ERROR, Message->MessageText); // removed %m
            else
                snprintf(Text, max_size, "%s%s: %s", SAS_MESSAGE_PREFIX_ERROR, Message->CallingProgram, Message->MessageText); // removed %m
            break;
        case EIE_MESSAGESEVERITY_EMPTY:
            snprintf(Text, max_size, "%s", Message->MessageText);
            break;
        }
        printf("%s\n", Text);
        STC_FreeMemory(Text);
    }

    /* since the messages were printed there is no need to keep them */
    EI_ClearMessageList();
}
/*
Prints statement name and string of list of variables in it.
Ex.: will print in log: "BY = VAR1 VAR2 ... VARn"
*/
void SUtil_PrintStatementVars(
    PARM_generic* pg,
    DSR_generic* dsr, 
    DS_varlist* vl,
    char* StatementName)
{
    if (vl->count > 0) {
        char* temp_vl_str = DSR_get_varname_str(dsr, vl->count, vl->positions);
        printf(SAS_MESSAGE_PREFIX_NOTE "%s = %s" "\n", StatementName, temp_vl_str); // removed %m
        STC_FreeMemory(temp_vl_str);
    }
    else if (pg->is_specified == IOSV_NOT_SPECIFIED) {
        printf(SAS_MESSAGE_PREFIX_NOTE MsgStatementNotSpecified "\n", StatementName); /* not specified */
    }
    else if (pg->is_specified == IOSV_SPECIFIED) {
            printf(SAS_MESSAGE_PREFIX_NOTE MsgStatementSpecifiedWithoutVariableList "\n",
                StatementName);
    }
}

/*
Print the system info for procedures.
Should be called by using the define:

SUtil_PrintSystemInfo ("Banff","1.00","OUTLIER","v1.01.001","banff@statcan.ca",
    "This procedure is for use at Statistics Canada only.");
*/
void Hidden_SUtil_PrintSystemInfo(
    char* SystemName,
    char* SystemVersion,
    char* ProcedureName,
    char* ProcedureVersion,
    char* EmailAddress,
    char* OrganisationMessage,
    char* CompilationDate,
    char* CompilationTime)
{
    IO_PRINT(SAS_MESSAGE_PREFIX_NOTE M10001 "\n", /* System developed by Statistics Canada */
        SystemName, SystemVersion);
    IO_PRINT(SAS_MESSAGE_PREFIX_NOTE M10002 "\n", /* PROCEDURE %s Version %s */
        ProcedureName, ProcedureVersion);
    IO_PRINT(SAS_MESSAGE_PREFIX_NOTE M10003 "\n", /* Created on %s at %s */
        CompilationDate, CompilationTime);
    IO_PRINT(SAS_MESSAGE_PREFIX_NOTE M10004 "\n", /* Email: %s */
        EmailAddress);
    if (OrganisationMessage != NULL) {
        IO_PRINT(SAS_MESSAGE_PREFIX_NOTE M10005 "\n", /* %s */
            OrganisationMessage);
    }
    IO_PRINT("\n");
}


/*
Returns the presence or absence of duplicate "variable position"
in a statement list of position of variables in a SAS data set
0: if no duplicate (absence of )
1: if at least one duplicate (presence of )
*/
int SUtil_AreDuplicateInListPosition(
    int NumberofVars,
    int* VarsPositionList)
{
    int i, j;
    for (i = 0; i < NumberofVars - 1; i++) {
        for (j = i + 1; j < NumberofVars; j++) {
            if (VarsPositionList[i] == VarsPositionList[j]) {
                return 1;
            }
        }
    }
    return 0;
}

/*
copy a string to another
DestinationMaxLength should be equal to the buffer length - 1,
i.e. the Max Length of the null-terminated string that fits in the buffer
*/
char* SUtil_CopyPad(
    char* Destination,
    char* Source,
    size_t DestinationMaxLength)
{
    return IOUtil_strcpy_s(Destination, DestinationMaxLength + 1, Source);
}

/*
While data is already null-terminated, the funciton being replaced
also removed trailing whitespace, a functionality carried over
in this replacement.  

Purpose: remove trailing whitespace from a string
*/
void SUtil_NullTerminate(
    char* String,
    int Length)
{
    if (String == NULL) {
        return;
    }
    // string exists

    int original_length = (int) strlen(String);

    if (original_length == 0) {
        return;
    }
    // replace all trailing whitespace characters with '\0'
    for (int i = original_length - 1; i >= 0; i--) {
        if (isspace(String[i])) {
            String[i] = '\0';
        }
        else {
            break;
        }
    }

    return;
}

/*
pad a string with blanks
*/
char* SUtil_Pad(
    char* Destination,
    size_t DestinationMaxLength)
{
    size_t DestinationLength;

    DestinationLength = strlen(Destination);
    memset(Destination + DestinationLength, ' ',
        DestinationMaxLength - DestinationLength);

    return Destination;
}

#ifndef EDITS_DISABLED
/*
Prints edits to log file
*/
void SUtil_PrintEdits(
    EIT_EDITS* Edits)
{
    char* TheEdits;

    TheEdits = EditsFormat(Edits);
    printf("%s\n", TheEdits);
    STC_FreeMemory(TheEdits);
}

/*
Writes the edits to a string suitable to be printed by SAS.
Side effect: allocates a string to be freed by the calling function.
*/
static char* EditsFormat(
    EIT_EDITS* Edits)
{
    char* TempEditsString;
    size_t l;

    /* make sure its big enough */
    l = EI_EditsFormatSize(Edits);
    TempEditsString = STC_AllocateMemory(l);
    EI_EditsFormat(Edits, TempEditsString);

    return TempEditsString;
}

#endif

/*
Count the occurences of character Character in string String.
Returns the count.
*/
static int CountCharacter(
    char* String,
    char Character)
{
    int n = 0;

    while (*String)
        if (*String++ == Character)
            n++;

    return n;
}

/* based off SASMemory.c:SAS_AllocateMemory(...)
    - allocate AND ZERO INITIALIZE memory
    The SAS Toolkit SAS_XMEMZER(...) zero initializes the memory
    which allowed for programming mistakes (like referencing uninitialized 
    memory) to go unchecked.  
    Switching to an allocation of uninitialized memory means program outcomes can differ
    
    IT IS IMPORTANT TO USE calloc(...) INSTEAD OF malloc(...) FOR THE ABOVE REASON */

void* SUtil_AllocateMemory(size_t Quantity) {
    void* Ptr;

    /* emulate `SAS_XMEM...` function behaviour for 0-byte allocations */
    if (Quantity == 0) {
        return (void*) 0x1;
    }

    Ptr = calloc(Quantity, 1);

    if (Ptr == NULL) {
        IO_PRINT(SAS_MESSAGE_PREFIX_ERROR MsgOutOfMemory);
        exit(EXIT_FAILURE);
    }

    return Ptr;
}

void* SUtil_ReallocateMemory(
    size_t OriginalQuantity,
    size_t NewQuantity,
    void* Source)
{
    void* Ptr;

    /* standards say if Source is NULL realloc behaves like malloc() */
    if (Source == NULL) {
        return SUtil_AllocateMemory(NewQuantity);
    }

    /* standards say if NewQuantity is 0 and Source is not NULL
    realloc () behaves like free () */
    if (NewQuantity == 0 && Source != NULL) {
        free(Source);
        return NULL;
    }

    /* if OriginalQuantity == NewQuantity, leave memory alone */
    if (OriginalQuantity == NewQuantity)
        return Source;

    /* if allocation fails, do not free Source */
    Ptr = SUtil_AllocateMemory(NewQuantity);
    if (Ptr == NULL)
        return NULL;

    /* Does it increase or shrink memory? */
    if (OriginalQuantity < NewQuantity) {
        /* space is increasing, copy all of original memory */
        memcpy(Ptr, Source, OriginalQuantity);
    } else {
        /* space is shrinking, copy only what will fit in NewQuantity */
        memcpy(Ptr, Source, NewQuantity);
    }

    /* free source */
    free(Source);

    return Ptr;
}

void SUtil_FreeMemory(
    void* Ptr)
{
    /* some legacy code expects this behaviour for special value `0x1` */
    if (Ptr == (void*)0x1) {
        return;
    }

    free(Ptr);
}