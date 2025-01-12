import sys

import pytest


def run_pytest():
    """Invoke pytest from within Python code.

    In files which implement pytest tests, add the following code
    ```python
    import sys
    if __name__ == "__main__":
        run_pytest()
    ```
    Execute the file using `python <filename>` and this function will launch pytest
    with preset options.
    """
    pytest_options = []
    pytest_options.append("--cache-clear")
    # --verbose     adds line-per-test "PASSED" or "FAILED"
    pytest_options.append("--verbose")
    # -rA           ensures all captured output is printed and prints line-per-test status
    pytest_options.append("-rA")
    # --tb=short    reduces traceback to essentially show the call stack
    #               unfortunately --tb=no suppresses console output from failed tests
    pytest_options.append("--tb=short")

    sys_args=sys.argv
    print("{}: Attempting to invoke Pytest on file {}".format(__package__, sys_args))
    print("  using the following options")
    print(f"  {pytest_options}")
    rc = pytest.main(sys_args + pytest_options)
    print("pytest returned {}".format(rc))
