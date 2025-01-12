#ifndef EI_MESSAGE_H
#define EI_MESSAGE_H

#include "EI_Common.h"

/*-the flag  to activate the code seting up the facilities for writing debugging information to a   */
/* disk file:                                                                                       */
/* #define DBG_ON */
#ifdef DBG_ON
/*-the flags to activate the print statements in sensitiv.c::sensitiv():                            */
/* (the statements printing the value of the datetimestamp and the path+name of the output file)    */
#define DBG_ON_0001_0000
/* (the statements printing the contents of "HTreeRoot" using the buffer "dbgscrch2"):              */
#define DBG_ON_0001_0002
/* (the statements printing the contents of "Parms" using the buffer "dbgscrch2"):                  */
#define DBG_ON_0001_0006
/* (the statements printing the contents of "SRule" using the buffer "dbgscrch2"):                  */
#define DBG_ON_0001_0007
/* (the statements printing the contents of "KdTree" using the buffer "dbgscrch2"):                 */
#define DBG_ON_0001_0008
/* (the statement printing the contents of "RangeRoot" using the buffer "dbgscrch2"):               */
#define DBG_ON_0005_0002
/* (the statements printing the contents of "Cell" after the sensitivity of "Cell" has been calc'ed */
/*  using the buffer "dbgscrch2"                                                                    */
/* ):                                                                                               */
#define DBG_ON_0012_0002
/* (the statements printing the contents of "Cell" to a .tdx file after the sensitivity of "Cell"   */
/*  has been calc'ed (without using a buffer)                                                       */
/* ):                                                                                               */
#define DBG_ON_0012_0003

/* (all of the other print statements in senstiv.c::sensitiv()):                                    */
/* #define DBG_ON_0001_0001 */
/* (the statement printing the contents of "CellSet" after the internal cells have been processed): */
/* #define DBG_ON_0001_0003 */
/* (the statement printing the contents of "CellSet" after the marginal cells have been processed): */
/* #define DBG_ON_0001_0004 */
/* (the statement printing the contents of "CellSet" after the unused cells have been deallocated): */
/* #define DBG_ON_0001_0005 */
/*-the flag  to activate the print statements in sensitiv.c::PrintParms():                          */
/* #define DBG_ON_0002_0001 */
/*-the flag  to activate the print statements in sensitiv.c::calculate_proxy_ratio():               */
/* #define DBG_ON_0003_0001 */
/*-the flag  to activate the print statements in sensitiv.c::ReadData():                            */
/* #define DBG_ON_0004_0001 */
/*-the flag  to activate the print statements in sensitiv.c::ValidateParms():                       */
/* (all of the other print statements in senstiv.c::ValidateParms()):                               */
/* #define DBG_ON_0005_0001 */
/*-the flag  to activate the print statements in sensitiv.c::WriteCellObs():                        */
/* #define DBG_ON_0006_0001 */
/*-the flag  to activate the print statements in STC_Build.c::STC_AddMicrodata():                   */
/* #define DBG_ON_0007_0001 */
/*-the flag  to activate the print statements in STC_Build.c::CombineCodes():                       */
/* #define DBG_ON_0008_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::STC_CellAdd():                         */
/* #define DBG_ON_0009_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::CalculateFXAndLargestFX():             */
/* #define DBG_ON_0010_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::CalculateSensitivityVariablesOld() and */
/*                                               STC_Cell.c::CalculateSensitivityVariables():       */
/* #define DBG_ON_0011_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::STC_CellSensitivity():                 */
/* (all of the other print statements in STC_Cell.c::STC_CellSensitivity()):                        */
/* #define DBG_ON_0012_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::CellAddItem():                         */
/* #define DBG_ON_0013_0001 */
/*-the flag  to activate the print statements in STC_SRule.c::STC_SRuleSensitivity():               */
/* #define DBG_ON_0014_0001 */
/*-the flag  to activate one special one-time print statement in STC_SRule.c::STC_SRuleSensitivity():               */
/* #define DBG_ON_0014_9999 */
/*-the flag  to activate the print statements in STC_Write.c::STC_WriteTargets():                   */
/* #define DBG_ON_0015_0001 */
/*-the flag  to activate the print statements in sensitiv.c::DefineOutTargetsDataSet():                          */
/* #define DBG_ON_0016_0001 */
/*-the flag  to activate the print statements in STC_SRule.c:: STC_SRuleParse():                    */
/* #define DBG_ON_0017_0001 */
/*-the flag  to activate the print statements in sensitiv.c::GetParms():                            */
/* #define DBG_ON_0018_0001 */
/*-the flag  to activate the print statements in STC_Build.c::STC_CalculateInternalCellsSensitivity(): */
/* #define DBG_ON_0019_0001 */
/*-the flag  to activate the print statements in STC_Cell.c::DBG_CellWriteTdx():                    */
/* #define DBG_ON_0020_0001 */
/*-the flag  to activate the print statements in STC_SRule.c::PtnSensitivity():                     */
/* #define DBG_ON_0021_0001 */
/*-the flag  to activate the print statements in STC_Build.c::CalculateMarginalCellsSensitivityForAllCells():                     */
/* #define DBG_ON_0022_0001 */
/*-the flag  to activate the print statements in STC_Build.c::CalculateMarginalCell():              */
/* #define DBG_ON_0023_0001 */
/*-the flag  to activate the print statements in STC_Build.c::STC_CalculateMarginalCellsSensitivity(): */
/* #define DBG_ON_0024_0001 */

#define DBGBUFSIZE 100000
#define DBGBUFHALF  50000
#else
#define DBGBUFSIZE      2
#define DBGBUFHALF      1
#endif
#define DBGFNMSIZE   2000

/* EIT_ADDMESSAGE_RETURNCODE - Return codes for EI_AddMessage() */
typedef enum {
    EIE_ADDMESSAGE_FAIL = EIE_FAIL,
    EIE_ADDMESSAGE_SUCCEED = EIE_SUCCEED
  } EIT_ADDMESSAGE_RETURNCODE;

/* EIT_ALLOCATEMESSAGELIST_RETURNCODE -
    Return codes for EI_AllocateMessageList() */
typedef enum {
    EIE_ALLOCATEMESSAGELIST_FAIL = EIE_FAIL,
    EIE_ALLOCATEMESSAGELIST_SUCCEED = EIE_SUCCEED
} EIT_ALLOCATEMESSAGELIST_RETURNCODE;

/* EIT_CLEARMESSAGELIST_RETURNCODE -
    Return codes for EI_ClearMessageList()*/
typedef enum {
    EIE_CLEARMESSAGELIST_FAIL = EIE_FAIL,
    EIE_CLEARMESSAGELIST_SUCCEED = EIE_SUCCEED
} EIT_CLEARMESSAGELIST_RETURNCODE;

/* EIT_REALLOCATEMESSAGELIST_RETURNCODE-Return codes for ReallocateMessageList() */
typedef enum {
    EIE_REALLOCATEMESSAGELIST_FAIL = EIE_FAIL,
    EIE_REALLOCATEMESSAGELIST_SUCCEED = EIE_SUCCEED
} EIT_REALLOCATEMESSAGELIST_RETURNCODE;

/* EIT_SEVERITY - Severity enumeration. OBSOLETE */
typedef enum {
    EIE_INFORMATION,
    EIE_WARNING,
    EIE_ERROR,
    EIE_MEMORY
} EIT_SEVERITY;


typedef void (*EIT_PRINTMESSAGESCALLBACK) (void);


CLASS_DECLSPEC EIT_PRINTMESSAGESCALLBACK EI_PrintMessages;


/* EIT_MESSAGESEVERITY - Almost like Severity enumeration. */
enum EIT_MESSAGESEVERITY {
    EIE_MESSAGESEVERITY_INFORMATION,
    EIE_MESSAGESEVERITY_WARNING,
    EIE_MESSAGESEVERITY_ERROR,
    EIE_MESSAGESEVERITY_MEMORY,
    EIE_MESSAGESEVERITY_EMPTY 
};
typedef enum EIT_MESSAGESEVERITY EIT_MESSAGESEVERITY;

/*
EIT_MESSAGE - Structure which contains the error message.
This structure holds one message.

Structure variables:
CallingProgram; The program that genereates the Message
Severity;       The severity of the Message
MessageText;    The Message itself; formatted with the arguments
*/
typedef struct{
    char * CallingProgram;
    EIT_MESSAGESEVERITY Severity;
    char * MessageText;
} EIT_MESSAGE;

/*
Adds a Message to the MessageList.
This function fills in the EIT_MESSAGE structure.  The EIT_MESSAGE.MessageText
will be set to the MessageText parameter formatted with any substitution
strings supplied.
The conversion is identical to the following:
    sprintf (EIT_MESSAGE.MessageText, MessageText, ...);

Example:  EI_AddMessage ("Calling Prog", EI_INFORMATION, "%s = %E", "pi", 3.14);

Returns: EIT_ADDMESSAGE_RETURNCODE.
*/
CLASS_DECLSPEC EIT_ADDMESSAGE_RETURNCODE EI_AddMessage (
        /* The calling program */
    char* CallingProgram,
        /* The severity of the error/warning */
    EIT_MESSAGESEVERITY Severity,
        /* The message text */
    char* MessageText,
        /* Any strings that get substituted into the MessageText */
    ...);

/*
Flags the Message List to output an "Out of Memory" Message.
Called by the memory allocation functions in STC_Memory_c.c when memory cannot
be allocated.  This function clears all but the first Message in the Message
List and sets the Number Of Allocated Messages to -1 as a flag that memory
allocation has failed.
*/
CLASS_DECLSPEC void EI_AddOutOfMemoryMessage (void);

/*
Allocates memory to the initial EIT_ALL_MESSAGES struct and initializes it.
Returns EIT_ALLOCATEMESSAGELIST_RETURNCODE
*/
CLASS_DECLSPEC EIT_ALLOCATEMESSAGELIST_RETURNCODE  EI_AllocateMessageList (void);

/*
Clears all Messages in the Message List.
All Messages in the Message List are removed and the memory free()d.  The
Number Of Messages is set to 0, and the Number Of Allocated Messages is set
to MSG_ALLOCATEION_SIZE. The function fails when memory for the Message array
cannot be reallocated.
Returns: EIT_CLEARMESSAGELIST_RETURNCODE
*/
CLASS_DECLSPEC EIT_CLEARMESSAGELIST_RETURNCODE EI_ClearMessageList (void);

/*
Deallocates memory for the Message List.
Deallocates memory for all Messages in the Message List, and resets all
counters to zero.
*/
CLASS_DECLSPEC void EI_FreeMessageList (void);


/*
Returns a reference to the ith Message
The Message is returned in the form of the EIT_MESSAGE struct.

Returns: Message[i] or NULL on failure.  A NULL return value indicates either
that i is out of bounds, or that memory allocation has failed previous to this
function call.  In the latter case, Message[0].Severity is set to EIE_MEMORY.
Example: 
    EIT_MESSAGE * msg; 
    if ((msg = EI_GetMessage (i)) == NULL)  {
        if (EI_GetMessage(0).Severity == EIE_MEMORY)  {
             \* memory allocation failure *\ 
             ...
        }
    }
*/
CLASS_DECLSPEC EIT_MESSAGE * EI_GetMessage (
        /* The index of the Message to retrieve */
    int i);


/*
Gets the number of Messages in the Message List
Returns: The number of Messages in the Message List
*/
CLASS_DECLSPEC int EI_GetNumberOfMessages (void);

CLASS_DECLSPEC void EI_PrintMessagesSetCB (EIT_PRINTMESSAGESCALLBACK);

CLASS_DECLSPEC void EI_mMessageList_Message_to_NULL (void);
CLASS_DECLSPEC int EI_mMessageList_Message_is_NULL (void);

/* extern char dbgscratch[DBGBUFSIZE];                              */
/* extern int dbgscratchused;                                       */

/* void DBG_ClearDbgscratch   (                                  ); */
/* int  DBG_dumptofile        (char               * filename     ); */

extern char dbgscrch          [DBGBUFSIZE];
extern char dbgscrch2         [DBGBUFSIZE];
/* extern int dbscrusd ; */
/* extern int dbscrusd2; */
extern char dbgoufnm          [DBGFNMSIZE];
extern char dbgoufnm_cellset  [DBGFNMSIZE];
extern char dbgoufnm_cells_tdx[DBGFNMSIZE];
extern char dbgoufnm_ditms_tdx[DBGFNMSIZE];
extern char dbgoufnm_htreeroot[DBGFNMSIZE];
extern char dbgoufnm_parms    [DBGFNMSIZE];
extern char dbgoufnm_srule    [DBGFNMSIZE];
extern char dbgoufnm_rangeroot[DBGFNMSIZE];
extern char dbgoufnm_kdtree   [DBGFNMSIZE];

extern void DBG_cdbs              (                 char * dbgscratch                 );
extern int  DBG_dtf               (char * dbgscrfn, char * dbgscratch                 );
extern int  DBG_printf            (char * dbgscrfn, char * dbgscratch, char * fmt, ...);
extern int  DBG_fprintf           (char * output_file_path_and_name, char * filemode, char * fmt, ...);

extern int    error_occurred_in_expression_macro;
extern double report_error_in_conditional_expression_returning_double(char * error_message, ...);
extern int    report_error_in_conditional_expression_returning_int   (char * error_message, ...);

#endif
