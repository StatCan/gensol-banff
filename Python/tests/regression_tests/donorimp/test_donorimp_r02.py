"""If a negative value or 0 is specified for the seed parameter, 
it should be replaced by the default value (random number).
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_r02_a(capfd, donorimp_indata_08, donorimp_instatus_02):
    """invalid `seed=0`
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_08,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        seed=0,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_contains="""(value chosen by the system) 
WARNING: The seed value specified is 0 and cannot be used. It has been replaced with a generated number.""",
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_r02_b(capfd, donorimp_indata_08, donorimp_instatus_02):
    """invalid `seed=-5`
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_08,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        seed=-5,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_contains="""(value chosen by the system) 
WARNING: The seed value specified is negative and cannot be used. It has been replaced with a generated number.""",
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()