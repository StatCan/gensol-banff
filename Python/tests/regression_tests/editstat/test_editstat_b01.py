import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_b01_a(capfd, editstat_indata_01):
    """The Edits= option is mandatory.  When not specified, Editstats should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_01,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: edits is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()