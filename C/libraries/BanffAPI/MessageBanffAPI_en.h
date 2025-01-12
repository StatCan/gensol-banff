#ifndef MessageBanffAPI_EN_H
#define MessageBanffAPI_EN_H

/**************************************************************/
/* M10000 series - Information messages                       */
/* - Severity is EIE_INFORMATION with EI_AddMessage()         */
/* - Other cases: word 'NOTE:' added at the beginning         */
/**************************************************************/
#define M10010 "Minimum time per record is %.2f. This parameter has been reset."
#define M10011 "Estimator %d will not be processed because there are no acceptable observations to compute the averages."
#define M10012 "Estimator %d will not be processed because at least one average has a zero denominator."
#define M10013 "Estimator %d will not be processed because there are no acceptable observations to compute the betas." 
#define M10014 "Estimator %d: Number of random error donors is %d."
#define M10015 "Estimator %d did not meet the COUNTCRITERIA:   target %-4d actual %-4d"
#define M10016 "Estimator %d did not meet the PERCENTCRITERIA: target %-4.1f actual %-4.1f"
#define M10017 "Estimator %d will not be processed."
#define M10018 "Number of records used in Outlier Detection : %ld."
#define M10019 "Value of the 25th percentile :"
#define M10020 "Value of the 50th percentile :"
#define M10021 "Value of the 75th percentile :"
#define M10022 "MDM specified will be ignored because the value of the median is 0."
#define M10023 "The median of the ratios : %.5f"
#define M10024 "MDM specified will be ignored because the value of the median of effects is 0."
#define M10025 "Acceptance interval  : [ %.5f , %.5f ]"
#define M10026 "Exclusion  intervals : [ %.5f , %.5f [ U ] %.5f , %.5f ]"
#define M10027 "Imputation intervals : ] -infinity , %.5f [ U ] %.5f , +infinity ["
#define M10028 "Exclusion  intervals : ] -infinity , %.5f [ U ] %.5f , +infinity ["
#define M10029 "The standard deviation is 0."

#define M10030 "Deviation"
#define M10031 "Exclusion Beta"
#define M10032 "Imputation Beta"
#define M10033 "Exclusion Sigma-gap"
#define M10034 "Imputation Sigma-gap"
#define M10035 "Left imputation bound"
#define M10036 "Left exclusion bound"
#define M10037 "Right exclusion bound"
#define M10038 "Right imputation bound"


/**************************************************************/
/* M20000 series - Warning messages                           */
/* - Severity is EIE_WARNING with EI_AddMessage()             */
/* - Other cases: word 'WARNING:' added at the beginning      */
/**************************************************************/
#define M20024 "Estimator %d: Number of random error donors is smaller than %d. Variance of the imputed values will not be reliable."
#define M20025 "Estimator %d: %s, division by zero."
#define M20026 "Number of records is less than MINOBS %d. Outlier not processed."
#define M20027 "Variable %s has been assigned a weight but is not in the edits."
/**************************************************************/
/* M30000 series - Error messages                             */       
/* - Severity is EIE_ERROR with EI_AddMessage()               */
/* - Other cases: word 'ERROR:' added at the beginning        */
/**************************************************************/
#define M30003 "Too many errors. Stop processing."
#define M30007 "Looking for '%s' but found '%s' instead."
#define M30009 "Variable name too large!"
#define M30010 "%s failed."
#define M30032 "Incompatible matrices (not the same number of observations): X and Y"
#define M30033 "Incompatible matrices (not the same number of observations): D and X, Y"
#define M30034 "Could not compute matrix Xt, the transpose of matrix X"
#define M30035 "Could not compute matrix U = X * Xt"
#define M30036 "Could not allocate matrix U1" 
#define M30037 "Could not allocate vector w"
#define M30038 "Could not allocate matrix V"
#define M30039 "Estimator %d will not be processed because no convergence could be achieved when computing betas."
#define M30040 "Failure at SVDCMP"
#define M30041 "Could not allocate XtY"
#define M30042 "Failure at SVBKSB."
#define M30043 "Estimator %d: algorithm name '%s' not defined in ALGORITHM data set or as a predefined algorithm."
#define M30044 "Estimator %d: VARIANCEPERIOD must be 'C', 'H' or missing."
#define M30045 "Estimator %d: EXCLUDEIMPUTED must be 'Y', 'N' or missing."
#define M30046 "Estimator %d: EXCLUDEOUTLIERS must be 'Y', 'N' or missing."
#define M30047 "Estimator %d: RANDOMERROR must be 'Y' or 'N'."
#define M30048 "Estimator %d: COUNTCRITERIA must be an integer greater than or equal to 1."
#define M30049 "Estimator %d: PERCENTCRITERIA must be a number greater than 0 and less than 100."
#define M30050 "Estimator %d: STATUS (%s) should be 1 to 3 characters."
#define M30051 "Estimator %d: TYPE must be '%s' or '%s'."
#define M30052 "Estimator %d: number of different auxiliary variable names specified is greater than number of formula placeholders."
#define M30053 "Estimator %d: number of different auxiliary variable names specified is less than number of formula placeholders."
#define M30054 "Estimator %d: %s cannot be specified for an EF estimator that does not calculate an average or does not have a random error to calculate." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30055 "Estimator %d: %s must be specified for an EF estimator that has a random error to calculate." /* %s: EXCLUDEIMPUTED or EXCLUDEOUTLIERS */
#define M30056 "Estimator %d: %s cannot be specified for an EF estimator that does not calculate an average." /* %s: COUNTCRITERIA, PERCENTCRITERIA or WEIGHTVARIABLE */
#define M30057 "Estimator %d: %s must be specified for an LR estimator or an EF estimator that calculates an average." /* %s: EXCLUDEIMPUTED, EXCLUDEOUTLIERS, COUNTCRITERIA or PERCENTCRITERIA */
#define M30058 "Estimator %d: %s must be specified for an LR estimator when VARIANCEVARIABLE is specified." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30059 "Estimator %d: %s cannot be specified for an LR estimator when VARIANCEVARIABLE is not specified." /* %s: VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30060 "Estimator %d: %s cannot be specified for an EF estimator." /* %s: VARIANCEVARIABLE, VARIANCEPERIOD or VARIANCEEXPONENT */
#define M30061 "Estimator %d: the current value of FIELDID %s is used as an auxiliary variable."
#define M30062 "Estimator %d: WEIGHTVARIABLE and FIELDID cannot be the same variable %s for an LR estimator."
#define M30063 "Estimator %d: WEIGHTVARIABLE and FIELDID cannot be the same variable %s for an EF estimator having at least one current average to compute."
#define M30064 "Estimator %d: VARIANCEVARIABLE and FIELDID cannot be the same variable %s when VARIANCEPERIOD is current."
#define M30065 "Estimator %d: WEIGHTVARIABLE and VARIANCEVARIABLE cannot be the same variable %s when VARIANCEPERIOD is current."
#define M30066 "Estimator %d: WEIGHTVARIABLE %s cannot be used as a current auxiliary variable for an LR estimator."
#define M30067 "Estimator %d: WEIGHTVARIABLE %s cannot be used as an auxiliary variable (including possibly FIELDID) for an EF estimator."
#define M30068 "Estimator %d: VARIANCEVARIABLE %s cannot be used as an auxiliary variable for the same period as VARIANCEPERIOD."
#define M30069 "Estimator %d: key %s has a 0.0 value for the %s %s."  /* ... 0.0 value for the 'VariablePeriod' 'VariableType'  */
#define M30070 "Estimator %d: key %s has a negative %s %s." /* ... a negative 'VariablePeriod 'VariableType' */
#define M30071 "Estimator %d: key %s has a missing %s %s."  /* ... a missing 'VariablePeriod 'VariableType' */
#define M30072 "Estimator %d: key %s has a status for the %s %s."    /* ... a status for the 'VariablePeriod 'VariableType' */
#define M30073 "Error when 'Finding matching fields' for recipient %s."

#define M30077 "ERROR: Set of edits inconsistent."
#define M30078 "ERROR: Not enough space."
#define M30079 "ERROR: Space exceeded. Old number of columns: max %d y_col %d"
#define M30080 "ERROR: Space exceeded. Cardinality has been reduced from %d to %d because of space constraints."
#define M30081 "Emit(): Unknown token"
#define M30082 "Illegal operation."
#define M30083 "Exponent(): '^' must be followed by a number"
#define M30084 "Looking for 'Aux' or 'FieldId' but found '%s' instead."
#define M30085 "Token too large."
#define M30086 "AUX not followed by integer."
#define M30087 "Syntax error."
#define M30088 "Zero constant is illegal."
#define M30089 "0 is not a valid AUX number."
#define M30090 "AUX%d is too big."
#define M30091 "Missing AUX%d."
#define M30092 "FIELDID cannot be specified for the current period."
#define M30093 "At least two regressors are identical including intercept. They have the same name, period and exponent combination."
#define M30094 "DECIMAL must be an integer between %d and %d inclusively."
#define M30095 "UPPERBOUND must be greater than LOWERBOUND."
#define M30096 "LOWERBOUND and UPPERBOUND must be both non-negative in the scaling method."
#define M30098 "Edits contain multiple 'prorating groups'. Only one 'prorating group' is allowed."
#define M30099 "EditGroupFixHierarchy(): Cannot have same variable twice!"
#define M30100 "The following variable(s) is/are repeated:"
#define M30101 "Variable '%s' is used twice as a total!"
#define M30102 "The weight must be positive!"
#define M30103 "Cannot have same variable twice in an edit!"
#define M30104 "A variable has no value."
#define M30105 "Number must be greater than 0."
/**************************************************************/
/* M40000 series - Statistics (reports)                       */
/**************************************************************/
/* EI_CHECK(): Edits Analysis Statistics */
#define M40001 "EDITS ANALYSIS STATISTICS"
        /* Edits read */
#define M40002 "Number of edits read %.*s:  %d"
#define M40003 "%.*sNumber of equalities read %.*s:  %d"    /* one tab */
#define M40004 "%.*sNumber of inequalities read %.*s:  %d"  /* one tab */  
        /* Minimal Set of Edits */
#define M40005 "Number of edits in minimal set %.*s:  %d"
#define M40006 "%.*sNumber of equalities kept %.*s:  %d"    /* one tab */  
#define M40007 "%.*sNumber of hidden equalities %.*s:  %d"  /* one tab */
#define M40008 "%.*sNumber of inequalities kept %.*s:  %d"  /* one tab */
        /* Redundant edits */ 
#define M40009 "Number of redundant edits %.*s:  %d"
#define M40010 "%.*sNumber of redundant equalities %.*s:  %d"          /* one tab */
#define M40011 "%.*sNumber of redundant inequalities %.*s:  %d"        /* one tab */
#define M40012 "%.*sNumber of hidden redundant equalities %.*s:  %d"   /* one tab */
        /* M40013 and M40014 form one message on two lines. */
#define M40013 "%.*sNumber of tight edits which do not " /* one tab */
#define M40014 "%.*srestrict feasible region %.*s:  %d"  /* two tabs*/
        /* About variables in edits */
#define M40015 "Number of variables %.*s:  %d"
#define M40016 "Number of unbounded variables %.*s:  %d"
#define M40017 "Number of deterministic variables %.*s:  %d"
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40002_LPH   6 
#define M40003_LPH  10
#define M40004_LPH  10
#define M40005_LPH   6
#define M40006_LPH  10
#define M40007_LPH  10
#define M40008_LPH  10
#define M40009_LPH   6
#define M40010_LPH  10
#define M40011_LPH  10
#define M40012_LPH  10
#define M40013_LPH   4
#define M40014_LPH  10
#define M40015_LPH   6
#define M40016_LPH   6
#define M40017_LPH   6

/* EI_CHECK(): Print variable bounds */
#define M40018 "VARIABLE BOUNDS"
#define M40019 "Variable Name"
#define M40020 "Lower"
#define M40021 "Upper"
#define M40022 "unbounded"
#define M40023 "determinant"

/* EI_IMPLY() */ 
#define M40024 "IMPLIED EDITS ANALYSIS DIAGNOSTICS"
#define M40025 "PART 1 - LIST OF IMPLIED EDITS"
#define M40026 "Edits analysed:"
#define M40027 "Implied edits found:"
#define M40028 "No implied edit found."
#define M40029 "PART 2 - IMPLIED EDITS STATISTICS"
#define M40030 "Specified maximum number of implied edits %.*s:  %s"  /* 2nd string : M40031 */ 
#define M40031 "max. integer (default)"  /* INT_MAX */
#define M40032 "Submitted maximum number of implied edits %.*s:  %d"
#define M40033 "Number of implied edits generated %.*s:  %d"
#define M40034 "%.*sNumber of implied equalities %.*s:  %d"    /* one tab */
#define M40035 "%.*sNumber of implied inequalities %.*s:  %d"  /* one tab */
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40030_LPH   6
#define M40032_LPH   6
#define M40033_LPH   6
#define M40034_LPH  10
#define M40035_LPH  10

/* EI_EXTREMAL() */
#define M40036 "EDITS EXTREMAL POINTS ANALYSIS DIAGNOSTICS"
#define M40037 "PART 1 - NO EXTREMAL POINT of cardinality less than or equal to %d has been found."
#define M40038 "PART 1 - LIST OF EXTREMAL POINTS"
#define M40039 "Extremal point number %d"
#define M40040 "  All variables are 0."       /* Potential description of one extremal point */
#define M40041 "* Variables not shown are 0." /* General note */
#define M40042 "PART 2 - EXTREMAL POINTS STATISTICS"
#define M40043 "Specified maximum cardinality %.*s:  %s"  /* 2nd string : M40044 */   
#define M40044 "number of variables (default)"
#define M40045 "Specified maximum cardinality %.*s:  %d"
#define M40046 "Effective maximum cardinality %.*s:  %d"
#define M40047 "Number of extremal points %.*s:  %d"
#define M40048 "Number of points with cardinality %3d %.*s:  %d"
        /* Paired Msg for Length of PlaceHolders: LPH */
#define M40043_LPH   6
#define M40045_LPH   6
#define M40046_LPH   6
#define M40047_LPH   6
#define M40048_LPH   6 /* do not count %3d */

/* EI_STATISTICS() */
#define M40049 "EDIT SUMMARY STATISTICS"
/* Table 1.1 */
#define M40050 "Table 1.1"
#define M40051 "Counts of records that passed, missed and failed for each edit."
#define M40052 "Edit Id."
#define M40053 "Records"
#define M40054 "passed"
#define M40055 "missed"
#define M40056 "failed"
/* Table 1.2 */
#define M40057 "Table 1.2"
#define M40058 "Distribution of records that passed, missed and failed 'k' edits."
#define M40059 "Number of edits"
#define M40060 "Total (records)"
/* Table 1.3 */
#define M40061 "Table 1.3"
#define M40062 "Overall counts of records that passed, missed and failed."
/* Table 2.1 */
#define M40063 "Table 2.1"
#define M40064 "Counts of edit applications of status PASS, MISS or FAIL that involve* each field." /* see M40075 for the asterix note */
#define M40065 "Field" 
#define M40066 "Edit"    /* in french: M40066 <-> M40067 */
#define M40067 "applic." /* in french: M40067 <-> M40066 */
#define M40068 "passed"
#define M40069 "missed"
#define M40070 "failed"
#define M40071 "not"
#define M40072 "involved"
#define M40073 "Number"
#define M40074 "of edits"
#define M40075 "An edit involves a field j, if the jth coefficient of the edit is non-zero.  An edit which does not involve field j is recorded under the 'Edit applic. not involved' column." 
/* Table 2.2 */
#define M40076 "Table 2.2"
#define M40077 "Counts of records of status PASS, MISS, or FAIL for which field j contributed* to the overall record status."
#define M40078 "not appl."   
#define M40079 "If a record has status FAIL, those fields which have status PASS or MISS do not contribute to the record's FAIL status. Therefore, the records which contain these fields are recorded under the 'Records not applicable' column."
/* Related to status definition */
#define M40080 "DEFINITION OF EDIT AND RECORD STATUSES"
#define M40081 "For a given record, an edit status is"
#define M40082 "     i) PASS--if all data values associated with non-zero coefficients in"
#define M40083 "              the edit are non-missing and the edit is satisfied"
#define M40084 "    ii) MISS--if one of more data values associated with non-zero"
#define M40085 "              coefficients in the edit are missing"
#define M40086 "   iii) FAIL--if all data values associated with non-zero coefficients in"
#define M40087 "              the edit are non-missing and the edit is not satisfied"
#define M40088 "An overall record status is"
#define M40089 "     i) PASS--if all edits have status PASS"
#define M40090 "    ii) MISS--if one of more edits have status MISS, those which do"
#define M40091 "              not, have status PASS"
#define M40092 "   iii) FAIL--if one or more edits have status FAIL"


/**************************************************************/
/* Special series - Algorithms predefined descriptions        */
/**************************************************************/
#define AUXTREND_D   "The value from the previous survey for the same unit, with a trend adjustment calculated from an auxiliary variable, is imputed."
#define AUXTREND2_D  "An average of two AUXTRENDs is imputed."
#define CURAUX_D     "The current value of a proxy variable for the same unit is imputed."
#define CURAUXMEAN_D "The current average of a proxy variable is imputed."
#define CURMEAN_D    "The mean value of all (user-defined) respondents for the current survey is imputed."
#define CURRATIO_D   "A ratio estimate, using values of all (user-defined) respondents from the current survey is imputed."
#define CURRATIO2_D  "An average of two CURRATIOs is imputed."
#define CURREG_D     "A simple linear regression based on one independant variable from the current data table."
#define CURREG_E2_D  "A regression based on the value and the squared value of a variable from the current data table."
#define CURREG2_D    "A linear regression based on two independent variables from the current data table."
#define CURREG3_D    "A linear regression based on three independent variables from the current data table."
#define CURSUM2_D    "The sum of two auxiliary variable from the current data table."
#define CURSUM3_D    "The sum of three auxiliary variable from the current data table."
#define CURSUM4_D    "The sum of four auxiliary variable from the current data table."
#define DIFTREND_D   "The value from the previous survey for the same unit, with a trend adjustment calculated from the difference of reported values for the variable, is imputed."
#define HISTREG_D    "A linear regression based on the historical value of the variable to impute."
#define PREAUX_D     "The historical value of a proxy variable for the same unit."
#define PREAUXMEAN_D "The historical average of a proxy variable for the same unit is imputed."
#define PREMEAN_D    "The mean value from the previous survey of all (user-defined) respondents is imputed."
#define PREVALUE_D   "The value from the previous survey for the same unit is imputed."


/**************************************************************/
/* M00000 series - Miscellaneous                              */
/**************************************************************/
#define M00002 "Error at or before"
#define M00004 "Colon"
#define M00005 "Done"
#define M00006 "Error"
#define M00008 "Modifier"
#define M00009 "Operator"
#define M00010 "Plus"
#define M00011 "SemiColon"
#define M00013 "Variable"
#define M00014 "Unknown character"
#define M00017 "EQUALITY EDIT: '%d' makes the set of edits inconsistent."
#define M00023 "INEQUALITY EDIT: '%d' lies entirely outside the feasible region. It is redundant."
#define M00026 "EDITS ANALYSIS DIAGNOSTICS"
#define M00027 "SET OF INCONSISTENT EDITS"
#define M00028 "Inconsistency between algorithms simplex and Given's: software problem."
#define M00029 "MINIMAL SET OF EDITS"
#define M00030 "The minimal set of edits is equivalent to the original set of edits specified."
#define M00031 "INEQUALITY EDIT: '%d' makes the set of edits inconsistent."
#define M00032 "current" /* for french grammar: 'male gender' form   */
#define M00033 "current" /* for french grammar: 'female gender' form */
#define M00034 "historical"
#define M00035 "Estimator function parser"
#define M00036 "Linear regression parser"
#define M00037 "Prorating edits parser"
#define M00038 "Weight"
#define M00039 "Weights parser"
#define M00040 "Equal"
#define M00041 "Number"
#define M00042 "Unknown token"

/**************************************************************/
/* the following messages are added for the                   */
/* addition of RANDNUMVAR                                     */
/**************************************************************/

/* the following messages are added for DonorInternal.c       */
#define MsgRandnumvarDonorInternalTslistLookupFail "DonorInternal: tSList lookup failed in DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalTadtlistIndexFail "DonorInternal: tADTList indexing failed in DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalNotFound "No random number found in DONOR_FindNextNearestDonorIndexAlt()"
#define MsgRandnumvarDonorInternalLessThanZero "DonorInternal: The random number from the recipient record %f is less than 0. Please fix the random number and rerun the procedure."
#define MsgRandnumvarDonorInternalGreaterThanOne "DonorInternal: The random number from the recipient record %f is greater than 1. Please fix the random number and rerun the procedure." 

/* the following messages are added for EI_RandomDonor        */
#define MsgRandnumvarRandomDonorlTslistLookupFail "EI_RandomDonor: tSList lookup failed in EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorTadtlistIndexFail "EI_RandomDonor: tADTList indexing failed in EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorNotFound "No random number found in EI_RandomDonorWithPostEdits()"
#define MsgRandnumvarRandomDonorLessThanZero "EI_RandomDonor: The random number from the recipient record %f is less than 0. Please fix the random number and rerun the procedure."
#define MsgRandnumvarRandomDonorGreaterThanOne "EI_RandomDonor: The random number from the recipient record %f is greater than 1. Please fix the random number and rerun the procedure."
#define MsgRandnumvarRandomDonorInvalidRange "DONOR: At least one random number from the recipient record is 1 or very close to 1, within the interval (1-1e-012,1]. Due to decimal precision constraints, 1-1e-012 will be used to make the random choice instead of the random number from the recipient record."

/* the following messages are added for EI_Chernikova.c       */
#define MsgRandnumvarErrorlocLessThanZero "ERRORLOC: The random number from the record %f is less than 0. Please fix the random number and rerun the procedure."
#define MsgRandnumvarErrorlocGreaterThanOne "ERRORLOC: The random number from the record %f is greater than 1. Please fix the random number and rerun the procedure."
#define MsgRandnumvarErrorlocInvalid "ERRORLOC: At least one random number is 1 or very close to 1, within the interval (1-1e-012,1]. Due to decimal precision constraints, 1-1e-012 will be used to make the random choice instead of the random number from the record."

#endif
