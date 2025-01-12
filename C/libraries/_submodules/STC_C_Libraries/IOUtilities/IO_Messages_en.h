#ifndef IO_MESSAGES_EN_H
#define IO_MESSAGES_EN_H

#define IO_TEST_MESSAGE "TEST MESSAGE for Native Language Support: Forest, naive, French, Creme Brulee"

#define JUM__NUMERIC "numeric"
#define JUM__CHARACTER "character"
#define JUM_MISSING "<missing>"
// translate these prefixes? Double check for "lph" length place holders in MessageBanff.h if doing that.  
#define MSG_PREFIX_ERROR "ERROR: "
#define MSG_PREFIX_WARN "WARNING: "
#define MSG_PREFIX_NOTE "NOTE: "
#define MSG_INDENT_ERROR "       "
#define MSG_INDENT_WARN "         "
#define MSG_INDENT_NOTE "      "

/* taken from MessageBanff_*.h */
#define MsgDataSetMandatory "%s data set is mandatory."

/* START: copied from MessageSAS_en.h */
#define MsgNumericVarNotInDataSet "Numeric variable %s is not in %s data set."
#define MsgCharacterVarNotInDataSet "Character variable %s is not in %s data set."

#define MsgVarNotNumericInDataSet "Variable %s is not numeric in %s data set."
#define MsgVarNotCharacterInDataSet "Variable %s is not character in %s data set."

#define MsgStatementNotSpecified "%s not specified."
#define MsgStatementSpecifiedWithoutVariableList "%s specified without a variable list."
#define M10001 "--- %s System %s developed by Statistics Canada ---"
#define M10002 "PROCEDURE %s Version %s"
#define M10003 "Created on %s at %s"
#define M10004 "Email: %s"
#define M10005 "%s" /* specific for printing header message */
#define MsgByVariableNotSameType "Variable %*s has been defined as both character and numeric."
#define MsgByVariableNotInDataSet "BY variable %*s is not on input data set %*s."
/* END: copied from MessageSAS_en.h */

// `JUM_PARM_`: parameter type validation
#define JUM_PARM_INVALID "parameter '%s' incorrect type, expecting %s, but received\n"
#define JUM_PARM_INVALID_INT "an INTEGER constant"
#define JUM_PARM_INVALID_REAL "a REAL numeric constant"
#define JUM_PARM_INVALID_QS "a quoted string"
#define JUM_PARM_INVALID_NAME "a short quoted string"
#define JUM_PARM_INVALID_FLAG "a boolean value"
#define JUM_PARM_INVALID_NAME_MSG "parameter %s length (%zu) exceeds limit of %d: %s\n"
#define JUM_PARM_TYPE_UNKNOWN "Received unknown parameter type (%d)\n"

/* Varlist Names: used in some error messages
    used during varlist initialization (`VL_init*()`)*/
    // DO NOT TRANSLATE the following identifiers: `instatus`, `inestimator`, `inalgorithm`, `unit_id`, or `by`
#define VL_NAME_in_var          "input variables"
#define VL_NAME_instatus        "instatus variables"
#define VL_NAME_instatus_hist   "instatus_hist variables"
#define VL_NAME_inestimator     "inestimator variables"
#define VL_NAME_inalgorithm     "inalgorithm variables"
#define VL_NAME_unit_id         "unit_id"
#define VL_NAME_by_var          "by"

/* related to datasets and their values */
#define JUM_SINGLE_VARLIST_COUNT_INVALID "List '%s' accepts at most 1 %s variable.\n"

#define JUM_VARLIST_WRONG_TYPE "Variable '%s' in list '%s' (%s dataset) must be %s, but it is %s.\n"

#define JUM_VARLIST_MEMBER_MISSING "Variable '%s' in list '%s' (%s dataset) not found.\n"

#define JUM_TYPE_MISMATCH_CHAR "invalid data detected while reading character variable '%s' on row %d\n"
#define JUM_TYPE_MISMATCH_NUM "invalid data detected while reading numeric variable '%s' on row %d\n"

#define JUM_DATA_NOT_SORTED_P1 "Data set %s is not sorted in ascending sequence. "
#define JUM_DATA_NOT_SORTED_P2 "The current by group has "
#define JUM_DATA_NOT_SORTED_P3 " and the next by group has "

#define JUM_PRINT_INPUT_DATASET_METADATA "%s: %d column(s), %d row(s)\n"
#define JUM_DATASET_NOT_SPECIFIED "%s: not specified\n"
#define JUM_DATASET_ENABLED "%s: enabled\n"
#define JUM_DATASET_DISABLED "%s: disabled\n"

/* JSON related */
#define JUM_ERROR_PREFIX_LENGTH 7   // length of string 'ERROR: '
#define JUM_JSON_PRINT_OBJECT "JSON Object\n"
#define JUM_JSON_PRINT_ARRAY "JSON Array of %lld element(s):\n"
#define JUM_JSON_PRINT_STRING "string: \"%s\"\n"
#define JUM_JSON_PRINT_INTEGER "integer: \"%" JSON_INTEGER_FORMAT "\"\n"
#define JUM_JSON_PRINT_REAL "real: %f\n"
#define JUM_JSON_PRINT_BOOL_TRUE "boolean: true\n"
#define JUM_JSON_PRINT_BOOL_FALSE "boolean: false\n"
#define JUM_JSON_PRINT_NULL "NULL\n"
#define JUM_JSON_PRINT_UNKNOWN "unrecognized JSON type %d\n"

#define JUM_JSON_PARSE_ERROR_INDATA "unable to parse input JSON for INDATA dataset\n"
#define JUM_JSON_DECODE_ERROR "json error on line %d, position %d :% s\n"

/* Apache Arrow related */
#define MSG_ARROW_ERROR_MESSAGE "unexpected nanoarrow error"
#define MSG_ARROW_OOB_REF_COL "attempt to reference out of bounds column index"
#define MSG_ARROW_OOB_REF_ROW "attempt to reference out of bounds row index"
#define MSG_ARROW_INVALID_REF_DS "invalid dataset reference"
#define MSG_ARROW_FAIL_DS_INIT "failed to initialize output dataset"
#define MSG_ARROW_FAIL_DS_WRITE "failed to write value to output dataset"

/* IOUtil related */
#define MSG_IO_OUT_COL_TYPE_INVALID "attempt to set invalid output column type"
#define MSG_IO_VAR_NAME_TOO_LONG "variable name exceeds max length (%d) in varlist '%s' (length %zd, name '%s')\n"

#endif