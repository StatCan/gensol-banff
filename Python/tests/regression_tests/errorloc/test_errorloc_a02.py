import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_errorloc_a02_a(capfd, errorloc_indata_05):
    """The key variable must be a character variable.  
    When it is numeric, error messages should be printed in the log.
    """
    banfftest.pytest_errorloc(
        # Banff parameters
        indata=errorloc_indata_05,
        edits="x1+x2<=50; x3>=100;",
        seed=1,
        unit_id="REC"
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'REC' in list 'unit_id' (indata dataset) must be character, but it is numeric."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()