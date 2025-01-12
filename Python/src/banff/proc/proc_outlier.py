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
    remove_rows_where,
)
from banff.proc import BanffProcedure

#******CLASS DEFINITIONS************************************************************

class ProcOutlier(BanffProcedure):
    """Identifies outlying observations using Hidiroglou-Berthelot or Sigma-Gap methods.

    This procedure offers two methods of univariate outlier detection. The Hidiroglou-Berthelot (HB)
    method selects outliers based on their distance from the median, relative to the interquartile
    distance. The Sigma-Gap (SG) method sorts the data in ascending order and searches for significant
    gaps (relative to the standard deviation) between consecutive values, selecting all subsequent
    values as outliers. Both methods can detect two types of outliers, which are flagged on the
    `outstatus` file:

    - Values that are extreme enough to be considered errors.
      These values are flagged as *fields to impute (FTI)* so they can be imputed in a subsequent step.
    - Values that are not extreme enough to be considered errors, but are sufficiently unusual to be
      deemed *fields to exclude (FTE)* by subsequent imputation procedures such as `donorimp` and
      `estimator`. (This flag can also be useful during weighting and robust estimation.)

    For both procedures, users must specify either an imputation or exclusion threshold; no default
    value is provided.

    Additional features of the procedure:

    - Users can run outlier detection on multiple variables (`var`) in one call.
    - Users can also run outlier detection on ratios of variables. In this case, only the numerators
      (`var`) are flagged on `outstatus`. For the denominator, users may select auxiliary variables
      (`with_var`) from the current period (`indata`) or from historical data (`indata_hist`).
    - Outlier detection can be performed to the right, left, or on both sides (`side`).
    - Outlier detection can be performed within by-groups (`by`), with a user-specified minimum number
      of observations (`min_obs`) required to perform outlier detection.
    """

    # static variables
    _proc_name = {"short": "outlier", "long": "Outlier"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # indata_hist

        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus_detailed
        c_argtype_output_dataset(),  # outstatus_detailed
        c_argtype_output_dataset(),  # outsummary
        c_argtype_output_dataset(),  # outsummary
    ]

    def __init__(self,
            # USER C code parameters
            unit_id: str | None = None,
            weight: str | None = None,
            by: str | None = None,
            var: str | None = None,
            with_var: str | None = None,
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            accept_zero: bool | None = None,
            outlier_stats: bool | None = None,
            beta_e: float | None = None,
            beta_i: float | None = None,
            exponent: float | None = None,
            mdm: float | None = None,
            mei: float | None = None,
            mii: float | None = None,
            start_centile: float | None = None,
            min_obs: int | None = None,
            method: str | None = None,
            side: str | None = None,
            sigma: str | None = None,
            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            indata_hist: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outstatus: Path | str | None = None,
            outstatus_detailed: Path | str | None = None,
            outsummary: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,
            exclude_where_indata   = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Identifies outlying observations using Hidiroglou-Berthelot or Sigma-Gap methods.

        :param unit_id: Identify key variable (unit identifier) on indata and indata_hist. Mandatory.
        :type unit_id: str | None, optional
        :param weight: Variable to be used for weighting.
        :type weight: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param var: Variables(s) for which to find outliers.
        :type var: str | None, optional
        :param with_var: Historical or auxiliary variables.
        :type with_var: str | None, optional
        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduces log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param accept_zero: Treat zero values as valid. Default=False in the presence of historical or auxiliary variables, True otherwise.
        :type accept_zero: bool | None, optional
        :param outlier_stats: Add more information to outstatus_detailed output table, including imputation and exclusion interval bounds. Default=False.
        :type outlier_stats: bool | None, optional
        :param beta_e: SG multiplier for exclusion interval (non-negative).
        :type beta_e: float | None, optional
        :param beta_i: SG multiplier for imputation interval (non-negative).
        :type beta_i: float | None, optional
        :param exponent: HB exponent for a ratio or historical trend (between 0 and 1). Default=0.
        :type exponent: float | None, optional
        :param mdm: HB minimum distance multiplier (positive). Default=0.05.
        :type mdm: float | None, optional
        :param mei: HB Multiplier for exclusion interval (positive).
        :type mei: float | None, optional
        :param mii: HB multiplier for imputation interval (positive).
        :type mii: float | None, optional
        :param start_centile: SG centile to be used to determine the starting point (between 0 and 100). Default=75 for 'side="BOTH"', 0 otherwise.
        :type start_centile: float | None, optional
        :param min_obs: Minimum number of observations that must exist in the input table or in a by-group (positive). Default=3 for HB, 5 for SG.
        :type min_obs: int | None, optional
        :param method: Method to be used to detect outlying observations ('CURRENT', 'RATIO', 'HISTORIC' or 'SIGMAGAP'). Mandatory.
        :type method: str | None, optional
        :param side: Side ('LEFT', 'RIGHT', or 'BOTH') of the ordered data to be used for detecting outliers. Default='BOTH'.
        :type side: str | None, optional
        :param sigma: SG type of deviation ('MAD' or 'STD') to be calculated. Default='MAD'.
        :type sigma: str | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param indata_hist: Input historical data.
        :type indata_hist: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outstatus: Contains the status of the fields (FTE/FTI) identified as outliers and their values.
        :type outstatus: Path | str | None, optional
        :param outstatus_detailed: Detailed status for the outliers (ODER/ODEL/ODIR/ODIL).
        :type outstatus_detailed: Path | str | None, optional
        :param outsummary: Outlier summary information such as observation counts and acceptance interval bounds.
        :type outsummary: Path | str | None, optional
        :param presort: Sorts input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param exclude_where_indata: Expression in SQL syntax to exclude observations from the outlier detection.
        :type exclude_where_indata: _type_, optional
        :param trace: Control which log levels are included when using the default logger.
        :type trace: int | bool | None, optional
        :param capture: Configure how console output is displayed.
        :type capture: bool | None, optional
        :param logger: Custom logger to use for procedure execution.
        :type logger: logging.Logger | None, optional
        """  # noqa: D401,E501
        # USER C code parameters
        parm_dict = {}
        parm_dict["unit_id"]           = unit_id
        parm_dict["weight"]            = weight
        parm_dict["by"]                = by
        parm_dict["var"]               = var
        parm_dict["with_var"]          = with_var
        parm_dict["accept_negative"]   = accept_negative
        parm_dict["no_by_stats"]       = no_by_stats
        parm_dict["accept_zero"]       = accept_zero
        parm_dict["outlier_stats"]     = outlier_stats
        parm_dict["beta_e"]            = beta_e
        parm_dict["beta_i"]            = beta_i
        parm_dict["exponent"]          = exponent
        parm_dict["mdm"]               = mdm
        parm_dict["mei"]               = mei
        parm_dict["mii"]               = mii
        parm_dict["start_centile"]     = start_centile
        parm_dict["min_obs"]           = min_obs
        parm_dict["method"]            = method
        parm_dict["side"]              = side
        parm_dict["sigma"]             = sigma
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._indata_hist        = GensysInputDataset("indata_hist", indata_hist)
        self._outstatus          = GensysOutputDataset("outstatus", outstatus)
        self._outstatus_detailed = GensysOutputDataset("outstatus_detailed", outstatus_detailed, mandatory=False)
        self._outsummary         = GensysOutputDataset("outsummary", outsummary, mandatory=False)

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[
                self._indata,
                self._indata_hist,
            ],
            output_datasets=[
                self._outstatus,
                self._outstatus_detailed,
                self._outsummary,
            ],
            presort=presort,
            prefill_by_vars=False,  # no input status dataset
            exclude_where_indata = exclude_where_indata,
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
    def indata_hist(self):
        return self._get_input_dataset(self._indata_hist)
    @indata_hist.setter
    def indata_hist(self, value):
        self._set_input_dataset(ds=self._indata_hist, value=value)

    @property
    def outstatus(self):
        return self._get_output_dataset(self._outstatus)
    @outstatus.setter
    def outstatus(self, value):
        self._set_output_dataset(ds=self._outstatus, value=value)

    @property
    def outstatus_detailed(self):
        return self._get_output_dataset(self._outstatus_detailed)
    @outstatus_detailed.setter
    def outstatus_detailed(self, value):
        self._set_output_dataset(ds=self._outstatus_detailed, value=value)

    @property
    def outsummary(self):
        return self._get_output_dataset(self._outsummary)
    @outsummary.setter
    def outsummary(self, value):
        self._set_output_dataset(ds=self._outsummary, value=value)

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,
            self._indata_hist.c_arg,


            self._outstatus.c_schema,
            self._outstatus.c_array,
            self._outstatus_detailed.c_schema,
            self._outstatus_detailed.c_array,
            self._outsummary.c_schema,
            self._outsummary.c_array,

        )

    def _pp_exclude_where_indata(self):
        """Remove rows matching user-specified criteria.

        Delete rows matching user-provided `exclude_where_indata` from indata.
        Based on Banff Processor 1.x `generateOutlier.sas`
        """
        self._indata.ds_intermediate = remove_rows_where(
            self._indata.ds_intermediate,
            where_stmt=self._exclude_where_indata,
        )
