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

class ProcMassimpu(BanffProcedure):
    """Performs donor imputation for a block of variables using a nearest neighbour approach or random selection.

    The `massimp` procedure is intended for use when a large block of variables is missing for a set of
    respondents, typically when detailed information is collected only for a subsample (or second phase
    sample) of units. While the `donorimp` procedure uses both system and user matching fields,
    mass imputation only considers user matching fields to find a valid record (donor) that is most
    similar to the one which needs imputation (recipient).

    Mass imputation considers a recipient any record for which all the variables to impute (`must_impute`)
    are missing on `indata`, and considers a donors any record for which none of the listed variables are
    missing. If matching fields (`must_match`) are provided by the user, the `massimp` procedure uses them
    to find the nearest donor using the same distance function as `donorimp`. If matching fields are not
    provided, a donor is selected at random.

    Unlike `donorimp`, the `massimp` procedure does not use edits. Before running the procedure, users
    should ensure that the pool of potential donors do not include any errors, including outliers or
    consistency errors.

    Users may create by-groups by specifying `by` variables. These by-groups act as imputation classes.
    Use the `min_donors` and `percent_donors` parameters to ensure an appropriate number or ratio of
    recipients and donors exist in each imputation class before performing imputation.
    """

    # static variables
    _proc_name = {"short": "massimpu", "long": "Mass Imputation"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata

        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outdonormap
        c_argtype_output_dataset(),  # outdonormap
    ]

    def __init__(self,
            # USER C code parameters
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            random: bool | None = None,
            mrl: float | None = None,
            percent_donors: float | None = None,
            min_donors: int | None = None,
            n_limit: int | None = None,
            seed: int | None = None,
            unit_id: str | None = None,
            by: str | None = None,
            must_impute: str | None = None,
            must_match: str | None = None,

            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outdata: Path | str | None = None,
            outstatus: Path | str | None = None,
            outdonormap: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Performs donor imputation for a block of variables using a nearest neighbour approach or random selection.

        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduces log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param random: Random selection of donors.
        :type random: bool | None, optional
        :param mrl: Multiplier ratio limit.
        :type mrl: float | None, optional
        :param percent_donors: Minimum percentage of donors required to perform imputation. Default=30.
        :type percent_donors: float | None, optional
        :param min_donors: Minimum number of donors required to perform imputation. Default=30.
        :type min_donors: int | None, optional
        :param n_limit: Maximum number of times a donor can be used.
        :type n_limit: int | None, optional
        :param seed: Specify the root for the random number generator.
        :type seed: int | None, optional
        :param unit_id: Identify key variable (unit identifier) on indata. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param must_impute: Variables(s) to be imputed. Mandatory.
        :type must_impute: str | None, optional
        :param must_match: User defined matching field(s).
        :type must_match: str | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outdata: Output statistical table containing imputed data.
        :type outdata: Path | str | None, optional
        :param outstatus: Output status file identifying imputed fields with IMAS status flags, and their values after imputation.
        :type outstatus: Path | str | None, optional
        :param outdonormap: Output table of recipient-donor pairs for successfully imputed records.
        :type outdonormap: Path | str | None, optional
        :param presort: Sorts input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
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
        parm_dict["random"]             = random
        parm_dict["mrl"]                = mrl
        parm_dict["percent_donors"]     = percent_donors
        parm_dict["min_donors"]         = min_donors
        parm_dict["n_limit"]            = n_limit
        parm_dict["seed"]               = seed
        parm_dict["unit_id"]            = unit_id
        parm_dict["by"]                 = by
        parm_dict["must_impute"]        = must_impute
        parm_dict["must_match"]         = must_match
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._outdata            = GensysOutputDataset("outdata", outdata)
        self._outdonormap        = GensysOutputDataset("outdonormap", outdonormap)
        self._outstatus          = GensysOutputDataset("outstatus", outstatus)

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[
                self._indata,
            ],
            output_datasets=[
                self._outdata,
                self._outstatus,
                self._outdonormap,
            ],
            presort=presort,
            prefill_by_vars=False,  # no input status dataset
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
    def outdonormap(self):
        return self._get_output_dataset(self._outdonormap)
    @outdonormap.setter
    def outdonormap(self, value):
        self._set_output_dataset(ds=self._outdonormap, value=value)

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,

            self._outdata.c_schema,
            self._outdata.c_array,
            self._outstatus.c_schema,
            self._outstatus.c_array,
            self._outdonormap.c_schema,
            self._outdonormap.c_array,
        )
