"""memory_usage: for measurement of memory usage.

Uses non-standard `psutil` package, self-disables if not available.
"""
from ..nls import _
from .gensys_diagnostics import (
    GensysDiagnostics,
    default_log_level,
)

# `psutil` is NOT standard, so it's implemented as optional
try:
    import psutil
    _dependency_missing=False
except ImportError:
    _dependency_missing=True

class MemoryUsage(GensysDiagnostics):
    """Measure changes in memory usage, optionally as a context manager."""

    def __init__(self, name, logger=None, log_level=default_log_level):
        super().__init__()
        self._enabled = self._enabled_global
        if self.is_disabled():
            return

        self.name = name
        self.logger=logger
        self.log_level = log_level

        if _dependency_missing:
            return

        self.process = psutil.Process()
        self.used_start = None
        self.used_end = None

    def __enter__(self):
        if self.is_disabled():
            return

        self.start()

    def __exit__(self, exc_type, exc_value, traceback):
        if self.is_disabled():
            return

        self.stop()

        if self.logger is not None:
            self.logger.log(self.log_level, self.print_usage())
        else:
            print(self.print_usage())  # noqa: T201

    def start(self):
        if self.is_disabled():
            return

        if _dependency_missing:
            return

        self.used_start = self.process.memory_info().vms

    def stop(self):
        if self.is_disabled():
            return

        if _dependency_missing:
            return

        self.used_end = self.process.memory_info().vms

    def print_usage(self):
        if self.is_disabled():
            return None

        if _dependency_missing:
            return _("[MEMORY] disabled (dependency missing, run `pip install psutil`)")

        u_start_kb = self.used_start/1000
        u_start_gb = self.used_start/1.0e9
        u_end_kb = self.used_end/1000
        u_end_gb = self.used_end/1.0e9

        # CONTEXT: [MEMORY] <name of memory measurement>
        return _("[MEMORY] {} change {:.0f} kB ({:.2f} GB), before {:.0f} kB ({:.2f} GB), after {:.0f} kB ({:.2f} GB)").format(
            (self.name + ":").ljust(18),
            u_end_kb-u_start_kb, u_end_gb - u_start_gb,
            u_start_kb, u_start_gb,
            u_end_kb, u_end_gb,
        )
