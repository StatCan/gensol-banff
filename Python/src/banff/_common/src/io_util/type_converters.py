"""type_converters module: convert between supported data formats.

This module implements functions for converting between the data formats supported by the
`.io_util` sub-package (see its `__init__.py` docstring for a summary).
These low level functions generally wrap a single line which calls on a 3rd party package
to perform the work.
This module intends to always use options to maximize floating point accuracy and balance
conversion time with output size, etc.
"""

# 3rd party libraries
import json
from enum import IntEnum

import nanoarrow as na
import pandas as pd
import pyarrow as pa
import pyarrow.csv
import pyarrow.feather
import pyarrow.parquet

from ..exceptions import (
    TypeConverterError,
)
from ..io_util.processing import handle_pandas_string_data
from ..nls import _

# Options
feather_mmap_mode = IntEnum(
    "feather_mmap_mode", [
        "pa_ipc_mmap",  # memory map with `ipc` method
        "pa_feather_mmap",  # memory map with `feather.read_table` method
        "pa_feather_nommap",  # no memory map with `feather.read_table` method
])
OPT_feather_mmap_mode_default = feather_mmap_mode.pa_ipc_mmap
OPT_feather_mmap_mode = OPT_feather_mmap_mode_default

OPT_parquet_use_mmap_default=False
OPT_parquet_use_mmap=OPT_parquet_use_mmap_default

# For analyzing Dataset Specifications
#   Users may specify in/output datasets as file paths, these lists match
#   file extensions with file types (and therefore functions, via `io_util.py`
#   For output datasets, "output to object" by specifying the object's "identifier"
# lists of supported file extensions
file_ext_parquet = [".parquet", ".parq"]
file_ext_sas     = [".sas7bdat"]
file_ext_csv  = [".csv"]
file_ext_feather = [".feather"]
# list of supported object type "identifiers"
#   Should match documentation's "Table of Supported Objects"
obj_id_pandas = ["pandas", "dataframe", "pandas dataframe"]
obj_id_pyarrow = ["pyarrow", "table", "pyarrow table"]

supported_output_obj_id = obj_id_pandas + obj_id_pyarrow

jansson_input_encoding = "utf-8"
jansson_output_encoding = jansson_input_encoding

# type mapping string data: arrow -> pandas
#   Control resulting column types when converting an arrow table to a pandas dataframe.
#   Use this dict's `.get` method as a `types_mapper` in `pa.Table.to_pandas(types_mapper=)`.
#   Uses `pd.StringDtype()` so we get the pandas recommended type.
#
#   pandas recommends `pd.StringDtype()` for text data:
#    - https://pandas.pydata.org/pandas-docs/version/2.2/user_guide/text.html
#    - default `storage_type` depends on `pd.options.mode.string_storage`
#    - for instance: when using `storage="pyarrow"` result depends on pandas version
#        - `pandas<=2.2.0`: `pa.string()`
#        - `pandas>=2.2.0`: `pa.large_string()`
#
#   to force pandas to use a specific pyarrow type:
#    - `pd.ArrowDtype(pa.large_string())`
#    - `pd.ArrowDtype(pa.string())`
type_mapping_arrow_pandas_string={
    pa.string():            pd.StringDtype(storage="pyarrow"),
    pa.large_string():      pd.StringDtype(storage="pyarrow"),
}

# default output is a supported object identifier
default_output_specification = obj_id_pyarrow[0]

def get_default_output_spec():
    """Return current default output dataset specification."""
    return default_output_specification

def set_default_output_spec(new_spec):
    """Set default output output dataset specification to a supported object type identifier."""
    supported_specs = obj_id_pandas + obj_id_pyarrow

    if new_spec not in supported_specs:
        mesg = _("Specification '{}' not supported, must use one of '{}'").format(new_spec, supported_specs)
        raise ValueError(mesg)

    global default_output_specification
    default_output_specification = new_spec

def DF_from_csv_file(file_path):
    """Load CSV file into Pandas DataFrame."""
    # python engine looks safest, and `float_precision` only applies with `engine='c'`, right?`
    try:
        df = pd.read_csv(filepath_or_buffer=file_path, engine="python")
        df = handle_pandas_string_data(df)
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("CSV", file_path)
        raise TypeConverterError(mesg) from e
    else:
        return df

def DF_from_parquet_file(file_path):
    """Load Apache Parquet file into Pandas DataFrame."""
    # `pyarrow` best ensures floating point accuracy
    try:
        return pd.read_parquet(path=file_path, engine="pyarrow")
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("Parquet", file_path)
        raise TypeConverterError(mesg) from e

def DF_from_sas_file(file_path):
    """Load SAS dataset file into Pandas DataFrame."""
    try:
        # `encoding='infer'`: SAS datasets specify their encoding, pandas uses it
        df = pd.read_sas(filepath_or_buffer=file_path, encoding="infer")
        df = handle_pandas_string_data(df)
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("SAS dataset", file_path)
        raise TypeConverterError(mesg) from e
    else:
        return df

def DF_to_PAT(pd_dataframe):
    """Convert Pandas DataFrame to PyArrow Table.

    `preserve_index=False`
      - do not include the pandas "index" as an additional column in the pyarrow table
    """
    try:
        pa_t = pa.Table.from_pandas(
            df=pd_dataframe,
            preserve_index=False,
        )
    except Exception as e:
        # CONTEXT: ...converting <object format> to <another object format>
        mesg = _("Unexpected error occurred while converting `{}` to `{}`").format("pandas.DataFrame", "pyarrow.Table")
        raise TypeConverterError(mesg) from e
    else:
        return pa_t
# alias
DF_to_arrow_table = DF_to_PAT

def DF_to_arrow_stream(pd_dataframe):
    """Convert Pandas DataFrame to PyArrow Stream."""
    try:
        stream = PAT_to_arrow_stream(DF_to_PAT(pd_dataframe))
    except Exception as e:
        mesg = _("Unexpected error occurred while converting `{}` to `{}`").format("pandas.DataFrame", "CArrayStream")
        raise TypeConverterError(mesg) from e
    else:
        return stream

def DF_to_csv_file(dataset, destination):
    """Write Pandas DataFrame to CSV file."""
    # explicit (default) overwrite existing file with `mode='w'`
    try:
        rc = dataset.to_csv(
            path_or_buf=destination,
            mode="w",
            index=False,
        )
        if rc is not None:
            mesg = _("pandas reports failure to write to file")
            raise ValueError(mesg)  # noqa: TRY301  #  This seems fine to me
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pandas.DataFrame", "CSV", destination)
        raise TypeConverterError(mesg) from e

def DF_to_feather_file(dataset, destination):
    """Write Pandas DataFrame to Feather file."""
    try:
        dataset.to_feather(path=destination)
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pandas.DataFrame", "Feather", destination)
        raise TypeConverterError(mesg) from e

def DF_to_parquet_file(dataset, destination, index=False):
    """Write Pandas DataFrame to Parquet file."""
    # `pyarrow` best ensures floating point accuracy, we don't use index
    try:
        rc = dataset.to_parquet(path=destination, engine="pyarrow", index=index)
        if rc is not None:
            mesg = _("pandas reports failure to write to file")
            raise ValueError(mesg)  # noqa: TRY301  #  This seems fine to me
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pandas.DataFrame", "Parquet", destination)
        raise TypeConverterError(mesg) from e

def dict_to_json(some_dict):
    """Convert key-value dictionary to JSON `str`."""
    # this function used to convert dictionary of user C-code parameters to JSON
    try:
        return json.dumps(some_dict)
    except Exception as e:
        mesg = _("Unexpected error occurred while converting `{}` to `{}`").format("dict", "JSON")
        raise TypeConverterError(mesg) from e

def PAT_from_parquet_file(
        file_path,
        memory_map: bool | None = None,
):
    """Load parquet file into PyArrow Table (PAT).

    `memory_map`:   Whether or not to use a memory map.
                    Can be `True` or `False`.
                    If unspecified, uses global `OPT_parquet_use_mmap`.
    """
    if memory_map is None:  # if unspecified, use global option
        memory_map = OPT_parquet_use_mmap
        if memory_map not in (True, False):  # if global option invalid, use default
            memory_map = OPT_parquet_use_mmap_default

    try:
        pa_t = pyarrow.parquet.read_table(
            source=file_path,
            memory_map=memory_map,
        )
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("Parquet", file_path)
        raise TypeConverterError(mesg) from e
    else:
        return pa_t

def PAT_from_csv_file(
        file_path,
):
    """Load a CSV file into a pyarrow Table.

    Uses default pyarrow settings.
    """
    try:
        pa_t = pyarrow.csv.read_csv(input_file=file_path)
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("CSV", file_path)
        raise TypeConverterError(mesg) from e
    else:
        return pa_t

def PAT_from_feather_file(
        file_path,
        memory_map : bool | feather_mmap_mode | None = None,
):
    """Load feather file into PyArrow Table (PAT).

    `memory_map`:   What memory map option to use.
                    Can be `True`, `False`, or a member of `feather_mmap_mode`.
                    If unspecified, uses global `OPT_feather_mmap_mode`.

    NOTE: memory map modes
        uncertain whether the `pa.memory_map()` with `pa.ipc.open_file(...).read_all()` method
        ends up using RAM or not, compared to `pa.feather.read_table(..., memory_map=True)`
        see https://arrow.apache.org/cookbook/py/io.html#memory-mapping-arrow-arrays-from-disk
    """
    if memory_map is True:  # if True, use ipc mmap
        memory_map = feather_mmap_mode.pa_ipc_mmap
    elif memory_map is False:  # if Fasle, no memory map
        memory_map = feather_mmap_mode.pa_feather_nommap
    elif memory_map is None:  # if unspecified, use global option
        memory_map = OPT_feather_mmap_mode
        if memory_map not in feather_mmap_mode:  # if global option invalid, use default
            memory_map = OPT_feather_mmap_mode_default

    try:
        match memory_map:
            case feather_mmap_mode.pa_ipc_mmap:
                with pa.memory_map(file_path, "r") as source:
                    pa_t = pa.ipc.open_file(source).read_all()
            case feather_mmap_mode.pa_feather_mmap:
                pa_t = pyarrow.feather.read_table(
                    source=file_path,
                    memory_map=True,
                )
            case feather_mmap_mode.pa_feather_nommap:
                pa_t = pyarrow.feather.read_table(
                    source=file_path,
                    memory_map=False,
                )
    except Exception as e:
        mesg = _("Unexpected error occurred while loading {} file: '{}'").format("Feather", file_path)
        raise TypeConverterError(mesg) from e
    else:
        return pa_t

def PAT_from_sas_file(file_path):
    """Load SAS dataset file into pyarrow Table."""
    return DF_to_PAT(
        DF_from_sas_file(file_path=file_path),
    )

def PAT_to_arrow_stream(pa_t):
    """Convert PyArrow Table to PyArrow Stream."""
    try:
        return na.c_array_stream(pa_t)
    except Exception as e:
        mesg = _("Unexpected error occurred while converting `{}` to `{}`").format("pyarrow.Table", "CArrayStream")
        raise TypeConverterError(mesg) from e

def PAT_to_DF(pa_t):
    """Convert PyArrow Table to Pandas DataFrame.

    pyarrow option `split_blocks=True` recommended for minimizing memory usage.
      see https://arrow.apache.org/docs/python/pandas.html#reducing-memory-use-in-table-to-pandas
    Uses `types_mapper` to ensure pandas uses `string`, not `object` for string data.
    """
    try:
        return pa_t.to_pandas(
            split_blocks=True,
            types_mapper=type_mapping_arrow_pandas_string.get,
        )
    except Exception as e:
        mesg = _("Unexpected error occurred while converting `{}` to `{}`").format("pyarrow.Table", "pandas.DataFrame")
        raise TypeConverterError(mesg) from e

def PAT_to_csv_file(
        dataset,
        destination,
):
    """Write pyarrow Table to CSV file.

    Uses default pyarrow settings
    """
    try:
        pyarrow.csv.write_csv(
            data=dataset,
            output_file=destination,
        )
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pyarrow.Table", "CSV", destination)
        raise TypeConverterError(mesg) from e


def PAT_to_feather_file(dataset, destination):
    """Write PyArrow Table to feather file."""
    try:
        pyarrow.feather.write_feather(
            df=dataset,
            dest=destination,
        )
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pyarrow.Table", "Feather", destination)
        raise TypeConverterError(mesg) from e

def PAT_to_parquet_file(dataset, destination):
    """Write PyArrow Table to parquet file."""
    try:
        pyarrow.parquet.write_table(table=dataset, where=destination)
    except Exception as e:
        mesg = _("Unexpected error occurred while writing `{}` to {} file: '{}'").format("pyarrow.Table", "Parquet", destination)
        raise TypeConverterError(mesg) from e

def SAS_file_to_feather_file(file_path, destination):
    """Read SAS file and write to feather file."""
    pa_t = DF_to_PAT(
        DF_from_sas_file(file_path),
    )
    PAT_to_feather_file(dataset=pa_t, destination=destination)

def SAS_file_to_parquet_file(file_path, destination):
    """Read SAS file and write to parquet file."""
    pa_t = DF_to_PAT(
        DF_from_sas_file(file_path),
    )
    PAT_to_parquet_file(dataset=pa_t, destination=destination)
