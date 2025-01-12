from logging import DEBUG

default_log_level = DEBUG


class GensysDiagnostics:
    """Superclass for Diagnostics classes."""

    _enabled_global = False
    def __init__(self):
        """Record global enabled state in instance `._enabled` attribute."""
        self._enabled = self._enabled_global

    @classmethod
    def disable_global(cls):
        """Globally disable diagnostic class."""
        cls._enabled_global = False

    def is_disabled(self):
        """Is an instance OR its class is disabled.

        - an instance cannot be enabled following instantiation
        - an enabled instance is considered disabled when the class is disabled
        """
        return self._enabled is False or self.__class__._enabled_global is False

    @classmethod
    def enable_global(cls):
        """Globally enable diagnostic class.

        Note that instances created while globally disabled cannot subsequently be enabled.
        """
        cls._enabled_global = True

__all__ = [
    "default_log_level",
]
