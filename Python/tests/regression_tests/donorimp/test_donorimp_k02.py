"""The postedits must be separated and terminated by semi-colons.  If not, 
Donor should not proceed.  Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_k02_a(capfd, donorimp_indata_02, donorimp_instatus_02):

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;", 
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0; Q1 >=0,",
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
        expected_error_count=2,
        msg_list_contains_exact="ERROR: Invalid post_edits.",
        msg_list_contains="""ERROR: Edits parser: Looking for 'SemiColon' but found 'Unknown character' instead.
       
       Error at or before:
       Q1 + Q2 + Q3 + Q4 - TOTAL <= 0; Q1 >=0,
                                              ^
"""
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_k02_b(capfd, donorimp_indata_02, donorimp_instatus_02):

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;", 
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0; Q1 >=0:",
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
        expected_error_count=2,
        msg_list_contains_exact="ERROR: Invalid post_edits.",
        msg_list_contains="""ERROR: Edits parser: Looking for 'SemiColon' but found 'Colon' instead.
       
       Error at or before:
       Q1 + Q2 + Q3 + Q4 - TOTAL <= 0; Q1 >=0:
                                              ^
"""
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_k02_c(capfd, donorimp_indata_02, donorimp_instatus_02):

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;", 
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0; Q1 >=0 ",
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
        expected_error_count=2,
        msg_list_contains_exact="ERROR: Invalid post_edits.",
        msg_list_contains="""ERROR: Edits parser: Looking for 'SemiColon' but found 'Done' instead.
       
       Error at or before:
       Q1 + Q2 + Q3 + Q4 - TOTAL <= 0; Q1 >=0
                                             ^
"""
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()