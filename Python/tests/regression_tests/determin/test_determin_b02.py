import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_b02_a(capfd, determin_indata_04, determin_instatus_02):
    """If the NOTSORTED option is not used with the BY statement, the current data set must 
    be sorted by the BY variable(s) first.  If not, Deterministic should not proceed.  
    Error messages should be printed to the log file.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_04,
        instatus=determin_instatus_02,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        prefill_by_vars=False,
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has AREA = 2.000000 and the next by group has AREA = 1.000000."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()