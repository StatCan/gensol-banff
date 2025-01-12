"""IT test of parameter type validation
If a noninteger value is specified for the seed parameter, 
error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_parm_type_a(capfd, donorimp_indata_08, donorimp_instatus_02):
    """invalid `seed=2.2`
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
        seed=2.2,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="ERROR: parameter 'seed' incorrect type, expecting an INTEGER constant, but received real: 2.200000",
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_parm_type_b(capfd, donorimp_indata_08, donorimp_instatus_02):
    """VALID `seed=2.0`
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
        seed=2.0,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()