#ifndef MESSAGECOMMONAPI_H
#define MESSAGECOMMONAPI_H

/* Define either the macro _en or _fr in makefile */

#if defined (_en)
#include "MessageCommonAPI_en.h"
#elif defined (_fr)
#include "MessageCommonAPI_fr.h"
#else
#error "Define either the macro _en or _fr"
#endif

#endif
