import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_a03_a(capfd, determin_indata_01, determin_instatus_01):
    """If the ID statement refers to a nonexistent variable, Deterministic should not proceed.
    Error messages should be printed to the log file.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_01,
        instatus=determin_instatus_01,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="RC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact=[
            "ERROR: Variable 'RC' in list 'unit_id' (indata dataset) not found.",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()