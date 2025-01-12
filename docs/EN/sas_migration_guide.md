# Banff Package SAS Migration Guide

## In this guide

The target audience for this guide is users of SAS based Banff 2.08.002 or earlier who are migrating to Python based Banff 3.x.  
It summarizes information on the changes made in version 3, including new names for parameters and tables, as well as examples of how SAS programs which call Banff procedures can be converted into equivalent Python programs.  

This document is not intended for new Banff users as it does not provide an extensive overview of the Banff Procedures.  Please refer to the [User Guide](/docs/EN/user_guide.md) for complete details on the use of each procedure, its parameters, and tables.  

## Table of contents

- [Available Procedures](#available-procedures)
- [Procedure Parameters](#procedure-parameters)
  - [Table of Procedure Parameters and Types](#table-of-procedure-parameters-and-types)
  - [New Procedure Options](#new-procedure-options)
- [Procedure Tables](#procedure-tables)
  - [Table of Procedure Table Identifiers](#table-of-procedure-table-identifiers)
  - [Table Changes](#table-changes)
- [Table Specification](#table-specification)
- [Notable Procedure Changes](#notable-procedure-changes)
  - [Input Table Changes](#input-table-changes)
  - [Output Table Changes](#output-table-changes)
- [Other Python Runtime Options](#other-python-runtime-options)
- [Performance Considerations](#performance-considerations)
- [Errors and Exceptions](#errors-and-exceptions)
- [Utility Functions](#utility-functions)

## Available Procedures

For a list of the available procedures, see the [User Guide](/docs/EN/user_guide.md#table-of-procedures).  

Each procedure has been converted by taking the SAS-dependent Banff 2.08.002 procedure source code and modifying it to produce an open-source based procedure which is *"wrapped"* in a Python package.  The underlying mathematical computations remain unaltered.  

Due to differences between SAS and Python, users must adapt *how* they specify parameters and tables; the sets of parameters and tables remains largely unchanged (although most [*parameter identifiers*](#table-of-procedure-parameters-and-types) and [*table identifiers*](#table-of-procedure-table-identifiers) have changed).  

## Procedure Parameters

Many parameter names have changed to better follow common Python naming conventions.

The identifiers used in SAS programs correspond to the following identifiers and Python types:

### Table of Procedure Parameters and Types

|SAS Identifier|Python Identifer|Python Type|Note|
|--|--|--|--|
|`ACCEPTNEGATIVE`|`accept_negative`|`bool`||
|`ACCEPTZERO`|`accept_zero`|`bool`||
|`BETAE`|`beta_e`|`int` or `float`||
|`BETAI`|`beta_i`|`int` or `float`||
|`BOUNDSTAT`|`outlier_stats`|`bool`|**MODIFIED**: now relates to contents of [`outstatus_detailed` table](#changes-to-outlier-outstatus)|
|`BY`|`by`|`str`||
|`CARDINALITY`|`cardinality`|`int` or `float`||
|`DATAEXCLVAR`|`data_excl_var`|`str`||
|`DECIMAL`|`decimal`|`int` or `float`||
|`DISPLAYLEVEL`|`display_level`|`int` or `float`||
|`EDITS`|`edits`|`str`||
|`ELIGDON`|`eligdon`|`str`||
|`EXPONENT`|`exponent`|`int` or `float`||
|`EXTREMAL`|`extremal`|`int` or `float`||
|`HISTEXCLVAR`|`hist_excl_var`|`str`||
|`ID`|`unit_id`|`str`||
|`IMPLY`|`imply`|`int` or `float`||
|`LOWERBOUND`|`lower_bound`|`int` or `float`||
|~~`MATCHFIELDSTAT`~~|~~`match_field_stat`~~|~~`bool`~~|**DEPRECATED**: use [`outmatching_fields` output table](#changes-to-donor-imputation-outstatus) instead|
|`MAXCARDINALITY`|`extremal`|`int` or `float`||
|`MAXIMPLIEDEDITS`|`imply`|`int` or `float`||
|`MDM`|`mdm`|`int` or `float`||
|`MEI`|`mei`|`int` or `float`||
|`METHOD`|`method`|`str`||
|`MII`|`mii`|`int` or `float`||
|`MINDONORS`|`min_donors`|`int` or `float`||
|`MINOBS`|`min_obs`|`int` or `float`||
|`MODIFIER`|`modifier`|`str`||
|`MRL`|`mrl`|`int` or `float`||
|`MUSTIMPUTE`|`must_impute`|`str`||
|`MUSTMATCH`|`must_match`|`str`||
|`N`|`n`|`int` or `float`||
|`NLIMIT`|`n_limit`|`int` or `float`||
|`NOBYSTATS`|`no_by_stats`|`bool`||
|`OUTLIERSTAT`|`outlier_stats`|`bool`|**MODIFIED**: now relates to contents of [`outstatus_detailed` table](#changes-to-outlier-outstatus)|
|`PCENTDONORS`|`percent_donors`|`int` or `float`||
|`POSTEDITS`|`post_edits`|`str`||
|`RANDNUMVAR`|`rand_num_var`|`str`||
|`RANDOM`|`random`|`bool`||
|~~`REJECTNEGATIVE`~~|~~`reject_negative`~~|~~`bool`~~|**DEPRECATED**: specify `accept_negative=False` instead|
|~~`REJECTZERO`~~|~~`reject_zero`~~|~~`bool`~~|**DEPRECATED**: specify `accept_zero=False` instead|
|`SEED`|`seed`|`int` or `float`||
|`SIDE`|`side`|`str`||
|`SIGMA`|`sigma`|`str`||
|`STARTCENTILE`|`start_centile`|`int` or `float`||
|`TIMEPEROBS`|`time_per_obs`|`int` or `float`||
|`UPPERBOUND`|`upper_bound`|`int` or `float`||
|`VAR`|`var`|`str`||
|`VERIFYEDITS`|`verify_edits`|`bool`||
|`VERIFYSPECS`|`verify_specs`|`bool`||
|`WEIGHT`|`weight`|`str`||
|`WEIGHTS`|`weights`|`str`||
|`WITH`|`with_var`|`str`||

### Example of Parameter Specification in Python

The following code demonstrates how to specify different *Python types* associated with some common parameters.  

```python
foo = banff.donorimp(
    min_donors=2,
    percent_donors=0.1,
    accept_negative=True,
    edits="""x1>=-5; 
    x1<=15; 
    x2>=30; 
    x1+x2<=50;""",
    by="province city",
    unit_id='IDENT',
    trace=True,
    # ... etc. (tables)
)
```

> *Sample SAS Parameter Speficiation*
>
> ```sas
> proc donorimputation
>     /* etc. (tables) ... */
>     mindonors=2
>     pcentdonors=0.1
>     acceptnegative
>     edits="x1>=-5; 
>     x1<=15; 
>     x2>=30; 
>     x1+x2<=50;"
>     ;
>     by province city;
>     id IDENT;
> run;
> ```

|parameter|note|
|--|--|
|`unit_id`|A single variable name|
|`by`|a list of 0 or more space-separated variable names|
|`cardinality`|excepts a number, see user guide for advice|
|`time_per_obs`|expects a number, see user guide for advice|
|`accept_negative`|boolean: `True`-> specified, \<*not-specified*\> -> not specified|
|`edits`| wrap multi-line strings with triple quotes (`"""edit>=string"""`)|

### New Procedure Options

New options include

- [`exclude_where_indata`](#exclude_where_indata-option)
- [`exclude_where_indata_hist`](#exclude_where_indata_hist-option)
- [`prefill_by_vars`](#prefill_by_vars-option)
- [`presort`](#presort-option)
- [`trace`](#trace-option)

#### `exclude_where_indata` Option

This option excludes records based on a user-specified SQL expression.  See the procedure-specific documentation for details

- [donorimp](/docs/EN/Procedure%20Guides/donorimp.md#parameters)
- [estimator](/docs/EN/Procedure%20Guides/estimator.md#parameters)
- [outlier](/docs/EN/Procedure%20Guides/outlier.md#parameters)

#### `exclude_where_indata_hist` Option

This option excludes records based on a user-specified SQL expression.  See the procedure-specific documentation for details

- [estimator](/docs/EN/Procedure%20Guides/estimator.md#parameters)

#### `prefill_by_vars` Option

This option is available and enabled by default in all procedures which accept an `instatus` table.  
See [User Guide](/docs/EN/user_guide.md#prefill_by_vars)

#### `presort` Option

This option is available and enabled by default in all procedures which accept input tables.  
See [User Guide](/docs/EN/user_guide.md#presort)

#### `trace` Option

This option is available in all procedures and controls console log verbosity.  
See [User Guide](/docs/EN/user_guide.md#python-log-verbosity-trace)

## Procedure Tables

Many table parameter names have changed to better follow common Python naming conventions.

The identifiers used in SAS programs correspond to the following identifiers in Python:

### Table of Procedure Table Identifiers

|SAS identifier|Python identifer|Note|
|--|--|--|
|`ALGORITHM`|`inalgorithm`||
|`AUX`|`indata_hist`|no "AUX" option in Python, use `indata_hist` instead|
|`DATA`|`indata`||
|`DATASTATUS`|`instatus`||
|`DONORMAP`|`outdonormap`||
|`ESTIMATOR`|`inestimator`||
|`HIST`|`indata_hist`||
|`HISTSTATUS`|`instatus_hist`||
|`INSTATUS`|`instatus`|*new*: now accepted by `errorloc` procedure, see [Addition of instatus to Errorloc](#addition-of-instatus-to-errorloc-procedure)|
|`OUT`|`outdata`||
|`OUTACCEPTABLE`|`outacceptable`||
|`OUTDATA`|`outdata`||
|`OUTEDITAPPLIC`|`outedit_applic`||
|`OUTEDITSTATUS`|`outedit_status`||
|`OUTGLOBALSTATUS`|`outglobal_status`||
|`OUTKEDITSSTATUS`|`outk_edits_status`||
|`OUTRANDOMERROR`|`outrand_err`||
|`OUTREDUCEDEDITS`|`outedits_reduced`||
|`OUTESTEF`|`outest_ef`||
|`OUTESTLR`|`outest_lr`||
|`OUTESTPARMS`|`outest_parm`||
|`OUTVARSROLE`|`outvars_role`||
|`OUTREJECT`|`outreject`||
|`OUTSTATUS`|`outstatus`|*new*: now produced by the `massimp` procedure, see [Addition of outstatus to Mass Imputation](#addition-of-outstatus-to-mass-imputation-procedure)|
|`OUTSUMMARY`|`outsummary`||
|`STATUS`|`instatus`||
|Added in Banff 3.1.1|`outstatus_detailed`|produced by `outlier`, see [Changes to Outlier Outstatus](#changes-to-outlier-outstatus)|
|Added in Banff 3.1.1|`outmatching_fields`|produced by `donorimp`, see [Changes to Donor Imputation Outstatus](#changes-to-donor-imputation-outstatus)|

### Table Changes

An overview of changes is provided in the following subsections, with links to detailed information.  
For a complete list of detailed information, see [Notable Procedure Changes](#notable-procedure-changes).

#### Removal of `by` Variables

`by` variables have been removed from many output tables.  See [*BY* Variables on Output Tables](#by-variables-on-output-tables) for more details.  

#### Standardization of `outstatus` Table

> also see [Output Tables > outstatus](/docs/EN/output_tables.md#outstatus)

The `outstatus` table is now [standardized](#standardization-of-outstatus-tables) across all procedures which produce it.  New output tables have been introduced to the [*Donor Imputation*](#changes-to-donor-imputation-outstatus), [*Mass Imputation*](#addition-of-outstatus-to-mass-imputation-procedure) and [*Outlier*](#changes-to-outlier-outstatus) procedures to accommodate this change.  

#### Addition of `instatus` Table

The *Errorloc* procedure now [accepts an `instatus` table](#addition-of-instatus-to-errorloc-procedure).  

## Table Specification

For information on specifying input and output tables, supported formats, etc. please see the [User Guide](/docs/EN/user_guide.md#input-and-output-table-specification)

## Notable Procedure Changes

### Input Table Changes

#### Addition of `instatus` to *Errorloc* Procedure

*Errorloc* now accepts an `instatus` table.  

The *Errorloc* procedure processes its `instatus` table somewhat differently than other procedures.  To favour selecting fields flagged for imputation, for each row in `instatus` with a status flag of `FTI`, the corresponding value in `indata` will be treated as if it were *missing*.  

### Output Table Changes

#### *BY* Variables on Output Tables  

In the SAS based procedures, `by` variables were included on many output tables essentially by default whenever *by-group processing* was performed.  In Banff 3.1.1 however, `by` variables are only ever included on the following tables:

- **Estimator**
  - `outacceptable`
  - `outest_ef`
  - `outest_lr`
  - `outest_parm`
  - `outrand_err`
- **Outlier**
  - `outsummary`

#### Standardization of `outstatus` Tables

All `outstatus` tables are now standardized and contain exactly the following columns

|Column Name|Note|
|--:|:--|
|`<unit-id>`|column named after the `unit_id` column from `indata` table|
|`FIELDID`|name of the column to which the status applies|
|`STATUS`|the status code|
|`VALUE`|value of the variable to which the status applies\*|

> \***`VALUE` column**
> For procedures which produce an `outdata` table, the value is sourced from there.  
> Otherwise, the value is sourced from the `indata` table.  

Furthermore, non-status information formerly produced by the *Donor Imputation* and *Outlier* procedures has been removed.  See below for more information.  

##### Changes to *Donor Imputation* `outstatus`

*Donor Imputation* now produces a standardized `outstatus` table.  Data which has been removed from `outstatus` is available in new optional output table `outmatching_fields`.  
This new table is disabled by default.  Specify `True`, or any valid output option to enable it.  This new table replaces the `match_field_stat` option, which is now deprecated.  

##### Addition of `outstatus` to *Mass Imputation* procedure

*Mass Imputation* produces an `outstatus` table with the flag `IMAS`.  

##### Changes to *Outlier* `outstatus`

*Outlier* now produces a standardized `outstatus` table.  Data which has been removed from `outstatus` is available in new optional table `outstatus_detailed`.  This new table contains the variables `<unit_id>`, `FIELDID`, `OUTLIER_STATUS` (formerly `OUTSTATUS`, not to be confused with the `oustatus` table's *`status`* variable), and any variables enabled by specifying `outlier_stats=True`.  The table is enabled by default.  Specify `False` to disable it.  

## Other Python Runtime Options

### Native Language Support

Banff produces a [log](/docs/EN/user_guide.md#banff-log) which can output messages in either English or French.  See [*setting the log language*](/docs/EN/user_guide.md#setting-the-log-language) from the user guide for details.  

### `capture` option

When running in Jupyter Notebooks, some log messages may be missing.  Specifying `capture=True` in a procedure call to may fix the issue.  See [suppressing and troubleshooting log messages](/docs/EN/user_guide.md#suppressing-and-troubleshooting-log-messages-capture) from the user guide for details.  

## Performance Considerations

Certain options and table formats can be expected to deliver optimal performance.  See [Performance Considerations](/docs/EN/user_guide.md#performance-considerations) for details.  

## Errors and Exceptions

Error's are handled differently in SAS vs in Python, where they are called *exceptions*.  See [Errors and Exceptions](/docs/EN/user_guide.md#errors-and-exceptions) from the user guide for details.  

## Utility Functions

### Working with SAS Files in Python

The banff package provides a few useful functions for reading SAS files in Python.  See [Working with SAS Files in Python](/docs/EN/user_guide.md#working-with-sas-files-in-python) in the user guide for details.  
