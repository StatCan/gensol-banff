import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_o01_a(capfd, donorimp_indata_02, donorimp_instatus_07, donorimp_outdonormap_03):
    """To verify that the default value of the eligdon option is ORIGINAL.  
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_07,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        # eligdon="any", # intentionally commented out
        unit_id="ident",
        # must_match="staff", # intentionally commented out
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains_exact="NOTE: eligdon = ORIGINAL (default)",
        msg_list_contains=[
            """Number of valid observations ......................:       2     100.00%
Number of donors ..................................:       1      50.00%
Number of donors to reach DONORLIMIT ..............:       0       0.00%
Number of recipients ..............................:       1      50.00%""",
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but they dont meet criteria related to eligdon."
        ],
        expected_outdonormap=donorimp_outdonormap_03,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()