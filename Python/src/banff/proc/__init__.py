# import for internal use
from banff.proc.banff_proc import (
    BanffProcedure,
)

# expoose each procedure class under aliases
from banff.proc.proc_determin import ProcDetermin as determin
from banff.proc.proc_donorimp import ProcDonorimp as donorimp
from banff.proc.proc_editstat import ProcEditstat as editstat
from banff.proc.proc_errorloc import ProcErrorloc as errorloc
from banff.proc.proc_estimato import ProcEstimato as estimato
from banff.proc.proc_massimpu import ProcMassimpu as massimpu
from banff.proc.proc_outlier import ProcOutlier as outlier
from banff.proc.proc_prorate import ProcProrate as prorate
from banff.proc.proc_verifyed import ProcVerifyed as verifyed

# expose methods for setting cross procedure default values
get_default = BanffProcedure.get_default
set_default = BanffProcedure.set_default

__all__ = [
    "BanffProcedure",
    "determin",
    "donorimp",
    "editstat",
    "errorloc",
    "estimato",
    "get_default",
    "massimpu",
    "outlier",
    "prorate",
    "set_default",
    "verifyed",
]
