import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_a07_a(capfd, determin_indata_03, determin_instatus_01):
    """Observations with a missing value for the key variable in the input data set should 
    not be processed.  A warning message should be entered in the log file with a counter 
    for the number of observations dropped.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_03,
        instatus=determin_instatus_01,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=2,
        msg_list_contains_exact=["WARNING: There were 1 observations dropped from indata data set because REC is missing.",
                      "WARNING: There were 1 observations dropped from indata data set because a value is missing and there is no corresponding FTI in instatus data set."]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()