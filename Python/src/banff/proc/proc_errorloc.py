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

class ProcErrorloc(BanffProcedure):
    """For each record, selects the minimum number of variables to impute such that each observation can be made to pass all edits.

    Consistency edits specify relationships between variables that a record must satisfy. When a
    record fails to satisfy these relationships, users must choose which variables to change, a
    process known as error localization. The Banff error localization procedure follows the
    Fellegi-Holt minimum-change principle, and uses an algorithm to select which variables to
    change. This process is performed independently on each record. Selected values are saved in
    the outstatus file, with a status flag of FTI (Field to impute).

    This procedure requires a set of edits, consisting of linear equalities and inequalities,
    that must be internally consistent. The procedure will only perform error localization on
    the variables included in the list of edits. Any missing values from amongst the listed
    variables will automatically be selected for imputation.

    By default, the procedure will minimize the number of variables to change. Users may also
    specify variable weights, in which case the procedure will minimize the weighted count of
    variables to change. For some records, the error localization problem may have multiple
    solutions (i.e., choices of variables) that satisfy the minimum-change principle; in this
    case one of the solutions is selected at random.
    """

    # static variables
    _proc_name = {"short": "errorloc", "long": "Error Localization"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # instatus

        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outreject
        c_argtype_output_dataset(),  # outreject
    ]

    def __init__(self,
            # USER C code parameters
            unit_id: str | None = None,
            by: str | None = None,
            rand_num_var: str | None = None,
            edits: str | None = None,
            weights: str | None = None,
            cardinality: float | None = None,
            time_per_obs: float | None = None,
            seed: int | None = None,
            display_level: int | None = None,
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outstatus: Path | str | None = None,
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
        """For each record, selects the minimum number of variables to impute such that each observation can be made to pass all edits.

        :param unit_id: Identify key variable (unit identifier) on indata. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param rand_num_var: Specify a random number variable to be used when having to make a choice during error localization.
        :type rand_num_var: str | None, optional
        :param edits: List of consistency edits. Mandatory.
        :type edits: str | None, optional
        :param weights: Specify the error localization weights.
        :type weights: str | None, optional
        :param cardinality: Specify the maximum cardinality.
        :type cardinality: float | None, optional
        :param time_per_obs: Specify the maximum processing time allowed per observation.
        :type time_per_obs: float | None, optional
        :param seed: Specify the root for the random number generator.
        :type seed: int | None, optional
        :param display_level: Value (0 or 1) to request detail output to the log in relation to the random number variable. Default=0.
        :type display_level: int | None, optional
        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduce log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus: Input status file containing FTI status flags.
        :type instatus: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outstatus: Output status file identifying selected fields with FTI status flags, and their values.
        :type outstatus: Path | str | None, optional
        :param outreject: Output table containing records that failed error localization.
        :type outreject: Path | str | None, optional
        :param presort: Sort input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param prefill_by_vars: Add by-group variable(s) to input status file(s) to improve performance. Default=True.
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
        parm_dict["unit_id"]            = unit_id
        parm_dict["by"]                 = by
        parm_dict["rand_num_var"]       = rand_num_var
        parm_dict["edits"]              = edits
        parm_dict["weights"]            = weights
        parm_dict["cardinality"]        = cardinality
        parm_dict["time_per_obs"]       = time_per_obs
        parm_dict["seed"]               = seed
        parm_dict["display_level"]      = display_level
        parm_dict["accept_negative"]    = accept_negative
        parm_dict["no_by_stats"]        = no_by_stats
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._instatus           = GensysInputDataset("instatus", instatus)
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
