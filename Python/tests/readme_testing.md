# Testing

## What Is Actually Tested

Although these tests may be found alongside the Python source code for `banff`, they are designed to test an *installed* version of the `banff` package.  If you intend to run tests on the source files in this repository, see [run tests directly on source files](#run-tests-on-source-files).  

## Tests in this folder

The `features` folder contains tests of features introduced in Banff version 3.  
The `low_level` folder contains basic execution tests for each procedure.  These test the procedure build directly, without any testing of the Python package code.  
The `ptest` folder contains files related to performance tests.  Some use synthetic data and others require sensitive data which is not included in the repository.  
The `python_code` folder tests Python code only, not including any execution of C coded procedures.  
The `regression_tests` folder contains procedure test cases written by methodologists to validate the correct operation of procedures.  
The `unit_tests` folder contains old implementations (many need to be updated) of methodology test cases.  

## How to run tests

The `run_pytest` batch script facilitates running tests and is implemented as []`run_pytest.cmd`](./run_pytest.cmd) for Windows and ['run_pytest.sh`](./run_pytest.sh) for Linux.  

- `run_pytest /auto_pass` runs all tests which are suitable for automated testing expected to pass.  This avoids running tests which are known to contain issues and skips any long running performance tests
- `run_pytest /lf` runs all tests which failed during the last run

These tests can however be run any way that you wish to [invoke `PyTest`](https://docs.pytest.org/en/7.1.x/how-to/usage.html).  

## Testing environment

### Run tests on source files

Run tests on source files, as opposed to running tests on "installed" files.  Under this setup, running `import banff` will cause Python to read the source files directly, instead of using an installed copy of the package.  

To achieve this we want to import the package into our Python session from its source files.  This allows immediate testing and live debugging of source code during development.  There is no need to build the package\* after each change.  

* \* this refers to changes of the *Python* code
* \* *C* code must be built at least once before testing the package from source and must always be rebuilt after *C* source code changes

I recommend [creating a virtual environment](#creating-a-virtual-environment) and activating a virtual environment specifically for this type of testing.  

Ensure that `banff` is not already installed in the Python environment you are using.  

```batch
C:\> python -m pip show banff
WARNING: Package(s) not found: banff
```

Install from source

```batch
C:\git\banff_redesign\Python> python -m pip install -e .
```

Note that `-e .` specifies to install the current folder in editable mode.  Alternatively an absolute path can be specified.  

Inspect the installed package.  

```batch
(dev2) C:\> python -m pip show banff
Name: banff
Version: 3.1.1a3
Summary: Initial Banff package
Home-page:
Author:
Author-email: Stephen Arsenualt <stephen.arsenault@statcan.gc.ca>
License:
Location: c:\git\banff_redesign\Python
Requires: pandas
Required-by:
```

Note that the *Location* field now indicates a folder in the `banff_redesign` git repository, as expected.  

## Creating a virtual environment

A *virtual environment* is like a "profile" for Python, having its own set of packages.  Installing packages to a virtual environment will not affect other virtual environments or the default Python environment.

To create the virtual environment we will invoke Python's `venv` module (using the `-m` switch) and specify a path to a folder which will be created to contain the virtual environment.  

```batch
C:\> python -m venv C:\temp\test_venv\
```

Alternatively, assuming you are already in the directory you will create your virtual environment in, you can pass a relative path.  

```batch
C:\temp> python -m venv test_venv
```

A new folder, `test_venv` will contain various folders and files.  
**To "*activate*" the new environment**, run the appropriate `activate` script.  
Inside the `Scripts` folder there will be activate scripts compatible with both Linux and Windows.  Running the following is sufficient for Windows to locate and run the correct script.  

```batch
C:\temp> test_venv\Scripts\activate

(test_venv) C:\temp\penv> rem Now the Python environment has been activated

(test_venv) C:\temp\penv>
```

Notice how the environment's name is prefixed in the terminal prompt.  
This change **only affects the current terminal session**.  
**To "*deactivate*" the environment**, run the `deactivate` script, or close your terminal and open a new one.  
