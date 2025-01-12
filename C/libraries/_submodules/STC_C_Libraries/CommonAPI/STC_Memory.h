#ifndef STC_MEMORY_H
#define STC_MEMORY_H

#include <stdlib.h>

#include "EI_Common.h"

typedef void * (*EIT_ALLOCATEMEMORYCALLBACK) (
    size_t Quantity);
typedef void (*EIT_FREEMEMORYCALLBACK) (
    void * Ptr);
typedef void * (*EIT_REALLOCATEMEMORYCALLBACK) (
    size_t OriginalQuantity,
    size_t NewQuantity,
    void * Ptr);

#ifdef _CRTDBG_MAP_ALLOC
#define STC_AllocateMemory(q) malloc(q)
#define STC_FreeMemory(p) free(p)
#define STC_ReallocateMemory(oq,nq,p) realloc(p,nq)
#else
CLASS_DECLSPEC EIT_ALLOCATEMEMORYCALLBACK STC_AllocateMemory;
CLASS_DECLSPEC EIT_FREEMEMORYCALLBACK STC_FreeMemory;
CLASS_DECLSPEC EIT_REALLOCATEMEMORYCALLBACK STC_ReallocateMemory;
#endif



CLASS_DECLSPEC void STC_AllocateMemorySetCB (EIT_ALLOCATEMEMORYCALLBACK);
CLASS_DECLSPEC void STC_FreeMemorySetCB (EIT_FREEMEMORYCALLBACK);
CLASS_DECLSPEC void STC_ReallocationMemorySetCB (EIT_REALLOCATEMEMORYCALLBACK);

CLASS_DECLSPEC char * STC_StrDup (char * String);

#endif
