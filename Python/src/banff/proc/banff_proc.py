import contextlib

from banff import (
    diagnostics as diag,
)
from banff._common.src._log import capture as log_capture
from banff._common.src.proc import GeneralizedProcedure
from banff._common.src.proc.validation import (
    string_parm_is_empty,
    validate_arg_type,
)
from banff.exceptions import (
    ProcedureValidationError,
)
from banff.nls import _


class BanffProcedure(GeneralizedProcedure):
    # create a copy of default args, so modifications do not affect superclass
    _default_args = GeneralizedProcedure.get_default().copy()

    # list of valid kwargs key words for this classes constructor
    # IMPORTANT: we need a copy of (as opposed to a reference to) the superclass' list
    # , so we don't modify the superclass' class attribute.  The `+` operator seems to do this.
    _valid_init_kwargs = GeneralizedProcedure._valid_init_kwargs + [  # noqa:  SLF001
        "_BP_c_log_handlers",
    ]

    def __init__(
            self,
            trace,
            capture,
            logger,
            input_datasets,
            output_datasets,
            presort=None,
            prefill_by_vars=None,
            exclude_where_indata=None,
            exclude_where_indata_hist=None,
            keyword_args=None,
    ):
        """Initialize Banff and execute Banff C procedure.

        After calling the superclass init method, performs Banff specific initialization tasks.
        Finally, it executes the procedure.

        Subclasses (i.e. Banff procedures) must store all procedure-specific arguments
        in `self` prior to calling this method, and pass a list of input and output datasets.
        """
        super().__init__(
            trace=trace,
            capture=capture,
            logger=logger,
            input_datasets=input_datasets,
            output_datasets=output_datasets,
            presort=presort,
            prefill_by_vars=prefill_by_vars,
            keyword_args=keyword_args,
        )

        log_lcl = self._get_stack_logger(self._log)

        # Banff specific initialization
        with self._py_log_redirect:
            # preprocessing options: `exclude_where_*`
            self._pp_init_exclude_parm(log=log_lcl, arg_val=exclude_where_indata, parm_name="exclude_where_indata")
            self._pp_init_exclude_parm(log=log_lcl, arg_val=exclude_where_indata_hist, parm_name="exclude_where_indata_hist")
            self._validate_exclude_where(log=log_lcl)

        # execute the procedure
        self._execute()

    def _init_capture(self, capture, logger, keyword_args, log):
        """Initialize 'capture' option in Banff.

        This method implements Banff specific handling of the 'capture' option.
        The superclass implementation is not called at all the procedure is called
        by the Banff processor.

        See superclass implementation for full details.
        """
        if "_BP_c_log_handlers" in keyword_args:
            # Banff Processor (BP) calling, configure C log redirection to BP stream
            log.debug(_("Configuring procedure-level logging for Banff Processor log redirection"))
            streams = [h.stream for h in keyword_args["_BP_c_log_handlers"]]
            self._c_log_redirect = log_capture.c_log_redirect(
                enabled=True,
                reprint=True,
                reprint_file=streams,
            )
            # and disable Python log redirection
            self._py_log_redirect = log_capture.proc_py_log_redirect(enabled=False)
        else:
            super()._init_capture(
                capture=capture,
                logger=logger,
                keyword_args=keyword_args,
                log=log,
            )

    ## preprocessing methods ##
    # not an `@abstractmethod` since only some subclasses define this
    def _pp_exclude_where_indata(self):
        """SUBCLASS TO IMPLEMENT.

        procedure-specific exclusion statement for indata.  Only relevant in some procedures.
        """
        mesg = _("Internal error: subclass must implement `{}` method").format("_pp_exclude_where_indata()")
        raise NotImplementedError(mesg)

    # not an `@abstractmethod` since only some subclasses define this
    def _pp_exclude_where_indata_hist(self):
        """SUBCLASS TO IMPLEMENT.

        procedure-specific exclusion statement for indata_hist.  Only relevant in some procedures.
        """
        mesg = _("Internal error: subclass must implement `{}` method").format("_pp_exclude_where_indata_hist()")
        raise NotImplementedError(mesg)

    def _pp_init_exclude_parm(self, log, arg_val, parm_name):
        """Validate and store `exclude_where_*` user parameter."""
        if arg_val is None:
            return  # nothing to do

        validate_arg_type(log=log, arg_val=arg_val, parm_name=parm_name, allowed_types=str)

        # sanitize
        for substr in [";"]:  # disallow semicolon: could be attempt to execute multiple SQL statements
            if substr in arg_val:
                mesg = _("Parameter `{}` cannot contain the substring '{}'").format(parm_name, substr)
                log.error(mesg)
                raise ValueError(mesg)

        if string_parm_is_empty(arg_val):
            mesg = _("Ignoring option `{}='{}'` (empty string)").format(parm_name, arg_val)
            log.info(mesg)
            return  # nothing more to do

        setattr(self, f"_{parm_name}", arg_val)

    def _preprocess_inputs(self, log=None):
        """Perform some processing on input datasets prior to packing for C code."""
        log_lcl = self._get_stack_logger(log)

        ## exclude_where_indata
        if hasattr(self, "_exclude_where_indata"):
            log_lcl.info(_("Preprocessing: `exclude_where_indata='{}'`").format(self._exclude_where_indata))

            with diag.SystemStats(_("exclude_where_indata"), logger=log_lcl):
                self._pp_exclude_where_indata()

        ## exclude_where_indata_hist
        if hasattr(self, "_exclude_where_indata_hist"):
            log_lcl.info(_("Preprocessing: `exclude_where_indata_hist='{}'`").format(self._exclude_where_indata_hist))

            with diag.SystemStats(_("exclude_where_indata_hist"), logger=log_lcl):
                self._pp_exclude_where_indata_hist()

        super()._preprocess_inputs(log=log)

    ## Validation methods ##
    def _validate_c_parameters(self, log):
        """Perform basic validation of C parameter types.

        Strict validation happens in C code.
        """
        log_lcl = self._get_stack_logger(log)

        flag_parms = [
            "accept_negative",
            "accept_zero",
            "no_by_stats",
            "outlier_stats",
            "random",
            "verify_edits",
            "verify_specs",
        ]

        for parm in flag_parms:
            with contextlib.suppress(KeyError):  # if exception, the parameter doesn't even exist
                validate_arg_type(
                    log=log_lcl,
                    arg_val=self.c_parms[parm],
                    parm_name=parm,
                    allowed_types=bool,
                    skip_none=True,
                )

        numeric_parms = [
            "beta_e",
            "beta_i",
            "lower_bound",
            "upper_bound",
            "cardinality",
            "exponent",
            "mdm",
            "mei",
            "mii",
            "mrl",
            "percent_donors",
            "start_centile",
            "time_per_obs",
            "decimal",
            "display_level",
            "extremal",
            "imply",
            "min_donors",
            "min_obs",
            "n",
            "n_limit",
            "seed",
        ]

        for parm in numeric_parms:
            with contextlib.suppress(KeyError):  # if exception, the parameter doesn't even exist
                validate_arg_type(
                    log=log_lcl,
                    arg_val=self.c_parms[parm],
                    parm_name=parm,
                    allowed_types=(int, float),
                    skip_none=True,
                )

        string_parms = [
            "edits",
            "eligdon",
            "method",
            "modifier",
            "post_edits",
            "side",
            "sigma",
            "weights",
            "data_excl_var",
            "hist_excl_var",
            "unit_id",
            "rand_num_var",
            "weight",
            "by",
            "must_impute",
            "must_match",
            "var",
            "with_var",
        ]

        for parm in string_parms:
            with contextlib.suppress(KeyError):  # if exception, the parameter doesn't even exist
                validate_arg_type(
                    log=log_lcl,
                    arg_val=self.c_parms[parm],
                    parm_name=parm,
                    allowed_types=str,
                    skip_none=True,
                )

    def _validate_deprecations(self, log, keyword_args):
        """Check for use of deprecated options across all procedures."""
        super()._validate_deprecations(log=log, keyword_args=keyword_args)

        log_lcl = self._get_stack_logger(log)

        if "reject_negative" in keyword_args.keys():
            mesg = _("Option `{}` is deprecated, use `{}` instead").format("reject_negative", "accept_negative=False")
            log_lcl.error(mesg)
            raise DeprecationWarning(mesg)

        if "reject_zero" in keyword_args.keys():
            mesg = _("Option `{}` is deprecated, use `{}` instead").format("reject_zero", "accept_zero=False")
            log_lcl.error(mesg)
            raise DeprecationWarning(mesg)

    def _validate_exclude_where(self, log):
        """Validate the combination of user-specified parameters.

        Certain parameters cannot be specified together.
        When invalid combinations are detected, a meaningful
        error message is logged and a `ValueError` is raised.
        """
        log_lcl = self._get_stack_logger(log)

        ### "avoid premature optimization": there is some repetition in the following code
        # In coming development cycles, more validations may be added, and this
        # design may be good, or may need changes.  If all is stable and repetition remains,
        # make it DRY using a function/method

        # disallow `data_excl_var` with `exclude_where_indata`
        if (
            "data_excl_var" in self.c_parms
            and hasattr(self, "_exclude_where_indata")
            and self.c_parms["data_excl_var"] is not None
        ):
            mesg = _("options `{}` and `{}` cannot be specified together").format("data_excl_var", "exclude_where_indata")
            log_lcl.error(mesg)
            raise ProcedureValidationError(mesg)

        # disallow `hist_excl_var` with `exclude_where_indata_hist`
        if (
            "hist_excl_var" in self.c_parms
            and hasattr(self, "_exclude_where_indata_hist")
            and self.c_parms["hist_excl_var"] is not None
        ):
            mesg = _("options `{}` and `{}` cannot be specified together").format("hist_excl_var", "exclude_where_indata_hist")
            log_lcl.error(mesg)
            raise ProcedureValidationError(mesg)

    @staticmethod
    def _get_bin_anchor():
        """Return package anchor for bin folder."""
        return "banff.proc.bin"

    @classmethod
    def _load_c(cls, debug=None, lang=None):
        if debug in (True, False):
            import os
            if debug is True:
                os.environ["BANFF_DEBUG_STATS"] = "TRUE"
            elif debug is False:
                with contextlib.suppress(KeyError):  # if it wasn't defined, no problem
                    os.environ.pop("BANFF_DEBUG_STATS")

        super()._load_c(debug=debug, lang=lang)
