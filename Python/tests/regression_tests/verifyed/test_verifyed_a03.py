import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_a03_a(capfd):
    """When the edits are not linear, Verifyedits should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_verifyed(
        edits="VA + VB + VC + VD = TOT1; VA/VB<=10000000; TOT1 + TOT2 = GT;",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=2,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'Operator' but found 'Unknown character' instead.",
        ]
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_a03_b(capfd):
    """When the edits are not linear, Verifyedits should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_verifyed(
        edits="VA + VB + VC + VD = TOT1; VA*VB<=10000000; TOT1 + TOT2 = GT;",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=2,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'Coefficient' but found 'Variable' instead.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()