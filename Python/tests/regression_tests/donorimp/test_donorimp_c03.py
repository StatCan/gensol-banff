import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_c03_a(capfd, donorimp_indata_03, donorimp_instatus_02, donorimp_outdata_01):
    """The exclusion variable should take the value 'E' if a donor is to be excluded.  
    If the value of the exclusion variable is something other than 'E', the 
    corresponding observation should not be excluded from the pool of donors.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_03,
        instatus=donorimp_instatus_02,
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
        expected_outdata=donorimp_outdata_01
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()