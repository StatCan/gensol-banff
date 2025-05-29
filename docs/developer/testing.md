# Testing Procedures with `pytest_<proc-name>` Functions

**In this guide**  
This file documents how to use the `banff.testing` subpackage to test Banff procedures.  The intended audience is beta testers of the Banff Procedures.  

## Table of Contents

- [`pytest` Helper Functions](#pytest-helper-functions)
  - [Installing and Importing](#installing-and-importing)
- [Function Inputs](#function-inputs)
  - [**Check Parameters**](#check-parameters)
    - [`msg_list_contains_exact`](#msg_list_contains_exact-str-list-of-str-defaults-to-empty-list)
    - [`msg_list_contains`](#msg_list_contains-str-list-of-str-defaults-to-empty-list)
    - [`expected_error_count`](#expected_error_count-integer-none-defaults-0)
    - [`expected_warning_count`](#expected_warning_count-integer-none-defaults-0)
    - [`rc_should_be_zero`](#rc_should_be_zero-bool-none-defaults-true)
    - [`expected_<output-dataset-name>`](#expected_output-dataset-name-input-dataset-none-defaults-none)
    - [`round_data`](#round_data-integer-none-defaults-none)
    - [`drop_columns`](#drop_columns-bool-none-str-list-of-str-defaults-true)
  - [`control_data` Folder](#control_data-folder)
  - [Function Internals](#function-internals)
- [Test Output](#test-output)
  - [Dataset Comparison Output](#dataset-comparison-output)
- [Running Tests](#running-tests)
  - [`run_pytest()` function (***recommended method***)](#run_pytest-function-recommended-method)
- [**Complete Example Testcase**: `test_donorimp_a03_sas.py`](#complete-example-testcase-test_donorimp_a03_sas.py)
  - [Python code](#python-code)
    - [Python Code: line by line explanation](#python-code-line-by-line-explanation)
      - [Import both `pytest` and Banff's `testing` subpackage](#import-both-pytest-and-banffs-testing-subpackage)
      - [Define a test function](#define-a-test-function)
      - [Call the *`pytest` helper function*](#call-the-pytest-helper-function)
      - [Insert helper code for invoking `pytest`](#insert-helper-code-for-invoking-pytest)
  - [SAS Code](#sas-code)
  - [Running the example program](#running-the-example-program)
    - [via Command Line](#via-command-line)
      - [Sample Output](#sample-output)
- [Other Example Test Cases](#other-example-test-cases)
  - [`test_donorimp_r04.py`](#test_donorimp_r04.py)
  - [`test_determin_a07.py`](#test_determin_a07.py)

## Change Log

**`2023-dec-12`** (*beta 6*) add note to `drop_columns` parameter regarding inclusion of BY variables

**`2023-nov-29`** (*beta 5*) add guidance on [installing and importing](#installing-and-importing), new features for [dataset comparison](#dataset-comparison-output) ([`drop_columns`](#drop_columns-bool-none-str-list-of-str-defaults-true))

**`2023-nov-06`** (*beta 4*) Initial Draft of document

# `pytest` Helper Functions

Subpackage `banff.testing` provides code for testing Banff procedures with the use of `pytest`.  It defines a "*`pytest` helper function*" for each procedure:

- `pytest_determin()`
- `pytest_donorimp()`
- `pytest_errorloc()`
- `pytest_estimato()`
- `pytest_massimpu()`
- `pytest_outlier()`
- `pytest_prorate()`

> To use a testing function, import the package and refer to the function like this
>
> ```python
> import banff.testing as banfftest
> banfftest.pytest_determin(...)
> ```

These functions are like wrappers for their respective procedures.  Use them to run the procedure and perform automatic checks on the output status, console messages, or output data.  

## Installing and Importing

> **IMPORTANT**: The `banff.testing` subpackage requires the [`pytest`](https://docs.pytest.org/en/7.4.x/index.html) library.  
>
> - if `pytest` is not installed automatically as a dependency when installing `banff`, install it separately using the terminal command `pip install pytest`

**IMPORTANT**: In order to employ `banff.testing` *subpackage*, it must be imported explicitly.  
It is not sufficient to import `banff`, the following code will result in an error

```python
import banff
test_call = banff.testing.pytest_donorimp(...)
```

Instead, import `banff.testing` explicitly

```python
import banff.testing
test_call = banff.testing.pytest_donorimp(...)
```

> **TIP**: use an alias (*`import ... `**`as`***)
>
> ```python
> import banff.testing as banfftest
> test_call = banfftest.pytest_donorimp(...)
> ```
>
> - this is how examples in this documentation will import the "*`testing` subpackage*"

# Function Inputs

Inputs to these functions can be organized into three categories

1. *Procedure Parameters*: the parameters the associated procedure accepts, such as `unit_id`, `indata`. etc.
2. *Check Parameters*: specification of test pass/fail conditions
3. *internal use*: parameters required in order for the test function to work (more on this later, search for "`capfd`" in this document)

Each function accepts the full set of *procedure parameters* (including input datasets) that its respective procedure defines.  
For example, here is a simplified signature for *deterministic*'s pytest helper function:

```python
def pytest_determin( 
      # normal procedure parameters
        accept_negative, by, edits, unit_id, no_by_stats, reject_negative, 
        indata, instatus, outdata, outstatus, capture=False, trace=False, 
        
      # pytest internal parameter
        pytest_capture = None, 
        
      # "check" parameters
        msg_list_contains    = [],   msg_list_contains_exact = [], 
        expected_error_count = 0,    expected_warning_count  = 0, 
        rc_should_be_zero    = True, 
        round_data           = None, 
        expected_outdata     = None, expected_outstatus      = None, 
    ):
```

## Check Parameters

This section documents the parameters of the `pytest_<proc-name>()` function which are used to specify "*checks*"; the conditions by which the testcase is considered a pass or fail.  The specified checks run immediately following the procedure's execution.  Use these to automatically validate that

- the procedure log contains certain messages
- the procedure log contains a certain number of ERROR or WARNING messages
- the procedure "return code" is as-expected
- output datasets contain expected values

---

### `msg_list_contains_exact`: *str, list of str, defaults to empty list*

- a `list` of zero or more messages that must appear in the procedure log  
  - for convenience, when specifying only a single message, a `str` may be supplied instead of a `list` of `str`  
- if the message spans multiple lines, use a """triple-quoted string"""  
- fails if any of the messages are not found  

> **example**: *specifying a single message*
>
> ```python
> msg_list_contains_exact = "ERROR: ID is mandatory."
> ```
>
> **example**: *specifying multiple messages*
>
> ```python
> msg_list_contains_exact = [
>         "NOTE: ID not specified.", 
>         "ERROR: ID is mandatory.",
>        ]
> ```

---

### `msg_list_contains`: *str, list of str, defaults to empty list*

- same as `msg_list_contains_exact`, except both the messages and procedure log are "*cleaned*" prior to searching for each message
  - *cleaning* replaces all consecutive whitespace characters with a single space, replaces consecutive `.` characters with a single `.` character, and removes "newline" characters  
- this is useful if the expected and actual message differ in their formatting but otherwise have equivalent content
  - this may occur when searching for multi-line messages which include a "...."

> **example**: *specifying a single message which spans multiple lines*
>
> ```python
> msg_list_contains = """Number of valid observations ......................:       4     100.00%
> Number of donors ..................................:       1      25.00%
> Number of donors to reach DONORLIMIT ..............:       0       0.00%
> Number of recipients ..............................:       3      75.00%
>     not imputed (insufficient number of donors) ...:       3      75.00%
> """
> ```
>
> internally, the "cleaned" message will look something like
>
> ```text
> Number of valid observations .: 4 100.00% Number of donors .: 1 25.00% Number of donors to reach DONORLIMIT .: 0 0.00% Number of recipients .: 3 75.00% not imputed (insufficient number of donors) .: 3 75.00%
> ```

---

### `expected_error_count`: *integer, `None`, defaults `0`*

- the number of times "*`ERROR:`*" is expected to occur in the procedure log  
- to disable this check, specify `None`  
- fails if the actual and expected counts differ

> **example**: *1 error message expected*
>
> ```python
> expected_error_count = 1
> ```

---

### `expected_warning_count`: *integer, `None`, defaults `0`*

- the number of times "*`WARNING:`*" is expected to occur in the procedure log  
- to disable this check, specify `None`  
- fails if the actual and expected counts differ

> **example**: *disable this check*
>
> ```python
> expected_warning_count = None
> ```

---

### `rc_should_be_zero`: *bool, `None`, defaults `True`*

- whether or not a return code of zero (`0`) is expected from the procedure  
  - Banff procedures return a non-zero return code if-and-only-if an error occurred during execution  
- to disable this check, specify `None`  
- fails if an unexpected return code is received from the procedure  

> **example**: *when an error is expected*
>
> ```python
> rc_should_be_zero = False
> ```

---

### `expected_<output-dataset-name>`: *input dataset, `None`, defaults `None`*

> **NOTE**: there will be one of these parameters for *each* of the associated procedure's output datasets

- a dataset containing the exact output values expected from the procedure
  - the actual and expected output datasets are compared in Python using the `pandas` package
  - prior to comparison
    - both rows and columns are sorted
    - all numeric values are converted to `float64` (the format which has equivalent numeric precision to both Python and SAS based Banff procedure output)
    - optionally, numeric data can be rounded prior to comparison (see `round_data` parameter)
- support the same formats supported for procedure input dataset
- fails if any difference is detected, such as differing
  - number of columns or rows
  - column names
  - values
  - datatypes (i.e. *numeric* instead of *character*)

> **example**: *use a SAS dataset file known to contain the correct outstatus output*
>
> ```python
> expected_outstatus = "C:/testing/sas_outstatus.sas7bdat"
> ```

---

### `round_data`: *integer, `None`, defaults `None`*

- number of decimal places to round numeric values to prior to comparing actual and expected output datasets (specified in `expected_<output-dataset>` parameters)
- by default no rounding occurs and the original numeric values are compared
- this is a helpful workaround in cases where the "expected" dataset comes from a potentially low-precision format, such as CSV

> example: *if we want numeric values rounded to four decimal places prior to comparison*
>
> ```python
> round_data = 4
> ```
>
> - in this case, values such as `89.1234567` would be rounded to `89.1234`

---

### `drop_columns`: *bool, `None`, str, list of str, defaults `True`*

- whether or not to drop columns from control ("*expected*") datasets prior to comparison with test ("*actual*") datasets
- specify `True` (*default*) to automatically drop any variable listed in the procedure's `by` statement (where applicable)
- to drop specific variables, specify
  - a variable name (`str`)
  - multiple space-delimited variable names (`str`)
  - list of variable names (list of `str`)
- this feature can be disabled by specifying one of `None`, `False`, or `""`

> ***NEW in version `3.1.1b6`***
> BY variables have been [reintroduced to some datasets](../EN/release-notes.md#2023-december-12-version-3-01-001b6).  Affected datasets should now be completely identical to their Banff 2.08.002 equivalents.  
> **This feature is disabled** for [datasets which now include BY variables](../EN/sas_migration_guide.md#by-variables-on-output-tables)

> **NOTE**
> variables specified for dropping do **not** need to exist on the control dataset, they will only be dropped if they are found (i.e. no errors will occur during dropping)

> **example**
> *if we want to drop the columns `to_excl` and `area` from control datasets we could specify a space-delimited string*
>
> ```python
> drop_columns = "to_excl area"
> ```
>
> *alternatively, we can specify a list of strings*
>
> ```python
> drop_columns = ["to_excl", "area"]
> ```

## `control_data` Folder

For convenience, test functions allow easy specification of dataset *files* which are stored in the *`control_data`* folder.  When a filename is specified without a path for an *input* or *expected* dataset, test functions will first look for that file in a folder named `control_data`, located in the same folder as the Python program being executed.  Otherwise, the dataset will be loaded identically as when calling a procedure directly (i.e., you can specify an absolute or relative file path).  

For example suppose we have the following structure

```text
test_folder
|   test_donorimp_a03_sas.py
|
\---control_data
        indata.sas7bdat
```

In test case `test_donorimp_a03_sas.py`, specify `indata="indata.sas7bdat"` and the file in the `control_data` subdirectory should will be loaded

A message will be printed to the log when data is loaded from `control_data`

```text
banff.testing: dataset 'indata.sas7bdat' found in control_data folder, will load file at path 'C:\regression_tests\determin/control_data/indata.sas7bdat'
```

## Function Internals

> **Note from documentation's author**
> This is the first draft of this section so it is sparse.  Similar documentation needs to be created for each procedure (*Procedure Internals*).  
> The contents of these objects is subject to change.  Explore the objects in a live debugging session to see what is available, and consult with IT if you have a specific need: we appreciate input on what users require

These "functions" are actually implemented as *classes* in Python, the same way that Banff procedures are implemented.  When called, an object (*instance* of the class) is returned.  
To use the returned object, first assign it to a variable

```python
test_call = banfftest.pytest_determin(...)
```

The object contains some useful members, such as `banff_call`, which is the object normally returned when a procedure is called directly.  
continuing from the code above, access it like this:

```python
# assign outdata dataset to variable
my_outdata = test_call.banff_call.outdata
print(my_outdata)
```

Another useful member is `rc`, which contains the *return code* from the procedure.  

```python
if test_call.rc != 0:
    print("an error occurred")
```

# Test Output

Test output is printed to the console following test execution.  If the test is executed using `pytest`'s default settings, much of the output will be suppressed.  See [Running Tests](#running-tests) for advice on executing the tests.  For a sample of test output, see [Running the example program](#running-the-example-program).  

Following the execution of a test, the procedure log and a summary of the checks performed will be printed to the console.  

## Dataset Comparison Output

When `expected_<output-dataset>` parameters are specified, the console will indicate the results of comparing the expected and actual datasets.  When the datasets are deemed equal, output will be minimal.  
> example which drops BY variables from expected data prior to finding datasets to be equal
>
> ```text
> assert_dataset_equal(dataset_name=OUTSTATUS)
>    sort datasets: True
>    drop columns: ['area', 'staff']
>      dropped column 'area' from control dataset
>      dropped column 'staff' from control dataset
>    datasets equal: True
> ```

Datasets are deemed inequal for a variety of reasons
> datasets equal except in the first (0th) row, variable (column) *Q4* has a value of `250` when `100` is expected
>
> ```text
> assert_dataset_equal(dataset_name=OUTDATA)
>    sort datasets: True
>    datasets equal: False
> 
>    ************** Inspecting Dataset Differences: START *********************
>      number of rows equal: True
> 
>      number of columns equal: True
> 
>      column names equal: True
>           {'total', 'Q2', 'ident', 'Q4', 'Q3', 'Q1'}
> 
>      The following values differ (printing output of pandas `.compare()`):
>            Q4
>          test control
>      0  250.0   100.0
> 
>      datatypes equal: True
> 
>    *************** Inspecting Dataset Differences: END **********************
> ```

> datasets with different number of rows and columns
> specifically, *expected* dataset has no `STATUS` column, *actual* does
>
> ```text
> assert_dataset_equal(dataset_name=OUTSTATUS)
>    sort datasets: True
>    datasets equal: False
> 
>    ************** Inspecting Dataset Differences: START *********************
>      number of rows equal: False
>        number of control (expected) rows: 4
>        number of test (actual) rows:      5
> 
> 
>      number of columns equal: False
>        number of control (expected) columns: 2
>        number of test (actual) columns:      3
> 
>      column names equal: False
>        columns only on control (expected) dataset:
>           {}
>        columns only on test (actual) dataset:
>           {'STATUS'}
>        columns common to both datasets:
>           {'FIELDID', 'ident'}
>    *************** Inspecting Dataset Differences: END **********************
> ```

> Column names are case sensitive, so case differences appear as follows
>
> ```text
> assert_dataset_equal(dataset_name=OUTSTATUS)
>    sort datasets: True
>    datasets equal: False
> 
>    ************** Inspecting Dataset Differences: START *********************
>      number of rows equal: True
> 
>      number of columns equal: True
> 
>      column names equal: False
>        columns only on control (expected) dataset:
>           {'status'}
>        columns only on test (actual) dataset:
>           {'STATUS'}
>        columns common to both datasets:
>           {'FIELDID', 'ident'}
>    *************** Inspecting Dataset Differences: END **********************
> ```

> look carefully for *datatypes equal: False*
>
> - *`object`* in the control dataset is a character data, "*150.0*"
> - *`float64`* in the test dataset is a numeric value, `150.0`
>
> ```text
> assert_dataset_equal(dataset_name=OUTDATA)
>    sort datasets: True
>    datasets equal: False
> 
>    ************** Inspecting Dataset Differences: START *********************
>      number of rows equal: True
> 
>      number of columns equal: True
> 
>      column names equal: True
>           {'total', 'Q2', 'ident', 'Q4', 'Q3', 'Q1'}
> 
>      The following values differ (printing output of pandas `.compare()`):
>            Q1
>          test control
>      0  150.0   150.0
> 
>      datatypes equal: False
>        comparing datatypes
>             test control
>      Q1  float64  object
>    *************** Inspecting Dataset Differences: END **********************
> ```

# Running Tests

> **IMPORTANT**:
>
> - test cases must be saved in python files whose names either start with `test_` or end with `_test.py`
> - test cases must be run in so-called "*script mode*"
>   - i.e., **not** in "*interactive mode*", which leads to runtime errors with these functions
>   - live debugging is allowed  

Test cases can be executed via any way that `pytest` can be invoked.  
> For a convenient method which configures `pytest` to print a reasonable amount of information, see [`run_pytest()` function](#run_pytest-function-recommended-method).  

By default `pytest` prints a summary of test results, suppressing all output from tests that pass, and printing a lot of information for test cases which fail.  For those who prefer to customize `pytest` output themselves, refer to [Usage and Invocations - pytest documentation](https://docs.pytest.org/en/6.2.x/usage.html).  

## `run_pytest()` function (*recommended method*)

Function `banff.testing.run_pytest()` can be used to invoke `pytest`. It configures `pytest` to print an ideal level of detail to the console log, including

- the procedure log  
- summary of checks  
- summary of test results

The following code can be inserted into any test file, outside of any function definition. With this, `pytest` will be invoked and the test case properly executed whenever the program is ran in Python in non-interactive mode.  

```python
import banff.testing as banfftest
if  __name__ == "__main__":
    banfftest.run_pytest()
```

It will instruct `pytest` to look for tests in Python's `sys.argv` variable. Generally, this contains the *current working directory* when using interactive Python, or the path to a Python file if running a Python program directly. In either case, [standard test discovery rules](https://docs.pytest.org/en/7.1.x/how-to/usage.html) are used.

# Complete Example Testcase: `test_donorimp_a03_sas.py`

## Python code

```python
import pytest
import banff.testing as banfftest

def test_donorimp_a03_a_sas(capfd):
    """The key variable is mandatory. If not specified, DonorImp should not proceed.
    Error messages should be printed to the log file.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata="donorimp_a03_a_indata.sas7bdat",
        instatus="donorimp_a03_a_instatus.sas7bdat",
        outmatching_fields=True,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        # unit_id="ident", # intentionally commented out
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: ID is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()
```

### Python Code: line by line explanation

#### Import both `pytest` and Banff's `testing` subpackage

> ```python
> import pytest 
> import banff.testing as banfftest
> ```
>
> - the use of `banfftest` as an alias for `banff.testing` is arbitrary, you may perform this import however you please

#### Define a test function

> ```python
> def test_donorimp_a03_a_sas(capfd):
>     """The key variable is mandatory. If not specified, DonorImp should not proceed.
>     Error messages should be printed to the log file.
>     """
> ```
>
> - **IMPORTANT**: the function name MUST start with `test_` or end with `_test`
> - **IMPORTANT**: the function MUST define the `capfd` parameter
>   - this is used by `pytest`
> - A description of the test may optionally be specified in a triple-quoted string immediately following the function's `def`inition line
>   - this is a standard way to document a function's purpose in Python

#### Call the *`pytest` helper function*

> ```python
>     banfftest.pytest_donorimp(
>         # Procedure Parameters
>         indata="donorimp_a03_a_indata.sas7bdat",
>         instatus="donorimp_a03_a_instatus.sas7bdat",
>         outmatching_fields=True,
>         edits="Q1 + Q2 + Q3 + Q4 = total;",
>         post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
>         min_donors=1,
>         percent_donors=1,
>         n=3,
>         # unit_id="ident", # intentionally commented out
>         must_match="staff",
>         data_excl_var="toexcl",
> 
>         # Test Parameters
>         pytest_capture=capfd, # this line must be included, internal use
>         rc_should_be_zero=False,
>         expected_error_count=1,
>         msg_list_contains_exact="ERROR: ID is mandatory.",
>     )
> ```
>
> - the first section of parameters are identical to the regular donor imputation parameters
>   - as no path was specified for the `indata` or `instatus` files, the [`control_data` folder](#control_data-folder) will be searched
> - the second section of parameters are common to all pytest helper functions and allow specification of what conditions constitute a pass
>   - `pytest_capture=capfd`: **IMPORTANT** this must always be specified, it is used by `pytest`
>   - `rc_should_be_zero=False`: test case passes only if non-zero return code (i.e. error occurred during procedure execution)
>   - `expected_error_count=1`: the substring "*`ERROR:`*" must occur exactly 1 time in the procedure's log
>   - `msg_list_contains_exact="ERROR: ID is mandatory."`: the exact message "*`ERROR: ID is mandatory.`*" must be found in the procedure log

#### Insert helper code for invoking `pytest`

>```python
> # invoke pytest if this file is executed directly
> if __name__ == "__main__":
>     banfftest.run_pytest()
> ```
>
> - allows the Banff Python package to invoke `pytest` when the python file is ran in "script mode" in Python

## SAS Code

This test case reads SAS datasets as its input.  These must be generated with SAS prior to executing the test in Python.  Simply run the SAS code which generates the datasets and copy them to a location of your choosing using a SAS `data` step.  

```sas
libname cntldata "C:/test_folder/control_data";

data donordata;
   infile cards;
   input ident $ area $ total Q1 Q2 Q3 Q4 staff;
   cards;
REC01 A1  500  120  150   80  150   50
REC02 A1  750  200  170  130  250   75
REC04 A1 1000  150  250  350  250  100
REC05 A2 1050  200  225  325  300  100
;
data cntldata.indata;
set donordata;
if (total > 1000) then toexcl='E';
else toexcl=' ';

data cntldata.instat;
   infile cards;
   input ident $ fieldid $ status $;
   cards;
REC01 Q3    IPR 
REC04 Q2    FTE 
REC04 Q4    FTI 
REC04 STAFF FTI 
;
```

## Running the example program

The testcase can be executed in a variety of ways.  The recommended approach is to run the Python file as a normal Python program, allowing the `banff.testing.run_pytest()` function to invoke `pytest` and execute the test.  There are many ways to run Python programs, such as with your preferred IDE (example *Visual Studio Code* or *IDLE*).  

### via Command Line

We have our test program saved at `C:/test_folder/test_donorimp_a03.py`.  
Open a *Command Prompt* or *PowerShell* window, change to the `test_folder` directory and run Python, specifying the testcase's filename.  

```shell
C:\>cd C:\test_folder

C:\test_folder>python test_donorimp_a03.py
```

#### Sample Output

Here is a sample of the output generated by running the test case

```text
2023-11-06 18:48:08,094 [INFO]:  Importing Banff package version 3.1.1b4.dev1
2023-11-06 18:48:08,129 [INFO]:  Banff package imported
banff.testing: Attempting to invoke Pytest on file ['test_donorimp_a03.py']
  using the following options
  ['--cache-clear', '--verbose', '-rA', '--tb=short']
=============================================== test session starts ================================================
platform win32 -- Python 3.10.11, pytest-7.4.3, pluggy-1.0.0 -- C:\temp\penv\dev2\Scripts\python.exe
cachedir: .pytest_cache
rootdir: C:\test_folder
collected 1 item

test_donorimp_a03.py::test_donorimp_a03_a_sas PASSED                                                          [100%]

====================================================== PASSES ======================================================
_____________________________________________ test_donorimp_a03_a_sas ______________________________________________
----------------------------------------------- Captured stdout call -----------------------------------------------
banff.testing: dataset 'donorimp_a03_a_indata.sas7bdat' found in control_data folder, will load file at path 'C:\test_folder/control_data/donorimp_a03_a_indata.sas7bdat'
banff.testing: dataset 'donorimp_a03_a_instatus.sas7bdat' found in control_data folder, will load file at path 'C:\test_folder/control_data/donorimp_a03_a_instatus.sas7bdat'
############################# PROCEDURE LOG STARTS on next line #################################
NOTE: --- Banff System 3.01.001b4.dev1 developed by Statistics Canada ---
NOTE: PROCEDURE DONORIMPUTATION Version 3.01.001b4.dev1
NOTE: Created on Nov  2 2023 at 12:22:40
NOTE: Email: banff@statcan.gc.ca
NOTE: Parallel processing enabled (4 threads, OpenMP: 200203)

NOTE: EDITS = Q1 + Q2 + Q3 + Q4 = total;
NOTE: POSTEDITS = Q1 + Q2 + Q3 + Q4 - total <= 0;
NOTE: MINDONORS = 1
NOTE: PCENTDONORS = 1.00%
NOTE: N = 3
NOTE: NLIMIT not specified.
NOTE: MRL not specified.
NOTE: ELIGDON = ORIGINAL (default)
NOTE: RANDOM not specified (random search of donors will not be performed)
NOTE: MATCHFIELDSTAT
NOTE: SEED = 1699314488 (value chosen by the system)
NOTE: REJECTNEGATIVE (default)
NOTE: DISPLAYLEVEL = 0
NOTE: ID not specified.
NOTE: DATAEXCLVAR = toexcl
NOTE: RANDNUMVAR not specified.
NOTE: MUSTMATCH = staff
NOTE: BY not specified.

ERROR: ID is mandatory.
############################# PROCEDURE LOG ENDED on previous line ##############################

####################################### RUNNING ASSERTIONS #######################################
assert_log_contains(clean_whitespace=False, ...)
   searching for:     ERROR: ID is mandatory.
   found in test log: True

asserted RC != 0


assert_substr_count(...)
   searching for:   ERROR:
   expected_count:  1
   test_log count:  1

assert_substr_count(...)
   searching for:   WARNING:
   expected_count:  0
   test_log count:  0


################################ ASSERTIONS COMPLETE WITHOUT ERROR ###############################
 # SAC: update this section
------------------------------------------------ Captured log call -------------------------------------------------
ERROR    banff.donorimp.submit:banff_proc.py:421 procedure Donorimp encountered an error and terminated early
ERROR    banff.donorimp.submit:banff_proc.py:422 RETURN CODE: 12
============================================= short test summary info ==============================================
PASSED test_donorimp_a03.py::test_donorimp_a03_a_sas
=========================================== 1 passed, 1 warning in 0.04s ===========================================
pytest returned 0
2023-11-06 18:48:08,244 [ERROR   , banff.donorimp.submit]:  procedure Donorimp encountered an error and terminated early
2023-11-06 18:48:08,253 [ERROR   , banff.donorimp.submit]:  RETURN CODE: 12
```

Note the "short test summary info".  When multiple test cases are executed at the same time, the result of each test will be included in this summary.  

# Other Example Test Cases

## `test_donorimp_r04.py`

In this test case the procedure is expected to pass without any errors or warnings, and produce predictable output values.  The donormap is compared against an "control" (expected) dataset.  

```python
import pytest
import banff.testing as banfftest

def test_donorimp_r04_a(capfd):
    """To verify that the output is the same for the same value of SEED.
    """
    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata="donorimp_r04_a_indata.sas7bdat",
        instatus="donorimp_r04_a_instatus.sas7bdat",
        outmatching_fields=False,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        eligdon="any",
        seed=371,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_donormap="donorimp_r04_a_donormap.sas7bdat",
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()
```

## `test_determin_a07.py`

In this testcase 2 warnings are expected.  The `msg_list_contains_exact` check is used to validate that both messages appear in the procedure log.  

```python
import pytest
import banff.testing as banfftest

def test_determin_a07_a(capfd, determin_indata_03, determin_instatus_01):
    """Observations with a missing value for the key variable in the input data set should 
    not be processed.  A warning message should be entered in the log file with a counter 
    for the number of observations dropped.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata="determin_a07_a_indata.sas7bdat",
        instatus="determin_a07_a_instatus.sas7bdat",
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=2,
        msg_list_contains_exact=[
                "WARNING: There were 1 observations dropped from DATA data set because REC is missing.",
                "WARNING: There were 1 observations dropped from DATA data set because a value is missing and there is no corresponding FTI in INSTATUS data set.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()
```
