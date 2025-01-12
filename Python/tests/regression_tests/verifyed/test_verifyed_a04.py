import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_a04_a(capfd):
    """When numbers are in comma format, Verifyedits should not proceed.
    Errors should be printed to the log file
    """
    banfftest.pytest_verifyed(
        edits="""
VA>=0.001; VB<=1,000,000; VC>=0.0001; VE>0.0001; VG<100,000;
	VA+VB+VC+VD=TOT1; VE+VF+VG=TOT2; TOT1+TOT2=GT;""",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=3,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Looking for 'SemiColon' but found 'Unknown character' instead.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()