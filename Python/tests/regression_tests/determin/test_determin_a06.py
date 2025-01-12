import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_a06_a(capfd, determin_indata_02, determin_instatus_01):
    """When the key values are not unique in the input dataset, Deterministic should not proceed. 
    Error messages should be printed in the log.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_02,
        instatus=determin_instatus_01,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: indata data set has duplicate keys. The current key is 3 and the previous key was 3."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()