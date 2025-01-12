/*-------------------------------------------------------------

  NAME         EI_Message.c

  DESCRIPTION  Logs and returns Error and Warning messages

-------------------------------------------------------------*/

/*-----------------------------------------------------------*/
/* Include Files                                             */
/*-----------------------------------------------------------*/
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>


#include "EI_Message.h"
#include "STC_Memory.h"

#define MSG_ALLOCATION_SIZE 50

/*
T_ALL_MESSAGES - Contains a list of all Messages
This structure contains a list of all Messages

Structure variables:
NumberOfMessages;          The actual number of Messages in the list.
NumberOfAllocatedMessages; The amount of space allocated for Messages.
Message;                   An array of all the messages.
*/
struct T_ALL_MESSAGES {
  int NumberOfMessages;
  int NumberOfAllocatedMessages;
  EIT_MESSAGE * Message;
};
typedef struct T_ALL_MESSAGES T_ALL_MESSAGES;


/*-------------------------------------------------------------*/
/* Module Global Variable                                      */
/*-------------------------------------------------------------*/
static T_ALL_MESSAGES mMessageList;
static EIT_MESSAGE mMessageOutOfMemory = {"EI_Message", EIE_ERROR, "Out of memory."};
static EIT_MESSAGE mMessageOutOfRange = {"EI_Message", EIE_ERROR, "Message index out of range."};
static int mOutOfMemory = 0;//false


/*--------------------------------------------------------------*/
/* Function Prototypes                                          */
/*--------------------------------------------------------------*/
static void DefaultPrintMessages (void);
static EIT_REALLOCATEMESSAGELIST_RETURNCODE Reallocate (int);
static void RemoveAllMessages (void);


/*-------------------------------------------------------------*/
/* Global Variable                                             */
/*-------------------------------------------------------------*/
CLASS_DECLSPEC EIT_PRINTMESSAGESCALLBACK EI_PrintMessages = DefaultPrintMessages;


/*------------------------------------------------------------------------*/
/* This function adds a Message to the Message List.
   1) Memory is reallocated for more messages if neccessary.
   2) The Message Text is parsed for the number of substitution strings.
   3) The function parameters are added to the next Message in the
         Message List.
*/
/*------------------------------------------------------------------------*/
EIT_ADDMESSAGE_RETURNCODE EI_AddMessage (
    char * CallingProgram,
    EIT_MESSAGESEVERITY Severity,
    char * MessageText,
    ...)
{
    va_list ap; /* Variable-length Argument list */
    char * s;

	if (mOutOfMemory) {
        return EIE_ADDMESSAGE_FAIL;
	}

    /* list full? */
    if (mMessageList.NumberOfMessages==mMessageList.NumberOfAllocatedMessages) {
#ifdef NEVER
        if (Reallocate (
                mMessageList.NumberOfAllocatedMessages+MSG_ALLOCATION_SIZE) ==
                    EIE_FAIL) {
            return EIE_ADDMESSAGE_FAIL;
        }
#endif
		EI_PrintMessages ();
    }

    /*----------------------------*/
    /* Add Message to Pile        */
    /*----------------------------*/
    /* Allocate memory for the Message Text, and copy the string into the struct */
#define MAXIMUM_MESSAGE_SIZE 50000

    /* allocate a big chunck of memory */
    s = STC_AllocateMemory (MAXIMUM_MESSAGE_SIZE+1);
    if (s == NULL)
        return EIE_ADDMESSAGE_FAIL;
    va_start (ap, MessageText);
    vsprintf (s, MessageText, ap);
    va_end (ap);

    /* make sure the message is not too long! */
    assert (strlen (s) < MAXIMUM_MESSAGE_SIZE);

    /* allocate only what we need for MessageText */
    mMessageList.Message[mMessageList.NumberOfMessages].MessageText =
        STC_StrDup (s);
    if (mMessageList.Message[mMessageList.NumberOfMessages].MessageText == NULL) {
        return EIE_ADDMESSAGE_FAIL;
    }

    /* free the big chunk */
    STC_FreeMemory (s);

    /* Allocate memory for Calling Program; Copy the string into the struct */
	mMessageList.Message[mMessageList.NumberOfMessages].CallingProgram =
        STC_StrDup (CallingProgram);
    if (mMessageList.Message[mMessageList.NumberOfMessages].CallingProgram ==
            NULL) {
        return EIE_ADDMESSAGE_FAIL;
    }

    /* Add the Severity to the struct. */
    mMessageList.Message[mMessageList.NumberOfMessages].Severity = Severity;

    /* Increment the Number Of Messages */
    mMessageList.NumberOfMessages++;

    return EIE_ADDMESSAGE_SUCCEED;
}
/*-----------------------------------------------------------------------
This function is called from the memory allocation functions in
STC_Memory_c.c when memory allocation fails.
1) All message are removed from the Message List
-----------------------------------------------------------------------*/
void EI_AddOutOfMemoryMessage (
	void)
{
	//leave if EI_AddOutOfMemoryMessage() already called
	if (mOutOfMemory)
		return;
	RemoveAllMessages ();
	mMessageList.NumberOfMessages = 1;
	mOutOfMemory = 1;
}
/*------------------------------------------------------------------------*/
/* This function initializes the Message List.  It should be the first
   function called.
   1) Memory for an initial array of Messages is allocated
   2) Message List variables are initialezed                              */
/*------------------------------------------------------------------------*/
EIT_ALLOCATEMESSAGELIST_RETURNCODE EI_AllocateMessageList (
	void)
{
    mMessageList.NumberOfMessages = 0;
    mMessageList.NumberOfAllocatedMessages = MSG_ALLOCATION_SIZE;
    mMessageList.Message = STC_AllocateMemory (
        MSG_ALLOCATION_SIZE * sizeof *mMessageList.Message);
    if (mMessageList.Message == NULL) {
        return EIE_ALLOCATEMESSAGELIST_FAIL;
    }

    return EIE_ALLOCATEMESSAGELIST_SUCCEED;
}
/*--------------------------------------------------------------------------*/
/* This function clears the Message List of all Messages.  All memory for   */
/* the Messages is free()d.  The Message array itself is reallocated to the */
/* minimum number (MSG_ALLOCATION_SIZE) if neccessary.                      */
/*--------------------------------------------------------------------------*/
EIT_CLEARMESSAGELIST_RETURNCODE EI_ClearMessageList (
	void)
{
    RemoveAllMessages ();

    /*reallocate memory only if neccessary*/
    if (mMessageList.NumberOfAllocatedMessages != MSG_ALLOCATION_SIZE) {
        if (Reallocate (MSG_ALLOCATION_SIZE)==EIE_REALLOCATEMESSAGELIST_FAIL) {
            return EIE_CLEARMESSAGELIST_FAIL;
        }
    }

    return EIE_CLEARMESSAGELIST_SUCCEED;
}
/*-------------------------------------------------------------------------*/
/* This function frees the Message List structure (and all Messages in it) */
/* This should be the last function called                                 */
/*-------------------------------------------------------------------------*/
void EI_FreeMessageList (
	void)
{
	EI_PrintMessages ();//print messages
    RemoveAllMessages ();
    mMessageList.NumberOfAllocatedMessages = 0;
    STC_FreeMemory (mMessageList.Message);
}
/*------------------------------------------------------------------------*/
/* This function returns a string which consists of the Message Text with
   the Substitution Strings inserted.
   The string that is returned must be 'free()'d by the programmer to avoid
   memory leaks.
   1) The input parameter is checked
   2) If the Number Of Allocated Messages == MEMORY_ALLOCATION_ERROR,
      memory allocation has previously failed.                             */
/*-------------------------------------------------------------------------*/
EIT_MESSAGE * EI_GetMessage (
    int i)
{
	if (mOutOfMemory) {
		mMessageList.NumberOfMessages = 0;
		return &mMessageOutOfMemory;
	}
    else if (((i + 1) > mMessageList.NumberOfMessages) || (i < 0)) {
		mMessageList.NumberOfMessages = 0;
        return &mMessageOutOfRange;
    }

    return &mMessageList.Message[i];
}
/*------------------------------------------------------------------------*/
/* This function returns the Number of Messages in the Message List       */
/*------------------------------------------------------------------------*/
int EI_GetNumberOfMessages (
	void)
{
    return mMessageList.NumberOfMessages;
}
/*------------------------------------------------------------------------*/
/* function to set the callback function to print messages                */
/*------------------------------------------------------------------------*/
void EI_PrintMessagesSetCB (
    EIT_PRINTMESSAGESCALLBACK f)
{
    EI_PrintMessages = f;
}
/*--------------------------------------------------------
Prints the message list.
----------------------------------------------------------*/
static void DefaultPrintMessages (
	void)
{
    int i;
    EIT_MESSAGE * Message;

    for (i = 0; i < EI_GetNumberOfMessages (); i++) {
        Message = EI_GetMessage (i);

        switch (Message->Severity) {
        case EIE_INFORMATION:
            printf ("INFORMATION\n%s\n", Message->MessageText);
            break;
        case EIE_WARNING:
            printf ("WARNING:\n%s returned: %s\n",
                Message->CallingProgram, Message->MessageText);
            break;
        case EIE_MESSAGESEVERITY_EMPTY:
            printf ("%s", Message->MessageText);
            break;
        case EIE_ERROR:
        default:
            printf ("ERROR:\n%s returned: %s\n",
                Message->CallingProgram, Message->MessageText);
            break;
        }
    }

    /* since the messages were printed there is no need to keep them */
    EI_ClearMessageList ();
}
/*----------------------------------------------------------------------- */
/* This function reallocates amount of memory                             */
/* to the Message List (ie 'amt' more messages).                          */
/* Called by EI_AddMessage()                                              */
/*----------------------------------------------------------------------- */
static EIT_REALLOCATEMESSAGELIST_RETURNCODE Reallocate (
    int amt)
{
    EIT_MESSAGE *TempMessage;

    TempMessage = STC_ReallocateMemory (
        mMessageList.NumberOfAllocatedMessages * sizeof *TempMessage,
        amt * sizeof *TempMessage,
        mMessageList.Message);
    if (TempMessage == NULL) {
        return EIE_REALLOCATEMESSAGELIST_FAIL;
    }
    mMessageList.Message = TempMessage;
    mMessageList.NumberOfAllocatedMessages = amt;
    return EIE_REALLOCATEMESSAGELIST_SUCCEED;
}
/*------------------------------------------------------------------------*/
/* Removes all but the first Message from the list                        */
/*------------------------------------------------------------------------*/
static void RemoveAllMessages (
	void)
{
    int i;

    for (i = 0; i < mMessageList.NumberOfMessages; i++) {
        STC_FreeMemory (mMessageList.Message[i].CallingProgram);
        STC_FreeMemory (mMessageList.Message[i].MessageText);
    }
    mMessageList.NumberOfMessages = 0;
}
/*------------------------------------------------------------------------*/
/* Sets mMessageList.Message to NULL                                      */
/*------------------------------------------------------------------------*/
void EI_mMessageList_Message_to_NULL (
	void)
{
    mMessageList.Message = NULL;
}
/*------------------------------------------------------------------------*/
/* Returns 1 if mMessageList.Message equals NULL and 0 if not             */
/*------------------------------------------------------------------------*/
int EI_mMessageList_Message_is_NULL (
	void)
{
    return (mMessageList.Message == NULL);
}

/* char dbgscratch[DBGBUFSIZE] = ""; */
/* int dbgscratchused = 0;       */

/* void DBG_ClearDbgscratch()    */
/* {                             */
/*     sprintf(dbgscratch, "");  */
/* }                             */

/* int DBG_dumptofile(char * filename)                         */
/* {                                                           */
/*     FILE * oufile;                                          */
/*     int rc;                                                 */
/*                                                             */
/*     oufile = fopen(filename, "ab");                         */
/*     if (oufile == NULL)                                     */
/*     {                                                       */
/*         SAS_XPSLOG("unable to open file \"%s\"", filename); */
/*         return -1;                                          */
/*     }                                                       */
/*     rc = fprintf(oufile, "%s", dbgscratch);                 */
/*                                                             */
/*     fclose(oufile);                                         */
/*                                                             */
/*     return(rc);                                             */
/* }                                                           */

char dbgscrch          [DBGBUFSIZE] = "";
char dbgscrch2         [DBGBUFSIZE] = "";
/* int dbscrusd                        = 0 ; */
/* int dbscrusd2                       = 0 ; */
char dbgoufnm          [DBGFNMSIZE] = "";
char dbgoufnm_cellset  [DBGFNMSIZE] = "";
char dbgoufnm_cells_tdx[DBGFNMSIZE] = "";
char dbgoufnm_ditms_tdx[DBGFNMSIZE] = "";
char dbgoufnm_htreeroot[DBGFNMSIZE] = "";
char dbgoufnm_parms    [DBGFNMSIZE] = "";
char dbgoufnm_srule    [DBGFNMSIZE] = "";
char dbgoufnm_rangeroot[DBGFNMSIZE] = "";
char dbgoufnm_kdtree   [DBGFNMSIZE] = "";

void DBG_cdbs(char * dbgscratch)
{
    sprintf(dbgscratch, "");
    /* dbscrusd = 0; */
}
#ifdef DBG_ON
int DBG_dtf(char * dbgscrfn, char * dbgscratch)
{
    FILE * oufile;
    int rc = 0;
    
    oufile = fopen(dbgscrfn, "ab");  // nosemgrep  // ignore this non-security code
    if (oufile == NULL)
    {
        EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, "unable to open file \"%s\"", dbgscrfn);
        return -1;
    }
    fprintf(oufile, "%s", (char*)dbgscratch);
    
    fclose(oufile);
    
    return(rc);
}
/*-dump the contents of the buffer "dbgscratch" to the file whose path+name   */
/* is the value of the string "dbgscrfn", then clear the buffer "dbgscratch", */
/* and finally use the format string "fmt" to format the remaining arguments  */
/* and write the resulting string to the same file:                           */

int DBG_printf(char * dbgscrfn, char * dbgscratch, char * fmt, ...) {
    va_list args;
    FILE * oufile;
    int rc;
    
    rc = DBG_dtf(dbgscrfn, dbgscratch);
    if (rc != 0)
    {
        EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, "error in call \"DBG_dtf(dbgscrfn, dbgscratch)\"");
        return -1;
    }
    DBG_cdbs(dbgscratch);
    
    oufile = fopen(dbgscrfn, "ab");  // nosemgrep  // ignore this non-security code
    if (oufile == NULL)
    {
        EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, "unable to open file \"%s\"", dbgscrfn);
        return -1;
    }
    
    va_start(args, fmt);
    vfprintf(oufile, fmt, args);
    va_end(args);
    
    fclose(oufile);
    
    return 0;
}

/*-use the format string "fmt" to format the arguments beyond the first 3     */
/* and write the resulting string to the file whose path+name is the value    */
/* of the argument "output_file_path_and_name" after opening that file using  */
/* the mode specified by the 2nd argument "filemode" (the value of the        */
/* argument "filemode" will normally be "ab" (append and binary))             */
/* (this is similar to "DBG_printf()" defined above, but doesn't use the      */
/* buffer "dbgscrch" or the default output file path+name "dbgoufnm", so      */
/* it can be used even if the preprocessor variable "DBG_ON" hasn't been      */
/* defined--it's intended only for writing short messages to aid in debugging */
/* when writing debugging messages to the sas log using "SAS_XPSLOG()" isn't  */
/* enough, but "DBG_ON" hasn't been defined)                                  */
/*-example call:                                                              */
/*     if (0 != DBG_fprintf("c:\\users\\user\\debug_log.txt",                 */
/*                          "ab",                                             */
/*                          "the value of an_integer is %d\n",                */
/*                          an_integer                                        */
/*                         )                                                  */
/*        ) {                                                                 */
/*         return -1                                                          */
/*     }                                                                      */
int DBG_fprintf(char * output_file_path_and_name, char * filemode, char * fmt, ...) {
    va_list args;
    FILE * oufile;
    /* int rc; */
    
    oufile = fopen(output_file_path_and_name, filemode);  // nosemgrep  // ignore this non-security code
    if (oufile == NULL)
    {
        EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, "unable to open file \"%s\"", output_file_path_and_name);
        return -1;
    }
    
    va_start(args, fmt);
    vfprintf(oufile, fmt, args);
    va_end(args);
    
    fclose(oufile);
    
    return 0;
}
#endif
int    error_occurred_in_expression_macro = 0;

/* double report_error_in_conditional_expression_returning_double(char * error_message) */
/* {                                                                                    */
/*     SAS_XPSLOG(error_message);                                                       */
/*     return 0.0;                                                                      */
/* }                                                                                    */
double report_error_in_conditional_expression_returning_double(char * error_message, ...)
{
    va_list args;
    
    va_start(args, error_message);
    EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, error_message, args);
    va_end(args);
    
    error_occurred_in_expression_macro = 1;
    
    return 0.0;
}

/* int    report_error_in_conditional_expression_returning_int   (char * error_message) */
/* {                                                                                    */
/*     SAS_XPSLOG(error_message);                                                       */
/*     return 0;                                                                        */
/* }                                                                                    */
int    report_error_in_conditional_expression_returning_int   (char * error_message, ...)
{
    va_list args;
    
    va_start(args, error_message);
    EI_AddMessage("", EIE_MESSAGESEVERITY_EMPTY, error_message, args);
    va_end(args);
    
    error_occurred_in_expression_macro = 1;
    
    return 0;
}

