#ifndef VALUE_HELPERS_H
#define VALUE_HELPERS_H 
#include <string.h>

/* IOUtil functions
    Useful functions... used in various places in new code...*/
void        IOUtil_copy_varname(char* dest, const char* src);
int         IOUtil_is_missing(          double value_to_test);
float       IOUtil_missing_value(       char missing_type);

char*       IOUtil_str_upcase(          char* src);
char*       IOUtil_strdup(              const char* src);
char*       IOUtil_strcpy_s(            char* dest, size_t dest_len, char* src);
char*       IOUtil_strcpy(              char* dest, const char* src);
int         IOUtil_stricmp_is_equal(    const char* a, const char* b);
#endif