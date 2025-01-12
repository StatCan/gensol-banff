# Deterministic Imputation

* Execution: *banff.deterministic()*
* SDE function type: *Treatment*
* Input status flags: *FTI (required)*
* Output status flags: *IDE*

## Description

Performs imputation when only one combination of values permits the record to pass the set of edits.

The deterministic imputation procedure analyzes each field previously identified as requiring imputation to determine if there is only one possible value which would satisfy the original edits. If such a value is found, it is imputed during execution of this procedure. This method can also be referred to as deductive imputation, since a missing or inconsistent value can be deduced with certainty based upon other fields of the same record.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](/docs/EN/Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input Table   | Description |
| ------------- | ----------- |
| indata        | Input statistical data. Mandatory. |
| instatus      | Input status file containing FTI status flags. Mandatory. <br><br> This procedure uses FTI flags to identify fields requiring deterministic imputation.  |

| Output Table | Description                                                       |
| -------------| ----------------------------------------------------------------- |
| outdata      | Output statistical table containing imputed data. <br><br> Note that outdata will only contain successfully imputed records and affected fields. Users should update indata with the values from outdata before continuing the data editing process.       |
| outstatus    | Output status file identifying imputed fields with IDE status flags, and their values after imputation.  |

For details on the content of output tables, please see the (TBC).

## Parameters

| Parameter       | Python type | Description                 | 
| ----------------| ------------| --------------------------- |
| unit_id         | str         | Identify key variable (unit identifier) on indata. Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| edits           | str         | List of consistency edits. Mandatory. <br><br> Example: `"Revenue - Expenses = Profit; Revenue >= 0; Expenses >= 0;"` |
| accept_negative | bool        | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| by              | str         | Variable(s) used to partition indata into by-groups for independent processing. <br><br> Deterministic imputation will be the same with and without by-group use. <br><br> Example: `by = "province industry"` |
| prefill_by_vars | bool        |  Adds by-group variable(s) to input status file to improve performance. Default=True.  |
| presort         | bool        | Sorts input tables before processing according to procedure requirements. Default=True. |
| no_by_stats     | bool        | Reduces log output by suppressing by-group specific messages. Default=False. |