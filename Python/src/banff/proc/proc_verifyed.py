import logging

from banff.io_util import c_argtype_parameters
from banff.proc import BanffProcedure

#******CLASS DEFINITIONS************************************************************

class ProcVerifyed(BanffProcedure):
    """Checks the edits for consistency and redundancy.

    The `verifyedits` procedure does not analyze statistical data or perform any SDE functions
    (review, selection, treatment). Instead, it is used to review a set of user-specified `edits`
    to verify consistency and identify any redundant edits, deterministic variables, or hidden
    qualities. Once these features are identified, the minimal set of edits is determined. Users
    are encouraged to review any set of proposed edits using `verifyedits` before calling the
    edit-based procedures `errorloc`, `deterministic`,`donorimp`, or `prorate`. Functions performed:

    * Consistency: the set of edits is checked for consistency, i.e., that the constraints define
      a non-empty feasible region.
    * Redundancy: produces a list of edits that are redundant, i.e., that can be removed without
      affecting the feasible region.
    * Bounds: produces implied upper and lower bounds for each variable. This also reveals any
      deterministic variables, i.e., variables that can only take on a single value.
    * Extremal points: generates the set of extremal points, or vertices, of the feasible region.
    * Hidden equalities: produces a list of hidden equalities not specified in the original list of edits.
    * Implied edits: generates a set of implied edits not specified in the original list of edits.
    * Minimal edits: generates a set of minimal edits required to define the feasible region generated
      by the original edits.

    Together, the outputs of `verifyedits` may give the user a better sense of the feasible region
    defined by the original edits, before using them in other procedures. Even if the original edits
    are consistent, the outputs may reveal unexpected or unintended constraints that can be addressed
    by adding, removing, or altering the edits. Using the minimal set of edits can also increase
    performance in other procedures.
    """

    # static variables
    _proc_name = {"short": "verifyed", "long": "Verify Edits"}
    _arg_types = [
        c_argtype_parameters(),  # parameters
    ]

    def __init__(self,
            # USER C code parameters
            accept_negative: bool | None = None,
            extremal: int | None = None,
            imply: int | None = None,
            edits: str | None = None,

            # super class options
            trace: int | bool | None = None,
            capture: bool | None = False,
            logger: logging.Logger | None = None,
            **kwargs,
        ):
        """Checks the edits for consistency and redundancy.

        :param accept_negative: Treat negative values as valid. Default=False.
        :type accept_negative: bool | None, optional
        :param extremal: Maximum cardinality of the shown extremal points. Default=0.
        :type extremal: int | None, optional
        :param imply: Number of implied edits to be generated. Default=0.
        :type imply: int | None, optional
        :param edits: List of consistency edits. Mandatory.
        :type edits: str | None, optional
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
        parm_dict["extremal"]           = extremal
        parm_dict["imply"]              = imply
        parm_dict["edits"]              = edits
        self.c_parms = parm_dict

        # call super constructor
        super().__init__(
            trace=trace, capture=capture, logger=logger,
            input_datasets=[],
            output_datasets=[],
            presort=False,  # no input datasets
            prefill_by_vars=False,  # no input status datasets
            keyword_args=kwargs,
        )

    def _call_c_code(self):
        return self._cproc_func(
            self._parm_dict,
        )
