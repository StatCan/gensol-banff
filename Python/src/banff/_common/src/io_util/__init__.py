"""Input/Output Utilities.

This package facilitates the conversion to and from various file and in memory dataset
formats and a *C-runtime-compatible format* which can be passed by reference to C-code.

In general the goal is to provide a dataset interface where users provide a dataset in
one of many supported formats, this package converts that to a format which is allows
- efficient manipulation of the dataset
- a lossless zero-copy mechanism for sharing with C code
- a losless zero-copy mechanism for receiving C code output datasets
- writing to a variety of supported formats

The `pyarrow` and `nanoarrow` packages are key in facilitating efficent memory usage,
particularly when exchanging data with C code.

I/O processing during a procedure call will generally follow a path like this:
  1. *load* input dataset, if the file format is supported, into *intermediate format*
  2. *pack* intermediate format data into a *C-runtime-compatible format*
  3. The procedure is executed
  4. *unpack* output dataset(s) to an *intermediate format*
  5. *write* output dataset(s) to file format specified by user

MODULES:
`io_util.type_converters` implements or integrates support from existing packages to
  - convert from one supported data format to another
  - read supported dataset files
  - write supported dataset files
`io_util.io_util` performs high-level functions like
  - load input dataset
  - prepare dataset for passing to procedure procedure
  - handle output datsets according to user specifications
  It inspects the types of datasets or specifications it receives and selects the
  appropriate `type_converters` function for the task.
"""

# import only the functions intended for external use

from .io_util import (
    GensysInputDataset,
    GensysOutputDataset,
    c_argtype_input_dataset,
    c_argtype_output_dataset,
    c_argtype_parameters,
    c_return_type,
    dest_is_file,
    dest_is_object,
    load_input_dataset,
    pack_dataset,
    pack_parms,
    unpack_output_dataset,
    write_output_dataset,
)
from .processing import (
    add_status_by_vars,
    flag_rows_where,
    remove_rows_where,
    sort_dataset,
)
from .type_converters import (
    DF_from_sas_file,
    PAT_from_sas_file,
    SAS_file_to_feather_file,
    SAS_file_to_parquet_file,
    get_default_output_spec,
    set_default_output_spec,
)
from .type_converters import (
    DF_to_PAT as pandas_to_arrow,
)
from .type_converters import (
    PAT_to_DF as arrow_to_pandas,
)

__all__ = [
    "DF_from_sas_file",
    "GensysInputDataset",
    "GensysOutputDataset",
    "PAT_from_sas_file",
    "SAS_file_to_feather_file",
    "SAS_file_to_parquet_file",
    "add_status_by_vars",
    "arrow_to_pandas",
    "c_argtype_input_dataset",
    "c_argtype_output_dataset",
    "c_argtype_parameters",
    "c_return_type",
    "dest_is_file",
    "dest_is_object",
    "flag_rows_where",
    "get_default_output_spec",
    "load_input_dataset",
    "pack_dataset",
    "pack_parms",
    "pandas_to_arrow",
    "remove_rows_where",
    "set_default_output_spec",
    "sort_dataset",
    "unpack_output_dataset",
    "write_output_dataset",
]
