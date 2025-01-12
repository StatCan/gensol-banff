import contextlib
import logging
import pathlib
from abc import (
    ABC,
    abstractmethod,
)

from .. import (
    _log as lg,
)
from .. import (
    diagnostics as diag,
)
from .. import (
    io_util as io,
)
from ..exceptions import (
    NormalizationError,
    ProcedureCError,
    ProcedureInputError,
    ProcedureNotLoadedError,
    ProcedureOutputError,
    ProcedureValidationError,
)
from ..nls import _
from .c_util import c_runtime_locale
from .error_handling import ReturnCode
from .loader import (
    get_shared_library_path,
    load_shared_library,
)
from .validation import (
    validate_arg_type,
)


class GeneralizedProcedure(ABC):
    ##### CLASS ATTRIBUTES #####
    ## function pointers defined for each subclass using `._load_c()`
    #  and the subclass' `_proc_name` and `_arg_types` members
    _cproc_func = None
    _fflush_func = None
    _free_func = None
    _rc_desc_func = None

    # default values for some options
    _default_args = {
        "presort" : True,
        "prefill_by_vars": True,
        "trace" : False,
    }

    # list of valid kwargs key words for this classes constructor
    _valid_init_kwargs = []

    def __init__(
            self,
            trace,
            capture,
            logger,
            input_datasets,
            output_datasets,
            presort=None,
            prefill_by_vars=None,
            keyword_args=None,
    ):
        """Initialize and execute a C procedure.

        This method validates, stores, and acts upon the options specified in the arguments.
        It configures logging and output capturing options first, allowing subsequent code to
        employ logging.
        It is generic to all C procedures and should be called as the FIRST line in all
        directly derived class' `__init__()` methods.
        """
        ##### initialize logging and related options #####
        # `trace` may be required for logging initialization, so initialize it first
        validate_arg_type(arg_val=trace, parm_name="trace", allowed_types=(bool, int), skip_none=True)
        self._init_attr_default(trace, "trace")

        # `logger`: initialize logger for this object instance
        validate_arg_type(arg_val=logger, parm_name="logger", allowed_types=logging.Logger, skip_none=True)
        if isinstance(logger, logging.Logger):
            # if logger was passed, use it
            self._log = logger
            log_lcl = self._get_stack_logger(self._log)
            log_lcl.debug(_("Procedure-level logging initialized using specified logger: '{}'").format(self._log.name))
        else:
            # Normal proc-level initialization
            self._log = lg.init_proc_level(
                logger_name=self._proc_name["short"],
                trace_level=self._trace,
            )
            log_lcl = self._get_stack_logger(self._log)
            log_lcl.debug(_("Procedure-level logging initialized with default parent: '{}'").format(self._log.parent.name))

        # `capture`: whether to capture/redirect Python and C console output
        validate_arg_type(arg_val=capture, parm_name="capture", allowed_types=(bool, type(None)), skip_none=False)
        self._init_capture(
            capture=capture,
            logger=logger,
            keyword_args=keyword_args,
            log=log_lcl,
        )

        ##### initialize procedure related options, inputs, and output specs #####
        with self._py_log_redirect:
            ##### print timestamp to log #####
            log_lcl.info(lg.get_timezone_message())
            ##### initialize datasets #####
            self._input_datasets = input_datasets
            self._output_datasets = output_datasets
            self._validate_output_datasets(log=log_lcl)

            ##### initialize preprocessing options #####
            # `presort`
            validate_arg_type(log=log_lcl, arg_val=presort, parm_name="presort", allowed_types=bool, skip_none=True)
            self._init_attr_default(presort, "presort")
            self._presort_datasets = self._pp_get_presort_datasets()

            # `prefill_by_vars`
            validate_arg_type(log=log_lcl, arg_val=prefill_by_vars, parm_name="prefill_by_vars", allowed_types=bool, skip_none=True)
            self._init_attr_default(prefill_by_vars, "prefill_by_vars")
            self._prefill_datasets = self._pp_get_prefill_datasets()

            ##### validate #####
            self._validate_deprecations(keyword_args=keyword_args, log=log_lcl)  # do this before validating keyword arguments
            self._validate_keyword_args(keyword_args=keyword_args, log=log_lcl)  # only do this after validating deprecations
            self._validate_c_parameters(log=log_lcl)

    ##### SUBCLASS TO IMPLEMENT #####
    @abstractmethod
    def _call_c_code(self):
        """SUBCLASS TO IMPLEMENT.

        the procedure-specific call to the C-code function
        passing the appropriate input and out parameters
        """
        mesg = _("Internal error: subclass must implement `{}` method").format("_call_c_code()")
        raise NotImplementedError(mesg)

    ##### BASE CLASS METHODS #####
    ## property getter/setter methods ##
    def _get_input_dataset(self, ds):
        """Return the user-provided input dataset argument."""
        return ds.user_spec

    def _set_input_dataset(self, ds, value):  # noqa: ARG002  # `value` reserved for future/subclass use
        """Prevent users from modifying input dataset attributes directly.

        Inputs can only be set during initialization, they cannot be modified
        """
        mesg = _("'{}' dataset cannot be modified").format(ds.name)
        raise AttributeError(mesg)

    def _get_output_dataset(self, ds):
        """Return the C-code generated output dataset in user-requested format."""
        return ds.user_output

    def _set_output_dataset(self, ds, value):  # noqa: ARG002  # `value` reserved for future/subclass use
        """Prevent users from modifying output dataset attributes directly.

        Output dataset specification can only be set during initialization,
        they cannot be modified.
        """
        mesg = _("'{}' dataset cannot be modified").format(ds.name)
        raise AttributeError(mesg)

    ## helper methods ##
    def _get_stack_logger(self, parent_logger):
        """Get stack logger."""
        if parent_logger is None:
            return lg.get_stack_logger(self._log, level = 2)

        return lg.get_stack_logger(parent_logger, level = 2)

    ## initialization methods ##
    def _init_attr_default(self, arg_val, parm_name):
        """Set an attribute to user-specified value, or default value.

        Given an argument value (`arg_val`) and parameter name (`parm_name`),
        set an attribute to the value, or its default value when `None`.
        The attribute name is the `parm_name` prefixed with an underscore.
        Default value comes from `self._default_args` dictionary.
        """
        if arg_val is None:
            arg_val = self._default_args[parm_name]

        setattr(self, f"_{parm_name}", arg_val)

    def _init_capture(self, capture, logger, keyword_args, log):  # noqa: ARG002  # `keyword_args` reserved for future/subclass use
        """Initialize 'capture' option.

        Configures the `self._c_log_redirect` and `self._py_log_redirect` context managers to handle
        console output according to the `capture` option.
        These context managers capture C and Python console logging, respectively,
        allowing output to be redirected to a particular stream handler, or suppressed.

        Option Values:
            `True`  - capture and reprint all Python and C console log output
            `False` - no capturing
            `None`  - suppress all Python and C console log output by capturing and not reprinting
          Special Cases
            - if `logger` specified, ignore capture
        """
        if isinstance(logger, logging.Logger):
            log.debug(_("`capture` option: enabled so `logger` captures C output"))
            self._c_log_redirect = lg.capture.c_log_redirect(enabled=True, reprint=True, reprint_file=logger)
            self._py_log_redirect = lg.capture.proc_py_log_redirect(enabled=False)
        elif capture is None:
            # capture, but don't print captured text
            log.debug(_("`capture` option: configuring to suppress all procedure output"))
            self._c_log_redirect = lg.capture.c_log_redirect(enabled=True, reprint=False)
            self._py_log_redirect = lg.capture.proc_py_log_redirect(enabled=True, reprint=False)
        elif capture is True:
            # capture and print captured text
            log.debug(_("`capture` option: configuring to capture and reprint all procedure output"))
            self._c_log_redirect = lg.capture.c_log_redirect(enabled=True, reprint=True)
            self._py_log_redirect = lg.capture.proc_py_log_redirect(enabled=True, reprint=True)
        elif capture is False:
            # don't capture anything
            log.debug(_("`capture` option: default configuration (disabled)"))
            self._c_log_redirect = lg.capture.c_log_redirect(enabled=False)
            self._py_log_redirect = lg.capture.proc_py_log_redirect(enabled=False)
        else:
            mesg = _("Invalid value for `{}` parameter").format("capture")
            raise ValueError(mesg)

    def _pp_get_prefill_datasets(
            self,
            attr_list=None,
    ):
        """Get pairs of datasets that can share 'by' variables.

        Returns a list named-pairs of input datasets in which 'by' variables can be
        taken from one dataset and added to the other.  Each pair is a dict with 2 members
            'status' - the status dataset
            'data' - the dataset from which by variables can be sourced

        This implementation checks to see if an instance contains common pairs of datasets.
        If any of these datasets does not exist in the instance it is skipped.  It may return
        an empty list if the instance does not have any of the common pairs.
        """
        # exhaustive list of possible relevant datasets from subclasses
        if attr_list is None:
            attr_list=[
                {
                    "status": "_instatus",
                    "data": "_indata",
                },
                {
                    "status": "_instatus_hist",
                    "data": "_indata_hist",
                },
            ]
        dataset_list = []

        for attr_name in attr_list:
            if (hasattr(self, attr_name["status"]) and hasattr(self, attr_name["data"])):
                dataset_list.append({  # noqa: PERF401  # no way list comprehension is better for readability here
                    "status": getattr(self, attr_name["status"]),
                    "data": getattr(self, attr_name["data"]),
                })

        return dataset_list

    def _pp_get_presort_datasets(
            self,
            attr_list=None,
    ):
        """Get list of input datasets which may require sorting prior to calling C code.

        This implementation checks to see if an instance contains datasets, skipping if
        they do not exist.  It may return an empty list if the instance contains none of
        the common datasets.
        """
        # exhaustive list of possible relevant datasets from subclasses
        if attr_list is None:
            attr_list=[
                "_indata",
                "_indata_hist",
                "_instatus",
                "_instatus_hist",
            ]

        dataset_list = [getattr(self, x) for x in attr_list if hasattr(self, x)]

        return dataset_list

    # could be useful for users
    def get_sort_list(self, include_by=True, include_unit_id=False):
        """Get list of variables for sorting an input or output dataset.

        List includes BY variables (by default).
        When `include_unit_id=True`, key variable appended *after* by variables (if `include_by`).
        May return an empty list.
        Variable names are taken from user specified `by` and `unit_id` parameters,
        are not validated to exist in any dataset and may use a different CaSe than the dataset(s).
        """
        sort_vars = []

        if include_by and isinstance(self.c_parms["by"], str):
            sort_vars.extend(self.c_parms["by"].split())

        if include_unit_id and isinstance(self.c_parms["unit_id"], str):
            sort_vars.extend(self.c_parms["unit_id"].split())

        return sort_vars

    ## execution methods ##
    def rerun(self):
        self._execute()

    def _execute(self, log=None):
        """Orchestrate execution of a procedure.

        Handles the execution of a procedure while managing logging and performance
        diagnostics at each step.

        This private method should be called automatically at the end of object initialization.
        Use the public `.rerun()` method to execute again manually.
        """
        log_lcl = self._get_stack_logger(log)

        with (  # multiple context managers
            diag.SystemStats("Execute", logger=log_lcl, log_level=logging.INFO),
            self._py_log_redirect,
        ):
            if self._cproc_func is None:
                mesg = _("Procedure '{}' C code not loaded").format(self._proc_name["long"])
                log_lcl.error(mesg)
                raise ProcedureNotLoadedError(mesg)

            log_lcl.info(_("Loading input datasets"))
            with diag.SystemStats(_("TOTAL load input"), logger=log_lcl, log_level=logging.INFO):
                self._load_inputs(log=log_lcl)

            log_lcl.info(_("Preprocessing input datasets"))
            with diag.SystemStats(_("TOTAL preprocess"), logger=log_lcl, log_level=logging.INFO):
                self._preprocess_inputs(log=log_lcl)

            # print input dataset metadata
            for ds in self._input_datasets:
                ds.log_metadata(log=log_lcl)

            log_lcl.info(_("Packing inputs for C-code"))
            with diag.SystemStats(_("TOTAL pack"), logger=log_lcl, log_level=logging.INFO):
                self._pack_inputs(log=log_lcl)

            log_lcl.info(_("Executing C-code"))
            with diag.SystemStats(_("Run Procedure"), logger=log_lcl, log_level=logging.INFO): # record start time
                self._run_proc(log=log_lcl)

            # if C-code caught an error and terminated early, there's nothing more to do
            if self.rc != ReturnCode.SUCCESS.value:
                return

            log_lcl.info(_("Extracting output datasets"))
            with diag.SystemStats(_("TOTAL unpack"), logger=log_lcl, log_level=logging.INFO):
                self._unpack_outputs(log=log_lcl)

            # print output dataset metadata
            for ds in self._output_datasets:
                ds.log_metadata(log=log_lcl)

            log_lcl.info(_("Writing output datasets"))
            with diag.SystemStats(_("TOTAL output"), logger=log_lcl, log_level=logging.INFO):
                self._write_outputs(log=log_lcl)

            log_lcl.info(_("Execution complete"))

    def _load_inputs(self, log=None):
        """Load user specified datasets into "intermediate" format."""
        log_lcl = self._get_stack_logger(log)

        for ds in self._input_datasets:
            # load input dataset
            with diag.SystemStats(_("load input '{}'").format(ds.name), logger=log_lcl):
                try:
                    log_lcl.debug(_("Loading dataset: '{}'").format(ds.name))
                    ds.ds_intermediate = io.load_input_dataset(ds.user_spec, log_lcl)
                except Exception as e:
                    mesg = _("Error occurred while loading '{}' input dataset, type: `{}`").format(ds.name, type(ds.user_spec).__name__)
                    log_lcl.exception(mesg)
                    raise ProcedureInputError(mesg) from e

    def _preprocess_inputs(self, log=None):
        """Perform some processing on input datasets prior to packing for C code."""
        log_lcl = self._get_stack_logger(log)

        ## prefill_by_vars
        if self._prefill_by_vars and len(self._prefill_datasets) > 0:
            # CONTEXT: Preprocessing: `<preprocessing option name>`
            log_lcl.info(_("Preprocessing: `{}`").format("prefill_by_vars"))

            for ds in self._prefill_datasets:
                # skip dataset if unspecified by user
                if (
                    ds["status"].not_specified
                    or ds["data"].not_specified
                    or self.c_parms["unit_id"] in [None, ""]
                    or self.c_parms["by"] in [None, ""]
                ):
                    continue

                with diag.SystemStats(_("add BY variables to '{}' (includes adding and sorting)").format(ds["status"].name), logger=log_lcl):
                    # add by variables and sort dataset
                    try:
                        log_lcl.debug(_("Adding BY variables to '{}' dataset").format(ds["status"].name))
                        temp_ds = io.add_status_by_vars(
                            ds_stat=ds["status"].to_arrow(),
                            ds_data=ds["data"].to_arrow(),
                            unit_id=self.c_parms["unit_id"],
                            by=self.c_parms["by"],
                        )

                        with diag.SystemStats(_("sort prefilled '{}' (includes only sorting)").format(ds["status"].name), logger=log_lcl):
                            sort_list = self.get_sort_list()
                            log_lcl.debug(_("Sorting prefilled '{}' dataset in ascending order of '{}' ").format(ds["status"].name, " ".join(sort_list)))
                            ds["status"].ds_intermediate = io.sort_dataset(
                                dataset=temp_ds,
                                by=sort_list,
                                inplace=False,
                                case_sensitive=False,
                            )

                        # optimize: remove the just-sorted dataset: incase `_presort=True`, this dataset should not be sorted twice
                        with contextlib.suppress(ValueError):
                            self._presort_datasets.remove(ds["status"])
                    # for certain exceptions, continue with original dataset
                    except (NormalizationError, KeyError) as e:
                        # NormalizationExeption - user parameters or dataset's column names not suitable
                        # KeyError              - likely user-error, C code will catch any real issues
                        log_lcl.debug(e)
                        log_lcl.warning(_("Unable to add BY variables to '{}' dataset, proceeding with original dataset").format(ds["status"].name))

        ## presort
        if self._presort and len(self._presort_datasets) > 0:
            log_lcl.info(_("Preprocessing: `{}`").format("presort"))

            # sort input datasets as necessary
            for ds in self._presort_datasets:
                # skip dataset if unspecified by user
                if ds.not_specified:
                    continue

                with diag.SystemStats(_("sort '{}'").format(ds.name), logger=log_lcl):
                    try:
                        # try sorting with BY variables
                        try:
                            sort_list = self.get_sort_list()
                            log_lcl.debug(_("Sorting '{}' dataset in ascending order of '{}' ").format(ds.name, " ".join(sort_list)))
                            ds.ds_intermediate = io.sort_dataset(
                                dataset=ds.ds_intermediate,
                                by=sort_list,
                                inplace=False,
                                case_sensitive=False,
                            )
                        # KeyError could mean that BY variables are missing, so try without them
                        # this could be improved by doing an explicit check for BY variables in advance
                        except KeyError as e:
                            # incase dataset is missing BY variables
                            # it may still require sort on key variable
                            # C code will detect issues if this is incorrect
                            sort_list = self.get_sort_list(include_by=False)
                            # CONTEXT: ...again with '<names of variables used for sorting>', KeyError occurred (<system generated error message>)
                            log_lcl.debug(_("Trying sort again with '{}', KeyError occurred ({})").format(" ".join(sort_list), e))
                            ds.ds_intermediate = io.sort_dataset(
                                dataset=ds.ds_intermediate,
                                by=sort_list,
                                inplace=False,
                                case_sensitive=False,
                            )
                    # for certain exceptions, continue with original dataset
                    except (NormalizationError, KeyError) as e:
                        # NormalizationExeption - user parameters or dataset's column names not suitable
                        # KeyError              - likely user-error, C code will catch any real issues
                        log_lcl.debug(e)
                        log_lcl.warning(_("Unable to sort '{}' dataset, proceeding with original dataset").format(ds.name))

    def _pack_inputs(self, log=None):
        """Prepare intermediate formatted input datasets and parameters for consumption by C code.

        User-specified C-code parameters and datasets are each converted from their
        intermediate format to whatever format C can consume.  Low level C arguments
        are prepared.

        Following packing, intermediate formatted datasets are freed as they are not needed.
        If datasets are not freed immediately they will be freed later on by garbage collection or during C-code execution.
        """
        log_lcl = self._get_stack_logger(log)

        # package parameters
        with diag.SystemStats(_("pack parameters"), logger=log_lcl):
            try:
                self._parm_dict = io.pack_parms(self.c_parms)
            except Exception as e:
                mesg = _("Error occurred while packing parameters")
                log_lcl.exception(mesg)
                raise ProcedureInputError(mesg) from e

        for ds in self._input_datasets:
            # package input dataset
            with diag.SystemStats(_("pack '{}'").format(ds.name), logger=log_lcl):
                try:
                    log_lcl.debug(_("Packing '{}' dataset").format(ds.name))
                    ds.c_input = io.pack_dataset(ds.ds_intermediate, log_lcl)
                    ds.init_c_args()
                except Exception as e:
                    mesg = _("Error occurred while packing '{}' dataset, type: `{}`").format(ds.name, type(ds.ds_intermediate).__name__)
                    log_lcl.exception(mesg)
                    raise ProcedureInputError(mesg) from e

            # free intermediate dataset
            with diag.MemoryUsage(_("free intermediate input '{}'").format(ds.name), logger=log_lcl):
                ds.ds_intermediate = None

    def _run_proc(self, log=None):
        """Execute a call to a procedure C code function, handling input, output, and errors.

        With all input datasets and parameters "packed", this method
            - prepares output dataset parameters
            - handles logging
            - execute C-code using subclass function
            - handles exceptions
            - handles expected error codes
            - frees C inputs
            - extract and free C outputs

        Logging
            `_c_flush_buffer()` flushes the C buffer of any pending `stdout` output.
            `_c_log_redirect` redirects C `stdout` and `stderr` output as configured during initialization

        Exceptions and Errors
            A custom exception is raised when a procedure encounters an error and returns early with
            a non-zero return code.  The custom exception stores the C return code.
            Exceptions that occur during C execution (such as a "read access violation") are unexpected errors.
            These should not occur during normal usage and are therefore raised, chained to the same custom
            exception described above.
        """
        log_lcl = self._get_stack_logger(log)

        #setup output parameters
        for ds in self._output_datasets:
            ds.init_c_args()

        log_lcl.debug(_("~~~~~ C CODE BEGIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"))
        try:
            # flush C buffers incase there's something in there
            self._c_flush_buffer()

            # redirect C output as configured during `super.__init__()`
            with (
                self._c_log_redirect,
                diag.MemoryUsage(_("C execution"), logger=log_lcl),
                c_runtime_locale(),
            ):
                # use try-finally to ensure buffer flushed before any exception handled (by parent context)
                try:
                    self.rc = self._call_c_code()
                except:
                    self.rc = ReturnCode.GENERIC_FAILURE.value
                    raise
                finally:
                    self._c_flush_buffer()
        except Exception as e:  # unexpected exception
            log_lcl.debug(_("~~~~~ C CODE END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"))
            mesg = _("Procedure '{}' encountered an error, execution did not complete").format(self._proc_name["long"])
            log_lcl.exception(mesg)
            raise ProcedureCError(mesg, return_code=ReturnCode.GENERIC_FAILURE.value) from e
        else:
            log_lcl.debug(_("~~~~~ C CODE END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"))
            # for user-errors caught by C code, print message, store C return code in -and raise- custom exception
            if self.rc != ReturnCode.SUCCESS.value:
                mesg = _("Procedure '{}' encountered an error and terminated early: {} (return code {})").format(
                    self._proc_name["long"],
                    self._c_rc_description(self.rc),
                    self.rc,
                )
                log_lcl.error(mesg)
                raise ProcedureCError(mesg, return_code=self.rc)
        finally:
            # DO NOT RETURN FROM HERE
            # `finally` executes AFTER the `except` or `else` clause
            #  even if those clauses `return` or `raise`, this code runs
            # but `return` in finally cancels "pending" `return` and `raise`

            # free C code inputs
            for ds in self._input_datasets:
                with diag.MemoryUsage(_("free input dataset {}").format(ds.name), logger=log_lcl):
                    ds.free_c_args()
            del self._parm_dict

            # only handle C-code outputs if return value indicates no errors
            if self.rc == ReturnCode.SUCCESS.value:
                for ds in self._output_datasets:
                    # extract and free C allocations
                    ds.extract_c_output()
                    ds.free_c_args()

    def _unpack_outputs(self, log=None):
        """Convert C-code generated output datasets to intermediate format datasets.

        Converts each C-code generated output datasets to an intermediate format and
        free the C output.
        """
        log_lcl = self._get_stack_logger(log)

        for ds in self._output_datasets:
            if ds.is_requested:
                # unpack output dataset
                with diag.SystemStats(_("unpack '{}'").format(ds.name), logger=log_lcl):
                    try:
                        log_lcl.debug(_("Unpacking '{}' dataset").format(ds.name))
                        ds.ds_intermediate = io.unpack_output_dataset(ds.c_output)
                    except Exception as e:
                        mesg = _("Error occurred while unpacking '{}' dataset").format(ds.name)
                        log_lcl.exception(mesg)
                        raise ProcedureOutputError(mesg) from e

                # free C code output
                with diag.MemoryUsage(_("free C output {}").format(ds.name), logger=log_lcl):
                    ds.c_output = None
            else:
                log_lcl.debug(_("Optional dataset '{}' not requested").format(ds.name))
                ds.ds_intermediate = None

    def _write_outputs(self, log=None):
        """Write C-code outputs according to user's specification.

        This function passes intermediate format output datasets to `io_util.write_output_dataset()`
        which implements support for various output formats.
        Users specify the output format for each subclass-declared output dataset parameter.
        """
        log_lcl = self._get_stack_logger(log)

        for ds in self._output_datasets:
            if ds.is_requested:
                # process output dataset
                with diag.SystemStats(_("output '{}'").format(ds.name), logger=log_lcl):
                    try:
                        log_lcl.debug(_("Processing '{}' dataset output").format(ds.name))
                        ds.user_output = io.write_output_dataset(ds.ds_intermediate, ds.user_spec, log_lcl)
                    except Exception as e:
                        mesg = _("Error occurred while writing '{}' output dataset").format(ds.name)
                        log_lcl.exception(mesg)
                        raise ProcedureOutputError(mesg) from e

                # free intermediate dataset
                with diag.MemoryUsage(_("free intermediate output '{}'").format(ds.name), logger=log_lcl):
                    ds.ds_intermediate = None
            else:
                log_lcl.debug(_("Optional dataset '{}' not requested").format(ds.name))
                ds.user_output = None

    ## Utility methods ##
    @classmethod
    def get_default(cls, key=None):
        """Get default value for Procedure option.

        Returns dictionary of available default arguments, or
        a specific default-value when `key` specified.

        Raises `KeyError` if `key` does not exist.
        """
        if key is None:
            return cls._default_args

        if key not in cls._default_args:
            raise KeyError(_("default '{}' does not exist").format(key))

        return cls._default_args[key]

    @classmethod
    def set_default(cls, key, value):
        """Set default value for some Procedure options.

        Sets the value of `key` in Procedure _default_args dictionary
        to `value`.

        Raises `KeyError` if `key` does not exist.
        """
        if key not in cls._default_args:
            raise KeyError(_("default '{}' does not exist").format(key))

        # copy before modifying: this ensures we don't modify a shared list
        cls._default_args = cls._default_args.copy()
        cls._default_args[key] = value

    @staticmethod
    @abstractmethod
    def _get_bin_anchor():
        """Return the package anchor to the folder containing C code binary.

        Package anchor is like the namespace, for instance 'banff.proc.bin'.
        See https://docs.python.org/3/library/importlib.resources.html
        """
        mesg = _("Internal error: subclass must implement `{}` method").format("_get_bin_anchor()")
        raise NotImplementedError(mesg)

    @classmethod
    def _load_all_procs(cls, debug=None, lang=None):
        """Load procedure binary for all procedures of a derived class.

        debug:  Whether or not to enable C code debug statistics
                optional, `True` or `False`
        lang:   What supported language procedure output should use
                optional, a `SupportedLanguage` member

        If a procedure's binary cannot be loaded due to it not being found,
        a warning message is issued and the process may or may not continue.
        Nothing is raised to the caller in these cases.
        """
        for proc in cls.__subclasses__():
            try:
                proc._load_c(debug=debug, lang=lang)
            except FileNotFoundError as e:  # noqa: PERF203  # no performance concern here
                # Issue warning and continue
                log_lcl = lg.get_misc_logger()
                mesg = _("Unable to load '{}' procedure ({})").format(proc._proc_name["long"], e)  # noqa: SLF001  # it's our own private attribute
                log_lcl.warning(mesg)
            except ModuleNotFoundError as e:
                # Issue warning, skip remaining procedures
                log_lcl = lg.get_misc_logger()
                mesg = _("Binary for all procedures is missing, none will be loaded ({})").format(e)
                log_lcl.warning(mesg)
                break  # break out of `for` loop

    @classmethod
    def _reload_all_procs(cls, debug=None, lang=None):
        """Reload procedure binary for all procedures of a derived class.

        debug:  Whether or not to enable C code debug statistics
                optional, `True` or `False`
        lang:   What supported language procedure output should use
                optional, a `SupportedLanguage` member
        """
        for proc in cls.__subclasses__():
            proc._reload_c(debug=debug, lang=lang)

    @classmethod
    def _unload_all_procs(cls):
        """Unload procedure binary for all procedures of a derived class."""
        for proc in cls.__subclasses__():
            proc._unload_c()

    @classmethod
    def _load_c(cls, debug=None, lang=None):  # noqa: ARG003  # `debug` reserved for future/subclass use
        """Load the C code Dynamic Link Library (Windows) or Shared Object (Linux) associated with subclass `cls`."""
        dll_path = get_shared_library_path(
            dll_name_base=cls._proc_name["short"],
            bin_root=cls._get_bin_anchor(),
            lang=lang,
        )

        cls._cproc_func, cls._fflush_func, cls._free_func, cls._rc_desc_func = load_shared_library(
            log             = lg.get_import_logger(),
            dll_path        = dll_path,
            proc_name       = cls._proc_name["short"],
            c_function_name = cls._proc_name["short"],
            arg_types       = cls._arg_types,
            return_type     = io.c_return_type(),
        )

    @classmethod
    def _reload_c(cls, debug=None, lang=None):
        """Unload (if loaded) and load C code."""
        if cls._cproc_func is not None:
            cls._unload_c()

        cls._load_c(debug=debug, lang=lang)

    @classmethod
    def _unload_c(cls):
        """Unload C code DLL (Windows) or SO (Linux) associated with subclass `cls`."""
        # Just clearing the reference to each function doesn't actually release the
        # shared library.  However, reloading a shared library seems to work as needed
        # regardless (environment variable changes are picked up un reload).
        # Forcing release is possible using the following code:
        #  - https://stackoverflow.com/a/21794318
        #  - https://stackoverflow.com/a/64483246
        # noqa: ERA001 # import _ctypes
        # noqa: ERA001 # import platform
        # noqa: ERA001 # if platform.system() == 'Windows':
        # noqa: ERA001 #     _ctypes.FreeLibrary(cls._cproc_func._objects['0']._handle)
        # noqa: ERA001 # elif platform.system() == 'Linux':
        # noqa: ERA001 #     _ctypes.dlclose(cls._cproc_func._objects['0']._handle)

        cls._cproc_func = None
        cls._fflush_func = None
        cls._free_func = None
        cls._rc_desc_func = None

    @classmethod
    def _c_flush_buffer(cls):
        """Call C fflush function, if available."""
        # `_fflush_func` attribute must be set by subclass
        if cls._fflush_func is not None:
            cls._fflush_func(None)

    @classmethod
    def _c_free_pointer(cls, c_ptr):
        """Call C memory freeing function, if available."""
        # `_free_func` attribute must be set by subclass
        if cls._free_func is not None:
            cls._free_func(c_ptr)

    @classmethod
    def _c_rc_description(cls, rc):
        """Get a text description for a procedure return code."""
        if cls._rc_desc_func is None:
            return _("<no description>")

        return cls._rc_desc_func(rc).decode("utf-8")

    ## Validation methods ##
    @abstractmethod
    def _validate_c_parameters(self, log):
        """Perform basic validation of C parameter types.

        Strict validation happens in C code.
        """
        mesg = _("Internal error: subclass must implement `{}` method").format("_validate_c_parameters()")
        raise NotImplementedError(mesg)

    def _validate_deprecations(self, log, keyword_args):  # noqa: ARG002  # `keyword_args` reserved for future/subclass use
        """Check for use of deprecated options during initialization.

        Deprecated parameters should appear in `keyword_args`, since the formal parameter definition
        should be removed upon deprecation.
        """
        return # currently no library-wide deprecations
        log_lcl = self._get_stack_logger(log) # noqa: F841 # will be used for cross-procedure deprecations

    def _validate_keyword_args(self, keyword_args, log):
        """Validate all keyword arguments.

        Keyword arguments are only valid if listed in `self._valid_init_kwargs`.
        """
        ##### validate kwargs #####
        for key in keyword_args.keys():
            if key not in self._valid_init_kwargs:
                # CONTEXT: "<procedure name> procedure received an unexpected keyword argument '<key>'"  # noqa: ERA001
                mesg = _("'{}' procedure received an unexpected keyword argument '{}'").format(
                    self._proc_name["long"],
                    key,
                )
                log.error(mesg)
                raise TypeError(mesg)

    def _validate_output_datasets(self, log):
        """Check for issues with output dataset specifications."""
        log_lcl = self._get_stack_logger(log)

        # check for duplicate output paths
        out_files = []
        for ds in self._output_datasets:
            if io.dest_is_file(ds.user_spec):
                abs_path = pathlib.Path(ds.user_spec).absolute()
                if abs_path in out_files:
                    mesg = _("The following path used for multiple output datasets: '{}'").format(str(abs_path))
                    log_lcl.error(mesg)
                    raise ProcedureValidationError(mesg)
                out_files.append(abs_path)
