import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_159
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_errorloc_a03_a(capfd, errorloc_indata_02):
    """Only one key variable is allowed, i.e. no composite keys are permitted.  
    If a composite key is specified, Errorloc should not proceed.  
    Error messages should be printed to the log file.
    """

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=errorloc_indata_02,
        edits="x1+x2<=50; x3>=100;",
        seed=1,
        unit_id="REC AREA",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable"
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()