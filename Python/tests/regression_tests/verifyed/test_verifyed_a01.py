import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_a01_a(capfd):
    """Edits are mandatory.  When not specified, Verifyedits should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_verifyed(
        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: edits is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()