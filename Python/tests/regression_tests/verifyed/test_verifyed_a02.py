import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_a02_a(capfd):
    """When the edit set is empty, Verifyedits should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_verifyed(
        edits=" ",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=2,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'Coefficient' or 'Variable' but found 'Done' instead.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()