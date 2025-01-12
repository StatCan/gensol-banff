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
from banff.nls import _
from banff.proc import BanffProcedure


#******CLASS DEFINITIONS************************************************************
class ProcDonorimp(BanffProcedure):
    """Performs nearest neighbour donor imputation such that each imputed record satisfies the specified post-imputation edits.

    The donorimp procedure splits records into recipients (records requiring imputation) and donors (records not requiring
    imputation that pass the edits). For each recipient, the procedure performs the following steps:

    1. From the fields in the edits, a subset are chosen as matching fields for the purpose of distance calculations.
       This selection can vary depending on which fields require imputation. Users can also specify must-match fields (`must_match`),
       which will automatically be included in distance calculations alongside the system-selected ones.
    2. Matching fields are transformed into normalized ranks to remove the effect of scale and clustering from the data.
       Without this transformation, original data with wide ranges, such as dollar values, would always dominate the distance calculation.
    3. Distances between the recipient and donors are calculated using a L-infinity norm on the transformed matching fields.
       This is sometimes referred to as the minimax distance because the closest donor is the one with the smallest maximum absolute
       difference between the transformed values of its matching fields and those of the recipient.
    4. From the donors, a search algorithm is used to efficiently find the closest donor whose values allow the recipient
       record to pass the user-specified post-imputation edits (`post_edits`). These are typically a more relaxed form of the edits
       to ensure a donor can be found.

    *Note: The Banff distance metric will usually select different donors than a typical Euclidean distance metric. This is by design.
    When using Euclidean distance metrics, scale differences and skewed distributions in economic data typically result in a distance
    metric that is dominated by a single field such as revenue. The Banff distance metric ensures that all matching fields are given
    the same weight in the distance calculation.*

    Recipients are defined as any record with at least one field within the edits requiring imputation, as indicated by an
    FTI (Field to Impute) flag on the input status (`instatus`) file. Donors are defined as any record satisfying all the edits that
    is not a recipient. The donorimp procedure requires a set of edits; for a version of donor imputation that does not, please
    see the `massimp` procedure.

    There are a number of ways to exclude records or values from the donor pool. Records can be excluded using the
    `exclude_where_indata` or `data_excl_var` parameters. This does not exclude them from the procedure completely;
    they may still be included as recipients if they require imputation. Records that have previously been imputed can
    also be excluded from the donor pool using the `eligdon` (eligible donor) parameter. The parameter `n_limit` will
    limit the number of times a single donor is used for imputation. Users may sometimes identify values that do not
    require imputation, but are sufficiently unusual that they should not be donated to other records; these should
    be flagged as FTE (Field to Exclude) on the instatus file.

    The Banff distance metric does not accommodate categorical variables. Instead, users may create by-groups by specifying
    `by` variables. These by-groups act as imputation classes. Use the `min_donors` and `percent_donors` parameters to ensure
    an appropriate number or ratio of recipients and donors exist in each imputation class before performing imputation.
    """

    # static variables
    _proc_name = {"short": "donorimp", "long": "Donor Imputation"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata
        c_argtype_input_dataset(),  # instatus

        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outdata
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outstatus
        c_argtype_output_dataset(),  # outdonormap
        c_argtype_output_dataset(),  # outdonormap
        c_argtype_output_dataset(),  # outmatching_fields
        c_argtype_output_dataset(),  # outmatching_fields
    ]

    def __init__(self,
            # USER C code parameters
            unit_id: str | None = None,
            by: str | None = None,
            must_match: str | None = None,
            data_excl_var: str | None = None,
            rand_num_var: str | None = None,
            random: bool | None = None,
            seed: int | None = None,
            edits: str | None = None,
            post_edits: str | None = None,
            display_level: int | None = None,
            accept_negative: bool | None = None,
            no_by_stats: bool | None = None,
            min_donors: int | None = None,
            percent_donors: float | None = None,
            n: int | None = None,
            eligdon: str | None = None,
            n_limit: int | None = None,
            mrl: float | None = None,
            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            instatus: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outdata: Path | str | None = None,
            outstatus: Path | str | None = None,
            outdonormap: Path | str | None = None,
            outmatching_fields: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,
            prefill_by_vars: bool | None = None,
            exclude_where_indata: str | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Performs nearest neighbour donor imputation such that each imputed record satisfies the specified post-imputation edits.

        :param unit_id: Identify key variable (unit identifier) on indata. Mandatory.
        :type unit_id: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param must_match: List of variables on indata to be used in distance calculations.
        :type must_match: str | None, optional
        :param data_excl_var: Identify donor exclusion variable on indata.
        :type data_excl_var: str | None, optional
        :param rand_num_var: Identify random number variable on indata.
        :type rand_num_var: str | None, optional
        :param random: Randomly select donors for recipients without matching fields.
        :type random: bool | None, optional
        :param seed: Specify a seed for the random number generator.
        :type seed: int | None, optional
        :param edits: List of consistency edits. Mandatory.
        :type edits: str | None, optional
        :param post_edits: List of edits a recipient must satisfy after imputation.
        :type post_edits: str | None, optional
        :param display_level: Value (0 or 1) to request detail output to the log in relation to the random number variable. Default=0.
        :type display_level: int | None, optional
        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param no_by_stats: Reduces log output by suppressing by-group specific messages. Default=False.
        :type no_by_stats: bool | None, optional
        :param min_donors: Minimum number of donors needed in a by-group to perform imputation.
        :type min_donors: int | None, optional
        :param percent_donors: Minimum percentage of donors needed in a by-group to perform imputation.
        :type percent_donors: float | None, optional
        :param n: Maximum number of donors to try for a given recipient.
        :type n: int | None, optional
        :param eligdon: Identify eligible donors as 'ANY' (include all potential donors) or 'ORIGINAL' (exclude records with imputed values).
        :type eligdon: str | None, optional
        :param n_limit: Limit the number of times a donor can be used.
        :type n_limit: int | None, optional
        :param mrl: Multiplier ratio limit used to limit number of times a donor can be used.
        :type mrl: float | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param instatus: Input status file containing FTI, FTE and I-- status flags. Mandatory.
        :type instatus: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outdata: Output statistical table containing imputed data.
        :type outdata: Path | str | None, optional
        :param outstatus: Output status file identifying imputed fields with IDN status flags, and their values after imputation.
        :type outstatus: Path | str | None, optional
        :param outdonormap: Output table of recipient-donor pairs for successfully imputed records.
        :type outdonormap: Path | str | None, optional
        :param outmatching_fields: Output table of matching fields for each recipient.
        :type outmatching_fields: Path | str | None, optional
        :param presort: Sorts input tables before processing, according to procedure requirements. Default=True.
        :type presort: bool | None, optional
        :param prefill_by_vars: Adds by-group variable(s) to input status file(s) to improve performance. Default=True.
        :type prefill_by_vars: bool | None, optional
        :param exclude_where_indata: Donor exclusion expression using SQL syntax.
        :type exclude_where_indata: str | None, optional
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
        parm_dict["must_match"]         = must_match
        parm_dict["data_excl_var"]      = data_excl_var
        parm_dict["rand_num_var"]       = rand_num_var
        parm_dict["random"]             = random
        parm_dict["seed"]               = seed
        parm_dict["edits"]              = edits
        parm_dict["post_edits"]         = post_edits
        parm_dict["display_level"]      = display_level
        parm_dict["accept_negative"]    = accept_negative
        parm_dict["no_by_stats"]        = no_by_stats
        parm_dict["min_donors"]         = min_donors
        parm_dict["percent_donors"]     = percent_donors
        parm_dict["n"]                  = n
        parm_dict["eligdon"]            = eligdon
        parm_dict["n_limit"]            = n_limit
        parm_dict["mrl"]                = mrl
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._instatus           = GensysInputDataset("instatus", instatus)
        self._outdata            = GensysOutputDataset("outdata", outdata)
        self._outdonormap        = GensysOutputDataset("outdonormap", outdonormap)
        self._outstatus          = GensysOutputDataset("outstatus", outstatus)
        self._outmatching_fields = GensysOutputDataset("outmatching_fields", outmatching_fields, mandatory=False, requested_by_default=False)

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
                self._outdonormap,
                self._outmatching_fields,
            ],
            presort=presort,
            prefill_by_vars=prefill_by_vars,
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
    def outmatching_fields(self):
        return self._get_output_dataset(self._outmatching_fields)
    @outmatching_fields.setter
    def outmatching_fields(self, value):
        self._set_output_dataset(ds=self._outmatching_fields, value=value)

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
            self._instatus.c_arg,

            self._outdata.c_schema,
            self._outdata.c_array,
            self._outstatus.c_schema,
            self._outstatus.c_array,
            self._outdonormap.c_schema,
            self._outdonormap.c_array,
            self._outmatching_fields.c_schema,
            self._outmatching_fields.c_array,
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

    def _validate_deprecations(self, log, keyword_args):
        """Procedure specific check for use of deprecated options."""
        super()._validate_deprecations(log=log, keyword_args=keyword_args)

        log_lcl = self._get_stack_logger(log)

        # Version 3.1.1b12: 'match_field_stat' flag replaced with optional output dataset 'outmatching_fields'
        if "match_field_stat" in keyword_args.keys():
            mesg = _("Option `{}` is deprecated, use `{}` instead").format("match_field_stat", "outmatching_fields")
            log_lcl.error(mesg)
            raise DeprecationWarning(mesg)
