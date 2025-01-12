"""To verify that the default value for MINDONORS is 30.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_l01_a(capfd, donorimp_indata_08, donorimp_instatus_02):
    """with `data_excl_var`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_08,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        # min_donors=1, # intentionally commented out
        # percent_donors=1, # intentionally commented out
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_contains_exact="WARNING: There are not enough donors in the group processed. No imputation will be done.",
        msg_list_contains=["Number of donors ..................................:      29      96.67%",
                           "Number of recipients ..............................:       1       3.33%",
                           "    not imputed (insufficient number of donors) ...:       1       3.33%"]
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_l01_b(capfd, donorimp_indata_08, donorimp_instatus_02):
    """without `data_excl_var`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_08,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        # min_donors=1, # intentionally commented out
        # percent_donors=1, # intentionally commented out
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff"
        # data_excl_var="toexcl" # intentionally commented out
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains=["Number of donors ..................................:      30      96.77%",
                           "Number of recipients ..............................:       1       3.23%",
                           "    with matching fields ..........................:       1       3.23%",
                           "        imputed ...................................:       1       3.23%"]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()