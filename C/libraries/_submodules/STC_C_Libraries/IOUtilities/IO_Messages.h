#ifndef IO_MESSAGES_H
#define IO_MESSAGES_H
/*  Many of the messages define in IO_Messages{_en,_fr}.h were created 
    during the redesign.  

    These messages generally relate to input and output.  

    The convention is to "factor out" prefixes (like "ERROR: "), 
    citing both the prefix and message at the location of usage.  */

#include "IOUtil.h"  // some message functions reference structs defined here

// include language-specific message file
#if defined (_en)
#include "IO_Messages_en.h"
#elif defined (_fr)
#include "IO_Messages_fr.h"
#else
#error "Define either the macro _en or _fr"
#endif

/* below are replacements for non-standard `SAS_XPSLOG()` format specifiers */
#ifndef SASUTIL_H
#define SAS_MESSAGE_PREFIX_ERROR        MSG_PREFIX_ERROR  //"%1z" 
#define SAS_MESSAGE_PREFIX_WARNING      MSG_PREFIX_WARN  //"%2z"
#define SAS_MESSAGE_PREFIX_NOTE         MSG_PREFIX_NOTE  //"%3z"
#endif //SASUTIL_H
#define SAS_NEWLINE "\n"  //"%n"

/* IO Message (IOM_) functions
    These functions are used to print complex messages introduced during the redesign */
void IOM_print_rec_by_group(DSR_generic* dsr, int index_ds, const char* pre_message, const char* kv_spaces, const char* separator, int num_newlines);
void IOM_print_data_not_sorted(DSR_generic* dsr, int index_ds);

#endif