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
class ProcDetermin(BanffProcedure):
    """Performs imputation when only one combination of values permits the record to pass the set of edits.

    The deterministic imputation procedure analyzes each field previously identified as requiring imputation
    to determine if there is only one possible value which would satisfy the original edits. If such a value is
    found, it is imputed during execution of this procedure. This method can also be referred to as deductive
    imputation, since a missing or inconsistent value can be deduced with certainty based upon other fields of
    the same record.
    """

    # static variables
    _proc_name = {"short": "determin", "long": "Deterministic"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # instatus

        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
    ]

    def __init__(self,
            # USER C code parameters
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            edits: str | None = None,
            unit_id: str | None = None,
            by: str | None = None,

            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outdata: Path | str | None = None,
            outstatus: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,
            prefill_by_vars: bool | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ) -> None:
        """Performs imputation when only one combination of values permits the record to pass the set of edits.

        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduces log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param edits: List of consistency edits. Mandatory.
        :type edits: str | None, optional
        :param unit_id: Identify key variable (unit identifier) on indata. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus: Input status file containing FTI status flags. Mandatory.
        :type instatus: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outdata: Output statistical table containing imputed data.
        :type outdata: Path | str | None, optional
        :param outstatus: Output status file identifying imputed fields with IDE status flags, and their values after imputation.
        :type outstatus: Path | str | None, optional
        :param presort: Sorts input tables before processing according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param prefill_by_vars: Adds by-group variable(s) to input status file to improve performance. Default=True.
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
        parm_dict["edits"]              = edits
        parm_dict["unit_id"]            = unit_id
        parm_dict["by"]                 = by
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._instatus           = GensysInputDataset("instatus", instatus)
        self._outdata            = GensysOutputDataset("outdata", outdata)
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

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,
            self._instatus.c_arg,

            self._outdata.c_schema,
            self._outdata.c_array,
            self._outstatus.c_schema,
            self._outstatus.c_array,
        )

    def get_sort_list(self, include_by=True, include_unit_id=True):
        """Call superclass implementation using custom default values."""
        return super().get_sort_list(
            include_by=include_by,
            include_unit_id=include_unit_id,
        )
