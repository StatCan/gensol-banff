"""MINDONORS must be a positive integer.  If not, DonorImp should not continue.  
Error messages should be written to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_l02_a(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `min_donors=0`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=0, # intentionally set to `0`
        #percent_donors=1, # intentionally commented out
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
        msg_list_contains_exact="ERROR: min_donors must be an integer greater than or equal to 1."
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_l02_b(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `min_donors=-4`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=-4, # intentionally set to `-4`
        #percent_donors=1, # intentionally commented out
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
        msg_list_contains_exact="ERROR: min_donors must be an integer greater than or equal to 1."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_157
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_l02_c(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `min_donors=2.2`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=2.2, # intentionally set to `2.2`
        #percent_donors=1, # intentionally commented out
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
        msg_list_contains_exact="""ERROR: parameter 'min_donors' incorrect type, expecting an INTEGER constant, but received
       real: 2.200000"""
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()