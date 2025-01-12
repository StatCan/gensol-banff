import logging
import sys
import time

from ..nls import _

_top_logger = None

# apply different format to different levels
class SpecialFormatter(logging.Formatter):
    """special formatter class defines and applies applies different log format depending on the log level.

    Add more entries to the FORMATS object as desired.
    """

    FORMATS = {logging.DEBUG :"%(asctime)s [%(levelname)-8s, %(name)s]:  %(message)s",
               logging.INFO : "%(asctime)s [%(levelname)s]:  %(message)s"}

    def format(self, record):
        # get log-level specific formatter, default to DEBUG formatter
        log_fmt = self.FORMATS.get(record.levelno, self.FORMATS[logging.INFO])
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)

def get_console_handler(trace_level=logging.DEBUG):
    """Create a handler (with formatting) for printing to the console's standard output."""
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setFormatter(SpecialFormatter())
    console_handler.setLevel(trace_level)

    return console_handler

def get_import_logger(trace_level=logging.WARNING):
    """Acquire import logger (child to top logger)."""
    import_logger = get_top_logger().getChild("_IMPORTS_")

    import_logger.setLevel(trace_level)
    return import_logger


def get_misc_logger(trace_level=logging.WARNING):
    """Acquire a misc logger (child to top logger)."""
    misc_logger = get_top_logger().getChild("_MISC_")

    misc_logger.setLevel(trace_level)
    return misc_logger

def init_top_level(logger_name="gensys_common", trace_level=logging.DEBUG):
    """Initialize top level logger for the package, "gensys_common" by default.

    In general, this will be the only logger to which a handler is attached.
    All child loggers should propagate up to here where they will be handled.

    The new logger is stored in a global variable which is what
    `get_top_logger()` later returns.
    """
    # create top level logger
    new_logger = logging.getLogger(logger_name)

    # set level
    new_logger.setLevel(trace_level)

    # add console handler
    new_logger.addHandler(get_console_handler(trace_level))

    global _top_logger
    _top_logger = new_logger
    return get_top_logger()

def get_timezone_message():
    """Return a string indicating the time zone used for log entries."""
    lcl_time = time.localtime()
    out = _("Time zone for log entries: {} (UTC{:+})").format(
        lcl_time.tm_zone,
        lcl_time.tm_gmtoff/60/60,
    )
    return out

def get_top_logger():
    """Retrieve the top level logger."""
    return _top_logger

def init_proc_level(logger_name, parent_name=None, trace_level=logging.WARNING):
    """Initialize procedure level logger.

    This is like a top level logger at the procedure level.
    All procedure level stuff will pass through here before propagating up
    """
    if parent_name is None:
        parent_name = get_top_logger().name
    # create procedure level logger
    new_logger = logging.getLogger(parent_name + "." + logger_name)

    # set level
    if isinstance(trace_level, bool):
        if(trace_level):    # if `trace_level==True`, set to DEBUG
            new_logger.setLevel(logging.DEBUG)
        else:   # if `trace_level==False`, set to default (WARNING)
            new_logger.setLevel(logging.WARNING)
    elif isinstance(trace_level, int):
        new_logger.setLevel(trace_level)

    return new_logger

def get_stack_logger(parent_logger, level = 1):
    """Initialize child logger named after function.

    By default this looks at the call stack to find the caller's name
    and appends that to the parent logger.
    To create a logger with a function name further up the call stack,
    increase `level` past its default value of 1.
    """
    try:
        # `sys._getframe()` "not guaranteed to exist in all implementations of Python"
        return parent_logger.getChild(sys._getframe(level).f_code.co_name)  # noqa: SLF001  # all risks mitigated by try-except
    except AttributeError:
        return parent_logger
