import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_errorloc_a01_a(capfd, errorloc_indata_01):
    """The key variable is mandatory.  If not specified, Errorloc should not proceed.
    Error messages should be printed to the log file.
    """

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=errorloc_indata_01,
        edits="x1+x2<=50; x3>=100;",
        seed=1,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: unit_id is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()