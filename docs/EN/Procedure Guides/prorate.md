# Prorating

* Execution: *banff.prorate()*
* SDE function types: *Review, Selection, Treatment*
* Input status flags: *I-- (optional)*
* Output status flags: *IPR*

## Description

Prorates and rounds records to satisfy user-specified edits.

Unlike other Banff procedures, the `edits` for this procedure follow specific criteria: only equalities are permitted, and the set of edits must form a hierarchical structure that sums to a grand-total. For example:

$$
subtotal1 + subtotal2 = grandtotal \\
a + b + c = subtotal1 \\
d + e + f = subtotal2
$$

Each individual edit must consist of a set of components *x<sub>i</sub>* that sum to a total *y*, i.e., of the form *x<sub>1</sub> + ... x<sub>n</sub> = y*. Inequalities and constants are not permitted. For each individual edit equation that is not satisfied, one of the two prorating algorithms (`basic` or `scaling`) is applied in order to rake the components to match the total. The procedure takes a top-down approach, beginning with the grand-total (which is never changed) and adjusting components as necessary, until the full set of edits is satisfied. Missing values are not prorated; they are set to zero during the procedure and reset to missing afterwards. Values of zero are never altered.

Additional features:

* Automatic rounding to the desired number of decimal places.
* Optional bounds to constrain the relative change of values during prorating.
* Control over which variables are eligible for prorating.
* Option to limit prorating to original or previously imputed values, either globally or for individual variables.
* Weights to adjust the relative change of individual variables.

For a full mathematical description of the procedure methods, with examples, please see the [Functional Description](../Banff%20Functional%20Description.pdf).

## Input and output tables

Descriptions of input and output tables are given below. Banff supports a number of input and output formats; please see the Banff User Guide for more information.

| Input table   | Description |
| ------------- | ----------- |
| indata        | Input statistical data. Mandatory. |
| instatus      | Input status file containing I-- status flags.<br><br> The instatus file is only required if at least one modifier is "ORIGINAL" or "IMPUTED". In this case, the procedure will search through imputation flags of the form I-- to determine which values were previously imputed. For the purpose of prorating, values with a status of FTI or IDE are considered original values.|

| Output table  | Description |
| --------------| ----------- |
| outdata       | Output statistical table containing imputed data. <br><br> Note that outdata will only contain successfully imputed records and affected fields. Users should update indata with the values from outdata before continuing the data editing process. |
| outstatus     | Output status file identifying imputed fields with IPR status flags, and their values after imputation. |
| outreject     | Output table containing records that failed prorating. <br><br> For an individual record, there are potentially many reasons prorating could not be performed; please see the notes below for more details.|

For details on the content of output tables, please see the [Output Tables](../output_tables.md) document.

## Parameters

| Parameter       | Python type  | Description                 | 
| ----------------| -------------| --------------------------- |
| unit_id         | str          | Identify key variable (unit identifier) on indata. Mandatory. <br><br> Must be unique for each record. Records with a missing value are dropped before processing. |
| edits           | str          | List of edits that the prorating procedure must satisfy. Mandatory. <br><br> Please see the [user guide](../user_guide.md#specifying-linear-edits) for general info on specifying edits. Unlike the other edit-based procedures in Banff, the prorate procedure places certain restrictions on the edit list; please see the notes below for details. |
| decimal         | int          | Number of decimals used in the rounding algorithm (between 0 and 9). Default=0. <br><br> The number of decimals specified must be equal to or greater than the actual number of decimals found on the total.|
| method          | str          | Prorating method ("SCALING" or "BASIC"). Default = "BASIC". <br><br> When `method="BASIC"` the signs of some variables may change during the prorating process. When `method="SCALING"` the signs of variables can never change. If all values are of the same sign (i.e., all positive or all negative), both methods produce identical results. |
| modifier        | str          | Global modifier ("ALWAYS", "IMPUTED", "ORIGINAL") to control which values are prorated. Default = "ALWAYS" <br><br> Specify `modifier="IMPUTED"` to only prorate previously imputed data, and `modifier="ORIGINAL"` to only impute original data. If either is specified, then `instatus` is required. Imputed values are identified as those with an input status flag of the form I--, except for IDE. Values with any other flag (including FTI and FTE) or without a flag are treated as original data. Specify `modifier="ALWAYS"` to impute all values regardless of their status. |
| lower_bound     | float        | Lower bound on the relative change of the variables. Default = 0. <br><br> Relative change of a variable is equal to the prorated value (after rounding) divided by the original value. The `lower_bound` parameter places a limit on this ratio; prorating will not be performed if the relative change is lower. For `method = "SCALING"`, the lower bound must be greater than or equal to zero; this ensures that the values cannot change sign during prorating. For `method = "BASIC"`, users may specify a negative lower bound if `accept_negative=True`. |
| upper_bound     | float        | Upper bound on the relative change of the variables. <br><br> Relative change of a variable is equal to the prorated value (after rounding) divided by the original value. The `upper_bound` parameter places a limit on this ratio; prorating will not be performed if the relative change exceeds this threshold.  |
| verify_edits    | bool         | Verify the consistency of the edits without performing any prorating. Default=False. |
| accept_negative | bool         | Treat negative values as valid. Default=False. <br><br> By default, a positivity edit is added for every variable in the list of edits; this parameter permits users to remove this restriction. If required, users may directly add positivity edits for individual variables. |
| by              | str          | Variable(s) used to partition indata into by-groups for independent processing.<br><br> Prorating is performed on one observation at a time. So specifying by-groups variables will not influence the results.<br><br> Example: `by = "province industry"` |
| prefill_by_vars | bool         | Add by-group variable(s) to input status file to improve performance. Default=True.  |
| presort         | bool         | Sort input tables before processing, according to procedure requirements. Default=True. |
| no_by_stats     | bool         | Reduce log output by suppressing by-group specific messages. Default=False. |

## Notes

### Edit syntax and restrictions

Unlike other Banff procedures, the prorate procedure includes a unique syntax and restrictions on the `edits`. Individual edits parameter must be written in the form 

$$ w_1 x_1 : m_1 + ... + w_n x_n : m_n = y $$

where *x<sub>i</sub>* and *y* are variables on indata, *w<sub>i</sub>* are weights and *m<sub>i</sub>* are modifiers. The *x<sub>i</sub>* variables are referred to as components and must appear on the left-hand-side of the equation. The *y* variable is referred to as the total and must appear on the right-hand-side of the equation. Individual edits are always separated by a semi-colon.

The set of `edits` must have a hierarchical structure. That is, there must be one variable, the *grand-total*, that shows up as a total *y* in one equation, but not as a component *x<sub>i</sub>* in any others. The components that contribute to the grand-total may act as *sub-totals* for other equations, but only once; double-decompositions are not permitted. There is no limit to the number of levels in the hierarchy specified by the user. These restrictions can be summarized with the following rules:

* *Grand-total*: Exactly one variable must appear as a total but never as a component. This variable is never altered by the procedure.
* *Sub-totals*: All other variables that appear as a total in one equation must appear as a component in another equation.
* In the complete set of `edits`, each variable can appear at most two times: once as a component *x<sub>i</sub>* and once as a total *y*.

Weights *w<sub>i</sub>* are optional. They must be a positive number; if not assigned to a variable, a default weight of one is applied. Weights can be used to control the relave amount of change in each component due to prorating, change being inversely proportional to the assigned weight. Weights are only assigned to the components, not the total.

Modifier codes *m<sub>i</sub>* are optional. Similar to weights, they can be assigned to individual variables, specified after the variable name and separated by a colon. Modifiers determine which variables are eligible for prorating. They behave in the same way as the global parameter `modifier`. The permitted codes are:

* "A" (Always): The variable can always be prorated. (Default approach.)
* "N" (Never): The variable is never prorated.
* "I" (Imputed): Only previously imputed values are prorated. Previously imputed values are identified by an imputation code I-- on the instatus file, except for IDE which is treated as an original value.
* "O" (Original): Only original values are prorated. This includes values with status codes of IDE, FTI, and FTE, as well as any values without a status code on instatus.

If modifier codes "A" or "I" are used, then instatus must be specified. Note that modifiers specified in the `edits` overrule the global modifier specified by the `modifier` parameter.

Example:

```python
prorate_call = banff.prorate(
    edits="""
    sub1 + sub2 + sub3:N = grandtotal;
    2a + b = sub1;
    c:I + d:I + e:I + f:I = sub2;
    2g:O + 3h:A = sub3;
    """,
    ... # other parameters
    )
```

In the above example:
* Variable `sub3` is never prorated.
* Variable `a` has a weight of 2.
* Variables `c,d,e,f` are only prorated if the values were previously imputed.
* Variable `g` has a weight of 2 and only original values are prorated.
* Variable `h` has a weight of 3 and is always prorated, whether or the the value is original or imputed.

### Top-down prorating procedure

The prorate procedure begins with the grand-total. If the components of the grand-total equation do not sum to the grand-total, the prorating procedure is performed, adjusting the component values as necessary. (The grand-total never changes). This process then repeats for any of the components that are sub-totals, i.e., component variables that appear as totals in other edit equations. Note that in this process, each value is only prorated once, when it appears as a component.

### Rounding

The prorate procedure includes a rounding algorithm to adjust all fields to the correct number of decimal places while satisfying the `edits`. Users can specify the desired number of decimal places using the `decimal` parameter; the default value is zero (i.e., whole numbers). The value of `decimals` must be an integer within [0,9], and must be equal to or greater than the actual number of decimals found on the total.

Note that the order of component variables in the edit equations can affect the results of the rounding algorithm, when the amount of required prorating cannot be evenly distributed amongst the components.

When there are very large summation components (i.e. larger than 9 digits with or without decimals), the procedure is not able to accurately prorate them to the total with a precision of 9 decimals. In such a case, the observation will be rejected and listed with a decimal error in the outreject table. To avoid this, one can reduce the value of the `decimal`, for example by setting `decimal=8` or lower.

### Rejected records

Sometimes prorating cannot be successfully performed on an individual, in which case the prorate procedure will output the record to the outreject table, alongside the reason for the failed prorating attempt. Reasons include:

* The prorated values are outside the user-specified bounds.
* The scaling value *k* is outside the interval [-1,1]. (Only for `method="scaling"`)
* The factor *k* cannot be calculated because the weighted sum of proratable columns is zero.
* The record includes negative values and `accept_negative=True` isn't specified.
* No values are eligible for prorating due to the "ORIGINAL" or "IMPUTED" modifier.
* The user has specified fewer decimal places than exist in the adjusted total.