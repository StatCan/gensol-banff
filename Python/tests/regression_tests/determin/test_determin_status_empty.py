import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_empty_a(capfd, determin_indata_empty, determin_instatus_empty):
    """Empty input datasets should NOT lead to errors
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_empty,
        instatus=determin_instatus_empty,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_empty_b(capfd, determin_indata_05, determin_instatus_empty):
    """Empty input status dataset should NOT lead to errors
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_05,
        instatus=determin_instatus_empty,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()