from enum import Enum


class ReturnCode(Enum):
    """Numeric return codes associated with Python specific errors.

    0 and any positive numbers must correspond to a
    C code return code
    Negative numbers are exclusive to Python.
    """

    SUCCESS = 0
    GENERIC_FAILURE = -1
