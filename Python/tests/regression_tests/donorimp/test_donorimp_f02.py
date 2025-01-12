import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_f02_a(capfd, donorimp_indata_02, donorimp_instatus_03, donorimp_outdata_02, donorimp_outdonormap_02):
    """The input status data set need not be sorted in ascending order by 
    the key variable.  If it is not sorted, Donor should proceed.  
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_03,
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
        expected_outdata=donorimp_outdata_02,
        expected_outdonormap=donorimp_outdonormap_02
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()