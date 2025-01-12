#ifndef IO_JANSSON_H
#define IO_JANSSON_H

#include "JLM_jansson.h"  // Jansson library and macro abstractions

/* in-house typedef for Jansson input JSON datatype */
typedef char* jansson_in_type;

/* JSON Operations
    These functions perform generic JSON operations such as
        - extracting a particular typed value from a JSON object
        - freeing JSON object
        - loading JSON string */
char*           JOP_encode_json(        _JS_T_ js_obj);
void            JOP_free_json(          _JS_T_ js_obj);

int             JOP_get_bool(           _JS_T_ js_obj);
double          JOP_get_double(         _JS_T_ js_obj);
float           JOP_get_float(          _JS_T_ js_obj);
int             JOP_get_int(            _JS_T_ js_obj);
long long       JOP_get_longlong(       _JS_T_ js_obj);
const char*     JOP_get_string(         _JS_T_ js_obj);

_JS_T_          JOP_load_json(          const char* text);
_JS_T_          JOP_load_json_string(   const char* json_str);

void            JOP_print_invalid_parm( _JS_T_ js_obj);

#endif