#ifndef _PLATFORM_SUPPORT_H
#define _PLATFORM_SUPPORT_H
/* This file handles differences between Windows and non-Windows
    standard C functions.  */

#include <string.h>  // memccpy and _memccpy

#define UTF8_ENCODING_ID 65001  // Windows specific code page ID

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
/* Manage Microsoft non-standard names */
#define memccpy     _memccpy
#define strtok_r    strtok_s // Microsoft has their own name for this standard function...
#else
/* Map missing Linux functions */
#define strcat_s(a, b, c) strncat(a, c, b - 1)
#define strncpy_s(a, b, c, d) IOUtil_strcpy_s(a, b, c)
/* map Microsoft non-standard types */
typedef unsigned int UINT;
#endif

UINT set_console_output_encoding(UINT encoding_id);

#endif