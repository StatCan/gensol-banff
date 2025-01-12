#ifndef MESSAGECOMMONAPI_EN_H
#define MESSAGECOMMONAPI_EN_H

/**************************************************************/
/* M10000 series - Information messages                       */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
/* STC_Logging: messages related to logging */
#define M10001 "Custom Server File not found. Logging will proceed with specified/default server."
#define M10002 "Skipping Logging. Custom Server File has host=none"

/**************************************************************/
/* M20000 serie - Warning messages                            */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'WARNING:' added at the beginning      */
/**************************************************************/
#define M20001 "Removing variable '%s' because its coefficient (or the sum of its coefficients) is smaller than the smallest acceptable value %g. %g is a platform dependant value."
#define M20002 "Changing constant to 0.0 because it is smaller than the smallest acceptable value %g. %g is a platform dependant value."
#define M20003 "Removing variable '%s' because its coefficient is 0.0."
#define M20004 "Combining two constants."
#define M20005 "Combining all coefficients of variable '%s' because it was specified more than once."
#define M20006 "Removing variable '%s' because the sum of its coefficients is 0.0."
/* STC_Logging: messages related to logging */
#define M20007 "Skipping Logging. Custom Server File Invalid."
#define M20008 "Skipping Logging. Custom Server File exists but cannot be opened."

/**************************************************************/
/* M30000 serie - Error messages                              */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERROR:' added at the beginning        */
/**************************************************************/
#define M30001 "Failed to converge."
#define M30002 "Cannot inverse a singular matrix in routine ludcmp()."
#define M30003 "Too many errors. Stop processing."
#define M30004 "No variable in edits."
#define M30005 "PASS edit cannot be !=."
#define M30006 "FAIL edit cannot be =."
#define M30007 "Looking for '%s' but found '%s' instead."
#define M30008 "Looking for 'Coefficient' or 'Variable' but found '%s' instead."
#define M30009 "Variable name too large!"
#define M30010 "%s failed."
/* STC_Logging: messages related to logging */
//#define M30011 "Logging access was not successful.  The procedure will continue."
//#define M30012 "Logging access was not successful.  The procedure will continue."
#define M30013 "Logging access was not successful.  The procedure will continue. (curl_no_init)"
#define M30014 "Logging access was not successful.  The procedure will continue.\n(curl_rc:%d, http_rc:%d, \n URL:%s)"
#define M30015 "Logging access was not successful.  The procedure will continue. "

/**************************************************************/
/* M40000 serie - Statistics (report)                         */
/**************************************************************/

/**************************************************************/
/* M00000 serie - Miscellaneous                               */
/**************************************************************/
#define M00001 "Edits parser"
#define M00002 "Error at or before"
#define M00003 "Coefficient"
#define M00004 "Colon"
#define M00005 "Done"
#define M00006 "Error"
#define M00007 "Minus"
#define M00008 "Modifier"
#define M00009 "Operator"
#define M00010 "Plus"
#define M00011 "SemiColon"
#define M00012 "Asterik"
#define M00013 "Variable"
#define M00014 "Unknown character"
#define M00015 "EQUALITY EDIT: '%d' causes determinacy."
#define M00016 "EQUALITY EDIT: '%d' is a linear combination of other edits. It is redundant."
#define M00017 "EQUALITY EDIT: '%d' makes the set of edits inconsistent."
#define M00018 "HIDDEN EQUALITY EDIT: '%d' causes determinacy."
#define M00019 "HIDDEN EQUALITY EDIT: '%d' makes the set of edits inconsistent."
#define M00020 "INEQUALITY EDIT: '%d' is a hidden equality."
#define M00021 "INEQUALITY EDIT: '%d' is a redundant hidden equality."
#define M00022 "INEQUALITY EDIT: '%d' is tight but does not restrict the feasible region."
#define M00023 "INEQUALITY EDIT: '%d' lies entirely outside the feasible region. It is redundant."
#define M00024 "At or before"








#endif
