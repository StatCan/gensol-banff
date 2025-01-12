"""N is a positive integer.  If not, DonorImp should not proceed.
Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_n02_a(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `n=0`
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=0,  # invalid value
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
        msg_list_contains_exact="ERROR: n must be an integer greater than or equal to 1."
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_n02_b(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `n=-5`
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=-5,  # invalid value
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
        msg_list_contains_exact="ERROR: n must be an integer greater than or equal to 1."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_157
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_n02_c(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `n=2.2`
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=2.2,  # invalid value
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
        msg_list_contains_exact="""ERROR: parameter 'n' incorrect type, expecting an INTEGER constant, but received
       real: 2.200000"""
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()