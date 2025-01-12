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

class ProcEditstat(BanffProcedure):
    """Produces edit summary statistics tables on records that pass, miss or fail each consistency edit.

    This procedure applies a group of `edits` to statistical data and determines if each record passes,
    misses (due to missing values) or fails each edit. Resulting diagnostics are saved to five output
    tables, and can be used to fine-tune the group of edits, estimate the resources required for later
    procedures, or to evaluate the effects of imputation. Note that this procedure only reviews the
    data, producing summary statistics; use `errorloc` (with the same set of edits) to select records
    and fields for further treatment.
    """

    # static variables
    _proc_name = {"short": "editstat", "long": "Editstats"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
        c_argtype_input_dataset(),  # indata

        c_argtype_output_dataset(),  # outedit_applic
        c_argtype_output_dataset(),  # outedit_applic
        c_argtype_output_dataset(),  # outedit_status
        c_argtype_output_dataset(),  # outedit_status
        c_argtype_output_dataset(),  # outglobal_status
        c_argtype_output_dataset(),  # outglobal_status
        c_argtype_output_dataset(),  # outk_edits_status
        c_argtype_output_dataset(),  # outk_edits_status
        c_argtype_output_dataset(),  # outedits_reduced
        c_argtype_output_dataset(),  # outedits_reduced
        c_argtype_output_dataset(),  # outvars_role
        c_argtype_output_dataset(),  # outvars_role
    ]

    def __init__(self,
            # USER C code parameters
            accept_negative: bool | None = None,
            edits: str | None = None,
            by: str | None = None,
            # USER dataset references
            indata: pyarrow.Table | pandas.DataFrame | Path | str | None = None,
            outedit_applic: Path | str | None = None,
            outedit_status: Path | str | None = None,
            outglobal_status: Path | str | None = None,
            outk_edits_status: Path | str | None = None,
            outedits_reduced: Path | str | None = None,
            outvars_role: Path | str | None = None,

            # Fancy New Options
            presort: bool | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Produces edit summary statistics tables on records that pass, miss or fail each consistency edit.

        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param edits: List of consistency edits. Mandatory.
        :type edits: str | None, optional
        :param by: Variable(s) used to partition indata into by-groups for independent processing.
        :type by: str | None, optional
        :param indata: Input statistical data. Mandatory.
        :type indata: pyarrow.Table | pandas.DataFrame | Path | str | None, optional
        :param outedit_applic: Number of times each variable is involved in an edit which passes, misses or fails.
        :type outedit_applic: Path | str | None, optional
        :param outedit_status: Number of records which pass, miss or fail each edit.
        :type outedit_status: Path | str | None, optional
        :param outglobal_status: Number of records with pass, miss or fail overall record status.
        :type outglobal_status: Path | str | None, optional
        :param outk_edits_status: Distribution of records which pass, miss or fail a given number of edits.
        :type outk_edits_status: Path | str | None, optional
        :param outedits_reduced: Minimal set of edits.
        :type outedits_reduced: Path | str | None, optional
        :param outvars_role: Number of times each variable contributes to the overall record status.
        :type outvars_role: Path | str | None, optional
        :param presort: Sorts input tables before processing according to procedure requirements. Default=True.
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
        parm_dict["edits"]              = edits
        parm_dict["by"]                 = by
        self.c_parms = parm_dict

        # INTERNAL dataset components (they store USER datasets/output specifications)
        self._indata             = GensysInputDataset("indata", indata)
        self._outedit_applic     = GensysOutputDataset("outedit_applic", outedit_applic, mandatory=False)
        self._outedit_status     = GensysOutputDataset("outedit_status", outedit_status, mandatory=False)
        self._outglobal_status   = GensysOutputDataset("outglobal_status", outglobal_status, mandatory=False)
        self._outk_edits_status  = GensysOutputDataset("outk_edits_status", outk_edits_status, mandatory=False)
        self._outedits_reduced   = GensysOutputDataset("outedits_reduced", outedits_reduced, mandatory=False)
        self._outvars_role       = GensysOutputDataset("outvars_role", outvars_role, mandatory=False)

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[
                self._indata,
            ],
            output_datasets=[
                self._outedit_applic,
                self._outedit_status,
                self._outglobal_status,
                self._outk_edits_status,
                self._outedits_reduced,
                self._outvars_role,
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
    def outedit_applic(self):
        return self._get_output_dataset(self._outedit_applic)
    @outedit_applic.setter
    def outedit_applic(self, value):
        self._set_output_dataset(ds=self._outedit_applic, value=value)

    @property
    def outedit_status(self):
        return self._get_output_dataset(self._outedit_status)
    @outedit_status.setter
    def outedit_status(self, value):
        self._set_output_dataset(ds=self._outedit_status, value=value)

    @property
    def outglobal_status(self):
        return self._get_output_dataset(self._outglobal_status)
    @outglobal_status.setter
    def outglobal_status(self, value):
        self._set_output_dataset(ds=self._outglobal_status, value=value)

    @property
    def outk_edits_status(self):
        return self._get_output_dataset(self._outk_edits_status)
    @outk_edits_status.setter
    def outk_edits_status(self, value):
        self._set_output_dataset(ds=self._outk_edits_status, value=value)

    @property
    def outedits_reduced(self):
        return self._get_output_dataset(self._outedits_reduced)
    @outedits_reduced.setter
    def outedits_reduced(self, value):
        self._set_output_dataset(ds=self._outedits_reduced, value=value)

    @property
    def outvars_role(self):
        return self._get_output_dataset(self._outvars_role)
    @outvars_role.setter
    def outvars_role(self, value):
        self._set_output_dataset(ds=self._outvars_role, value=value)

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,

            self._indata.c_arg,

            self._outedit_applic.c_schema,
            self._outedit_applic.c_array,
            self._outedit_status.c_schema,
            self._outedit_status.c_array,
            self._outglobal_status.c_schema,
            self._outglobal_status.c_array,
            self._outk_edits_status.c_schema,
            self._outk_edits_status.c_array,
            self._outedits_reduced.c_schema,
            self._outedits_reduced.c_array,
            self._outvars_role.c_schema,
            self._outvars_role.c_array,
        )
