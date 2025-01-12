import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_k06_a(capfd, donorimp_indata_07, donorimp_instatus_02, donorimp_outdonormap_02):
    """To verify that if all post-imputation edits are positivity edits 
    then they are processed as usual.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_07,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        post_edits="Q1 >=0; Q2>=0; Q3>=0; Q4>=0; total>=0;",
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
        expected_outdonormap=donorimp_outdonormap_02
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()