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
    flag_rows_where,
)
from banff.proc import BanffProcedure

#******CLASS DEFINITIONS************************************************************

class ProcEstimato(BanffProcedure):
    """Performs imputation using estimation functions and/or linear regression estimators.

    The estimator procedure offers imputation methods such as mean, ratio and regression imputation
    using current (`indata`) and/or historical data (`indata_hist`) for the variable to impute and
    potentially auxiliary variables. Users may choose from twenty (20) pre-defined imputation
    estimator algorithms that are included in the procedure, or define their own custom algorithms.

    Only fields with an FTI (Field to Impute) from the `instatus` file are imputed. Fields with
    FTE (Field to Excluded) or I-- (Imputed Field) flags are excluded from the imputation model.
    (Note that this does not include the flag IDE, which indicates deterministic imputation.)

    Estimator or linear regression parameters (e.g. means or regression coefficients) can be
    calculated on all records or on a particular subset of acceptable records. The restriction
    of the acceptable records can be applied using an exclusion parameter or by specifying
    by-groups imputation.
    """

    # static variables
    _proc_name = {"short": "estimato", "long": "Estimator"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # instatus
        c_argtype_input_dataset(),  # indata_hist
        c_argtype_input_dataset(),  # instatus_hist
        c_argtype_input_dataset(),  # inalgorithm
        c_argtype_input_dataset(),  # inestimator

        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outacceptable
        c_argtype_output_dataset(),  # outacceptable
        c_argtype_output_dataset(),  # outest_ef
        c_argtype_output_dataset(),  # outest_ef
        c_argtype_output_dataset(),  # outest_lr
        c_argtype_output_dataset(),  # outest_lr
        c_argtype_output_dataset(),  # outest_parm
        c_argtype_output_dataset(),  # outest_parm
        c_argtype_output_dataset(),  # outrand_err
        c_argtype_output_dataset(),  # outrand_err
    ]

    def __init__(self,
            # USER C code parameters
            unit_id: str | None = None,
            by: str | None = None,
            data_excl_var: str | None = None,
            hist_excl_var: str | None = None,
            seed: int | None = None,
            verify_specs: bool | None = None,
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            indata_hist: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            inalgorithm: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            inestimator: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus_hist: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outstatus: Path | str | None = None,
            outdata: Path | str | None = None,
            outacceptable: Path | str | None = None,
            outest_ef: Path | str | None = None,
            outest_lr: Path | str | None = None,
            outest_parm: Path | str | None = None,
            outrand_err: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,
            prefill_by_vars: bool | None = None,
            exclude_where_indata: str | None = None,
            exclude_where_indata_hist: str | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Performs imputation using estimation functions and/or linear regression estimators.

        :param unit_id: Identify key variable (unit identifier) on indata and indata_hist. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param data_excl_var: Variable of the input table used to exclude observations from the set of acceptable observations.
        :type data_excl_var: str | None, optional
        :param hist_excl_var: Variable of the historical input table used to exclude historical observations from the set of acceptable observations.
        :type hist_excl_var: str | None, optional
        :param seed: Specify the root for the random number generator.
        :type seed: int | None, optional
        :param verify_specs: Estimator specifications verified without running the imputation.
        :type verify_specs: bool | None, optional
        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduce log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus: Input status file containing FTI, FTE and I-- status flags. Mandatory.
        :type instatus: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param indata_hist: Input historical data.
        :type indata_hist: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param inalgorithm: User defined algorithms table.
        :type inalgorithm: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param inestimator: Estimator specifications table. Mandatory.
        :type inestimator: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus_hist: Input historical status file containing FTI, FTE and I-- status flags.
        :type instatus_hist: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outstatus: Output status file identifying imputed fields with I-- status flags, and their values after imputation.
        :type outstatus: Path | str | None, optional
        :param outdata: Output statistical table containing imputed data.
        :type outdata: Path | str | None, optional
        :param outacceptable: Report on acceptable observations retained to calculate parameters for each estimator.
        :type outacceptable: Path | str | None, optional
        :param outest_ef: Report on calculation of averages for estimator functions.
        :type outest_ef: Path | str | None, optional
        :param outest_lr: Report on calculation of « beta » coefficients for linear regression estimators (type LR).
        :type outest_lr: Path | str | None, optional
        :param outest_parm: Report on imputation statistics by estimator.
        :type outest_parm: Path | str | None, optional
        :param outrand_err: Random error report when a random error is added to the imputed variable.
        :type outrand_err: Path | str | None, optional
        :param presort: Sort input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param prefill_by_vars: Add by-group variable(s) to input status file(s) to improve performance. Default=True.
        :type prefill_by_vars: bool | None, optional
        :param exclude_where_indata: Exclusion expression using SQL syntax to specify which observations to exclude from the set of acceptable observations.
        :type exclude_where_indata: str | None, optional
        :param exclude_where_indata_hist: Exclusion expression using SQL syntax to specify which historical observations to exclude from the set of acceptable observations.
        :type exclude_where_indata_hist: str | None, optional
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
        parm_dict["data_excl_var"]      = data_excl_var
        parm_dict["hist_excl_var"]      = hist_excl_var
        parm_dict["seed"]               = seed
        parm_dict["verify_specs"]       = verify_specs
        parm_dict["accept_negative"]    = accept_negative
        parm_dict["no_by_stats"]        = no_by_stats
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._inalgorithm        = GensysInputDataset("inalgorithm", inalgorithm)
        self._indata             = GensysInputDataset("indata", indata)
        self._indata_hist        = GensysInputDataset("indata_hist", indata_hist)
        self._inestimator        = GensysInputDataset("inestimator", inestimator)
        self._instatus           = GensysInputDataset("instatus", instatus)
        self._instatus_hist      = GensysInputDataset("instatus_hist", instatus_hist)

        if verify_specs is True:
            # all output datasets (including mandatory ones) disabled when `verify_specs` is `True`
            self._outacceptable      = GensysOutputDataset("outacceptable",     output_specification=False, mandatory=False)
            self._outdata            = GensysOutputDataset("outdata",           output_specification=False, mandatory=False)
            self._outest_ef          = GensysOutputDataset("outest_ef",         output_specification=False, mandatory=False)
            self._outest_lr          = GensysOutputDataset("outest_lr",         output_specification=False, mandatory=False)
            self._outest_parm        = GensysOutputDataset("outest_parm",       output_specification=False, mandatory=False)
            self._outrand_err        = GensysOutputDataset("outrand_err",       output_specification=False, mandatory=False)
            self._outstatus          = GensysOutputDataset("outstatus",         output_specification=False, mandatory=False)
        else:
            self._outacceptable      = GensysOutputDataset("outacceptable", outacceptable, mandatory=False)
            self._outdata            = GensysOutputDataset("outdata", outdata)
            self._outest_ef          = GensysOutputDataset("outest_ef", outest_ef, mandatory=False)
            self._outest_lr          = GensysOutputDataset("outest_lr", outest_lr, mandatory=False)
            self._outest_parm        = GensysOutputDataset("outest_parm", outest_parm, mandatory=False)
            self._outrand_err        = GensysOutputDataset("outrand_err", outrand_err, mandatory=False)
            self._outstatus          = GensysOutputDataset("outstatus", outstatus)

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[
                self._indata,
                self._instatus,
                self._indata_hist,
                self._inalgorithm,
                self._inestimator,
                self._instatus_hist,
            ],
            output_datasets=[
                self._outstatus,
                self._outdata,
                self._outacceptable,
                self._outest_ef,
                self._outest_lr,
                self._outest_parm,
                self._outrand_err,
            ],
            presort=presort,
            prefill_by_vars=prefill_by_vars,
            exclude_where_indata = exclude_where_indata,
            exclude_where_indata_hist = exclude_where_indata_hist,
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
    def indata_hist(self):
        return self._get_input_dataset(self._indata_hist)
    @indata_hist.setter
    def indata_hist(self, value):
        self._set_input_dataset(ds=self._indata_hist, value=value)

    @property
    def inalgorithm(self):
        return self._get_input_dataset(self._inalgorithm)
    @inalgorithm.setter
    def inalgorithm(self, value):
        self._set_input_dataset(ds=self._inalgorithm, value=value)

    @property
    def inestimator(self):
        return self._get_input_dataset(self._inestimator)
    @inestimator.setter
    def inestimator(self, value):
        self._set_input_dataset(ds=self._inestimator, value=value)

    @property
    def instatus_hist(self):
        return self._get_input_dataset(self._instatus_hist)
    @instatus_hist.setter
    def instatus_hist(self, value):
        self._set_input_dataset(ds=self._instatus_hist, value=value)

    @property
    def outstatus(self):
        return self._get_output_dataset(self._outstatus)
    @outstatus.setter
    def outstatus(self, value):
        self._set_output_dataset(ds=self._outstatus, value=value)

    @property
    def outdata(self):
        return self._get_output_dataset(self._outdata)
    @outdata.setter
    def outdata(self, value):
        self._set_output_dataset(ds=self._outdata, value=value)

    @property
    def outacceptable(self):
        return self._get_output_dataset(self._outacceptable)
    @outacceptable.setter
    def outacceptable(self, value):
        self._set_output_dataset(ds=self._outacceptable, value=value)

    @property
    def outest_ef(self):
        return self._get_output_dataset(self._outest_ef)
    @outest_ef.setter
    def outest_ef(self, value):
        self._set_output_dataset(ds=self._outest_ef, value=value)

    @property
    def outest_lr(self):
        return self._get_output_dataset(self._outest_lr)
    @outest_lr.setter
    def outest_lr(self, value):
        self._set_output_dataset(ds=self._outest_lr, value=value)

    @property
    def outest_parm(self):
        return self._get_output_dataset(self._outest_parm)
    @outest_parm.setter
    def outest_parm(self, value):
        self._set_output_dataset(ds=self._outest_parm, value=value)

    @property
    def outrand_err(self):
        return self._get_output_dataset(self._outrand_err)
    @outrand_err.setter
    def outrand_err(self, value):
        self._set_output_dataset(ds=self._outrand_err, value=value)

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,
            self._instatus.c_arg,
            self._indata_hist.c_arg,
            self._instatus_hist.c_arg,
            self._inalgorithm.c_arg,
            self._inestimator.c_arg,

            self._outdata.c_schema,
            self._outdata.c_array,
            self._outstatus.c_schema,
            self._outstatus.c_array,
            self._outacceptable.c_schema,
            self._outacceptable.c_array,
            self._outest_ef.c_schema,
            self._outest_ef.c_array,
            self._outest_lr.c_schema,
            self._outest_lr.c_array,
            self._outest_parm.c_schema,
            self._outest_parm.c_array,
            self._outrand_err.c_schema,
            self._outrand_err.c_array,
        )

    def _pp_exclude_where_indata(self):
        """Flag rows for exclusion, if matching user-specified criteria.

        Adds "exclusion" flag to indata rows matching user-provided `exclude_where_indata`.
        Based on Banff Processor 1.x `generateDonorImputation.sas`
        """
        self._indata.ds_intermediate, self.c_parms["data_excl_var"] = flag_rows_where(
            self._indata.ds_intermediate,
            where_stmt=self._exclude_where_indata,
        )

    def _pp_exclude_where_indata_hist(self):
        """Flag rows for exclusion, if matching user-specified criteria.

        Adds "exclusion" flag to indata_hist rows matching user-provided `exclude_where_indata_hist`.
        Based on Banff Processor 1.x `generateEstimator.sas`
        """
        self._indata_hist.ds_intermediate, self.c_parms["hist_excl_var"] = flag_rows_where(
            self._indata_hist.ds_intermediate,
            where_stmt=self._exclude_where_indata_hist,
        )
