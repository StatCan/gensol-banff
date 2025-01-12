import pytest
import banff.testing as banfftest

@pytest.mark.m_auto_pass
def test_errorloc_it01_a(capfd, errorloc_indata_07, errorloc_outstatus_01):
    """Outstatus' VALUE column should contain the original value from indata
    """

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=errorloc_indata_07,
        edits="X1+X2=TOT;",
        cardinality = 1,
        seed = 1,
        unit_id = "REC",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=errorloc_outstatus_01
    )

@pytest.mark.m_auto_pass
def test_errorloc_it01_b(capfd, errorloc_indata_07, errorloc_outstatus_02):
    """Outstatus' VALUE column should contain the original value from indata
    """

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=errorloc_indata_07,
        edits="X1+X2=TOT;",
        cardinality = 1,
        seed = 13,
        unit_id = "REC",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=errorloc_outstatus_02
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()