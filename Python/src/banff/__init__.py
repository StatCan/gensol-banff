from importlib.metadata import version

import banff._log as logging
import banff.nls
from banff._log import log_levels as log_level  # expose log levels as `log_level`, refer to like `banff.log_level.INFO`
from banff.io_util import (
    get_default_output_spec,
    set_default_output_spec,
)
from banff.nls import (
    SupportedLanguage,  # `SupportedLanguage` for user reference
    _,  # expose `_()` for local internal use
)
from banff.proc import (
    determin,
    donorimp,
    editstat,
    errorloc,
    estimato,
    massimpu,
    outlier,
    prorate,
    verifyed,
)

# get version from package metadata
__version__ = version("banff")

lg = logging  # alias for logging

# Initialize Native Language Support (bilingual messages)
banff.nls.set_language()

# Define method of modifying language (affects both Python and C)
def set_language(new_lang=None):
    """Set the languaged used for console log messages.

    For `new_lang`, specify a value from `banff.SupportedLanguage`.
    """
    banff.nls.set_language(lang=new_lang)
    banff.proc.BanffProcedure._reload_all_procs(lang=banff.nls.get_language())  # noqa: SLF001

# Initialize top level logger
log_lcl = lg.init_top_level(logger_name="banff")
log_lcl.info(_("Importing Banff package version {}").format(__version__))

# additional procedure aliases
deterministic   = determin
editstats       = editstat
estimator       = estimato
massimp         = massimpu
verifyedits     = verifyed

banff.proc.BanffProcedure._load_all_procs(lang=banff.nls.get_language())  # noqa: SLF001

log_lcl.info(_("Banff package imported"))

__all__ = [
    "SupportedLanguage", # ?for user reference?
    "determin",
    "deterministic",
    "donorimp",
    "editstat",
    "editstats",
    "errorloc",
    "estimato",
    "estimator",
    "get_default_output_spec",
    "log_level", # for user reference
    "massimp",
    "massimpu",
    "outlier",
    "prorate",
    "set_default_output_spec",
    "set_language",
    "verifyed",
    "verifyedits",
]
