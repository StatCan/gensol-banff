#ifndef MESSAGEBANFF_EN_H
#define MESSAGEBANFF_EN_H

#include "BanffIdentifiers.h"

/********************************************************************************************/
/* Introduced during redesign conversion                                                    */
/********************************************************************************************/
/* return code descriptions */
#define RC_DESC_SUCCESS "successful execution"
#define RC_DESC_FAIL_UNHANDLED "unexpected error"
#define RC_DESC_FAIL_INIT_IN_DATASET "failed to initialize input dataset"
#define RC_DESC_FAIL_READ_PARMS_LEGACY "failed to get parameters"
#define RC_DESC_FAIL_MISSING_DATASET "missing mandatory dataset"
#define RC_DESC_FAIL_VARLIST_NOT_FOUND "variable in varlist not found"
#define RC_DESC_FAIL_VARLIST_INVALID_COUNT "invalid number of variables in varlist"
#define RC_DESC_FAIL_VARLIST_SYSTEM_GENERATED "system generated varlist invalid"
#define RC_DESC_FAIL_SETUP_DATASET_IN "failed to setup input dataset"
#define RC_DESC_FAIL_SETUP_OTHER "unexpected setup error"
#define RC_DESC_FAIL_ALLOCATE_MEMORY "failed to allocate memory"
#define RC_DESC_FAIL_LPI_INIT "LPI initialization failed"
#define RC_DESC_FAIL_EDITS_PARSE "failed to parse edits"
#define RC_DESC_FAIL_VALIDATION_LEGACY "failed legacy validation"
#define RC_DESC_FAIL_VALIDATION_NEW "failed validation"
#define RC_DESC_FAIL_EDITS_OTHER "unexpected edits related error"
#define RC_DESC_FAIL_EDITS_CONSISTENCY "failed edits consistency check"
#define RC_DESC_FAIL_EDITS_REDUNDANCY "failed edits redundancy check"
#define RC_DESC_FAIL_NAME_TOO_LONG "variable name exceeds maximum length"
#define RC_DESC_FAIL_READ_GENERIC "unexpected error reading dataset"
#define RC_DESC_FAIL_WRONG_SORT_ORDER "sort order invalid"
#define RC_DESC_FAIL_READ_SYNC "failed to synchronize datasets"
#define RC_DESC_FAIL_READ_DUPLICATE_DATA "duplicate data detected"
#define RC_DESC_FAIL_WRITE_GENERIC "failed to write to output dataset"
#define RC_DESC_FAIL_PROCESSING_GENERIC "unexpected processing error"
#define RC_DESC_EIE_TRANSFORM_FAIL "transform failure"
#define RC_DESC_EIE_KDTREE_FAIL "KDTREE failure"
#define RC_DESC_EIE_MATCHFIELDS_FAIL "match fields error"

/********************************************************************************************/
/* Generic to Banff                                                                         */
/********************************************************************************************/
#define MsgParmMandatory "%s is mandatory."
#define MsgNumericVarNotInDataSet "Numeric variable %s is not in %s data set."
#define MsgVarNotNumericInDataSet "Variable %s is not numeric in %s data set."
#define MsgCharacterVarNotInDataSet "Character variable %s is not in %s data set."
#define MsgVarNotCharacterInDataSet "Variable %s is not character in %s data set."
#define MsgParmNotSpecified "%s not specified."

#define MsgNumberObs "Number of observations in %s data set %.*s: %*d"
#define MsgNumberValidObs "Number of valid observations in %s data set %.*s: %*d"
#define MsgNumberObs_LPH       9  /* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObs_LPH  9

/********************************************************************************************/
/* Generic to Banff procedures                                                              */
/********************************************************************************************/
/* All procedures */
#define MsgDefault " (default)"
#define MsgNotUsed " (not used)"
#define MsgFunctionFailure "%s failed."
#define MsgParmInvalid "Invalid %s."
#define MsgPositivityOptionInvalid "Options " BPN_REJECT_NEGATIVE " and " BPN_ACCEPT_NEGATIVE " cannot be specified at the same time."
#define MsgStatementMandatory "%s statement is mandatory."
#define MsgParmGreater0 "The value must be greater than 0."
#define MsgParmGreaterEqual0 "The value must be greater than or equal to 0."

/* Flag type parameters */
#define MsgAcceptNegative BPN_ACCEPT_NEGATIVE " = True"
#define MsgRejectNegative BPN_ACCEPT_NEGATIVE " = False"
#define MsgRejectNegativeDefault BPN_ACCEPT_NEGATIVE " = False" MsgDefault
#define MsgAcceptZero BPN_ACCEPT_ZERO " = True"
#define MsgAcceptZeroDefault BPN_ACCEPT_ZERO " = True" MsgDefault
#define MsgRejectZero BPN_ACCEPT_ZERO " = False"
#define MsgRejectZeroDefault BPN_ACCEPT_ZERO " = False" MsgDefault

/* LP solver */
#define MsgInitLPFail "Unable to initialize %s LP solver."
#define MsgInitLPSuccess "The %s LP solver version %s was initialized."

/* Seed */
#define MsgSeedEqualIntegerChosenBySystem "" BPN_SEED " = %d (value chosen by the system) "
#define MsgSeedOutOfRangeGeneratedNumberTaken "The " BPN_SEED " value specified is out of the range acceptable by the platform and cannot be used. It has been replaced with a generated number."
#define MsgSeedNegativeGeneratedNumberTaken "The " BPN_SEED " value specified is negative and cannot be used. It has been replaced with a generated number."
#define MsgSeedZeroGeneratedNumberTaken "The " BPN_SEED " value specified is 0 and cannot be used. It has been replaced with a generated number."

/* Edits */
#define MsgAddingPositivityEditsFail "" BPN_REJECT_NEGATIVE " option is in effect but the system could not add positivity edits to the original set of edits."
#define MsgEditsVarNotInDataSet "Variable %s specified in " BPN_EDITS " is not in %s data set."
#define MsgEditsVarNotNumericInDataSet "Variable %s specified in " BPN_EDITS " is not numeric in %s data set."
#define MsgHeaderEdits "Set of " BPN_EDITS ": "
#define MsgHeaderReducedEdits "REDUCED set of " BPN_EDITS ":"
#define MsgInconsistentEdits "The " BPN_EDITS " are inconsistent. Please verify your " BPN_EDITS "."
#define MsgPositivityEditsAdded "(Positivity " BPN_EDITS " were added because " BPN_REJECT_NEGATIVE " option is in effect.)"
#define MsgReducedEditsEquivalent "The minimal set of edits is equivalent to the original set of edits specified."

/* Miscellaneous (found in more than one procedure)*/
#define MsgDataSetWithTwoKeysDuplicate "%s data set has duplicate keys. The current key is %s and the previous key was %s."
#define MsgDataSetWithTwoKeysNotSorted "%s data set is not sorted in ascending order. The current key is %s and the previous key was %s."
#define MsgByVarsNotFoundInDataSet "One or more " BPN_BY " variables were not found in %s data set." /* HIST or AUX */
#define MsgFasterPerformanceIfByVarsInDataSet "This procedure can perform faster if the %s data set contains the " BPN_BY " variables."
#define MsgKeyNameWithDuplicateValueInDataSet "Duplicate %s '%s' in %s data set."
#define MsgNoObservationsInDataSet "No observations in %s data set."
#define MsgNumberDroppedMissingKeyNameInDataSet "There were %d observations dropped from %s data set because %s is missing."
#define MsgNumberDroppedMissingKeyNameOrFieldidInDataSet "There were %d observations dropped from %s data set because %s or FIELDID is missing."
#define MsgNumberDroppedMissingValuesInDataSet1NotFTIInDataSet2 "There were %d observations dropped from %s data set because a value is missing and there is no corresponding FTI in %s data set."
#define MsgNumberDroppedNegativeValuesInDataSet1NotFTIInDataSet2 "There were %d observations dropped from %s data set because a value is negative and there is no corresponding FTI in %s data set."
#define MsgTwoStatementsExclusive "The lists of variables of the %s and %s statements are mutually exclusive."
#define MsgVarNameInTwoStatementsExclusive "Variable %s is listed in %s and %s statements. These statements are mutually exclusive."

/********************************************************************************************/
/* Verifyedits                                                                              */
/********************************************************************************************/
#define MsgNoExtremalPrinted "The " BPN_EXTREMAL " option is not available when the " BPN_ACCEPT_NEGATIVE " option has been specified. No extremal points will be printed."

/* Parm: " BPN_IMPLY " and " BPN_EXTREMAL " */
#define MsgParmInvalidNegativeValueReplacedByDefault "The %s value specified is negative and cannot be used. It has been replaced by the default value."

/********************************************************************************************/
/* Editstats                                                                              */
/********************************************************************************************/
#define MsgAllOutDataSetsNULL "No analysis is done by the procedure. _NULL_ has been specified for each output data sets."
#define MsgEditSizeExceededMax "Space needed to define EDIT character variable for %s output data set exceeded maximum length acceptable."

/********************************************************************************************/
/* Outlier                                                                                  */
/********************************************************************************************/
#define MsgAtLeastMIIorMEIMustBeSpecified "Either " BPN_MEI ", " BPN_MII ", or both must be specified."
#define MsgAtLeastBETAIorBETAEMustBeSpecified "Either " BPN_BETA_E ", " BPN_BETA_I ", or both must be specified."
#define MsgMIIMustBeGreaterThanMEI "Value of " BPN_MII " must be greater than value of " BPN_MEI "."
#define MsgBETAIMustBeGreaterThanBETAE "Value of " BPN_BETA_I " must be greater than value of " BPN_BETA_E "."

#define MsgHeaderForByGroup "The following messages are for the " BPN_BY " group:%-2n%*s"  /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForByGroup_SAS_FREE "The following messages are for the " BPN_BY " group:" "\n" MSG_INDENT_NOTE
#define MsgHeaderForByGroupAbove_SAS_FREE "The above message was for the following " BPN_BY " group:" "\n" MSG_INDENT_NOTE
#define MsgHeaderForVariable "The following messages are for the variable:%-2n%*s" /* %-2n: specific to SAS_XP routines */
#define MsgHeaderForVariable_SAS_FREE "The following messages are for the variable:\n      %*s"

#define MsgIdValueDuplicatedInDataSet "Duplicate key '%s' in %s data set."
#define MsgMinimumValueOfValidObsNotReached "Number of valid observations in " DSN_INDATA " data set less than " BPN_MIN_OBS " %d. Outlier not processed."
#define MsgVarNameNotInDataSet "Variable %s is not in %s data set."
#define MsgStatementEmpty "At least one variable must be specified with a %s statement."
#define MsgMINOBSInvalidValue "Invalid value for " BPN_MIN_OBS " option. Value must be greater than or equal to %d."
#define MsgMINOBSWarningValue "Value of " BPN_MIN_OBS " less than %d. The method works better with a minimum of %d observations."
#define MsgInvalidExponent "The value must be greater than or equal to 0 and less than or equal to 1."
#define MsgInvalidMethod "Expecting one of the following: CURRENT, HISTORIC, RATIO, SIGMAGAP."
#define MsgInvalidSigma "Expecting one of the following: STD, MAD."
#define MsgInvalidSide "Expecting one of the following: LEFT, BOTH, RIGHT."
#define MsgInvalidStartCentile "The value must be greater than or equal to %.0f and less than %.0f."
#define MsgMultipleStatements "Multiple %s statements. Only one is permitted."

/* CU method */
#define MsgHISTDataSetIgnored "%s data set is specified for CURRENT method. The data set will be ignored." /* HIST or AUX */
#define MsgExponentIgnored "" BPN_EXPONENT " option is specified for CURRENT or SIGMAGAP method. The option will be ignored."
#define MsgNoDefaultForVar "No numeric variable in " DSN_INDATA " data set to use as default list for " BPN_VAR " statement."
#define MsgWithIgnored "" BPN_WITH " statement is specified for CURRENT method. The statement will be ignored."

/* HT/Ratio method */
#define MsgHTRatioMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " should not be specified when historical trend method or ratio method is used. Zero and negative values will never be used for methodological reasons."
#define MsgHTRatioMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " should not be specified when historical trend method or ratio method is used. Zero and negative values will never be used for methodological reasons."
#define MsgHTRatioMethodBadSpecifications "Historical trend method or ratio method cannot be used with specifications given."
#define MsgHTRatioMethodDifferentSizeForVarAndWith "Different number of variables in " BPN_VAR " and " BPN_WITH " statements."

/* SIGMAGAP method */
/*
#define MsgSigmagapMethodAcceptNegativeIgnored "" BPN_ACCEPT_NEGATIVE " should not be specified when 2 variables sigmagap method is used. Zero and negative values will never be used for methodological reasons."
#define MsgSigmagapMethodAcceptZeroIgnored "" BPN_ACCEPT_ZERO " should not be specified when 2 variables sigmagap method is used. Zero and negative values will never be used for methodological reasons."
*/
/* Print count of "bad data" by " BPN_VAR ".*/
#define MsgNumberDroppedInDataSetMissingValueForVar "There were %d observations dropped from the %s data set because the variable %*s is missing."
#define MsgNumberDroppedInDataSetNegativeValueForVar "There were %d observations dropped from the %s data set because the variable %*s is negative."
#define MsgNumberDroppedInDataSetNegativeValueForWeightedVar "There were %d observations dropped from the %s data set because the weighted variable %*s is negative."
#define MsgNumberDroppedInDataSetZeroValueForVar "There were %d observations dropped from the %s data set because the variable %*s is 0."
#define MsgNumberDroppedInDataSetZeroValueForWeightedVar "There were %d observations dropped from the %s data set because the weighted variable %*s is 0."

/* Print weight data quality warnings. */
#define MsgNegativeWeightWarning "At least one weight is negative."
#define MsgZeroWeightWarning "At least one weight is equal to zero."

/* Print Statistics -
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string
   Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberDroppedIDOnlyInDataSet "%.*s dropped because " BPN_UNIT_ID " only in %s %.*s: %*d"
#define MsgNumberDroppedMissingID "%.*sdropped because missing " BPN_UNIT_ID " %.*s: %*d"
#define MsgNumberDroppedMissingWeight "%.*sdropped because missing " BPN_WEIGHT " %.*s: %*d"
#define MsgNumberDroppedIDOnlyInDataSet_LPH  13  /* Paired Msg for Length of PlaceHolders (LPH) */
#define MsgNumberDroppedMissingID_LPH        11
#define MsgNumberDroppedMissingWeight_LPH    11

/********************************************************************************************/
/* Errorloc                                                                                 */
/********************************************************************************************/
#define MsgNotConstrainedByCardinality "The result will not be constrained by cardinality."
#define MsgRedundantEditsFail "Could not compute redundant " BPN_EDITS "."
#define MsgTimePerObsAdjustedToMinValue "Value for " BPN_TIME_PER_OBS " is invalid and has been adjusted to %.2f seconds, the minimum time per record."
#define MsgVarNameUnbounded "Variable %s is unbounded. It MUST have either an upper or lower bound."
#define MsgWeightVarDefault "Weight will be set to 1 for each variable in " BPN_EDITS "."
/*this message is used for both errorloc and donorimp when both the " BPN_SEED " and " BPN_RAND_NUM_VAR " parameters have been specified*/
#define MsgRanndumvarSeedSpecified "Both " BPN_SEED " and " BPN_RAND_NUM_VAR " specified. The " BPN_SEED " parameter will be ignored."
/*the following messages are used for validating the value of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarErrorlocLessThanZero_main "ERRORLOC: The random number from the record %f is less than 0. Please fix the random number and rerun the procedure."
#define MsgRandnumvarErrorlocGreaterThanOne_main "ERRORLOC: The random number from the record %f is greater than 1. Please fix the random number and rerun the procedure."

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count and percent */
#define MsgNoValidDataInGroup "No valid data in the group processed."
#define MsgNumberPercentObs "Number of observations %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsDroppedMissingKey "Number of observations dropped because the key is missing %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsFailedEdits "Number of observations that failed edits %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsPassedEdits "Number of observations that passed edits %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentObsNoSolution "%.*sNumber of observations where no solution is possible %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsWithVarsToImpute "%.*sNumber of observations with variables to impute %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentObsNoSolutionCardinality "%.*scaused by cardinality constraint %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionSpace "%.*scaused by space constraint %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentObsNoSolutionTime "%.*scaused by time constraint %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 2 tabs) */
#define MsgNumberVarsToImpute "%.*sNumber of variables to impute %.*s: %*d"

#define MsgNumberPercentObs_LPH                   18  /* Paired Msg for Length of PlaceHolders (LPH) */
#define MsgNumberPercentObsDroppedMissingKey_LPH  18
#define MsgNumberPercentObsFailedEdits_LPH        18
#define MsgNumberPercentObsPassedEdits_LPH        18
#define MsgNumberPercentObsNoSolution_LPH         22
#define MsgNumberPercentObsWithVarsToImpute_LPH   22
#define MsgNumberPercentObsNoSolutionCardinality_LPH  22
#define MsgNumberPercentObsNoSolutionSpace_LPH    22
#define MsgNumberPercentObsNoSolutionTime_LPH     22
#define MsgNumberVarsToImpute_LPH                 11

/********************************************************************************************/
/* Deterministic                                                                            */
/********************************************************************************************/
/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberValidObsWithFTI "%.*swith FTI variables %.*s: %*d"
#define MsgNumberValidObsWithoutFTI "%.*swithout FTI variables %.*s: %*d"
/* Subtotal: count only (heading spaces: 2 tabs) */
#define MsgNumberValidObsImputed "%.*simputed %.*s: %*d"
#define MsgNumberValidObsNotImputed "%.*snot imputed %.*s: %*d"

/* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberValidObsWithFTI_LPH     11
#define MsgNumberValidObsWithoutFTI_LPH  11
#define MsgNumberValidObsImputed_LPH     11
#define MsgNumberValidObsNotImputed_LPH  11

/********************************************************************************************/
/* Donorimputation and Massimputation                                                       */
/********************************************************************************************/
#define MsgFooterAllByGroup "The above message was for the total of all by-groups."
#define MsgInvalidPcentdonors "Invalid " BPN_PERCENT_DONORS ". " BPN_PERCENT_DONORS " must be greater than or equal to 1 and less than 100."
#define MsgNoImputationDoneNoRecipient "There are no recipients in the group processed. No imputation will be done."
#define MsgNoImputationDoneNotEnoughDonors "There are not enough donors in the group processed. No imputation will be done."
#define MsgNoImputationDoneNoValidObs "There are no valid observations in the group processed. No imputation will be done."
#define MsgNoRandomSearch "%s not specified (random search of donors will not be performed)" /* for print parameters */
#define MsgNoLimit "No limit"
#define MsgParmIntegerGE1 "%s must be an integer greater than or equal to 1."
#define MsgParmOutOfRange "The %s value specified is out of the range acceptable by the platform."
#define MsgParmWithDuplicateVariable "A variable is repeated in %s."
#define MsgInvalidNLimit "Invalid " BPN_N_LIMIT ". " BPN_N_LIMIT " must be greater than or equal to 1."
#define MsgInvalidMrl "Invalid " BPN_MRL ". " BPN_MRL " must be greater than 0."
#define MsgDonorLimitation "DONORLIMIT parameters might prevent recipients from being imputed."
#define MsgPcentdonorsEqualDoublePcentSymbol "" BPN_PERCENT_DONORS " = %.*f%%"
#define MsgPcentdonorsEqualDoublePcentSymbolDefault "" BPN_PERCENT_DONORS " = %.*f%% (default)"

/* Print Statistics
   Format: %.*s --> field width of string "" to output
           %*s  --> field width to extract from a line of 'dot' string  */
/* Total: count only */
#define MsgNumberObservations "Number of observations %.*s: %*d"
#define MsgNumberObservationsDropped "Number of observations dropped %.*s: %*d"
/* Total: count and percent */
#define MsgNumberPercentDonors "Number of donors %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentReachedDonorLimit "Number of donors to reach DONORLIMIT %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipients "Number of recipients %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentValidObs "Number of valid observations %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count only (heading spaces: 1 tab) */
#define MsgNumberObsDroppedExcludedDonors "%.*sexcluded donors %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedNegativeData "%.*snegative data %.*s: %*d"
#define MsgNumberObsDroppedNonEligibleDonors "%.*snon-eligible donors %.*s: %*d" /* only for Donorimputation */
#define MsgNumberObsDroppedMissingData "%.*smissing data %.*s: %*d"
#define MsgNumberObsDroppedMissingKey "%.*smissing key %.*s: %*d"
#define MsgNumberObsDroppedMixed "%.*smixed observations %.*s: %*d"
/* Subtotal: count and percent (heading spaces: 1 tab) */
#define MsgNumberPercentRecipientsWithoutEnoughDonors "%.*snot imputed (insufficient number of donors) %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithoutMatchingFields "%.*swithout matching fields %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsWithMatchingFields "%.*swith matching fields %.*s: %*d %.*s %*.*f%%"
/* Subtotal: count and percent (heading spaces: 2 tabs) */
#define MsgNumberPercentRecipientsImputed "%.*simputed %.*s: %*d %.*s %*.*f%%"
#define MsgNumberPercentRecipientsNotImputed "%.*snot imputed %.*s: %*d %.*s %*.*f%%"

#define MsgNumberObservations_LPH                  7  /* Paired message for Length of PlaceHolders (LPH) */
#define MsgNumberObservationsDropped_LPH           7
#define MsgNumberPercentDonors_LPH                18
#define MsgNumberPercentReachedDonorLimit_LPH     18
#define MsgNumberPercentRecipients_LPH            18
#define MsgNumberPercentValidObs_LPH              18
#define MsgNumberObsDroppedExcludedDonors_LPH     11
#define MsgNumberObsDroppedNegativeData_LPH       11
#define MsgNumberObsDroppedNonEligibleDonors_LPH  11
#define MsgNumberObsDroppedMissingData_LPH        11
#define MsgNumberObsDroppedMissingKey_LPH         11
#define MsgNumberObsDroppedMixed_LPH              11
#define MsgNumberPercentRecipientsWithoutEnoughDonors_LPH    22
#define MsgNumberPercentRecipientsWithoutMatchingFields_LPH  22
#define MsgNumberPercentRecipientsWithMatchingFields_LPH     22
#define MsgNumberPercentRecipientsImputed_LPH     22
#define MsgNumberPercentRecipientsNotImputed_LPH  22

/********************************************************************************************/
/* Donorimputation                                                                          */
/********************************************************************************************/
#define MsgInvalidEligdon "" BPN_ELIGDON " must be ORIGINAL/O or ANY/A."
#define MsgParmGenericProblem "There is a problem with %s."
/*the following messages are added for addition of " BPN_RAND_NUM_VAR "*/
#define MsgRandnumvarRandkeylistNotFound "In function ReadData() failed to find position in RANDKEY_LIST to insert the value of the key of the current record read from the " DSN_INDATA "= input file"
#define MsgRandnumvarDuplicateKey "In function ReadData(), duplicate key value %s found in list of keys for list of random numbers in function ReadData()"
#define MsgRandnumvarKeyInsertFailed "In function ReadData(), failed to insert the value of the key of the current record read from the " DSN_INDATA "= input file into RANDKEY_LIST"
#define MsgRandumvarInvalidValues "The variable specified in " BPN_RAND_NUM_VAR " contains one or more missing values. Please verify the contents of the " BPN_RAND_NUM_VAR " variable."

#define MsgRandnumvarDonorImputationLessThanZero "DONOR: The random number from the recipient record %f is less than 0. Please fix the random number and rerun the procedure."
#define MsgRandnumvarDonorImputationGreaterThanOne "DONOR: The random number from the recipient record %f is greater than 1. Please fix the random number and rerun the procedure." 


/* PostEdits */
#define MsgAddingPositivityPostEditsFail "" BPN_REJECT_NEGATIVE " option is YES but could not add positivity " BPN_POST_EDITS "."
#define MsgDifferentSetVarsInEditsPostEdits "" BPN_EDITS " and " BPN_POST_EDITS " do not contain the same set of variables. Please verify your " BPN_EDITS " and " BPN_POST_EDITS "."
#define MsgEditsTakenAsPostEdits "No " BPN_POST_EDITS " have been specified. " BPN_EDITS " will be taken as " BPN_POST_EDITS "."
#define MsgHeaderPostEdits "Set of " BPN_POST_EDITS ":"
#define MsgHeaderReducedPostEdits "REDUCED set of " BPN_POST_EDITS ":"
#define MsgInconsistentPostEdits "The " BPN_POST_EDITS " are inconsistent. Please verify your " BPN_POST_EDITS "."
#define MsgPositivityPostEditsAdded "(Positivity " BPN_POST_EDITS " were added because " BPN_REJECT_NEGATIVE " option has been specified.)"
#define MsgReducedPostEditsEquivalent "The minimal set of post imputation edits is equivalent to the original set of edits specified."

/* Print info counters */
#define MsgNumberDroppedExcludedDonors "There were %d observations dropped from " DSN_INDATA " data set because they are potential donors but %s value is 'E' for these observations."
#define MsgNumberDroppedMissingValuesInEditsMustmatchNotFTI "There were %d observations dropped from " DSN_INDATA " data set because one/more " BPN_EDITS " variables or one/more " BPN_MUST_MATCH " variables outside the " BPN_EDITS " group are missing in " DSN_INDATA " data set and there is no corresponding FTI in " DSN_INSTATUS " data set."
#define MsgNumberDroppedMixedObs "There were %d mixed observations dropped from " DSN_INDATA " data set because one or more " BPN_MUST_MATCH " variables outside the " BPN_EDITS " group have a corresponding FTI in " DSN_INSTATUS " data set."
#define MsgNumberDroppedNegativeValuesInEditsMustmatchNotFTI "There were %d observations dropped from " DSN_INDATA " data set because one/more " BPN_EDITS " variables or one/more " BPN_MUST_MATCH " variables outside the " BPN_EDITS " group are negative in " DSN_INDATA " data set and there is no corresponding FTI in " DSN_INSTATUS " data set."
#define MsgNumberDroppedNonEligibleDonors "There were %d observations dropped from " DSN_INDATA " data set because they are potential donors but they dont meet criteria related to " BPN_ELIGDON "."

/********************************************************************************************/
/* Massimputation                                                                           */
/********************************************************************************************/
#define MsgNoImputationInvalidOptions "Invalid options. At least one of " BPN_MUST_MATCH " and " BPN_RANDOM " must be specified."

/* Print info counters */
#define MsgNumberDroppedSomeMissingButNotAllMustimpute "There were %d observations dropped from " DSN_INDATA " data set because one or more variables listed in " BPN_MUST_IMPUTE " statement are missing, but not all of them."
#define MsgNumberDroppedNegativeMustimputeOrMustmatch "There were %d observations dropped from " DSN_INDATA " data set because one or more variables listed in " BPN_MUST_IMPUTE " or " BPN_MUST_MATCH " statements are negative."
#define MsgNumberDroppedMixedMissingMustmatch "There were %d mixed observations dropped from " DSN_INDATA " data set because one or more variables listed in " BPN_MUST_MATCH " statement are missing for a potential donor not yet dropped."

/********************************************************************************************/
/* Estimator                                                                                */
/********************************************************************************************/
#define MsgAlgorithmWithHistVarAndNoHistDataSet "An " DSN_INDATA_HIST " data set was not specified and at least one algorithm uses a historical variable."
#define MsgCountCriteriaDoubleReplacedByInteger "Conversion error on the COUNTCRITERIA variable in " DSN_INESTIMATOR " data set. COUNTCRITERIA should be an integer. %f was specified but %d will be used."
#define MsgExclVarStatementIgnored "%s should not be specified when there is no average to calculate or no linear regression or no random error requested."
#define MsgHeaderEstimator "Estimator"
#define MsgHeaderPrintStatistics "Statistics on observations:"
#define MsgHistOptionIgnored "An " DSN_INDATA_HIST " data set was specified but no estimator use a historical variable."
#define MsgHistStatusOptionIgnored "An " DSN_INSTATUS_HIST " data set was specified but no " DSN_INDATA_HIST " data set was."
#define MsgMustSpecifyEstimatorSpecs "No estimator specification specified."
#define MsgNotEnoughValidObsToProcessEstimators "Not enough valid data to process the estimators."
#define MsgNumberDroppedMissingKeyNameOneDataSet "There were %d observations dropped because %s is missing in %s data set."
#define MsgOutAcceptableOptionIgnored "An " DSN_OUTACCEPTABLE " data set was specified but there is no average to calculate and no linear regression is requested."
#define MsgOutEstEFOptionIgnored "An " DSN_OUTEST_EF " data set was specified but no estimator function with at least one parameter is requested."
#define MsgOutEstLROptionIgnored "An " DSN_OUTEST_LR " data set was specified but no linear regression is requested."
#define MsgOutRandomErrorOptionIgnored "An " DSN_OUTRAND_ERROR " data set was specified but no random error is requested."
#define MsgRedefinitionOfPredefinedAlgorithmName "The predefined algorithm '%s' has been redefined."
#define MsgVarNameInByAndCurrentEstimatorSpecs "Variable %s is listed in " BPN_BY " and current estimator specifications."
#define MsgVarNameInByAndHistoricalEstimatorSpecs "Variable %s is listed in " BPN_BY " and historical estimator specifications."

/* Print Estimator Specifications
   Format: %.*s --> field width of string "" to output */
#define MsgHeaderPrintEstimatorSpecifications "Set of estimator specifications:"
#define MsgEstimatorNumber "Estimator %d."
#define MsgDescriptorNameStringValue "%s %.*s: %s"   /* 1st string: see List of DescriptorName */ // removed %m
/* List of DescriptorName */
#define MsgDescriptorAlgorithmName "Algorithm name"
#define MsgDescriptorAuxiliaryVariables "Auxiliary variables"
#define MsgDescriptorCountCriteria "Count criteria"
#define MsgDescriptorDescription "Description"
#define MsgDescriptorExcludeImputed "Exclude imputed"
#define MsgDescriptorExcludeOutliers "Exclude outliers"
#define MsgDescriptorFieldid "Field id"
#define MsgDescriptorFormula "Formula"
#define MsgDescriptorPercentCriteria "Percent criteria"
#define MsgDescriptorRandomError "Random error"
#define MsgDescriptorStatus "Status"
#define MsgDescriptorType "Type"
#define MsgDescriptorVarianceExponent "Variance exponent"
#define MsgDescriptorVariancePeriod "Variance period"
#define MsgDescriptorVarianceVariable "Variance variable"
#define MsgDescriptorWeightVariable "Weight variable"
/* Others */
#define MsgTypeInitInvalid "INVALID TYPE"
#define MsgAlgorithmNameNotFound "%.*s Algorithm Name not found."

/* Print Formulas. */
#define MsgHeaderPrintFormulas "Formula of estimator specifications:"
#define MsgFieldidEqualFormula "%s = %s"
#define MsgFieldidEqualFormulaRandomError "%s = %s + Random Error"

/* Print Imputation Statistics. */
#define MsgHeaderPrintImputationStatistics "Statistics on imputation:"
#define MsgColumnNegative "Neg"
#define MsgEstimatorNotActive "not active"

/* Print Parameters Statistics. */
#define MsgHeaderPrintParameters "Statistics on estimators:"
#define MsgAcceptableObsCountPercentForEF "Count = %d Percent = %.*f %sweighted." /* String: MsgIndicatorNotWeighted or "" */
#define MsgAcceptableObsCountPercentForLR "Count = %d Percent = %.*f %sweighted and %s a variance variable." /* 1st string IndicatorNotWeighted or "" / 2nd string: MsgIndicatorHasNotVariance or MsgIndicatorHasVariance*/
#define MsgHeaderAverage "Average"
#define MsgHeaderExponent "Exponent"
#define MsgHeaderPeriod "Period"
#define MsgIndicatorHasVariance "has"
#define MsgIndicatorHasNotVariance "does not have"
#define MsgIndicatorNotWeighted "not "
#define MsgIntercept "Intercept"
#define MsgNoParameters "No parameters."
#define MsgPeriodCurr "CURR"

/********************************************************************************************/
/* Prorate                                                                                  */
/********************************************************************************************/
#define MsgBasicMethodLowerBoundAndPositivityInconsistent "With the basic method, lower bound cannot be negative while " BPN_REJECT_NEGATIVE " option is in effect."
#define MsgDataSetInstatusIgnored "" DSN_INSTATUS " data set will be ignored: none of modifiers are ORIGINAL or IMPUTED."
#define MsgDataSetInstatusMandatory "" DSN_INSTATUS " data set is mandatory: one or more modifiers are ORIGINAL or IMPUTED."
#define MsgDecimalMustBeIntegerBetweenTwoValues "" BPN_DECIMAL " must be an integer between %d and %d inclusively."
#define MsgInvalidMethodStringPlusValidKeywords "Invalid " BPN_METHOD ": %s. Expecting one of the following: %c, %s, %c, %s."
#define MsgInvalidModifierStringPlusValidKeywords "Invalid " BPN_MODIFIER ": %s. Expecting one of the following: %c, %s, %c, %s, %c, %s, %c, %s."
#define MsgNoProratingAllModifiersAreString "All modifiers are %s. There are no variables to prorate."
#define MsgNotConstrainedByUpperBound "The result will not be constrained by upper bound."
#define MsgScalingMethodLowerBoundGEDouble "Invalid " BPN_LOWER_BOUND ". " BPN_LOWER_BOUND " must be greater than or equal to %.0f in the scaling method of prorating."
#define MsgScalingMethodUpperBoundGEDouble "Invalid " BPN_UPPER_BOUND ". " BPN_UPPER_BOUND " must be greater than or equal to %.0f in the scaling method of prorating."
#define MsgUpperBoundGTLowerBound "" BPN_UPPER_BOUND " must be greater than " BPN_LOWER_BOUND "."
#define MsgVarInTwoStatementsExclusive "A variable is listed in %s and %s statements. These statements are mutually exclusive."

/* Print Edits */
#define MsgHeaderProrateEdits "" BPN_EDITS " PARSED for PRORATE procedure "
#define MsgHeaderEditsModifier "(if no modifier is specified for a variable, the value of the " BPN_MODIFIER " option will be used):"
#define MsgHeaderGroup "--- GROUP ---"
#define MsgEditLevelEqualInteger "EDIT Level: %d"

/* Print Statistics */
#define MsgNoImputationDoneNoValidObsByGroup "No valid observations in the group processed. No imputation has been done."
#define MsgNumberObsDroppedNegativeValueAndPositivityInconsistent "There were %d observations dropped because a value is negative in " DSN_INDATA " data set and option " BPN_ACCEPT_NEGATIVE " is not specified."


#endif
