import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
def test_determin_h01_a(capfd, determin_indata_05, determin_instatus_01, determin_outdata_01, determin_outstatus_01):
    """To ensure that the FTI status is recognized and the imputed value is correct. 
    VB should be imputed with the value 113.95.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_05,
        instatus=determin_instatus_01,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        expected_outdata=determin_outdata_01,
        expected_outstatus=determin_outstatus_01
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()