"""# Diagnostics utilities.

## Available Diagnostics

This package facilitates diagnostics of Python packages.
It defines a class for each type of diagnostic supported.
A list of `available_diagnostics` classes is available.

## SystemStats

For convenience, the `SystemStats` class (context manager) can be used to invoke
all available diagnostics using only a single class.

## Enabling and Disabling Diagnostics

By default, diagnostics are disabled.
All diagnostics classes are derived from `GensysDiagnostics`, which implements the class methods
- `.enable_global()`   enable the diagnostic globally
- `.disable_global()`  disable the diagnostic globally
Use them to turn individual diagnostics on or off.
    for example, `.diagnotics.MemoryUsage.enable_global()`

For convenience, functions `enable_all()` and `disable_all()` can be used to
enable or disable all `available_diagnostics` classes, respectively.
- Exception: `SystemStats` is enabled, since it's just a wrapper for other diagnostics classes.

### Diagnostic Enablement Caveats
To allow a global flag for enabling and disable diagnostics classes, each class
maintains a class variable, `._enabled_global`.  When `False`, all instances of the class
will be disabled (all methods return immediately).
To simplificy implementation, once instantiated a disabled instance cannot be enabled.
"""

from ..diagnostics.memory_usage import MemoryUsage
from ..diagnostics.time_measurement import ExecTimer
from .gensys_diagnostics import (
    GensysDiagnostics,
    default_log_level,
)

available_diagnostics = [
    ExecTimer, # Time Measurement
    MemoryUsage, # Memory Usage
]

def set_enabled_global_all(state):
    """Set global enabled state for all diagnostics.

    `state=True`  - enable all diagnostics classes
    `state=False` - disable all diagnostics classes
    """
    for d in available_diagnostics:
        if state:
            d.enable_global()
        else:
            d.disable_global()

def enable_all():
    """Enable all diagnostics classes."""
    set_enabled_global_all(True)

def disable_all():
    """Disable all diagnostics classes."""
    set_enabled_global_all(False)

class SystemStats(GensysDiagnostics):
    _enabled_global=True # enabled by default, since it's just a wrapper for other classes
    """Convenience class (context manager) calls a set of diagnostics context managers.  """
    def __init__(self, name, logger=None, log_level=default_log_level):
        super().__init__()
        if self.is_disabled():
            return

        self.name = name
        self.logger = logger
        self.log_level = log_level

        self.MemoryUsage = MemoryUsage(
            name=self.name,
            logger=self.logger,
            log_level=self.log_level,
        )

        self.timer = ExecTimer(
            name=self.name,
            logger=self.logger,
            log_level=self.log_level,
        )

    def __enter__(self):
        if self.is_disabled():
            return

        # start time AFTER analyzing memory: don't time RAM measurements
        self.MemoryUsage.__enter__()
        self.timer.__enter__()

    def __exit__(self, exc_type, exc_value, traceback):
        if self.is_disabled():
            return

        # stop timer BEFORE analyzing memory: don't time RAM measurements
        self.timer.__exit__(exc_type=exc_type, exc_value=exc_value, traceback=traceback)
        self.MemoryUsage.__exit__(exc_type=exc_type, exc_value=exc_value, traceback=traceback)
