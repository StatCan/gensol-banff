import pytest
import banff.testing as banfftest
from banff.exceptions import ProcedureValidationError

@pytest.mark.m_auto_pass
def test_errorloc_d02_a(capfd, errorloc_indata_01):
    """When the specified names for the OUTSTATUS and OUTREJECT files are the same, 
    Errorloc should not proceed.  Error messages should be printed in the log.
    """
    try:
        banfftest.pytest_errorloc(
            # Procedure Parameters
            indata=errorloc_indata_01,
            outstatus="out.parquet",
            outreject="out.parquet",
            edits="x1+x2<=50; x3>=100;",
            seed=1,

            # Test Parameters
            pytest_capture=capfd,
        )
    except ProcedureValidationError as e:
        print(e)
        return
    else:
        assert False, "exception should occur"

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()