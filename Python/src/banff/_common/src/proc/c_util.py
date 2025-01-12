"""C Utilities.

Related to integration with C code
"""

import locale


class c_runtime_locale:
    """Set C's numeric locale setting to ensure correct processing of user inputs.

    Some C standard library function's behaviour depends the locale.
    For instance the category LC_NUMERIC determines the decimal separator (`.` for EN, `,` for FR).

    This context manager sets the LC_NUMERIC locale category to 'C', a seemingly always-available locale
    which produces expected behaviour.
    """

    def __init__(self):
        pass

    def __enter__(self):
        self.original_numeric_locale=locale.setlocale(locale.LC_NUMERIC)
        locale.setlocale(locale.LC_NUMERIC, "C")

    def __exit__(self, exc_type, exc_value, traceback):
        locale.setlocale(locale.LC_NUMERIC, self.original_numeric_locale)
