import ctypes
import importlib.resources
import pathlib
import platform
import warnings

from .. import _log as lg
from ..nls import (
    SupportedLanguage,
    _,
    get_language,
)

fflush_func_name="flush_std_buffers"
free_func_name="free_memory"
rc_desc_func_name="get_rc_description"

def get_shared_library_path(dll_name_base, bin_root=None, lang=None):
    """Given a `dll_name_base` - filename without extension, generate the expected.

    path to a procedure's shared library (.dll or .so) file.
    """
    # add language suffix to base name
    if lang is None:
        lang=get_language()

    if lang == SupportedLanguage.en:
        lang_part="_en"
    elif lang == SupportedLanguage.fr:
        lang_part="_fr"
    else:
        lang_part="_en"
        mesg = _("library loader unable to determine language, defaulting to English")
        warnings.warn(mesg, stacklevel=1)

    dll_name_base = dll_name_base + lang_part

    # add extension to base name
    d = importlib.resources.files(bin_root)
    if platform.system() == "Windows":
        platform_name="windows"
        dll_name = dll_name_base + ".dll"
    elif platform.system() == "Linux":
        platform_name="linux"
        dll_name = "lib" + dll_name_base + ".so"

    # return full path to file
    return str(d / platform_name / "x64" / dll_name)

def load_shared_library(dll_path, proc_name, c_function_name, log, return_type, arg_types=None):
    """Load a shared library.

    This should be called only once (per proc) during package import.
    The libraries loaded during import remain loaded and are utilized throughout the python session
    - load shared library
    - setup correct arguments (`arg_types`) for function `c_function_name`
    - setup `fflush_func_name` function
    - setup `free_func_name` function
    """
    if arg_types is None:
        arg_types=[]
    if log is None:
        log_lcl = lg.get_misc_logger(trace_level=lg.WARNING)
    else:
        log_lcl = log

    if not pathlib.Path(dll_path).exists():
        mesg = _("C build not found at '{}'").format(dll_path)
        raise FileNotFoundError(mesg)

    try:
        my_cdll = ctypes.CDLL(dll_path)# , use_errno=True, use_last_error=True)

        # *** procedure ***
        #set function pointer
        cproc_func =  my_cdll[c_function_name]
        #define C code argument type(s)
        cproc_func.argtypes = arg_types
        #define C code return type
        cproc_func.restype = return_type

        # *** utility functions ***
        # load C fflush function, if available
        try:
            my_cdll[fflush_func_name].restype = None
            fflush_func = my_cdll[fflush_func_name]
        except Exception:
            log_lcl.exception(_("Unexpected exception occurred while loading '{}' function").format(fflush_func_name))
            fflush_func = None
            raise

        # load C free_memory function, if available
        try:
            my_cdll[free_func_name].argtypes = [ctypes.c_void_p]
            my_cdll[free_func_name].restype = None
            free_func = my_cdll[free_func_name]
        except Exception:
            log_lcl.exception(_("Unexpected exception occurred while loading '{}' function").format(free_func_name))
            free_func = None
            raise

        # load return code description function, if available
        try:
            rc_desc_func = my_cdll[rc_desc_func_name]
            rc_desc_func.argtypes = [ctypes.c_int]
            rc_desc_func.restype = ctypes.c_char_p
        except Exception:
            log_lcl.exception(_("Unexpected exception occurred while loading '{}' function").format(rc_desc_func_name))
            rc_desc_func = None
            raise

    except OSError:
        log_lcl.exception(_("OSError exception occurred while loading shared library '{}'").format(dll_path))
        raise
    log_lcl.info(_("Successfully loaded '{}' module from '{}'").format(proc_name, dll_path))

    return (cproc_func, fflush_func, free_func, rc_desc_func)
