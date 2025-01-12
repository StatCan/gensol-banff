#include <stdio.h>
#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <string.h>

#include "EI_Message.h"
#include "STC_Memory.h"

static void * DefaultAllocateMemory (size_t);
static void DefaultFreeMemory (void *);
static void * DefaultReallocateMemory (size_t, size_t, void *);

CLASS_DECLSPEC EIT_ALLOCATEMEMORYCALLBACK STC_AllocateMemory = DefaultAllocateMemory;
CLASS_DECLSPEC EIT_FREEMEMORYCALLBACK STC_FreeMemory = DefaultFreeMemory;
CLASS_DECLSPEC EIT_REALLOCATEMEMORYCALLBACK STC_ReallocateMemory = DefaultReallocateMemory;

/*********************************************************************
function to set the callback function to allocate memory
*********************************************************************/
void STC_AllocateMemorySetCB (
    EIT_ALLOCATEMEMORYCALLBACK f)
{
    STC_AllocateMemory = f;
}
/*********************************************************************
function to set the callback function to free memory
*********************************************************************/
void STC_FreeMemorySetCB (
    EIT_FREEMEMORYCALLBACK f)
{
    STC_FreeMemory = f;
}
/*********************************************************************
function to set the callback function to reallocate memory
*********************************************************************/
void STC_ReallocationMemorySetCB (
    EIT_REALLOCATEMEMORYCALLBACK f)
{
    STC_ReallocateMemory = f;
}
/*********************************************************************
returns a pointer to a new string that is a duplicate of the string pointed to
by String. The returned pointer can be passed to STC_FreeMemory(). The space
for the new string is obtained using STC_AllocateMemory(). If the new string
cannot be created, a null pointer is returned.
*********************************************************************/
char * STC_StrDup (
    char * String)
{
    char * Ptr;
    Ptr = STC_AllocateMemory (strlen (String) + 1);
    if (Ptr != NULL)
        strcpy (Ptr, String);
    return Ptr;
}


/*********************************************************************
The default function to allocate memory.
could be NULL, it's the caller's responsability to check
*********************************************************************/
static void * DefaultAllocateMemory (
    size_t Quantity)
{
#ifdef NEVERTOBEDEFINED
    return malloc (Quantity);
#endif
	void * Ptr;
	Ptr = malloc (Quantity);

	//if ((UTIL_Random(1, 1000000)) == 1) Ptr = NULL;

	if ( Ptr == NULL && Quantity != 0) {
		EI_AddOutOfMemoryMessage ();
	}
    return Ptr;
}
/*********************************************************************
The default function to free memory.
*********************************************************************/
static void DefaultFreeMemory (
    void * Ptr)
{
    free (Ptr);
}
/*********************************************************************
The default function to reallocate memory.
could be NULL, it's the caller's responsability to check
*********************************************************************/
static void * DefaultReallocateMemory (
    size_t OriginalQuantity,
    size_t NewQuantity,
    void * Source)
{
#ifdef NEVERTOBEDEFINED
    return realloc (Source, NewQuantity);
#endif
	void * Ptr;
	Ptr = realloc (Source, NewQuantity);

	//if ((UTIL_Random(1, 1000000)) == 1) Ptr = NULL;

	if (Ptr == NULL && NewQuantity != 0) {
		EI_AddOutOfMemoryMessage ();
	}
    return Ptr;
}
