#include "JLM_jansson.h"


bool JANSSON_bool_get(_JS_T_ json_ptr) {
    if (JLM_typeof(json_ptr) == JLT_TRUE)
        return true;
    else
        return false;
}