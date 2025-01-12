"""time_measurement: facilitate common time measurement code.

Uses only standard packages
"""
import time

from ..nls import _
from .gensys_diagnostics import (
    GensysDiagnostics,
    default_log_level,
)


class ExecTimer(GensysDiagnostics):
    def __init__(self, name, logger=None, log_level=default_log_level):
        super().__init__()
        if self.is_disabled():
            return

        self.name = name
        self.logger=logger
        self.log_level = log_level

        self.end_time = None
        self.duration = None

    def __enter__(self):
        if self.is_disabled():
            return

        self.start()

    def __exit__(self, exc_type, exc_value, traceback):
        if self.is_disabled():
            return

        self.stop()

        if self.logger is not None:
            self.logger.log(self.log_level, self.print_duration())
        else:
            print(self.print_duration())  # noqa: T201

    def start(self):
        if self.is_disabled():
            return

        self.start_time = get_time()

    def stop(self):
        if self.is_disabled():
            return

        self.end_time = get_time()

    def print_duration(self):
        if self.is_disabled():
            return None

        t_wall_seconds = (self.end_time["wall"] - self.start_time["wall"])/1e+09
        t_cpu_seconds = (self.end_time["cpu"] - self.start_time["cpu"])/1e+09

        # CONTEXT: [TIME] <name of time measurement>
        return _("[TIME] {} {:.3f} seconds (WALL), {:.3f} seconds (CPU)").format(
            (self.name + ":").ljust(20),
            t_wall_seconds,
            t_cpu_seconds,
        )

def get_time():
    return {"cpu": time.process_time_ns(), "wall": time.time_ns()}
