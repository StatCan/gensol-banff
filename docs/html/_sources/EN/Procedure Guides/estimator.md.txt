# Estimator Imputation

* Execution: *banff.estimator()*
* SDE function type: *Treatment*
* Input status flags: *FTI (required), FTE (optional), I--(optional)*
* Output status flags: *I--* (exact code depends on specified algorithm)

## Description

Performs imputation using estimation functions and/or linear regression estimators.

The estimator procedure offers imputation methods such as mean, ratio and regression imputation using current (`indata`) and/or historical data (`indata_hist`) for the variable to impute and potentially auxiliary variables. Users may choose from twenty (20) pre-defined imputation estimator algorithms that are included in the procedure, or define their own custom algorithms. 

Only fields with an FTI (Field to Impute) from the `instatus` file are imputed. Fields with FTE (Field to Excluded) or I-- (Imputed Field) flags are excluded from the imputation model. (Note that this does not include the flag IDE, which indicates deterministic imputation.)

Estimator or linear regression parameters (e.g. means or regression coefficients) can be calculated on all records or on a particular subset of acceptable records. The restriction of the acceptable records can be applied using an exclusion parameter or by specifying by-groups imputation.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](../Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input Table   | Description |
| ------------- | ----------- |
| indata        | Input statistical data. Mandatory. <br><br>Records with missing values for one or more variables specified in the estimator specifications without the status 'FTI' for these fields in instatus will not be processed.|
| instatus      | Input status file containing FTI, FTE and I-- status flags. Mandatory.<br><br> Values flagged as FTI are identified as requiring imputation. outliers (FTE) and previously imputed values (I--) can be excluded from the set of acceptable records. Values with an IDE status are considered as having original values (not previously imputed).|
| inestimator     | Estimator specifications table. Mandatory.<br><br> At least one imputation method is defined for every field requiring imputation. |
| inalgorithm     | User defined algorithms table.<br><br> User defined algorithm cannot have the same name as a pre-defined one.|
| indata_hist   | Input historical data. <br><br> Records on indata are linked with records on indata_hist by the `unit_id` variable. Records that appear on indata_hist but not on indata are dropped before processing, as are records with missing values for `unit_id`.   |
| instatus_hist | Input historical status file containing FTI, FTE and I-- status flags.<br><br> If instatus_hist is not specified but the indata_hist is, then all historical values will be considered valid values. Values with 'FTI' flag will be excluded from the set of acceptable records. |

| Output Table   | Description                  |
| ---------------| ---------------------------- |
| outdata        | Output statistical table containing imputed data. <br><br> Note that outdata will only contain successfully imputed records and affected fields. Users should update indata with the values from outdata before continuing the data editing process. |
| outstatus      | Output status file identifying imputed fields with I-- status flags, and their values after imputation.<br><br> The status for the imputed fields will start with a leading "I", followed by the status associated with the algorithm used in imputation. |
| outacceptable  | Report on acceptable observations retained to calculate parameters for each estimator. |
| outest_ef      | Report on calculation of averages for estimator functions.  |
| outest_lr      | Report on calculation of « beta » coefficients for linear regression estimators (type LR). |
| outest_parm    | Report on imputation statistics by estimator.  |
| outrand_err    | Random error report when a random error is added to the imputed variable.  |

For details on the content of output tables, please see the [Output Tables](../output_tables.md) document.

## Parameters

| Parameter                 | Python type  | Description                 | 
| ------------------------- | -------------| --------------------------- |
| unit_id                   | str          | Identify key variable (unit identifier) on indata and indata_hist. Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| data_excl_var             | str          | Variable of the input table used to exclude observations from the set of acceptable observations.<br><br> When the value of the variable identified by `data_excl_var` is `'E'` for an observation, then it will not be used to compute parameters. `data_excl_var` cannot be combined with `exclude_where_indata`|
| hist_excl_var             | str          | Variable of the historical input table used to exclude historical observations from the set of acceptable observations.<br><br> When the value of the variable identified by `hist_excl_var` is `'E'` for an observation, then it will not be used to compute parameters. `hist_excl_var` cannot be combined with `exclude_where_indata_hist`.|
| exclude_where_indata      | str          | Exclusion expression using SQL syntax to specify which observations to exclude from the set of acceptable observations.<br><br> Imputation will still be carried out on fields that are FTI even if they satisfy the expression in `exclude_where_indata`. `exclude_where_indata` cannot be combined with `data_excl_var`.|
| exclude_where_indata_hist | str          | Exclusion expression using SQL syntax to specify which historical observations to exclude from the set of acceptable observations.<br><br> `exclude_where_indata_hist` cannot be combined with `hist_excl_var `.|
| seed                      | flo          | Specify the root for the random number generator. <br><br> The seed is used to ensure consistent results from one run to the next. If not specified or specified as a non-positive value, a random number is generated by the procedure. |
| verify_specs              | bool         | Estimator specifications verified without running the imputation.<br><br> Verifies the specifications, calculates parameters and stops after printing them in the log window.|
| accept_negative           | bool         | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| by                        | str          | Variable(s) used to partition indata into by-groups for independent processing. <br><br> In estimator, by-groups can also be seen as imputation classes. <br><br> Example: `by = "province industry"` |
| prefill_by_vars           | bool         | Add by-group variable(s) to input status file(s) to improve performance. Default=True.  |
| presort                   | bool         | Sort input tables before processing, according to procedure requirements. Default=True. |
| no_by_stats               | bool         | Reduce log output by suppressing by-group specific messages. Default=False. |


## Notes

The `inalgorithm` and `inestimator` tables are used to define the models (mean imputation, linear regression, etc.) used for imputation and to specify certain parameters and options. While the inestimator table is mandatory, the inalgorithm table is only required when using a custom algorithm instead of one of the 20 pre-defined algorithms available in the procedure. 

### Inestimator

The `inestimator` table needs te be prepared before running the procedure. It is used to specify the algorithm (i.e., model) to use for imputation, the variables to impute, and other parameters. The specified algorithm can either pre-defined or user-defined in the algorithm table. Multiple algorithms can be specified in this table, and will be processed in the order they appear. Note that the same variable to impute can be specified for multiple algorithms; in this case, the first algorithm will be applied, but if it fails to impute a value requiring imputation, the next algorithm will be applied, and so on.

The following describes the variables that must appear in the inestimator table. All columns are mandatory.

|  Column          | Description                                                                 |
| ---------------- | --------------------------------------------------------------------------- |
| fieldid          | Name of the variable to be imputed. |
| algorithmname    | Name of the algorithm used to impute the variable. <br><br> It can be a pre-defined inalgorithm or one found in the algorithm table.                     |
| auxvariables     | Comma separated list of auxiliary variable names on indata or indata_hist.<br><br> These names are used to replace the placeholders in the formula variable of the inalgorithm table or in the formula of the pre-defined algorithms. The first variable name will replace aux1 in the formula variable, the second, aux2 and so on.   |
| weightvariable   | Name of the weight variable.<br><br> It is optional if the algorithm calculates a parameter, otherwise it must be left blank. For "LR" algorithms, weightvariable must exist on indata, but for "EF" algorithms it can exist on indata or indata_hist depending on the period of the average requested.<br><br>          |
| countcriteria    | A positive integer indicating the minimum number of acceptable observations needed in the current by-group.<br><br> It is mandatory when an algorithm is computing a parameter. For algorithms not calculating parameters, it must be left blank.          |
| percentcriteria  | Minimum percentage of acceptable observations needed in the current by-group (between 0 and 100).<br><br> Mandatory when an algorithm is computing a parameter. For algorithms not calculating parameters, it must be left blank.        |
| variancevariable | Name of the variance variable.<br><br> It is optional if the algorithm calculates a parameter, otherwise it must be left blank. It must exist on the indata or indata_hist table depending on the value of `varianceperiod`.         |
| varianceperiod   | Period of the variance ('C' for current or 'H' for historical).          |
| varianceexponent | A number indicating the power of the variance.          |
| excludeimputed   | 'Y' (yes) or 'N' (no) to indicate whether previously imputed values should be excluded from the set of acceptable observations.<br><br> Mandatory when an algorithm is computing a parameter. For algorithms not calculating parameters, it must be left blank.          |
| excludeoutliers  | 'Y' (yes) or 'N' (no) to indicate whether observations with an FTE status should be excluded from the set of acceptable observations.<br><br> Mandatory when an algorithm is computing a parameter. For algorithms not calculating parameters, it must be left blank.          |
| randomerror      | 'Y' (yes) or 'N' (no) to indicate whether a random error is added to the imputed variable.<br><br> A warning will be printed if less than 5 observations are available for the random choice of the error.         |

### Inalgorithm

In addition to the 20 pre-defined algorithms in the procedure, custom defined algorithms can be defined by the user in the `inalgorithm` table. These user-defined algorithms consist of two type:

- Estimator function (EF): mathematical expression involving constants, current and/or historical values of some variables of the record, and current and/or historical averages of some variables, those averages being calculated from acceptable records. The mathematical expressions may include parentheses and the arithmetic operators addition (+), 
subtraction (-), multiplication (*), division (/), and exponentiation (^).
- Linear regression (LR): Regression imputation consists of imputing a variable $y$<sub>$i$</sub> by a linear regression estimation like

$$
\hat{y_i} = \hat{\beta_0} + \hat{\beta_1} x_{i_1 T_1}^{p_1} + \hat{\beta_2} x_{i_2 T_2}^{p_2} + ... + \hat{\beta_m} x_{i_m T_m}^{p_m} + \hat{\epsilon_i}  
$$

where : $T$<sub>$j$</sub> refer to current or historical periods, and $p$<sub>$j$</sub> are exponents. The variable $y$<sub>$j$</sub> being imputed is the dependent variable in the model, and the auxiliary variables $x$<sub>$ij$</sub> are the independent variables, or regressors. $\hat\beta$<sub>$j$</sub> are the regression coefficients, the values of which are solved for 
by using the method of least squares. $\hat\epsilon$<sub>$i$</sub> is a random error term, which can be added to the model to introduce some variability into the fitted values of the $y$<sub>$i$</sub>. 
Note that $\beta$<sub>$0$</sub>, which is the intercept in the regression line, is optional and can be omitted from the model.

The following table describes the variables that must appear in the inalgorithm table. All columns but `description` are mandatory.  

| Column            | Description                                                                         |
| ------------------| ----------------------------------------------------------------------------------- |
| algorithmname     | Name of the algorithm.                                                              |
| type              | Type of the algorithm: 'EF' for estimator function and 'LR' for linear regression. |
| status            | 1 to 3 character string that will be inserted in the outstatus table (after adding the prefix "I") when a variable is estimated by this algorithm.      |
| formula           | The algorithm formula. <br><br> Only placeholders like `aux1` and `fieldid` can be used in formula. The placeholders must have the format `aux1(period, aggregation)` where:<br><br> - period: `c` for current and `h` for historical.<br><br> - aggregation: `v` for using the variable's value of the observation and `a` for using the average of the variable based on acceptable observations.                                             |
| description       | Text to describe the algorithm.                                                     |   