"""To verify that the default value for percent_donors is 30.0.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_m01_a(capfd, donorimp_indata_02, donorimp_instatus_05):
    """Records 1 & 4 are recipients; 2 is a donor."""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_05,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
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
        rc_should_be_zero=True,
        msg_list_contains=["Number of donors ..................................:       1      33.33%",
                           "Number of recipients ..............................:       2      66.67%",
                           "        imputed ...................................:       1      33.33%",
                           ]
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_modified # spec needs updating, Banff 2.08.001 message used here
def test_donorimp_m01_b(capfd, donorimp_indata_02, donorimp_instatus_06):
    """Records 1, 4, & 5 are recipients; 2 is a donor."""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_06,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
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
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_contains_exact="WARNING: There are not enough donors in the group processed. No imputation will be done.",
        msg_list_contains=["Number of donors ..................................:       1      25.00%",
                           "Number of recipients ..............................:       3      75.00%",
                           "    not imputed (insufficient number of donors) ...:       3      75.00%",
                           ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()