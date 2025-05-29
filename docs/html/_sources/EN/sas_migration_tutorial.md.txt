# Tutorial

This tutorial will show the Python equivalent (`/Python/sample_programs/conversion_examples/Errorloc01.py`) of the SAS language Proc Errorloc sample program 1 (`/Python/sample_programs/conversion_examples/Errorloc01.sas`).  
Numerous additional SAS and Python equivalents are also available in the `/Python/sample_programs/conversion_examples/` folder.  

The example program shows how to

- create a synthetic table
- sort the table
- specify
  - parameters
  - input table
  - output table options
- access results

It will discuss some relevant differences between SAS and Python as well.  

## SAS Language example

```sas
/* create synthetic input table */
data example_indata;
input IDENT $ X1 X2 ZONE $1.;
cards;
R03 10 40 B
R02 -4 49 A
R04 4 49 A
R01 16 49 A
R05 15 51 B
R07 -4 29 B
R06 30 70 B
;
run;

/* sort by BY and KEY variables */
proc sort data=example_indata; by ZONE IDENT;run;

/* create Banff call */
proc errorloc
data=example_indata
outstatus=outstatus
outreject=outreject
edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;"
weights="x1=1.5;"
cardinality=2
timeperobs=.1
;
id IDENT;
by ZONE;

/* execute Banff call */
run; 
```

## Python language equivalent

```python
# import Banff package
import banff
import pyarrow as pa

# create a schema for the indata table
indata_schema = pa.schema([
            ("IDENT", pa.string()),
            ("X1", pa.int64()),
            ("X2", pa.int64()),
            ("ZONE", pa.string()),
    ])

# create table using schema and lists of values for each column
indata = pa.table(
    schema=indata_schema,
    data=[
        ["R03", "R02", "R04", "R01", "R05", "R07", "R06"],
        [10, -4, 4, 16, 15, -4, 30],
        [40, 49, 49, 49, 51, 29, 70],
        ["B", "A", "A", "A", "B", "B", "B"],
    ],
)

# sort according to `by` variable and `unit_id`
indata = indata.sort_by([
    ("ZONE", "ascending"),
    ("IDENT", "ascending"),
])

# run Banff procedure
banff_call = banff.errorloc(
    indata=indata,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE",
)
```

## Line by line explanation

### Import packages

In Python, *packages* must be *imported* into a session before they can be used.  

```python
import banff
import pyarrow as pa
```

The [`pyarrow`](https://pypi.org/project/pyarrow/) package is used for creating and manipulating tables.  Note that the *alias* `pa` is used for the `pyarrow` package.  

### Create synthetic data

We create the same synthetic table that was created in SAS.  

A *Pyarrow Schema* object is created and assigned to the variable `indata_schema`.  

```python
# create a schema for the indata table
indata_schema = pa.schema([
            ("IDENT", pa.string()),
            ("X1", pa.int64()),
            ("X2", pa.int64()),
            ("ZONE", pa.string()),
    ])
```

- this is used to define the name and datatype of each column in a table
- Pyarrow offers an [extensive set of datatypes](https://arrow.apache.org/docs/python/api/datatypes.html#factory-functions)
- for more details, see [`pyarrow.schema` documentation](https://arrow.apache.org/docs/python/generated/pyarrow.schema.html)

A *Pyarrow Table* object is created and stored in the `indata` variable.  

```python
# create table using schema and lists of values for each column
indata = pa.table(
    schema=indata_schema,
    data=[
        ["R03", "R02", "R04", "R01", "R05", "R07", "R06"],
        [10, -4, 4, 16, 15, -4, 30],
        [40, 49, 49, 49, 51, 29, 70],
        ["B", "A", "A", "A", "B", "B", "B"],
    ],
)
```

- the order of columns in the schema corresponds to the order of lists of data
- for more details, see [`pyarrow.table` documentation](https://arrow.apache.org/docs/python/generated/pyarrow.table.html)
- unlike SAS, where tables are typically stored as files, Pyarrow Tables are typically stored in memory as "*objects*", hence the need to assign to the `indata` variable

The `indata` table is sorted using its `sort_by()` *method* and assigned back to itself.  

```python
# sort according to `by` variable and `unit_id`
indata = indata.sort_by([
    ("ZONE", "ascending"),
    ("IDENT", "ascending"),
])
```

- in SAS this would be analogous to using `proc sort data=indata` without specifying the `out` option
- most operations in pyarrow return a sort of copy of the table, meaning the source table is not modified
- to retain the unsorted table, simply assign the sorted table to a different variable
  - `indata_sorted = indata.sort_by(...` will give you a new copy which is sorted and leave the original `indata` copy as-is

#### Python Concepts

> - in Python, variables are often *objects*, containing not only data but also *methods*
> - methods are often useful for getting information about, or performing operations on, the data
> - the `indata` variable is a *Pyarrow Table* (`pyarrow.Table`) object, and accordingly has its own [`sort_by()` method](https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table.sort_by)
>   - documentation on other methods available for Pyarrow Tables can be found [here](https://arrow.apache.org/docs/python/generated/pyarrow.Table.html#pyarrow.Table).

### Running The Banff Procedure

```python
banff_call = banff.errorloc(
    indata=indata,
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE",
)
```

Calling `banff.errorloc()` results in the errorloc procedure executing and an object being assigned to the `banff_call` variable.  The object can be used to access output tables.  

- note that all parameters and tables are specified as comma separated key-value pairs
- they can appear in any order
- `indata=indata` specifies that the `indata` table (recently sorted) should be provided as the "indata" table

### Procedure Execution

During procedure execution, the text written to the console by the procedure should be nearly identical to the SAS log from the equivalent example.  

### Accessing output tables

After execution completes, output table options are processed.  The default option was specified above, so the output tables are available as pyarrow tables.  

They are stored in the `banff_call` object, access them using `banff_call.outstatus` and `banff_call.outreject`.  From here they can be handled by users like any other pyarrow table, for example:

- written to file
- manipulated (sorted, merged, etc.)
- used as input for another procedure (`instatus=banff_call.outstatus`)

### Other input and output options

To provide a consistent means of reading/writing files and converting between table formats, while maintaining the highest possible floating-point precision, support for various input and output table formats has been implemented.  For complete details, see the [User Guide](./user_guide.md#supported-formats).  

The following code will demonstrate the use of files for input and output tables by modifying the above example.  

```python
banff.errorloc( 
    indata=r"C:\temp\in_data.feather",
    outstatus=r"C:\temp\out_status.parquet",
    outreject=r"C:\temp\out_reject.parquet",
    edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
    weights="x1=1.5;",
    cardinality=2,
    time_per_obs=0.1,
    unit_id="IDENT",
    by="ZONE"
)
```

- note that there is no need to assign the object returned by `banff.errorloc()` to a variable because output data is written to disk
