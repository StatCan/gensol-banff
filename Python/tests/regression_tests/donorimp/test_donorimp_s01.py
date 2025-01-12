import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_s01_a(capfd, donorimp_indata_10, donorimp_instatus_08, donorimp_outdonormap_03):
    """To verify that the default option is accept_negative=False.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_10,
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
        expected_warning_count=1,
        expected_outdonormap=donorimp_outdonormap_03,
        msg_list_contains="""WARNING: There were 1 observations dropped from indata data set because one/more edits 
         variables or one/more must_match variables outside the edits group are negative 
         in indata data set and there is no corresponding FTI in instatus data set."""
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()