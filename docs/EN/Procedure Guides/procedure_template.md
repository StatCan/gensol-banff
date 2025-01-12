# Error Localization

* Execution: *banff.prorate()*
* SDE function types: *Review, Selection, Treatment*
* Input status flags: *FTI (required), I-- (optional)*
* Output status flags: *IPR*

## Description

Single sentence description to be used in the docstring.

Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines. Second paragraph gives more details, if necessary. Can span multiple lines.

Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine. Third, fourth paragraphs, etc. are also fine.

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input Dataset | Description |
| ------------- | ----------- | 
| name1         | Single line description for docstring. Mandatory. |
| name2         | Single line description for docstring. <br><br> Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. |
| indata        | Input statistical data. Mandatory.
| indata_hist   | Input historical data. <br><br> Records on `indata` are linked with records on `indata_hist` by the `unit_id` variable. Records that appear on `indata_hist` but not on `indata` are dropped before processing, as are records with missing values for `unit_id`. |
| instatus      | Input status file containing (list) status flags. (Mandatory.) <br><br> Description of the relevant input status flags. |
| instatus_hist | Input historical status file containing (list) status flags. <br><br> Description of the relevant input status flags. (Only if necessary.) |

| Output Dataset | Description |
| -------------- |------------ |
| name3          | Single line description for docstring. |
| name4          | Single line description for docstring. <br><br> Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. |
| outdata        | Output statistical table containing imputed data. <br><br> Note that `outdata` will only contain successfully imputed records and affected fields. Users should update `indata` with the values from `outdata` before continuing the data editing process. |
| outstatus      | Output status file identifying imputed/selected fields with (list) status flags, and their values (after imputation). |
| outreject      | Output table containing records that failed (procedure name). <br><br> Brief description of why records may be rejected.
| outdonor_map   | Output table of recipient-donor pairs, for successfully imputed records. | 

For details on the content of output tables, please see the (TBC).

## Parameters

| Parameter           | Python type | Description | 
| ------------------- | ----------- | ----------- |
| unit_id             | str         | Identify key variable (unit identifier) on indata (and indata_hist). Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| edits               | str         | List of consistency edits. Mandatory. <br><br> Example: `"Revenue - Expenses = Profit; Revenue >= 0; Expenses >= 0;"` |
| acceptnegative      | bool        | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| by                  | Variable(s) used to partition indata into by-groups for independent processing. <br><br> Procedure specific explanation (if necessary). <br><br> Example: `by = "province industry"` |
| prefill_by_vars     | Adds by-group variable(s) to input status file(s) to improve performance. Default=True.  |
| presort             | Sorts input tables before processing, according to procedure requirements. Default=True. |
| no_by_stats         | Reduces log output by suppressing by-group specific messages. Default=False. |
| display_level       | TBC | TBC |

| Parameter           | Python type | Status      | Description | 
| ------------------- | ----------- | ----------- | ----------- |
| Parameter1          | str         | Mandatory   | Single line for docstrings. |
| Parameter2          | str         | Optional    | Single line for docstrings. <br><br> This parameter is optional. |
| Parameter3          | integer     | Mandatory   | Single line for docstrings. <br><br> Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details.|
| Parameter4          | BOOL        | Optional, default = FALSE | Single line for docstrings. <br><br> Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. Additional lines can be used for more details. |
| Parameter5          | BOOL        | Optional, default = TRUE |Single line for docstrings. |

## Notes

Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs.

Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs.

Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs. Additional notes here. These can be arranged in paragraphs.

In addition, or alternatively, notes could be arranged in bullets:
* Note 1
* Note 2
* Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. Note 3 takes up multiple lines. 

