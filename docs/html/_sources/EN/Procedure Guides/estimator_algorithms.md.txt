# Algorithms in Estimator Procedure

The `inestimator` and `inalgorithm` tables define the models used for imputation (mean, historic value, linear regression, etc.) and to specify certain parameters and options. While the `inestimator` table is mandatory, the `inalgorithm` table is only required when using a custom algorithm instead of one of the 20 pre-defined algorithms available in the procedure.

Algorithms used by the `estimator` procedure are classified into two types:
* Estimator functions (EF) are user-defined mathematical expressions of auxiliary and/or historical data, consisting of addition, subtraction, multiplication, division, and exponentiation. For each variable in the function, users may specify whether to use the targeted record's individual value, or to average over all records within the same imputation class, and whether to use current data or historical data. Every parameter in an estimator function must be specified by the user; no modelling is performed by the procedure.
* Linear regression (LR) estimators use linear regression models to impute missing values. Users specify which auxiliary variables to include in the model, as well as the time period to use and exponents to apply. The regression coefficients are then solved for within each imputation class, and the resulting linear model is used to impute the targeted variable for the record in question.

This document explains how to specify a custom algorithm of either type, and lists the available built-in algorithms.

## Interaction between inestimator and inalgorithm

Whether it is predefined or custom, an algorithm can be referred to in the `inestimator` table in order to be utilized in the imputation of one or multiple variables. With at least one row for each variable to impute, `inestimator` contains columns that define the imputation method and parameters. In particular, three columns are used to specify the elements related to the algorithm: 

- `fieldid`: the variable to impute.
- `algorithmname`: the algorithm name to be used, whether it is predefined or custom.
- `auxvariables`: auxiliary variables used in the algorithm. It may contain multiple auxiliary variables (with no duplication even if both historical and current values of the same variable are used); multiple variables need to be separated by a comma. `auxvariables` needs to contain the same number of auxiliary variable as the ones used in the algorithm; their order of appearance is important and is used to assign them to variables in the algorithm's formula. `auxvariables` must be left blank if the associated algorithm doesn't use any auxiliary variable (imputation using the historical value of the variable to impute for example).

## Custom algorithm

Just like pre-defined algorithms, a custom algorithm can take the form of an estimator function (EF type), or a linear regression (LR type) imputation. Both types use placeholders to define the algorithm's formula.

### Placeholders and their attributes

The algorithm name, type, status code and formula are specified using the columns `algorithmname`, `type`, `status` and `formula` respectively. Note that no distinction is made between upper and lower case. In `formula`, it is only possible to use placeholders `fieldid` and `aux<n>` where n is a positive integer; `aux<n-1>` must be used in the formula if `aux<n>` is used. 

- `fieldid` is a placeholder for the name of the variable to impute as specified in `inestimator`.
- `aux<n>` is a placeholder for the name of the auxiliary variable as specified in `inestimator`. The first variable in `auxvariables` replaces placeholder `aux1`, the second, `aux2`, and so on. The number of variables in `auxvariables` must be equal to the number of different `aux<n>` placeholders.

Depending on the type of the algorithm (EF or LR), the formats used with the placeholders (`aux<n>` and/or `fieldid`) are the following:

- **EF algorithms**:
    - `<placeholder>(period, aggregation)`,
    - `<placeholder>(aggregation, period)`.
- **LR algorithms**
    - `<placeholder>(period)`.

The `period` and `aggregation` options are called placeholder attributes. Possible values for `period` are `c` for current data, and `h` for the historical data. As for `aggregation`, it takes `v` for using the variable's value of the observation, and `a` for the average of the variable based on acceptable observations. Note that LR algorithms are a particular case since they only use values of observations (with no averages), which makes the attribute `aggregation` not applicable in their case.

### EF algorithms

An estimator function is a mathematical expression involving constants, current and/or historical values of some variables of the record, and current and/or historical averages of some variables, those averages being calculated from acceptable records (roughly speaking, an acceptable record is a record such that all variables for which an average must be calculated in the algorithm are available, and where imputed and/or outlier values are excluded if required by the user.). Arithmetic operators include addition (+), subtraction (-), multiplication (*), division (/), exponentiation (^) and parenthesis.

The default `period` is **current** and the default `aggregation` is **value**. As a consequence:
- `aux1`, `aux1(c)`, `aux1(v)` and `aux1(c, v)` are equivalent to each other.
- `aux1(h)` and `aux1(v, h)` are equivalent to each other.
- `aux1(a)` and `aux1(c, a)` are equivalent to each other.

Note that `fieldid`, `fieldid(c)`, `fieldid(v)` and `fieldid(c,v)` must never be used in `formula` since they refer to the value to be imputed. 

**Exponents** can be applied to constants (different from 0), placeholders and expressions:
- The exponent for a constant simply follows the constant: `3^4`.
- The exponent for a placeholder follows the closing parenthesis of the placeholder attributes:
`aux1^3`, `aux1(c)^3` and `aux1(v)^3` are equivalent to each other.
- The exponent for an expression requires parenthesis enclosing the expression:
`(aux1(h) + aux2(h))^2`.

In addition, the exponent must be a constant (a number different from 0). A placeholder or an expression (even if it is a constant) cannot be used as an exponent. Thus, `aux1^aux2` and `aux1^(2+1)` are not allowed.

### LR algorithms

Regression imputation consists of imputing a variable $y$<sub>$i$</sub> by a linear regression estimation like

$$
\hat{y_i} = \hat{\beta_0} + \hat{\beta_1} x_{i_1 T_1}^{p_1} + \hat{\beta_2} x_{i_2 T_2}^{p_2} + ... + \hat{\beta_m} x_{i_m T_m}^{p_m} + \hat{\epsilon_i}  
$$

where : $T$<sub>$j$</sub> refer to current or historical periods, and $p$<sub>$j$</sub> are exponents. The variable $y$<sub>$j$</sub> being imputed is the dependent variable in the model, and the auxiliary variables $x$<sub>$ij$</sub> are the independent variables, or regressors. $\hat\beta$<sub>$j$</sub> are the regression coefficients, the values of which are solved for 
by using the method of least squares. $\hat\epsilon$<sub>$i$</sub> is a random error term, which can be added to the model to introduce some variability into the fitted values of the $y$<sub>$i$</sub>. 
Note that $\beta$<sub>$0$</sub>, which is the intercept in the regression line, is optional and can be omitted from the model.

As the `aggregation` attribute is not applicable to LR algorithms, only the `period` attribute placeholder is used, with a default value of `c` (current data). For example, `aux1` and `aux1(c)` are equivalent to each other.

The formula takes the following elements separated by a comma:
- Intercept: if `intercept` is added to the formula, then a linear regression with intercept will be used.
- Regressors: lists the variables to include in the linear regression, separated by a comma. These variables can take the form of auxiliary variables (`aux1`, `aux2`, ..) or historical values of the variable to impute (`fieldid(h)`). Exponents can be applied to regressors using the symbol `^` after the closing parenthesis of the placeholder attribute (`aux1^3` and `aux1(c)^3` are equivalent to each other).

**Exponents** must be a constant (a number different from 0) to be applied to placeholders. A placeholder or an expression (even if it is a constant) cannot be used as an exponent. Thus, `aux1^aux2` and `aux1^(2+1)` are not allowed.

The following table shows linear regressions examples and how they are specified as a formula:

| Linear Regression| Algorithm Formula |
| -----------| -------|
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub>| $aux1$ |
| $y = \beta$<sub>$0$</sub> $+ \beta$<sub>$1$</sub> $* y$<sub>$h$</sub>| $intercept, fieldid(h)$  |
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub> $+ \beta$<sub>$2$</sub> $* x$<sub>$2h$</sub> $+ \beta$<sub>$3$</sub> $* x$<sub>$3c$</sub> | $aux1, aux2(h), aux3$ |
| $y = \beta$<sub>$1$</sub> $* x$<sub>$1c$</sub> $+ \beta$<sub>$2$</sub> $* x$<sub>$1c$</sub><sup>$2$</sup> $+ \beta$<sub>$3$</sub> $* x$<sub>$1c$</sub><sup>$3$</sup>  | $aux1, aux1$^2 $, aux1$^3  |

Note that `fieldid(h)` is permissible in an algorithm formula, but not `fieldid` or `fieldid(c)` as they both refer to the value to be imputed.

## Pre-defined algorithms tables

Each pre-defined algorithm is given a name and a status code associated with the model. In the following, the same structure of placeholders and their attributes previously shown is used to display the formula for every pre-defined algorithm, alongside a brief description. 

### EF algorithms
 
| Algorithm Name | Imputation Status | Algorithm Formula | Imputed Value | 
| -----------| -------| --------| ------------  |
| AUXTREND   | IAT    | $fieldid(h,v)*(aux1(c,v)/aux1(h,v))$ | Trend adjustment calculated from an auxiliary variable and applied to the historical value of the field to impute.|
| AUXTREND2  | IAT2   | $fieldid(h,v)*(aux1(c,v)/aux1(h,v) + aux2(c,v)/aux2(h,v))/2$ | Average trend adjustment calculated from two auxiliary variables and applied to the historical value of the field to impute. |
| CURAUX     | ICA    | $aux1(c,v)$  | Current value of an auxiliary variable for the record to impute. |
| CURAUXMEAN | ICAM   | $aux1(c,a)$  | Current average of an auxiliary variable. |
| CURMEAN    | ICM    | $fieldid(c,a)$  | Current average of the variable to impute. |
| CURRATIO   | ICR    | $fieldid(c,a) * (aux1(c,v) / aux1(c,a))$ | Current ratio estimate calculted from an auxiliary variable and applied to the current average value of the variable to impute.|
| CURRATIO2  | ICR2   | $fieldid(c,a) * (aux1(c,v) / aux1(c,a) + aux2(c,v) / aux2(c,a))/2$ | Current average ratio estimate calculated from two auxiliary variables and applied to the current average value of the variable to impute. |
| CURSUM2    | ISM2   | $aux1 + aux2$  | Sum of current values of two auxiliary variables. |
| CURSUM3    | ISM3   | $aux1 + aux2 + aux3$ | Sum of current values of three auxiliary variables. |
| CURSUM4    | ISM4   | $aux1 + aux2 + aux3 + aux4$  | Sum of current values of four auxiliary variables. |
| DIFTREND   | IDT    | $fieldid(h,v)* (fieldid(c,a) / fieldid(h,a))$  | Trend adjustment of the average calculated from the field to impute and applied to its historical value. |
| PREAUX     | IPA    | $aux1(h,v)$ | Historical value of an auxiliary variable for the record to impute. |
| PREAUXMEAN | IPAM   | $aux1(h,a)$  | Historical average of an auxiliary variable. |
| PREMEAN    | IPM    | $fieldid(h,a)$ | Historical average of the field to impute.  |
| PREVALUE   | IPV    | $fieldid(h,v)$ | Historical value of the field to impute. |

### LR algorithms

| Algorithm Name | Imputation Status | Algorithm Formula | Imputed Value | 
| -----------| -------| --------| ------------  |
| CURREG   | ILR1    | $intercept, aux1(c)$ | Simple linear regression based on current values of an auxiliary variable.|
| CURREG_E2  | ILRE   | $intercept, aux1(c), aux1(c)$^2 | Linear regression based on current values of an auxiliary variable and its squared. |
| CURREG2     | ILR2    | $intercept, aux1(c), aux2(c)$  | Linear regression based on current values of two auxiliary variables. |
| CURREG3 | ILR3   | $intercept, aux1(c), aux2(c), aux3(c)$  | Linear regression based on current values of three auxiliary variables. |
| HISTREG    | IHLR    | $intercept, fieldid(h)$  | Simple linear regression based on historical values of the variable to impute. |