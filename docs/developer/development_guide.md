# Banff Procedure Development Guide

## Handling Submodules

A number of *Git submodules* are employed to integrate code into this repository.  Having a basic understanding of submodules is important before making changes involving submodules.  Once properly initialized, each submodule folder contains the complete git repo associated with the submodule (it is essentially cloned into the folder).  We can interact with the submodule as-if we cloned it separately.  

The submodules in this repository can be broken down into 2 categories: 3rd party code and shared in-house code.  
3rd party code is essentially *read-only* in-house submodules may require modification during development.  These include the *jansson* library for providing JSON support and the *Apache Nanoarrow* library which provide I/O support for C code.  
In-house code includes both C and Python code which is shared among multiple in-house products.  

See the [build guide](./build_guide.md#acquiring-source-code) for advice regarding the cloning and browsing of code which involves submodules.  

### Modifying In-House Submodules

- when uncommited changes to files within a submodule exist
  - running `git status` in the submodule will show the changes
  - running `git status` in the superproject will show that the submodule contains uncommitted modifications
- when committing changes to a submodule
  - ensure that you are not in "detached head mode" (see more below)
- when changes have been committed to a submodule
  - ensure you push the new commit using `git push` from the submodule
  - running `git status` in the superproject will show that the submodule is on a different commit than expected
    - running `git diff` will show the old and new commit hashes
  - in the superproject, `git add` the submodule and `git commit` to commit to using the new submodule revision

## Debugging C Code

Debugging is done on Windows using Visual Studio Enterprise.  To achieve this, C code must be built with debugging information and then executed via Python while Visual Studio attaches a debugger to the Python process and facilicates live debugging of C code.  

There is no convenient means of debugging both C and Python code in a single execution.  Instead, debug Python code using Visual Studio Code or another preferred Python debugger.  

Consult the [build guide](./build_guide.md) and generate debug builds of the C code.  

Open [`banff.sln`](../../C/Banff.sln) in Visual Studio Enterprise/Professional.  

Visual Studio must now be configured to execute its C code via Python.  The Python code must `import banff` and call the procedure which is to be debugged.  In order to debug local builds without the need to build and install the `banff` package an environment variable configures Python `import banff` from source.  

> **Use Default Python Environment**  
> Visual Studio Enterprise will invoke Python and use the default environment.  To ensure that `banff` can be imported, all of its dependencies must be installed in the default environment.  
> If a dependency is missing, Visual Studio will fail to successfully execute the C code.  
> If `banff` is installed in the default environment, it is not known whether `import banff` will import the installed package or import from source.  

- select the `debug_en` configuration
- in the *Solution Explorer*, expand the `BanffProcedures` folder
- right click the project you want to debug and select *Set as Startup Project*
- right click the project again and select *Properties*
  - on the left side select *Debugging* and fill fields as described in the following table

|field|note|example value|
|--|--|--|
|`Command`|A path to an appropriate `python.exe`|`C:\Program Files\Python310\python.exe`|
|`Command Arguments`|A path to a Python program, or other Python arguments|`C:\git\banff-procs\Python\sample_programs\conversion_examples\Determin01.py` **or** `-m pytest ...`|
|`Working Directory`|**optionally**, working directory to be used by Python|`C:\git\banff-procs\Python\tests`|
|`Environment`|Configures Python to `import banff` from source|`PYTHONPATH=$(SolutionDir)..\Python\src\`|

Now run the solution by pressing `F5`.  When fatal errors occur, the debugger should pause on the offending code.  To debug code in which no fatal error occurs, insert a breakpoint.  

### C Codebase Layout And Flow

Each procedure project contains a file named after the project, such as `determin.c`.  It contains the main C function for the particular procedure (function name should match the filename).  Each also has a set of files with the suffix `_JIO` which implement procedure specific input and output related code introduced during the redesign.  In these `_JIO` files the function `SP_init()` is present for all procedures and coordinates the initialization of user input parameters and datasets, performing some associated validations.  This is a good place to insert an initial breakpoint when following how code executes.  

Each procedure has its own *project* and are each built into a unique shared library (`.dll` or `.so` file).  
Within each procedure project there are generally 3 procedure-specific files.  

- a main file named after the procedure
- a header and C file using the main file name with the suffix `_JIO`

> **Example**: *Deterministic Procedure Files*  
>
> - `determin.c`
> - `determin_JIO.c`
> - `determin_JIO.h`
>

The main file exposes a function named function whose name matches the filename (for instance `determin()`), this function executes the procedure start to finish.  
The flow of this file is very similar procedure to procedure.  

All other C code is generic to either Banff or to Banff and similar systems.  

The `IOUtilities` library was written as part of the redesign and generally relates to input and output from Python.  
Other libraries are legacy or 3rd party.  

#### Data Structures

Inputs from and outputs to Python are managed using a system of C structures.  Each procedure's `_JIO` files define a procedure specific structure named `SP_<proc-name>` (for instance `SP_determin`).  
This structure's first member is the `SP_generic spg` "StatCan Procedure Generic" structure which can be thought of as a super-structure (like a superclass in other languages).  It is a generic structure for tracking procedure-specific parameters, input datasets, and output datasets.  
Next there are procedure-specific structures for input datasets, generic structures for output datasets, and finally generic structures for parameters.  

```c
typedef struct __SP_determin {
    /* GENERIC INFO */
    SP_generic spg;

    /* INPUT DATA */
    DSR_indata dsr_indata;
    DSR_instatus dsr_instatus;

    /* OUTPUT DATA*/
    DSW_generic dsw_outdata;
    DSW_generic dsw_outstatus;

    /* INPUT PARAMETERS */
    UP_FLAG accept_negative;
    UP_FLAG no_by_stats;
    UP_QS edits;
    UP_QS unit_id;
    UP_QS by;
} SP_determin;
```

Input datasets get their own procedure-input-dataset-specific structure to associate procedure-specific variable lists with generic input dataset structures.  
For instance, the donor imputation's indata dataset has custom variable lists such as the *data exclusion*, *must match* and *randon number* variable lists.  

```c
typedef struct __DSR_indata {
    /* GENERIC INFO */
    DSR_generic dsr;

    /* VARIABLE LISTS */
    DS_varlist VL_data_excl;  // DATA_EXCL_VAR: optional single variable
    DS_varlist VL_must_match; // MUST_MATCH: list with 0 or more elements
    DS_varlist VL_rand_num; // RAND_NUM_VAR: optional single variable
} DSR_indata;
```

Generic variable lists such as `unit_id` or `by` are stored in the `DSR_generic` structure.  

#### Handle Inputs From Python

These structures are initialized near the beginning of the main function's execution with a call to `SP_init()`, which accepts

- the procedure-specific `SP_<proc-name>` structure
- JSON encoded string with parameter key-value pairs
- 1 reference for each input dataset
- 2 references for each output dataset

`SP_init()` parses all the inputs from Python and performs some initial validations while storing the information in the system of structures described above.  It makes extensive use of the `IOUtilities` library which supplies the generic structures and associated library functions.  
This function returns a code which is assessed and if it passes, further validation on the structure commences with a call to `SPG_validate()`, which automatically validates generic structures in the now-initialized structure system.  

#### Legacy Setup Code

Each procedure now generally calls `GetParms()`, `PrintParms()` and `ValidateParms()`, in that order.  Each is a procedure-specific function which references data from the now-validated structure system and stores it in legacy variables and structures which are used later on in processing.  

There can be some variability from one procedure to another at this point.  

Next, legacy code performs setup on input and output datasets.  This step associates the initialized and validated input datasets in the structure system with specific locations in memory where data from input datasets will be written to, and data for output datasets will be sourced from.  In general, these memory locations are managed by legacy code.  
For input datasets these functions may have the substring `ScatterRead`, and output datasets `GatherWrite`.  

#### Legacy Processing Code

Once all setup is complete, some procedures call a function in which all processing is performed.  Other procedures implement the processing in a loop right in the main function.  
Processing code will make calls to `IOUtilities` functions in order to read data from input datasets, write data to output datasets, among other things.  

#### Cleanup Code

Procedures generally define "labels" `normal_cleanup` and `error_cleanup`.  For former is often never called directly, however `error_cleanup` is used as a means of skipping the remaining processing when an error occurs while ensuring that memory allocations are safely freed.  

Before the `error_cleanup` label there is a call to `SP_wrap()`.  This finalizes the writing of output datasets, marking them as ready for consumption by Python.  

Legacy code frees legacy memory structures and finally a call to `SPG_free()` frees the entire structure system.  

### Tracing Log Messages and Return Codes

C code produces console log messages, particularly when errors occur.  These can often be used to determine the C execution flow for a given input.  Visual Studio Enterprise/Professional's *Find in Files* can be used to search the entire C codebase for such messages, which are typically defined stored in a preprocessor macro.  Many messages substitute values at runtime, so search for a substring of the message which does not include any specific option names or values.  Once a message is found, use *Find in Files* or *Find All References* to determine where in the code the message is used.  

Similarly, return codes can be used to trace the flow of execution by finding the enumeration member associated with a given return code and determining where in the C code that member is referenced.  

## Where in Python Code does C Get Called

Each procedure is exposed to users as a procedure-specific class (for instance `ProcDetermin`), the user supplying all execution information in the class constructor.  These classes are all derived from a banff-specific class `BanffProcedure` which itself inherits from a generic class `GeneralizedProcedure`.  

`GeneralizedProcedure` implements the majority of execution logic for execution in its `_execute()` method.  Specifically, a call is made to the `_run_proc()` method which in turn calls `self._call_c_code()`.  This latter method is defined by the procedure-specific class `_call_c_code()` method which implements the actual line of code that invokes C execution.  Putting a breakpoint in any of these locations will allow you to determine if an error is occuring *debug* C code execution, but will not provide any debugging facilities for that C code.  
