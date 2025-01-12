import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_o02_a(capfd, donorimp_indata_02, donorimp_instatus_08, donorimp_outdonormap_02):
    """To verify that no observations (other than those excluded through the 
    exclusion variable) are excluded from the donor pool when eligdon=ANY.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_08,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
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
        rc_should_be_zero=True,
        msg_list_contains_exact="NOTE: eligdon = ANY",
        msg_list_contains=[
            """Number of valid observations ......................:       3     100.00%
Number of donors ..................................:       2      66.67%
Number of donors to reach DONORLIMIT ..............:       0       0.00%
Number of recipients ..............................:       1      33.33%"""
        ],
        expected_outdonormap=donorimp_outdonormap_02,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()