/* IO_Jansson
    This file defines functions related to JSON input/output.

    **JSON Library Mapping**
    This file is coded to partially decouple the JSON related operations from the
    actual library implementing JSON support.  This is done using 
    "JSON Library Mapping" (`JLM_*`), which maps the specific libraries 
    functions and types to a set of in-house #defines and functions.  
    This allows us to swap out the JSON library while require minimal changes to this code

    **Unsafe Operations**
    Many of the getter functions (`JOP_get_*()`) will return a default value if the JSON data
    is of the incorrect type.  The caller is responsible for determining the validity of the JSON
    data returned by these functions, such as by using `PARM_is_specified()`.  
    */

#include "IO_Jansson.h"

#include "IO_Messages.h"

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* ~~~~~ JSON OPERATIONS FUNCTIONS ~~~~~ */
/* JOP_encode_json
    This function wraps the library specific JSON encoding function */
char* JOP_encode_json(_JS_T_ js_obj) {
    /* for JANSSON library, apply flags */
#if defined(USING_JANSSON)
    return JLM_encode_json(js_obj, JSON_COMPACT);
#else
#error macro USING_<library name> must be defined!!
#endif

}
/* JOP_free_json
    This function wraps the library specific JSON freeing function
    for library Jansson, function `json_decref(_JS_T_)` is used */
void JOP_free_json(_JS_T_ js_obj) {
    if (js_obj == NULL)
        return;
    JLM_free(js_obj);
}

/* JOP_get_bool
    Get BOOLEAN from JSON data
    - if type BOOL -> IOB_TRUE or IOB_FALSE
    - if type OTHER -> IOB_FALSE 
    DIFFERENT APPROACHES
        Some JSON libraries have only one type, BOOL
        Others have TRUE and FALSE types
        This function handles both: When abstracted 
        JLT TRUE and FALSE types are equal, it means 
        the library only has 1 bool type => extract value
        Otherwise, return value indicated by type itself*/
int JOP_get_bool(_JS_T_ js_obj) {
    // library with one BOOL type 
    if (JLT_TRUE == JLT_FALSE) {
        if (JLM_typeof(js_obj) == JLT_TRUE) {
            if (JLM_bool_get(js_obj))
                return IOB_TRUE;
            else
                return IOB_FALSE;
        } else {
            return IOB_FALSE;
        }
    }
    // library with TRUE and FALSE bool types
    else {
        if (JLM_typeof(js_obj) == JLT_TRUE) {
            return IOB_TRUE;
        } else if (JLM_typeof(js_obj) == JLT_FALSE) {
            return IOB_FALSE;
        } else {
            return IOB_FALSE;
        }
    }
}

/* JOP_get_double
    Get DOUBLE from JSON data
    - if type REAL -> return (double) casted value
    - if type INT -> return (double) casted value
    - if type OTHER -> return 0.0 */
double JOP_get_double(_JS_T_ js_obj) {
    if (JLM_typeof(js_obj) == JLT_REAL) {
        return (double)JLM_real_get(js_obj);
    } else if (JLM_typeof(js_obj) == JLT_INTEGER) {
        return (double)JLM_integer_get(js_obj);
    } else {
        return 0.0;
    }
}

/* JOP_get_float
    Get FLOAT from JSON data
    - if type REAL -> return (float) casted value
    - if type INT -> return (float) casted value
    - if type OTHER -> return 0.0 */
float JOP_get_float(_JS_T_ js_obj) {
    if (JLM_typeof(js_obj) == JLT_REAL) {
        return (float)JLM_real_get(js_obj);
    } else if (JLM_typeof(js_obj) == JLT_INTEGER) {
        return (float)JLM_integer_get(js_obj);
    } else {
        return 0.0;
    }
}

/* JOP_get_int
    Get INT from JSON data
    - if type INT -> return value
    - if type REAL -> return (int) casted value
    - if type OTHER -> return 0*/
int JOP_get_int(_JS_T_ js_obj) {
    if (JLM_typeof(js_obj) == JLT_REAL) {
        return (int)JLM_real_get(js_obj);
    } else if (JLM_typeof(js_obj) == JLT_INTEGER) {
        return (int)JLM_integer_get(js_obj);
    } else {
        return 0;
    }
}

/* JOP_get_longlong
    Get LONG LONG from JSON data
    - if type INT -> return (long long) value
    - if type REAL -> return (long long) casted value
    - if type OTHER -> return 0 */
long long JOP_get_longlong(_JS_T_ js_obj) {
    if (JLM_typeof(js_obj) == JLT_REAL) {
        return (long long)JLM_real_get(js_obj);
    } else if (JLM_typeof(js_obj) == JLT_INTEGER) {
        return (long long)JLM_integer_get(js_obj);
    } else {
        return 0;
    }
}

/* JOP_get_string
    Get STRING parameter from JSON data
    RETURNS
        pointer to string contents of js_obj
     or NULL if parsing doesn't result in a string */
const char* JOP_get_string(_JS_T_ js_obj) {
    return JLM_string_get(js_obj);
}

/* JOP_load_json
    Parse text into a JSON object. If text is valid JSON, returns a
    json_t structure, otherwise prints and error and returns null.
 */
_JS_T_ JOP_load_json(const char* text) {
    _JS_T_ root;
#ifdef USING_JANSSON
    _JS_ERROR_T_ error;
#endif

    root = JLM_decode_json_str(text, 0, & error);

    if (root) {
        return root;
    } else {
        printf(MSG_PREFIX_ERROR JUM_JSON_DECODE_ERROR, error.line, error.position, error.text);
        return NULL;
    }
}

/* JOP_load_json_string
    load JSON from string
    return JSON object
*/
_JS_T_ JOP_load_json_string(const char* json_str) {
    _JS_T_ json_root = JOP_load_json(json_str);
    if (json_root == (_JS_T_) 0) {
        return NULL;
    }
    else {
        return json_root;
    }
}

/* JOP_print_invalid_parm 
    Print a JSON object's type and (if applicable) value to the log.
*/
void JOP_print_invalid_parm(_JS_T_ js_obj) {
    // pre-pad with spaces to align message with "ERROR: " on previous line
    printf("%*s", JUM_ERROR_PREFIX_LENGTH, ""); // * in %*s means field width specified in arguments 

    switch (JLM_typeof(js_obj)) {
    case JLT_OBJECT:
        printf(JUM_JSON_PRINT_OBJECT);
        break;
    case JLT_ARRAY:
        printf(JUM_JSON_PRINT_ARRAY, (long long)JLM_array_size(js_obj));
        break;
    case JLT_STRING:
        printf(JUM_JSON_PRINT_STRING, JLM_string_get(js_obj));
        break;
    case JLT_INTEGER:
        printf(JUM_JSON_PRINT_INTEGER, JLM_integer_get(js_obj));
        break;
    case JLT_REAL:
        printf(JUM_JSON_PRINT_REAL, JLM_real_get(js_obj));
        break;
    case JLT_TRUE:
        printf(JUM_JSON_PRINT_BOOL_TRUE);
        break;
    case JLT_FALSE:
        printf(JUM_JSON_PRINT_BOOL_FALSE);
        break;
    case JLT_NULL:
        printf(JUM_JSON_PRINT_NULL);
        break;
    default:
        printf(JUM_JSON_PRINT_UNKNOWN, JLM_typeof(js_obj));
    }
}