#ifndef MESSAGELPI_H
#define MESSAGELPI_H

/* Define either the macro _en or _fr in makefile */
#if defined (_en)
#include "MessageLPI_en.h"
#elif defined (_fr)
#include "MessageLPI_fr.h"
#else
#error "Define either the macro _en or _fr"
#endif

#endif
