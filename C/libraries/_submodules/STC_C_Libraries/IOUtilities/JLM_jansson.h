#ifndef JLM_JANSSON_H
#define JLM_JANSSON_H

// indicate library in use
#define USING_JANSSON

// library header files
#include "jansson.h"

// other headers
#include <stdbool.h>

/* Json Library mapping macros 
    These macros are used to abstract JSON library functions.  
    Create a different mapping for a different JSON library */

/* JSON types */
#define _JS_T_          json_t*
#define _JS_ERROR_T_    json_error_t
#define JLT_OBJECT      JSON_OBJECT
#define JLT_ARRAY       JSON_ARRAY
#define JLT_STRING      JSON_STRING
#define JLT_INTEGER     JSON_INTEGER
#define JLT_REAL        JSON_REAL
#define JLT_TRUE        JSON_TRUE
#define JLT_FALSE       JSON_FALSE
#define JLT_NULL        JSON_NULL

/* METADATA functions */
#define JLM_array_size(json_ptr)            json_array_size(json_ptr)
#define JLM_is_null(json_ptr)               json_is_null(json_ptr)
#define JLM_typeof(json_ptr)                json_typeof(json_ptr)

/* GETTER functions */
#define JLM_array_get(json_ptr, index)      json_array_get(json_ptr, index)
#define JLM_bool_get(json_ptr)              JANSSON_bool_get(json_ptr)
#define JLM_integer_get(json_ptr)           json_integer_value(json_ptr)
#define JLM_object_get(json_ptr, key)       json_object_get(json_ptr, key)
#define JLM_real_get(json_ptr)              json_real_value(json_ptr)
#define JLM_string_get(json_ptr)            json_string_value(json_ptr)

/* SETTER/CREATION functions */
#define JLM_new_array()                     json_array()
#define JLM_new_integer(value)              json_integer(value)
#define JLM_new_object()                    json_object()
#define JLM_new_real(value)                 json_real(value)
#define JLM_new_string(value)               json_string(value)
#define JLM_new_null()                      json_null()

/* OPERATIONS functions */
#define JLM_append_key_value(json_ptr, key, value)              json_object_set_new(json_ptr, key, value)
#define JLM_array_append_obj(json_ptr_array, json_ptr_obj)      json_array_append_new(json_ptr_array, json_ptr_obj)
#define JLM_decode_json_str(json_str, flags, error_ptr)         json_loads(json_str, flags, error_ptr)
#define JLM_encode_json(json_ptr, flags)                        json_dumps(json_ptr, flags)
#define JLM_free(json_ptr)                                      json_decref(json_ptr)
/* JLM_copy_shallow: in Jansson, "reference count" must be increased so the copied object isn't freed until all referenced released 
    for details: https://jansson.readthedocs.io/en/latest/apiref.html#reference-count */
#define JLM_copy_shallow(json_ptr)                              json_incref(json_ptr)

/* Custom functions */
bool JANSSON_bool_get(_JS_T_ json_ptr);


#endif