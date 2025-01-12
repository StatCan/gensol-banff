#ifndef PUBLIC_HELPERS_H
#define PUBLIC_HELPERS_H
/* This file defines functions which are useful for the calling-application.  */

#include "ExportHelper.h"

EXPORTED_FUNCTION void flush_std_buffers();
EXPORTED_FUNCTION void free_memory(void* ptr);

#endif