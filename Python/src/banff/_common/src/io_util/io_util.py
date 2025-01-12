"""io_util module: generic dataset input, output, and conversion.

This module implements a set of functions for reading files, converting between data formats, and writing files.
Three sets of supported formats exist:
  - input
  - intermediate
  - output

This library implements the logic for automatically determining which of the supported formats the file is in
and which function to use to convert it to the specified output format.
It abstracts the low-level functions which perform these actions (see `type_converters` module)
"""
import ctypes
from abc import (
    ABC,
    abstractmethod,
)
from io import StringIO
from pathlib import Path

import pandas as pd
import pyarrow as pa
from nanoarrow.c_array import allocate_c_array
from nanoarrow.c_schema import allocate_c_schema

from .. import _log as lg
from ..io_util import type_converters as tc
from ..nls import _

jansson_input_encoding = "utf-8"
jansson_output_encoding = jansson_input_encoding
class GensysDataset(ABC):
    """Abstract Base Class for procedure input and output datasets.

    This class used primarily by the GeneralizedProcedure class and its subclasses
    It is used as a base class for both input and output datasets.
    It keeps track of the user-provided dataset (or output dataset specification) while
    providing an "intermediate format" copy of the dataset suitable for processing without
    regard to the user provided/requested format.
    `user_spec` holds the user-provided value in both cases, this value shall not be modified.
        For input datasets
            - `ds_intermediate` will be populated with the
            intermediate formatted version of `user_spec`
        For output datasets
            - `ds_intermediate` will hold the "unpacked" output dataset from
            the C code

    MEMBERS:
    `user_spec` - user-provided input dataset OR output dataset specification
    `ds_intermediate` - intermediate format dataset
    `mandatory` - if the dataset is mandatory (user provides input OR c must generate output)
    """

    def __init__(self, name, user_spec, mandatory=True):
        self.name = name
        self.user_spec = user_spec
        self.ds_intermediate = None
        self.mandatory=mandatory

        self.validate_user_spec()

    @property
    def is_specified(self):
        """Whether or not a dataset was specified by the user."""
        return self.user_spec is not None

    @property
    def not_specified(self):
        return not self.is_specified

    @property
    def num_rows(self):
        """Return the number of rows in the `ds_intermediate` object or `None` if that object is `None` or invalid."""
        if isinstance(self.ds_intermediate, pd.DataFrame):
            return len(self.ds_intermediate.axes[0])

        if isinstance(self.ds_intermediate, pa.Table):
            return self.ds_intermediate.num_rows

        return None  # is `None` or is invalid, return `None`

    @property
    def num_columns(self):
        """Return the number of columns in the `ds_intermediate` object or `None` if that object is `None` or invalid."""
        if isinstance(self.ds_intermediate, pd.DataFrame):
            return len(self.ds_intermediate.axes[1])

        if isinstance(self.ds_intermediate, pa.Table):
            return self.ds_intermediate.num_columns

        return None  # is `None` or is invalid, return `None`

    def get_ds_info(self):
        """Return string with info about intermediate dataset.

        When `.ds_intermediate` is not None, generate a string containing:
            - number of rows
            - number of columns
            - names and types of each column
            - space used (not necessarily RAM used)
        Otherwise, return an empty string.
        """
        if isinstance(self.ds_intermediate, pd.DataFrame):
            temp_buff=StringIO()
            self.ds_intermediate.info(buf=temp_buff, memory_usage=True, show_counts=True)
            out_str = temp_buff.getvalue()
        elif isinstance(self.ds_intermediate, pa.Table):
            out_str = self.ds_intermediate.schema.to_string(show_schema_metadata=False)
            out_str += "\n"
            out_str += _("Bytes used: {}").format(self.ds_intermediate.nbytes)
        else:
            out_str = ""

        return out_str

    def log_metadata(self, log):
        """Print various intermediate dataset metadata to log.

        Does nothing if intermediate dataset is `None`
        """
        if self.ds_intermediate is not None:
            log.info(_("Dataset '{}' has {} rows and {} columns").format(self.name, self.num_rows, self.num_columns))
            log.debug(_("Dataset '{}' has the following metadata\n{}").format(self.name, self.get_ds_info()))

    def to_arrow(self):
        """Return intermediate dataset in arrow format, converting if necessary."""
        if self.ds_intermediate is None:
            return None

        self.ds_intermediate = interm_to_PAT(self.ds_intermediate)
        return self.ds_intermediate

    def to_pandas(self):
        """Return intermediate dataset in pandas format, converting if necessary."""
        if self.ds_intermediate is None:
            return None

        self.ds_intermediate = interm_to_DF(self.ds_intermediate)
        return self.ds_intermediate

    @abstractmethod
    def validate_user_spec(self):
        """Validate the user specification."""

class GensysInputDataset(GensysDataset):
    """Wrapper for procedure input datasets.

    Please also read super-class (`GensysDataset`) documentation.
    """

    def __init__(self, name, input_dataset, mandatory=True):
        self.c_input = None
        self.c_arg = None

        super().__init__(
            name=name,
            user_spec=input_dataset,
            mandatory=mandatory,
        )

    def init_c_args(self):
        """Low-level preparation of C input dataset arguments.

        Given the packed input dataset, cast it to a datatype suitable for C to consume.

        For Apache Arrow "C Array Stream", we're creating a pointer to the nanoarrow created
        structure and passing that as a typeless "void pointer", instead of implementing some complex logic
        for passing the structure directly.
        If the dataset is `None`, send a "null pointer".
        """
        if self.c_input is None:
            self.c_arg = None  # `None` seems equivalent to `ctypes.cast(0, ctypes.c_void_p)`
        else:
            # it's a `nanoarrow.c_lib.CArrayStream`
            # nanoarrow uses `_addr()` in public examples, doesn't discourage use
            # See https://arrow.apache.org/nanoarrow/0.5.0/reference/python/advanced.html
            self.c_arg = ctypes.cast(self.c_input._addr(), ctypes.c_void_p)  # noqa: SLF001

    def free_c_args(self):
        self.c_arg = None
        self.c_input = None

    def validate_user_spec(self):
        """Validate the user specification."""
        # currently handled by C code

class GensysOutputDataset(GensysDataset):
    """Wrapper for procedure output datasets.

    INTENDED USE:
        - `user_output` will be populated based on `user_spec` with either
            - the requested object format of `ds_intermediate`
            - the path which `ds_intermediate` was written to

    MEMBERS:
        `c_output` - C code generated output dataset
        `requested_by_default` - whether or not to produce, when unspecified by user
        `user_output` - output dataset in user requested format (or user_spec when written to file)

    Please also read super-class (`GensysDataset`) documentation.
    """

    def __init__(self, name, output_specification, mandatory=True, requested_by_default=True):
        self.schema = None
        self.array = None
        self.c_schema = None
        self.c_array = None
        self.c_output = None

        self.user_output = None

        self.requested_by_default = requested_by_default

        super().__init__(
            name=name,
            user_spec=output_specification,
            mandatory=mandatory,
        )

    @property
    def is_requested(self):
        """Whether the output dataset is requested to be generated by C code.

        If mandatory it is always considered to be requested.
        Otherwise, this is determined by the value of `user_spec`
          - `None` means *use default* (`requested_by_default`)
          - `False` means it is not requested
          - any other value is means it is requested
            - for example: `True`, `"pyarrow table"`, `"out.parquet"`
        """
        if self.mandatory:
            return True

        if self.not_specified:
            return self.requested_by_default

        return self.user_spec is not False

    def init_c_args(self):
        """Low-level preparation of C output dataset arguments.

        Some output datasets are optional.
        If a dataset is not requested, C-code must receive `NULL` (C).
        Accomplish this by passing `None` (Python).
        Otherwise, Python passes a string pointer (`ctypes.c_char_p()`) to
         C code "by reference" (`ctypes.byref()`).
        """
        if self.is_requested:
            # allocate schema and array, to be populated by C code
            # associate schema with array during array creation
            self.schema = allocate_c_schema()
            self.array = allocate_c_array(self.schema)

            # prepare ctypes parameters
            #   nanoarrow uses `_addr()` in public examples, doesn't discourage use
            #   See https://arrow.apache.org/nanoarrow/0.5.0/reference/python/advanced.html
            self.c_schema = ctypes.cast(self.schema._addr(), ctypes.c_void_p)  # noqa: SLF001
            self.c_array = ctypes.cast(self.array._addr(), ctypes.c_void_p)  # noqa: SLF001
        else:
            self.schema = None
            self.array = None
            self.c_schema = None
            self.c_array = None

    def extract_c_output(self):
        if self.is_requested:
            # array holds reference to schema, so only save array
            self.c_output = self.array
            # ALTERNATIVELY: use `_import_from_c()`, which marks the array and schema as released
            #  unsure which method is better
            # struct_array=pa.Array._import_from_c(self.array._addr(), self.schema._addr())  # noqa: SLF001, ERA001
            # self.c_output = struct_array  # noqa: ERA001
        else:
            self.c_output = None

    def free_c_args(self):
        """Free C output dataset arguments (NOT THEIR CONTENTS)."""
        self.schema = None
        self.array = None
        self.c_schema = None
        self.c_array = None

    def validate_user_spec(self):
        """Validate the user specification."""
        # disallow specifying False for mandatory output datasets
        if self.mandatory is True and self.user_spec is False:
            mesg = _("Specifying `False` not allowed for mandatory dataset '{}'").format(self.name)
            raise ValueError(mesg)

def c_argtype_input_dataset():
    """Return the *type* of argument C code uses for input datasets."""
    return ctypes.c_void_p

def c_argtype_output_dataset():
    """Return the *type* of argument C code uses for output datasets."""
    return ctypes.c_void_p

def c_argtype_parameters():
    """Return the *type* of argument C code uses for parameters."""
    return ctypes.c_char_p

def c_return_type():
    """Return the *type* of value C code returns."""
    return ctypes.c_int

def dest_is_file(destination):
    """Whether or not the requested destination appears to be a file path."""
    if isinstance(destination, Path):
        dst = destination
    elif isinstance(destination, str):
        if dest_is_object(destination=destination):
            return False
        dst = Path(destination)
    else:
        return False

    # CHECK: file name must include non-empty name and extension
    dst = Path(destination)
    if (  # noqa: SIM103
        "." not in dst.name
        or len(dst.name.split(".")[0]) == 0
        or len(dst.name.split(".")[1]) == 0
    ):
        return False

    return True

def dest_is_object(destination):
    """Whether or not the requested destination is an object."""
    return (
        isinstance(destination, str)
        and destination in tc.supported_output_obj_id
    )

def interm_from_input_file(dataset_ref, log):
    """Load supported input file format into intermediate format.

    This function takes a path to a dataset and returns that dataset in an "intermediate" data format.
    It performs basic validation on the file path, analyzes the extension, and calls the appropriate
    `io_util.type_converters` function to load the file into memory in an intermediate format.
    For unsupported file types, a `TypeError` is raised.
    """
    log_lcl = lg.get_stack_logger(log)
    log_lcl.info(_("Loading dataset from file: '{}'").format(dataset_ref))

    # some validation
    if not Path(dataset_ref).exists():
        mesg = _("File does not exist: '{}'").format(dataset_ref)
        raise FileNotFoundError(mesg)

    log_lcl.debug(_("File exists: '{}'").format(dataset_ref))

    # get file extension and call the appropriate loader function
    fn_ext = Path(dataset_ref).suffix
    if fn_ext in tc.file_ext_sas:
        # CONTEXT: Loading <file format> file into <object format>
        log_lcl.debug(_("Loading {} file into `{}`").format("SAS Dataset", "pandas.DataFrame"))
        return tc.DF_from_sas_file(dataset_ref)
    if fn_ext in tc.file_ext_parquet:
        log_lcl.debug(_("Loading {} file into `{}`").format("Parquet", "pyarrow.Table"))
        return tc.PAT_from_parquet_file(dataset_ref)

    if fn_ext in tc.file_ext_csv:
        log_lcl.debug(_("Loading {} file into `{}`").format("CSV", "pyarrow.Table"))
        log_lcl.warning(_("csv files are not recommended for input due to low numeric precision and inconsistent data types"))
        return tc.PAT_from_csv_file(dataset_ref)

    if fn_ext in tc.file_ext_feather:
        log_lcl.debug(_("Loading {} file into `{}`").format("Feather", "pyarrow.Table"))
        return tc.PAT_from_feather_file(dataset_ref)

    mesg = _("Input file type not supported: '{}'").format(fn_ext)
    raise TypeError(mesg)

def interm_to_arrow_stream(dataset, log):
    """Take intermediate format dataset, return Arrow stream."""
    log_lcl = lg.get_stack_logger(log)

    # inspect type, call appropriate `type_converters` function.
    if isinstance(dataset, pd.core.frame.DataFrame):
        log_lcl.debug(_("Converting `{}` to `{}`").format ("pandas.DataFrame", "CArrayStream"))
        return tc.DF_to_arrow_stream(dataset)

    if isinstance(dataset, pa.Table):
        log_lcl.debug(_("Converting `{}` to `{}`").format ("pyarrow.Table", "CArrayStream"))
        return tc.PAT_to_arrow_stream(dataset)

    if dataset is None:
        log_lcl.debug(_("Received `None`, returning `None`"))
        return None

    mesg = _("Intermediate dataset's type not supported: `{}`").format(type(dataset).__name__)
    raise TypeError(mesg)

def interm_to_DF(dataset):
    """Convert intermediate dataset to pandas dataframe."""
    if isinstance(dataset, pd.DataFrame):
        return dataset

    if isinstance(dataset, pa.Table):
        return tc.PAT_to_DF(dataset)

    mesg = _("Intermediate dataset's type not supported: `{}`").format(type(dataset).__name__)
    raise TypeError(mesg)

def interm_to_output_file(dataset, destination, log):
    """Take intermediate format dataset, write to a supported output format file .

    This function inspects the file extension defined in `destination` and selects the appropriate
    file converstion function.

    Returns:
      Upon successfully writing the dataset to file, the path of the written file is returned.

    Exceptions:
      If the requested output format is unsupported, a `ValueError` is raised.

    """
    log_lcl = lg.get_stack_logger(log)
    log_lcl.info(_("Writing dataset to file: '{}'").format(destination))

    # get output file's extension (including the '.')
    fn_ext = Path(destination).suffix

    # determin output type and write output
    if fn_ext in tc.file_ext_parquet:  # FILE_TYPE: parquet
        # CONTEXT: Saving dataset to <file format> file: '<file path>'
        log_lcl.debug(_("Saving dataset to {} file: '{}'").format("Parquet", destination))
        if isinstance(dataset, pa.Table):
            tc.PAT_to_parquet_file(dataset, destination)
        elif isinstance(dataset, pd.DataFrame):
            tc.DF_to_parquet_file(dataset, destination)
    elif fn_ext in tc.file_ext_feather:  # FILE_TYPE: feather
        log_lcl.debug(_("Saving dataset to {} file: '{}'").format("Feather", destination))
        if isinstance(dataset, pa.Table):
            tc.PAT_to_feather_file(dataset, destination)
        elif isinstance(dataset, pd.DataFrame):
            tc.DF_to_feather_file(dataset, destination)
    elif fn_ext in tc.file_ext_csv:  # FILE_TYPE: CSV
        log_lcl.debug(_("Saving dataset to {} file: '{}'").format("CSV", destination))
        if isinstance(dataset, pa.Table):
            tc.PAT_to_csv_file(dataset, destination)
        elif isinstance(dataset, pd.DataFrame):
            tc.DF_to_csv_file(dataset, destination)
    else:  # FILE_TYPE: UNKNOWN!!
        mesg = _("Output file type not supported: '{}'").format(fn_ext)
        raise ValueError(mesg)

    return destination

def interm_to_PAT(dataset):
    if isinstance(dataset, pd.DataFrame):
        return tc.DF_to_PAT(dataset)

    if isinstance(dataset, pa.Table):
        return dataset

    mesg = _("Intermediate dataset's type not supported: `{}`").format(type(dataset).__name__)
    raise TypeError(mesg)

def load_input_dataset(dataset, log):
    """Inspect and (if applicable) load user specified input dataset into intermediate format.

    This function takes a user-provided input dataset object or file path and returns
    that dataset in an "intermediate" data format.

    The format of the user-provided input dataset determines how it is loaded.
    When a file path is provided, the data is loaded with `interm_from_input_file()`.
    This function will immediately return a dataset which is already in an intermediate format.

    Exceptions:
      `TypeError`         - input dataset's type is not supported: `TypeError` raised
      `FileNotFoundError` - input dataset appears to be a non-existent file path
      `ValueError`        - input dataset is an empty string
    """
    log_lcl = lg.get_stack_logger(log)

    if dataset is None or isinstance(dataset, (pa.Table, pd.DataFrame)):
        log_lcl.debug(_("Input dataset type is `{}`, returning as-is").format(type(dataset)))
        return dataset

    # dataset parameter requires some processing if it is a string or path
    if isinstance(dataset, (str, Path)):
        # create Path (`dataset_path`) from string, if needed
        if isinstance(dataset, str):
            log_lcl.debug(_("Input dataset is `str`, inspecting further"))
            if len(dataset) == 0: #empty string
                mesg = _("Input dataset invalid: empty string")
                raise ValueError(mesg)
            dataset_path = Path(dataset)
        elif isinstance(dataset, Path):
            dataset_path = dataset

        if dataset_path.exists():
            # Its a file, load dataset from file
            log_lcl.debug(_("Input dataset is file path: '{}'").format(dataset_path))
            return interm_from_input_file(dataset_path, log_lcl)

        if dataset_path.parent.exists():
            # The full path doesn't resolve, but the folder exists
            # definitely an error here
            mesg = _("Input dataset file path does not exist: '{}'").format(dataset_path)
            raise FileNotFoundError(mesg)

        # if user specified a string that we can't process, raise an exception
        if isinstance(dataset, str):
            # ERROR: unrecognized input
            log_lcl.debug(_("Input Dataset is non-empty string with length: {}").format(len(dataset)))
            if len(dataset) < 64:  # noqa: PLR2004  # 64 is an arbitrary length after which we cut the string down and display only a portion
                input_substr = dataset
            else:
                input_substr = dataset[0:45] + " ... " + dataset[len(dataset)-19:]
            mesg = _("Input dataset specification unrecognized, assuming non-existent file path: '{}'").format(input_substr)
            raise FileNotFoundError(mesg)

    # if type unrecognized, raise an exception
    mesg = _("Input dataset type not supported: `{}`").format(type(dataset).__name__)
    raise TypeError(mesg)

def pack_dataset(dataset, log):
    """Format dataset for consumption by C code."""
    return interm_to_arrow_stream(dataset, log)

def pack_parms(parms):
    """Get parameters in JSON form, encode as utf-8, return it."""
    return tc.dict_to_json(parms).encode(jansson_input_encoding)

def unpack_output_dataset(dataset):
    """Load C generated output dataset to intermediate format."""
    pa_t=pa.table(dataset)

    return pa_t

def write_output_dataset(dataset, destination, log):
    """Output dataset (`dataset`) according to user specification (`destination`).

    Handle converstion from intermediate-format to user specified output format.
    This function looks at the type of arguent `destination` and calls the appropriate conversion function
    to facilitate further destination inspection or file conversion.
    Returns the dataset object in the output format, or the path to the written file.

    `dataset`
      - must be a valid dataset in one of the "intermediate" formats
      - validation of `dataset` only occurs in the called conversion functions

    `destination` is a user provided value which determines various output settings.
      - If `None` a Pandas DataFrame is returned
      - If valid path to a supported file type is provided, dataset written to file, path returned

    If certain custom exceptions occur during output, an attempt will be made to return a DataFrame of
    the output dataset.
      - `None` -> Return pandas DataFrame
      - file path to supported file type
        - write output to file
        - return path file was written to
    """
    log_lcl = lg.get_stack_logger(log)
    if destination is None or destination is True:
        # default action: `None`- implicitly requested default type, `True`-explicitly requested default type
        log_lcl.debug(_("No destination specified, returning default format: '{}'").format(tc.get_default_output_spec()))
        return write_output_dataset(
            dataset=dataset,
            destination=tc.get_default_output_spec(),
            log=log,
        )

    if dest_is_object(destination=destination):
        # object identifier: Pandas DataFrame
        if destination.lower() in tc.obj_id_pandas:
            log_lcl.debug(_("Output to object: `{}`").format("pandas.DataFrame"))
            return interm_to_DF(dataset)

        if destination.lower() in tc.obj_id_pyarrow:
            log_lcl.debug(_("Output to object: `{}`").format("pyarrow.Table"))
            return interm_to_PAT(dataset)

        raise ValueError(_("Requested output object type not supported: '{}'").format(destination))

    if dest_is_file(destination=destination):
        # May be a path, lets do basic validation
        log_lcl.debug(_("Validating: is destination a file path"))
        dst = Path(destination)

        # CHECK: Directory must exist
        try:
            dst.parent.resolve(strict=True)  # strict: exception if not exists
        except FileNotFoundError as e:
            mesg = _("Directory of output file does not exist: '{}'").format(str(dst.parent.resolve(strict=False)))
            raise FileNotFoundError(mesg) from e

        if str(dst.parent) == ".":
            dst = dst.resolve()
            log_lcl.debug(_("No directory specified for output file, using current directory: '{}'").format(str(dst.parent)))

        # Arrived here? Must be a valid file path
        return interm_to_output_file(dataset, str(dst), log_lcl)

    mesg = _("Requested output options unrecognized, type: `{}`").format(type(destination).__name__)
    raise ValueError(mesg)
