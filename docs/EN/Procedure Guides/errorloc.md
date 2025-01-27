# Error Localization

* Execution: *banff.errorloc()*
* SDE function type: *Review, Selection*
* Input status flags: *FTI (optional)*
* Output status flags: *FTI*

## Description

For each record, selects the minimum number of variables to impute such that each observation can be made to pass all edits.

Consistency edits specify relationships between variables that a record must satisfy. When a record fails to satisfy these relationships, users must choose which variables to change, a process known as error localization. The Banff error localization procedure follows the Fellegi-Holt minimum-change principle, and uses an algorithm to select which variables to change. This process is performed independently on each record. Selected values are saved in the outstatus file, with a status flag of FTI (Field to impute).

This procedure requires a set of edits, consisting of linear equalities and inequalities, that must be internally consistent. The procedure will only perform error localization on the variables included in the list of edits. Any missing values from amongst the listed variables will automatically be selected for imputation.

By default, the procedure will minimize the number of variables to change. Users may also specify variable weights, in which case the procedure will minimize the weighted count of variables to change. For some records, the error localization problem may have multiple solutions (i.e., choices of variables) that satisfy the minimum-change principle; in this case one of the solutions is selected at random.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](/docs/EN/Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input Tables  | Description |
| ------------- | ----------- |
| indata        | Input statistical data. Mandatory. |
| instatus      | Input status file containing FTI status flags. <br><br> Values previously flagged as FTI will be prioritized for selection. This ensures the procedure doesn't select a field for imputation when a previously flagged value already solves the error localization problem, thereby minimizing the overall number of variables requiring imputation. |

| Output Table   | Description |
| -------------- | ----------- |
| outstatus      | Output status file identifying selected fields with FTI status flags, and their values. |
| outreject      | Output table containing records that failed error localization. <br><br> The outreject table contains records for which error localization could not be performed, either because they exceeded maximum allowable cardinality (error = "CARDINALITY EXCEEDED") or time per observation (error = "TIME EXCEEDED"). |

For details on the content of output tables, please see the [Output Tables](/docs/EN/output_tables.md) document.

## Parameters

| Parameter           | Python type  | Description | 
| ------------------- | -----------  | ----------- |
| unit_id             | str          | Identify key variable (unit identifier) on indata. Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| edits               | str          | List of consistency edits. Mandatory. <br><br> Example: `"Revenue - Expenses = Profit; Revenue >= 0; Expenses >= 0;"` |
| weights             | str          | Specify the error localization weights. <br><br> Weights can be used to influence which variables are selected for subsequent treatment. When specified, the error procedures minimizes the weighted count of variables to change, such that the record in question may be altered to satisfy the consistency edits. By default, error localization assigns a weight of one to each variable. To assign a different weight, specify `variable = value` where *variable* is one of the variable specified in the `edits`, and *value* is a number greater than zero. Multiple weights can be assigned in this way, separated by a semi-colon. <br><br> Example: `weights = "revenue = 1.5; expenses = 0.8"` |
| accept_negative     | bool         | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| cardinality         | float | Specify the maximum cardinality. <br><br> Cardinality refers to the weighted number of variables requiring imputation, which can vary by record. For records with high cardinality, the error localization problem can take a long time to solve. Specifying a maximum cardinality can improve processing time but may result in the procedure failing to solve the error localization problem for all records; these will be identified in the outreject table. |
| time_per_obs        | float | Specify the maximum processing time allowed per observation. |
| seed                | float | Specify the root for the random number generator. <br><br> The seed is used to ensure consistent results from one run to the next. If not specified or specified as a non-positive value, a random number is generated by the procedure. |
| rand_num_var        | str          | Specify a random number variable to be used when having to make a choice during error localization. <br><br> The random number variable must exist on indata; it must be numeric and values must be non-negative real numbers less than or equal to 1. The values should be unique for each record. |
| by                  | str          | Variable(s) used to partition indata into by-groups for independent processing. <br><br> Because error localization is performed on each record independently, this parameter has no effect. Future versions may use this parameter to improve the processing efficiency of the procedure.<br><br> Example: `by = "province industry"` |
| prefill_by_vars     | bool         | Add by-group variable(s) to input status file(s) to improve performance. Default=True. |
| presort             | bool         | Sort input tables before processing, according to procedure requirements. Default=True. |
| no_by_stats         | bool         | Reduce log output by suppressing by-group specific messages. Default=False. |
| display_level       | int | Value (0 or 1) to request detail output to the log in relation to the random number variable. Default=0. |

## Notes

### Multiple equivalent solutions

In some cases, there may be multiple solutions that solve the error localization problem. For example, for a record failing the edit `"Profit = Revenue - Expenses;"`, changing any one variable would be a valid solution. When this occurs, the procedure selects one of these solutions at random.

For development or testing purposes, users may wish to produce consistent results over multiple runs of the procedure, and may do so using the `seed` or `rand_num_var` parameters. Both ensure that the same solutions will be selected from one run to the next, if executed on the same set of inputs. Note that if both `seed` and `rand_num_var` are specified, `seed` is ignored. If neither is specified, the system generates a default seed.