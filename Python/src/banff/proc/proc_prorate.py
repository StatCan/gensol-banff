import logging
from pathlib import Path

import pandas
import pyarrow

from banff.io_util import (
    GensysInputDataset,
    GensysOutputDataset,
    c_argtype_input_dataset,
    c_argtype_output_dataset,
    c_argtype_parameters,
)
from banff.proc import BanffProcedure

#******CLASS DEFINITIONS************************************************************

class ProcProrate(BanffProcedure):
    """Prorates and rounds records to satisfy user-specified edits.

    Unlike other Banff procedures, the `edits` for this procedure follow specific criteria: only
    equalities are permitted, and the set of edits must form a hierarchical structure that sums
    to a grand-total. For example:

    ```plaintext
    subtotal1 + subtotal2 = grandtotal
    a + b + c = subtotal1
    d + e + f = subtotal2
    ```

    Each individual edit must consist of a set of components `x(i)` that sum to a total `y`, i.e.,
    of the form `x(1) + ... x(n) = y`. Inequalities and constants are not permitted. For each individual
    edit equation that is not satisfied, one of the two prorating algorithms (`basic` or `scaling`) is
    applied in order to rake the components to match the total. The procedure takes a top-down approach,
    beginning with the grand-total (which is never changed) and adjusting components as necessary, until
    the full set of edits is satisfied. Missing values are not prorated; they are set to zero during the
    procedure and reset to missing afterwards. Values of zero are never altered.

    Additional features:

    * Automatic rounding to the desired number of decimal places.
    * Optional bounds to constrain the relative change of values during prorating.
    * Control over which variables are eligible for prorating.
    * Option to limit prorating to original or previously imputed values, either globally or for individual variables.
    * Weights to adjust the relative change of individual variables.
    """

    # static variables
    _proc_name = {"short": "prorate", "long": "Prorate"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # instatus

        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outreject
        c_argtype_output_dataset(),  # outreject
    ]

    def __init__(self,
            # USER C code parameters
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            verify_edits: bool | None = None,
            lower_bound: float | None = None,
            upper_bound: float | None = None,
            decimal: int | None = None,
            edits: str | None = None,
            method: str | None = None,
            modifier: str | None = None,
            unit_id: str | None = None,
            by: str | None = None,

            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outstatus: Path | str | None = None,
            outdata: Path | str | None = None,
            outreject: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,
            prefill_by_vars: bool | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Prorates and rounds records to satisfy user-specified edits.

        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduce log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param verify_edits: Verify the consistency of the edits without performing any prorating. Default=False.
        :type verify_edits: bool | None, optional
        :param lower_bound: Lower bound on the relative change of the variables. Default = 0.
        :type lower_bound: float | None, optional
        :param upper_bound: Upper bound on the relative change of the variables.
        :type upper_bound: float | None, optional
        :param decimal: Number of decimals used in the rounding algorithm (between 0 and 9). Default=0.
        :type decimal: int | None, optional
        :param edits: List of edits that the prorating procedure must satisfy. Mandatory.
        :type edits: str | None, optional
        :param method: Prorating method ("SCALING" or "BASIC"). Default = "BASIC".
        :type method: str | None, optional
        :param modifier: Global modifier ("ALWAYS", "IMPUTED", "ORIGINAL") to control which values are prorated. Default = "ALWAYS"
        :type modifier: str | None, optional
        :param unit_id: Identify key variable (unit identifier) on indata. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus: Input status file containing I-- status flags.
        :type instatus: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outstatus: Output status file identifying imputed fields with IPR status flags, and their values after imputation.
        :type outstatus: Path | str | None, optional
        :param outdata: Output statistical table containing imputed data.
        :type outdata: Path | str | None, optional
        :param outreject: Output table containing records that failed prorating.
        :type outreject: Path | str | None, optional
        :param presort: Sort input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param prefill_by_vars: Add by-group variable(s) to input status file to improve performance. Default=True.
        :type prefill_by_vars: bool | None, optional
        :param trace: Control which log levels are included when using the default logger.
        :type trace: int | bool | None, optional
        :param capture: Configure how console output is displayed.
        :type capture: bool | None, optional
        :param logger: Custom logger to use for procedure execution.
        :type logger: logging.Logger | None, optional
        """  # noqa: D401,E501
        # USER C code parameters
        parm_dict = {}
        parm_dict["accept_negative"]    = accept_negative
        parm_dict["no_by_stats"]        = no_by_stats
        parm_dict["verify_edits"]       = verify_edits
        parm_dict["lower_bound"]        = lower_bound
        parm_dict["upper_bound"]        = upper_bound
        parm_dict["decimal"]            = decimal
        parm_dict["edits"]              = edits
        parm_dict["method"]             = method
        parm_dict["modifier"]           = modifier
        parm_dict["unit_id"]            = unit_id
        parm_dict["by"]                 = by
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._instatus           = GensysInputDataset("instatus", instatus)

        if verify_edits is True:
            self._outdata            = GensysOutputDataset("outdata",   output_specification=False, mandatory=False)
            self._outreject          = GensysOutputDataset("outreject", output_specification=False, mandatory=False)
            self._outstatus          = GensysOutputDataset("outstatus", output_specification=False, mandatory=False)
        else:
            self._outdata            = GensysOutputDataset("outdata",   outdata)
            self._outreject          = GensysOutputDataset("outreject", outreject)
            self._outstatus          = GensysOutputDataset("outstatus", outstatus)

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[
                self._indata,
                self._instatus,
            ],
            output_datasets=[
                self._outdata,
                self._outstatus,
                self._outreject,
            ],
            presort=presort,
            prefill_by_vars=prefill_by_vars,
            keyword_args=kwargs,
        )

    ##### property methods
    @property
    def indata(self):
        return self._get_input_dataset(self._indata)
    @indata.setter
    def indata(self, value):
        self._set_input_dataset(ds=self._indata, value=value)

    @property
    def instatus(self):
        return self._get_input_dataset(self._instatus)
    @instatus.setter
    def instatus(self, value):
        self._set_input_dataset(ds=self._instatus, value=value)

    @property
    def outdata(self):
        return self._get_output_dataset(self._outdata)
    @outdata.setter
    def outdata(self, value):
        self._set_output_dataset(ds=self._outdata, value=value)

    @property
    def outstatus(self):
        return self._get_output_dataset(self._outstatus)
    @outstatus.setter
    def outstatus(self, value):
        self._set_output_dataset(ds=self._outstatus, value=value)

    @property
    def outreject(self):
        return self._get_output_dataset(self._outreject)
    @outreject.setter
    def outreject(self, value):
        self._set_output_dataset(ds=self._outreject, value=value)

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,
            self._instatus.c_arg,

            self._outdata.c_schema,
            self._outdata.c_array,
            self._outstatus.c_schema,
            self._outstatus.c_array,
            self._outreject.c_schema,
            self._outreject.c_array,
        )

    def get_sort_list(self, include_by=True, include_unit_id=True):
        """Call superclass implementation using custom default values."""
        return super().get_sort_list(
            include_by=include_by,
            include_unit_id=include_unit_id,
        )
