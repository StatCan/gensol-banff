import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_modified # error message in .docx doesn't match Banff 2.08 message
def test_donorimp_e03_a(capfd, donorimp_instatus_02):
    """The input data set and the input status data set must be different.  If not, 
    DonorImp should not proceed.  Error messages should be printed to the log file.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_instatus_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable TOTAL specified in edits is not in indata data set."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()