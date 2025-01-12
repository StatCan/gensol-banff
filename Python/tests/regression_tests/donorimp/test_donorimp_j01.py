import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_j01_a(capfd, donorimp_indata_02, donorimp_instatus_02):
    """The edits are mandatory.  If not present, Donor should not proceed.
    Error messages should be printed to the log file.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        #edits="Q1 + Q2 + Q3 + Q4 = total;", # intentionally commented out
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: edits is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()