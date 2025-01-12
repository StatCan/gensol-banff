# Edit Statistics

* Execution: *banff.editstats()*
* SDE function type: *Review*
* Input status flags: *None*
* Output status flags: *None*

## Description

Produces edit summary statistics tables on records that pass, miss or fail each consistency edit.

This procedure applies a group of `edits` to statistical data and determines if each record passes, misses (due to missing values) or fails each edit. Resulting diagnostics are saved to five output tables, and can be used to fine-tune the group of edits, estimate the resources required for later procedures, or to evaluate the effects of imputation. Note that this procedure only reviews the data, producing summary statistics; use `errorloc` (with the same set of edits) to select records and fields for further treatment.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](/docs/EN/Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input Table | Description |
| ----------- | ----------- |
| indata      | Input statistical data. Mandatory. |

| Output Table      | Description                                           |
| ------------------| ----------------------------------------------------- |
| outedits_reduced  | Minimal set of edits.                                 |
| outedit_status    | Number of records which pass, miss or fail each edit. |
| outk_edits_status | Distribution of records which pass, miss or fail a given number of edits.    |
| outglobal_status  | Number of records with pass, miss or fail overall record status.        |
| outedit_applic    | Number of times each variable is involved in an edit which passes, misses or fails. |
| outvars_role      | Number of times each variable contributes to the overall record status. |

For details on the content of output tables, please see the (TBC).

## Parameters

| Parameter       | Python type | Description                 | 
| ----------------| ------------| --------------------------- |
| edits           | str         | List of consistency edits. Mandatory. <br><br> Example: `"Revenue - Expenses = Profit; Revenue >= 0; Expenses >= 0;`        |
| accept_negative | bool        | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables.  |
| by              | str          | Variable(s) used to partition indata into by-groups for independent processing. <br><br> Output tables will be generated for each by-group independently. <br><br> Example: `by = "province industry"` |
| presort         | bool         | Sorts input tables before processing according to procedure requirements. Default=True. |