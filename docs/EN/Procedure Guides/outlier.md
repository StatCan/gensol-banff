# Outlier Detection

* Execution: *banff.outlier()*
* SDE function type: *Review, Selection*
* Input status flags: *None*
* Output status flags: *FTI, FTE*

## Description

Identifies outlying observations using Hidiroglou-Berthelot or Sigma-Gap methods.

This procedure offers two methods of univariate outlier detection. The Hidiroglou-Berthelot (HB) method selects outliers based on their distance from the median, relative to the interquartile distance. The Sigma-Gap (SG) method sorts the data in ascending order and searches for significant gaps (relative to the standard deviation) between consecutive values, selecting all subsequent values as outliers. Both methods can detect two types of outliers, which are flagged on the `outstatus` file:

- Values that are extreme enough to be considered errors. These values are flagged as *fields to impute (FTI)* so they can be imputed in a subsequent step. 
- Values that are not extreme enough to be considered errors, but are sufficiently unusual to be deemed *fields to exclude (FTE)* by subsequent imputation procedures such as `donorimp` and `estimator`. (This flag can also be useful during weighting and robust estimation.)

For both procedures, users must specify either an imputation or exclusion threshold; no default value is provided.

Additional features of the procedure:

- Users can run outlier detection on multiple variables (`var`) in one call.
- Users can also run outlier detection on ratios of variables. In this case, only the numerators (`var`) are flagged on `outstatus`. For the denominator, users may select auxiliary variables (`with_var`) from the current period (`indata`) or from historical data (`indata_hist`).
- Outlier detection can be performed to the right, left, or on both sides (`side`).
- Outlier detection can be performed within by-groups (`by`), with a user-specified minimum number of observations (`min_obs`) required to perform outlier detection.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](/docs/EN/Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.    

| Input Table   | Description |
| ------------- | ----------- |
| indata        | Input statistical data. Mandatory. |
| indata_hist   | Input historical data. <br><br> Records on indata are linked with records on indata_hist by the `unit_id` variable. Records that appear on indata_hist but not on indata are dropped before processing, as are records with missing values for `unit_id`. |

| Output Table      | Description                                                                           |
| ------------------- | ------------------------------------------------------------------------------------- |
| outstatus           | Contains the status of the fields (FTE/FTI) identified as outliers and their values.  |
| outstatus_detailed  | Detailed status for the outliers (ODER/ODEL/ODIR/ODIL).<br><br> Detailed status indicates whether the outlier falls outside the exclusion interval on the right (ODER) or on the left (ODEL). If it concerns an FTI outlier, the detailed status distinguishes between an outlier falling outside the imputation interval on the right (ODIR) and on the left (ODIL).<br><br> It will contain more information (imputation and exclusions bounds, current and auxiliary values, ..) with the parameter `outlier_stats=True`.                               |
| outsummary          | Outlier summary information such as observation counts and acceptance interval bounds.|

For details on the content of output tables, please see the (TBC).

## Parameters

| Parameter       | Python type  | Description                 | 
| ----------------| -------------| --------------------------- |
| unit_id         | str          | Identify key variable (unit identifier) on indata and indata_hist. Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| method          | str          | Method to be used to detect outlying observations ('CURRENT', 'RATIO', 'HISTORIC' or 'SIGMAGAP'). Mandatory.<br><br> SG method is applied when `method='SIGMAGAP'`, otherwise HB method is used; please see the notes below for details.|
| var             | str          | Variables(s) for which to find outliers. <br><br> `var` becomes mandatory when historical or auxiliary variables are used. If they are not used, `var` can be omitted; in which case all numeric variables in indata (except by-group variables) will be processed. <br><br> Example: `var = "Revenue Expenses"` |
| with_var        | str          | Historical or auxiliary variables.<br><br> The number of variables in the `with_var` list must be the same as the `var` list. If the table indata_hist is used, then `with_var` variables are read from it, otherwise they are read from indata. If `var` and `with_var` are identical (i.e. each variable in `var` has a historical variable with the same name on indata_hist), then `with_var` can be omitted. |
| weight          | str          | Variable to be used for weighting.<br><br> The weight is at the record level and must contain a value for each record. `weight` will be multiplied by the values of the variables for which one wants to detect outliers. Any record(s) in the input table with a missing weight will be dropped from the outlier detection.|
| exclude_where_indata | str     | Expression in SQL syntax to exclude observations from the outlier detection.|
| mii             | float        | HB multiplier for imputation interval (positive).<br><br> `mii` controls the width of the imputation interval. A higher multiplier value for the imputation interval will lead to a lower number of detected outliers to impute. `mii` becomes mandatory for HB if `mei` is not specified. |
| mei             | float        | HB Multiplier for exclusion interval (positive).<br><br> `mei` controls the width of the exclusion interval. A higher multiplier value for the exclusion interval will lead to a lower number of detected outliers to exclude. `mei` becomes mandatory for HB if `mii` is not specified. |
| mdm             | float        | HB minimum distance multiplier (positive). Default=0.05.<br><br> `mdm` refers to the minimum interquartile distance required to calculate intervals. |
| exponent        | float        | HB exponent for a ratio or historical trend (between 0 and 1). Default=0. |
| min_obs         | integer      | Minimum number of observations that must exist in the input table or in a by-group (positive). Default=3 for HB, 5 for SG.<br><br> `min_obs` >= 3 for HB; `min_obs` >= 5 for SG.<br><br> No outlier will be detected if the number of records is equal to 3 for HB **(Contradiction!)**. A minimum of 10 observations per by-group is recommended; outlier detection results for by-groups less then 10 observations should be used with caution. |
| side            | str          | Side ('LEFT', 'RIGHT', or 'BOTH') of the ordered data to be used for detecting outliers. Default='BOTH'. |
| start_centile   | float        | SG centile to be used to determine the starting point (between 0 and 100). Default=75 for 'side="BOTH"', 0 otherwise. <br><br> The centile must be greater than or equal to 0 and less than 100 when `side='LEFT'` or `side='RIGHT'`. The centile must be greater than or equal to 50 and less than 100 when `side='BOTH'`. |
| beta_i          | float        | SG multiplier for imputation interval (non-negative).<br><br> 0<`beta_e`<`beta_i`. `beta_i` becomes mandatory for SG if `beta_e` is not specified.|
| beta_e          | float        | SG multiplier for exclusion interval (non-negative).<br><br> 0<`beta_e`<`beta_i`. `beta_e` becomes mandatory for SG if `beta_i` is not specified. |
| sigma           | str          | SG type of deviation ('MAD' or 'STD') to be calculated. Default='MAD'.<br><br> MAD: median absolute value; STD: classical standard deviation. |
| outlier_stats   | bool         | Add more information to outstatus_detailed output table, including imputation and exclusion interval bounds. Default=False.|
| accept_zero     | bool         | Treat zero values as valid. Default=False in the presence of historical or auxiliary variables, True otherwise. |
| acceptnegative  | bool         | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| by              | str          | Variable(s) used to partition indata into by-groups for independent processing. <br><br> Outlier detection is performed on each by-group separately. <br><br> Example: `by = "province industry"` |
| prefill_by_vars | bool         | Adds by-group variable(s) to input status file(s) to improve performance. Default=True.<br><br>**This parameter can be specified even if outlier doesn't take any instatus?**  |
| presort         | bool         | Sorts input tables before processing, according to procedure requirements. Default=True. |
| no_by_stats     | bool         | Reduces log output by suppressing by-group specific messages. Default=False. |

## Notes

### Imputation and exclusion thresholds

The identification of outliers, either for imputation or exclusion, requires user-specified thresholds. There are no default or suggested values; these depend entirely on the user's criteria for what is considered extreme. At least one threshold must be specified for each method: `mii` or `mei` for the HB method, and `beta_i` or `beta_e` for the SG method. 

### Specifying outlier detection methods

The `outlier` method offers a variety of ways to specify outlier detection for both single variables and ratios, with and without historical data. These depend on the combination of the `method`, `var`, and `with_var` parameters, and whether or not `indata_hist` is provided:

| To do this:                              | `method` parameter | `with_var` parameter | `indata_hist` provided |
| ---------------------------------------- | ------------------ | -------------------- | ---------------------- |
| Apply HB method to current data          | `"CURRENT"`        | No                   | No                     |
| Apply HB method to ratio of current data | `"RATIO"`          | Yes                  | No                     |
| Apply HB method to historical trend      | `"HISTORICAL"`     | Yes                  | Yes                    |
| Apply SG method to current data          | `"SIGMAGAP"`       | No                   | No                     |
| Apply SG method to ratio of current data | `"SIGMAGAP"`       | Yes                  | No                     |
| Apply SG method to historical trend      | `"SIGMAGAP"`       | Yes                  | Yes                    |

The outlier detection method is always applied to the variables listed in `var`. (If not specified, outlier detection will be applied to all numerical variables on `indata` except those that are listed in the `by` parameter.) To apply outlier detection to a ratio of variables, specify the list of numerators by `var` and denominators by `with_var`; the procedure will run through the ordered pairs one-by-one. (Note that it is possible to list the same variables multiple times in both the `var` and `with_var` lists.)

In `indata_hist` is provided, the procedure will use current data (from `indata`) in the numerator and historical data (from `indata_hist`) in the denominator. If `with_var` is not specified, the procedure will use the same variable in both the numerator and denominator.

