import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_errorloc_b02_a(capfd, errorloc_indata_06):
    """If the NOTSORTED option is not used with the BY statement, the current data set 
    must be sorted by the BY variable(s) first.  If not, Errorloc should not proceed.  
    Error messages should be printed to the log file
    """
    #Create Banff call
    banfftest.pytest_errorloc(
        indata=errorloc_indata_06,
        outstatus=None,
        outreject=None,
        edits="x1+x2<=50; x3>=100;",
        seed=1,
        unit_id="REC",
        by="A2",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has A2 = 2 and the next by group has A2 = 1."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()