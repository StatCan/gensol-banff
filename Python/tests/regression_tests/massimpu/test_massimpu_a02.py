import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_massimpu_a02_a(capfd, massimpu_indata_01):
    """The key variable is mandatory.  If not specified, MassImp should not proceed. 
    Error messages should be printed to the log file.
    """

    banfftest.pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_01,
        min_donors=2,
        percent_donors=10,
        # unit_id="ident", # intentionally commented out
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: unit_id is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()