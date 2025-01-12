#ifndef MESSAGEBANFF_H
#define MESSAGEBANFF_H

/* MsgPadding: in SAS, `SAS_XPSLOG()` expanded `""` to any number of spaces using "%.*s",
    in standard C, `printf()` does NOT always expand `""`, so it is hardcoded */
#define MsgPadding "                                                            "

/* Define either the macro _en or _fr in makefile */
#if defined (_en)
#include "MessageBanff_en.h"
#elif defined (_fr)
#include "MessageBanff_fr.h"
#else
#error "Define either the macro _en or _fr"
#endif

#define MsgParmEqualDouble "%s = %.*f" /* use sprintf (..., "%.*g",...) + MsgParmEqualString instead */
#define MsgParmEqualInteger "%s = %d"
#define MsgParmEqualLongString "%s = %s" /* %m: specific to SAS_XP routines */ // removed %m
#define MsgParmEqualString "%s = %s"

#endif
