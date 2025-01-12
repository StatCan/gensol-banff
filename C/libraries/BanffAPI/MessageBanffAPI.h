#ifndef MessageBanffAPI_H
#define MessageBanffAPI_H

/* Define either the macro _en or _fr in makefile */

#if defined (_en)
#include "MessageBanffAPI_en.h"
#include "IO_Messages.h"
#elif defined (_fr)
#include "MessageBanffAPI_fr.h"
#include "IO_Messages.h"
#else
#error "Define either the macro _en or _fr"
#endif

#endif
