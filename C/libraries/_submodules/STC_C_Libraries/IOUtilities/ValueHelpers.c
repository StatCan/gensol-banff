#include "ValueHelpers.h"
#include "IOUtil.h"

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "PlatformSupport.h"  // platform specific string functions (memccpy)

#include "STC_Memory.h"

#include <math.h>  // NAN value
#include <ctype.h>  // for calls to `tolower()`, `toupper()`

/* IOUtil_copy_varname
    Secure copy function for variable names (buffers of size `IO_COL_NAME_LEN + 1`).  

    TRUNCATION
        Any source string of excessive length will be truncated.  
        Caller must validate that user-specified values are a valid length.  */
void IOUtil_copy_varname(char* dest, const char* src) {
    strncpy_s(dest, IO_COL_NAME_LEN + 1, src, IO_COL_NAME_LEN);
}

/* IOUtil_is_missing
    Replacement for `SAS_ZMISS()` */
int IOUtil_is_missing(double value_to_test) {
    if (isnan(value_to_test)) {
        return 1 == 1; // TRUE
    }
    else {
        return 1 == 0; // FALSE
    }
}

/* IOUtil_missing_value
    Replacement for `SAS_ZMISSV()

    `missing_type` intentionally ignored
        SAS' "special" missing values are identified
        by characters like '_' or 'A'.
        In converted procedures, all are treated the same. */
float IOUtil_missing_value(char missing_type) {
    return NAN;
}

/* IOUtil_str_upcase
    Change a string to uppercase, in-place.
    Returns the original string pointer */
char* IOUtil_str_upcase(char* src) {
    char* s = src;
    while (s != NULL && *s) {
        *s = toupper((unsigned char)*s);
        s++;
    }
    return src;
}

/* IOUtil_strdup
    Optimized (secure?) string duplication function */
char* IOUtil_strdup(const char* src) {
    if (src == NULL) {
        return NULL;
    }
    size_t src_len = strlen(src); // doesn't include the '\0'

    char* dest = STC_AllocateMemory(sizeof(*dest) * (src_len + 1)); // +1 for '\0'

    return IOUtil_strcpy(dest, src);
}

/* IOUtil_strcpy_s
    In-house secure string copying with guarinteed null termination
    returns pointer to `dest`.  
    returns NULL if
        - src is NULL
        - or dest is NULL
        - or cannot fit `src` (including null terminator) in `dest` */
char* IOUtil_strcpy_s(char* dest, size_t dest_len, char* src) {
    if (src == NULL) {
        return NULL;
    }

    if (dest == NULL) {
        return NULL;
    }

    size_t src_len = strlen(src);

    if (src_len >= dest_len) {  // i.e. if it cannot fit with null terminator
        return NULL;
    }

    memccpy(dest, src, '\0', src_len);
    dest[src_len] = '\0';

    return dest;
}

/* IOUtil_strcpy
    Optimized (secure?) string copying function
    `dest` MUST have a length `>= strlen(src) + 1`
   OPTIMIZED: use memccpy(...) for copying strings
    supposedly when `strncpy()` finds the first '\0' in its source, it
    fills the remaining `n` characters with '\0`, where as `memccpy(,'\n',)`
    will just stop after its copied the first '\0'*/
char* IOUtil_strcpy(char* dest, const char* src) {
    if (src == NULL) {
        return NULL;
    }

    if (dest == NULL) {
        return NULL;
    }

    size_t src_len = strlen(src);

    memccpy(dest, src, '\0', src_len);
    dest[src_len] = '\0';

    return dest;
}

/* case insensitive string comparison
    Strings of unequal length are unequal
    - "true" if strings match
    - "false" otherwise*/
int IOUtil_stricmp_is_equal(const char* a, const char* b) {
    int len_a = (int)strlen(a);
    int len_b = (int)strlen(b);
    if (len_a != len_b) {
        return 1 == 0; // "false"
    }
    for (int i = 0; i < len_a && i < len_b; i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return 1 == 0; // "false"
        }
    }
    return 1 == 1; // "true"
}