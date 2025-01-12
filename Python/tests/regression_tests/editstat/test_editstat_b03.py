"""When the edits are not linear, Editstats should not proceed.
Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_b03_a(capfd, editstat_indata_01):
    """'x2*x3' apparently invalid'
    """
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_01,
        edits="x1+1>=x2; x1<=5; x2*x3>=1; x1+x2+x3<=9;",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=2,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'Coefficient' but found 'Variable' instead.",
        ]
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_b03_b(capfd, editstat_indata_01):
    """'x2/x3' apparently invalid'
    """
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_01,
        edits="x1+1>=x2; x1<=5; x2/x3>=1; x1+x2+x3<=9;",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=2,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'Operator' but found 'Unknown character' instead.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()