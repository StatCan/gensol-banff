# Table of Contents

- [Introduction](#introduction)
    - [List of output tables](#list-of-output-tables)
    - [Output tables common to multiple procedures](#output-tables-common-to-multiple-procedures)
- [Standard output tables](#standard-output-tables)
    - [outdata](#outdata)
    - [outstatus](#outstatus)
- [editstats](#editstats)
    - [outedits_reduced](#outedits_reduced)
    - [outedit_status](#outedit_status)
    - [outk_edits_status](#outk_edits_status)
    - [outglobal_status](#outglobal_status)
    - [outedit_applic](#outedit_applic)
    - [outvars_role](#outvars_role)
- [outlier](#outlier)
    - [outstatus_detailed](#outstatus_detailed)
    - [outsummary](#outsummary)
- [errorloc](#errorloc)
    - [outreject](#outreject)
- [donorimp](#donorimp)
    - [outmatching_fields](#outmatching_fields)
    - [outdonormap](#outdonormap)
- [estimator](#estimator)
    - [outacceptable](#outacceptable)
    - [outest_ef](#outest_ef)
    - [outest_lr](#outest_lr)
    - [outest_parm](#outest_parm)
    - [outrand_err](#outrand_err)
- [prorate](#prorate)
    - [outreject](#outreject-1)
- [massimp](#massimp)
    - [outdonormap](#outdonormap-1)

# Introduction

The Banff procedures generate a number of output tables, which can be categorized as follows:
- Standard output tables:
    - `outdata`, containing the results of imputation (for procedures performing treatment)
    - `outstatus`, containing status flags (for procedures performing selection or treatment)
- Diagnostic files, containing useful information specific to the procedure

Note that in many tables, **\<unit_id\>** appears as a column header. This is a placeholder; the actual column header is the name of the variable by the `unit_id` parameter.

### List of output tables
| Procedure     | Output Tables                                  | 
| ------------- | --------------------------------------------   |
| verifyedits   | *none - outputs printed to log*                |
| editstats     | `outedits_reduced` `outedit_status` `outk_edits_status` `outglobal_status` `outedit_applic` `outvars_role`   |
| outlier       | `outstatus` `outstatus_detailed` `outsummary`  |
| errorloc      | `outstatus` `outreject`                        |
| deterministic | `outdata` `outstatus`                          |  
| donorimp      | `outdata` `outstatus` `outdonormap` `outmatching_fields`  |  
| estimator     | `outdata` `outstatus` `outacceptable` `outest_ef` `outest_lr` `outest_parm` `outrand_err` |  
| prorate       | `outdata` `outstatus` `outreject`              |  
| massimp       | `outdata` `outstatus` `outdonormap`            |  

### Output tables common to multiple procedures
| Procedure     | outdata | outstatus | outdonormap | outreject | 
| ------------- | :-----: | :-------: | :---------: | :-------: |
| outlier       |         | X         |             |           |
| errorloc      |         | X         |             | X         |
| deterministic | X       | X         |             |           |  
| donorimp      | X       | X         | X           |           |  
| estimator     | X       | X         |             |           |  
| prorate       | X       | X         |             | X         |  
| massimp       | X       | X         | X           |           |  

# Standard output tables

The Banff procedures are designed to be run sequentially as part of an SDE process flow. The outputs from one procedure often act as inputs for subsequent procedures, and the statistical data that is the target of the SDE process is updated continuously throughout the process. Two standard outputs that are common to multiple procedures are `outdata` and `outstatus`. Please see the [Interaction between procedures](/docs/EN/user_guide.md#interaction-between-procedures) section of the user guide for more details.

### outdata

Procedures performing treatment functions (`deterministic`, `donorimp`, `estimator`, `prorate` and `massimp`) produce the `outdata` table, output statistical data (i.e., microdata) that includes the result of the treatment function. This includes both imputed values (e.g., imputed from `donorimp`) and modified values (e.g., prorated values from `prorate`). Some important notes about `outdata`:

* The `outdata` table is typically **not a complete copy of `indata`** but only contains rows and columns affected by the procedure. For example, if `indata` includes 2000 rows and 25 columns, but only 500 rows and 10 columns are affected by a procedure, then `outdata` will only include 500 rows and 10 columns. To continue the SDE process, users should manually update the `indata` file with the new information from `outdata`. (*Note: The Banff team is looking into automatically updating `indata` with the results of `outdata` in a future release.*)
* The `outdata` table always contains variables specified by the `unit_id` and `by` parameters.
* If no records are successfully imputed or altered by the procedure, then `outdata` is empty. No error will occur.

### outstatus

Selection and imputation flags are always associated with individual values on `indata`. Because `indata` is tabular, each observation can be associated with a specific record (row) and field (column). Records are identified by the user-specified unique record identifier `unit_id`, while fields are referenced by their variable name. Status flags are stored in the `outstatus` file with the following columns:

| Column                    | Description |
| ------------              | ----------- |
| <nobr>\<unit-id\></nobr>  | Record identifier (i.e., row) to which the status flag applies. (The actual column header is the name of the variable by the `unit_id` parameter.) |
| `FIELDID`                   | Field identifier (i.e., column) to which the status flag applies.   |
| `STATUS`                    | Status flag such as "FTI", "FTE", or "IDN".                         |
| `VALUE`                     | Value of the variable when the status code was generated. For procedures performing selection (`errorloc` and `outlier`), this column reflects the value of the observation on `indata` when selection occurred. For procedures performing treatment (`deterministic`, `donorimp`, `estimator`, `prorate`, `massimp`), this column reflects the value of the observation on `outdata`, after treatment has occurred. |

All procedures performing selection or treatment functions (i.e., all but `verifyedits` and `editstats`) automatically produce `outstatus` containing selection or imputation flags. Some procedures also read status files as inputs (`instatus`); these may be required, depending on the procedure.

# editstats

### outedits_reduced

Contains the minimal set of edits. The `EDITID` value is created by the procedure to cross-reference the other procedure outputs.

| Column         | Description |
| -------------- | ----------- |
| EDITID         | Identification number of the edit belonging to the minimal set.     |
| EDIT_EQUATION  | The formulation of the edit.                                        |

### outedit_status

Number of records which pass, miss, or fail each edit.

| Column      | Description |
| ------------| ----------- |
| EDITID      | Identification number of the edit belonging to the minimal set.     |
| OBS_PASSED  | Number of observations that passed the edit.                   |
| OBS_MISSED  | Number of observations that have one or more missing variables involved in the edit.   |
| OBS_FAILED  | Number of observations that failed the edit because of one or more non-missing values. |

### outk_edits_status

Number of records which pass, miss, or fail a given number of edits.

| Column      | Description |
| ------------| ----------- |
| K_EDITS      | « k » cumulated edits that belong to the minimal set.     |
| OBS_PASSED  | Number of observations that passed « k » edits.                   |
| OBS_MISSED  | Number of observations that have one or more missing variables involved in « k » edits.   |
| OBS_FAILED  | Number of observations that failed « k » edits because of one or more non-missing values. |

### outglobal_status

Number of records which pass, miss, or fail the overall record status.

| Column      | Description |
| ------------| ----------- |
| OBS_PASSED  | Number of observations with `PASS` as overall status (i.e. that passed all the edits belonging to the minimal set).                          |
| OBS_MISSED  | Number of observations with `MISS` as overall status (i.e. that have one or more missing variables involved in one or more edits belonging to the minimal set but without FAIL edit status for any rule).                                   |
| OBS_FAILED  | Number of observations with `FAIL` as overall status (i.e. having at least one FAIL edit status for one edit belonging to the minimal set). |
| OBS_TOTAL   | Total number of observations.           |

### outedit_applic

Number of times each variable was involved in an edit which passed, missed or failed.

| Column              | Description |
| ------------------- | ----------- |
| FIELDID             | Name of the variable.                        |
| EDIT_APPLIC_PASSED  | Number of times the variable has « inherited » a `PASS` status code given to observations for the edits involving the variable.                   |
| EDIT_APPLIC_MISSED  | Number of times the variable has « inherited » a `MISS` status code given to observations for the edits involving the variable.                   |
| EDIT_APPLIC_FAILED  | Number of times the variable has « inherited » a `FAIL` status code given to observations for the edits involving the variable.                   |
| EDIT_APPLIC_NOTINVOLVED  |  Number of edits not involving the variable multiplied by the number of observations.                                                        |
| EDITS_INVOLVED  | Number of edits involving the variable.          |

### outvars_role

Number of times each variable contributed to the overall record status.

| Column               | Description |
| -------------------  | ----------- |
| FIELDID              | Name of the variable.  |
| OBS_PASSED           | Number of times a variable has an incidence on the `PASS` overall status code given to observations.                          |
| OBS_MISSED           | Number of times a variable has an incidence on the `MISS` overall status code given to observations.                          |
| OBS_FAILED           | Number of times a variable has an incidence on the `FAIL` overall status code given to observations.                          |
| OBS_NOT_APPLICABLE   | Number of times a variable has no incidence on the `MISS` or `FAIL` overall status code given to observations.              |

# outlier

### outstatus_detailed

Contains the `outlier_status` of each selected outlier. Additional information is displayed if `outlier_stats=True` is specified when the procedure is called.

| Column         | Description |
| -------------- | ----------- |
| \<unit_id\>    | Record identifier.     |
| FIELDID        | Field identifier.   |
| OUTLIER_STATUS | Detailed status of the outliers that has four possible values: <br><br> - `ODER`: Outlier with values falling outside the exclusion interval, on the right.  <br> - `ODEL`: Outlier with values falling outside the exclusion interval, on the left.   <br> - `ODIR`: Outlier with values falling outside the imputation interval, on the right. <br> - `ODIL`: Outlier with values falling outside the imputation interval, on the left.      |
| METHOD | Outlier detection method.                                |
| CURRENT_VALUE | Value of the outlier from `indata`.                         |
| WEIGHT | Weight applied to `CURRENT_VALUE` or to ratio or historical trend when `with_var` is specified. |
| HIST_AUX | Name of the variable paired with the corresponding `var` variable.     |
| HIST_AUX_VALUE | Value of the variable paired with the corresponding `var` variable (when `with_var` is specified).                        |
| EFFECT | Effect value calculated for the record and compared with the interval boundaries as part of outlier detection steps.      |
| GAP | The gap between this record and the previous one with a different value (SG method).     |
| IMP_SIGMAGAP | Imputation sigma-gap calculated as `beta_i` multiplied by the deviation (SG method). |
| EXCL_SIGMAGAP | Exclusion sigma-gap calculated as `beta_e` multiplied by the deviation (SG method). |
| IMP_BND_L | **HB Method**: Upper bound of left imputation interval.<br> **SG Method**: First gap identifying records to impute on the left. |
| EXCL_BND_L | **HB Method**: Upper bound of left exclusion interval.<br> **SG Method**: First gap identifying records to exclude on the left. |
| EXCL_BND_R | **HB Method**: Lower bound of right exclusion interval.<br> **SG Method**: First gap identifying records to exclude on the right.         |
| IMP_BND_R | **HB Method**: Lower bound of right imputation interval.<br> **SG Method**: First gap identifying records to impute on the right.    |

### outsummary

Summary information about outlier detection, including observation counts, status counts, and statistical outputs (such as acceptance interval bounds) calculated by the procedure.

| Column         | Description |
| -------------- | ----------- |
| NObs        | Number of observations from `indata`.     |
| NValid  | Number of valid observations.     |
| NRejected_NoMatch | Number of observations rejected because an observation matching on `unit_id` could not be found indata_hist table.  |
| FIELDID        | Field identifier. If outlier detection is applied to a ratio or historical trend, `FIELDID` is the numerator.    |
| NUsed         | Number of observations used for outlier detection.         |
| NRejected     | Total number of observations rejected, for the current by-group and `FIELDID`.  |
| NRejected_VarMissing | Number of observations rejected because the variable of interest is missing. |
| NRejected_VarZero     | Number of observations rejected because the variable of interest is zero (only applicable if `accept_zero=False`).            |
| NRejected_VarNegative     | Number of observations rejected because the variable of interest is negative (only applicable if `accept_negative=False`).            |
| NFTI     | Number of observations flagged for imputation (i.e. with an `FTI` status).            |
| NFTE     | Number of observations flagged for exclusion (i.e. with an `FTE` status).            |
| IMP_BND_L     | **HB Method**: Upper bound of left imputation interval.<br> **SG Method**: First gap identifying records to impute on the left.           |
| EXCL_BND_L     | **HB Method**: Upper bound of left exclusion interval.<br> **SG Method**: First gap identifying records to exclude on the left.            |
| EXCL_BND_R     | **HB Method**: Lower bound of right imputation interval.<br> **SG Method**: First gap identifying records to exclude on the right.            |
| IMP_BND_R     | **HB Method**: Lower bound of right imputation interval.<br> **SG Method**: First gap identifying records to impute on the right.            |
| AuxVarID       | Auxiliary variable used as the denominator when outlier detection is used on a ratio or historical trend. |
| NRejected_AuxVarMissing  | Number of observations rejected because auxiliary variable is missing. |
| NRejected_AuxVarZero     | Number of observations rejected because auxiliary variable is zero.    |
| NRejected_AuxVarNegative | Number of observations rejected because auxiliary variable is negative. |
| Q1     | First quartile (HB Method).            |
| M     | Median (HB Method).                     |        
| Q3     | Third quartile (HB Method).            |
| DEVIATION     | Median absolute deviation if `sigma='MAD'` or standard deviation if `sigma='STD'` (SG Method).            |
| IMP_SIGMAGAP   | Imputation sigma-gap calculated as beta_i multiplied by the deviation (SG method). |
| EXCL_SIGMAGAP  | Exclusion sigma-gap calculated as beta_e multiplied by the deviation (SG method).  |

# errorloc

### outreject

Records for which the error localization problem could not be solved.

| Column       | Description |
| ------------ | ----------- |
| \<unit_id\>  | Record identifier.     |
| NAME_ERROR   | Can take on one of two possible values:<br><br> - `CARDINALITY EXCEEDED`: cardinality for this observation's solution exceeds the maximum cardinality specified in  `cardinality` parameter.<br> - `TIME EXCEEDED`: processing time for this observation exceeds the maximum time allowed per observation specified in `time_per_obs` parameter. |

# donorimp

### outmatching_fields

Statuses of matching fields used for distance calculations. These can be different for each recipient.

| Column       | Description |
| ------------ | ----------- |
| \<unit_id\>  | Recipient identifier.     |
| FIELDID      | Field identifier.   |
| STATUS       | Status code that can take on one of four possible values:<br><br> - `IDN`: field imputed by donor imputation. <br> - `MFS`: system matching field. <br> - `MFU`: user-specified matching field. <br> - `MFB`: matching field that is both system and user-specified.  |

### outdonormap

Mapping of recipient/donor pairs.

| Column            | Description |
| ----------------- | ----------- |
| RECIPIENT         | Recipient identifier.     |
| DONOR             | Donor identifier.   |
| NUMBER_OF_ATTEMPTS | Number of donors tried before the recipient passed the postedits. |
| DONORLIMIT        | Reproduces the value of the `n_limit` parameter (same value for all records).   |

# estimator

### outacceptable

List of acceptable observations used in the calculation of model parameters for imputation.

| Column         | Description |
| -------------- | ----------- |
| ESTIMID        | Identification number for the estimator defined based on the inestimator input table order (begins with number 0).   |
| ALGORITHMNAME  | Name of the algorithm (pre-defined in procedure or custom defined). |
| \<unit_id\>    | Record identifier.      |

### outest_ef

Report on calculation of averages (estimator functions).

| Column         | Description |
| -------------- | ----------- |
| ESTIMID        | Identification number for the estimator defined based on the estimator input table order (begins with number 0).  |
| ALGORITHMNAME  | Name of the algorithm (pre-defined in procedure or custom defined).     |
| FIELDID        | Field identifier for which an average is calculated.    |
| PERIOD         | Current (C) or historical (H) period specific for the FIELDID variable. |
| AVERAGE_VALUE  | Average value for the FIELDID variable.     |
| COUNT          | Number of acceptable observations used to calculate the average. The number of acceptable observations is the same in the calculation of all averages present in the formula of one estimator.     |

### outest_lr

Report on calculation of « beta » coefficients (linear regression estimators).

| Column         | Description |
| -------------- | ----------- |
| ESTIMID        | Identification number of the linear regression estimator based on the estimator input table order (begins with number 0).     |
| ALGORITHMNAME  | Name of the algorithm (pre-defined in procedure or custom defined).     |
| FIELDID        | name of the variable or regressor for which a beta coefficient is calculated.    |
| EXPONENT       | Regressor exponent. |
| PERIOD         | Current (C) or historical (H) period specific to the regressor.         |
| BETA_VALUE     | Value of « beta » coefficient associated with the regressor.            |
| COUNT          | Number of acceptable observations used to calculate the « beta » coefficients The number of acceptable observations is the same for all « beta » coefficients present in the formula of one estimator identification.     |

### outest_parm

Imputation statistics.

| Column         | Description |
| -------------- | ----------- |
| ESTIMID        | Identification number for the estimator defined based on the estimator input table order (begins with number 0).                   |
| ALGORITHMNAME  | Name of the algorithm (pre-defined in procedure or custom defined).     |
| FIELDID        | Field identifier.      |
| FTI            | Count of FTI flags on `instatus`, i.e., number of records requiring imputation.                                           |
| IMP            | Count of I-- flags on `outstatus`, i.e., number of records successfully imputed.                                |
| DIVISIONBYZERO | Number of failed imputations because calculation implies a division by 0.        |
| NEGATIVE       | Number of imputations discarded because imputed value is negative. (Not applicable if `accept_negative=True`.)    |

### outrand_err

Random error report.

| Column         | Description |
| -------------- | ----------- |
| ESTIMID        | Identification number for the estimator defined based on the estimator input table order (begins with number 0).                   |
| ALGORITHMNAME  | Name of the algorithm (pre-defined in procedure or custom defined).     |
| RECIPIENT      | Recipient identifier.                                                  |
| DONOR          | Donor identifier.                                                      |
| FIELDID        | Field identifier.      |
| RESIDUAL       | Calculated as difference between observed and estimated value, taken from donor record.   |
| RANDOMERROR    | If algorithm type is LR and variance is used then it is equal to: RESIDUAL* sqrt ((recipient variance ^ exponent) / (donor variance ^ exponent)), otherwise it is the same as RESIDUAL.    |
| ORIGINALVALUE  | Reported value of the variable before imputation.                       |
| IMPUTEDVALUE   | Value of variable after imputation.                       |

# prorate

### outreject

Records for which prorating could not be performed. 

| Column       | Description |
| ------------ | ----------- |
| \<unit_id\>  | Record identifier.     |
| NAME_ERROR   | Can take on one of seven possible values:<br><br> - `DECIMAL ERROR`: The user has specified fewer decimal places than exist in the adjusted total.<br> - `SCALING VALUE K GREATER THAN +1`: Acceptable interval for factor k is: -1 <= k <= +1 .<br> - `SCALING VALUE K LESS THAN -1`: Acceptable interval for factor k is: -1 <= k <= +1 .<br> - `NOTHING TO PRORATE`: No variables are left to prorate; they have all been eliminated because the modifier does not identify the variables as proratable (taking into account their status in `instatus` if the modifier is O or I) or their value is 0. .<br> - `OUT OF BOUNDS`: The rounded value divided by the original value is not within the interval defined by the bounds.<br> - `SUM OF PRORATABLE COLUMNS IS 0`: The factor k cannot be calculated because the weighted sum of the proratable columns is 0.<br> - `NEGATIVE VALUE IN DATA`: A variable has a negative value and `accept_negative=False`. |

# massimp

### outdonormap

Mapping of recipient/donor pairs.

| Column            | Description |
| ----------------- | ----------- |
| RECIPIENT         | Recipient identifier.     |
| DONOR             | Donor identifier.   |
| NUMBER_OF_ATTEMPTS | Can take on one of two possible values: (1) if the donor was selected by the nearest neighbour method or (0) if it was selected randomly.  |
| DONORLIMIT        | Reproduces the value of `n_limit` parameter (same value for all records).   |