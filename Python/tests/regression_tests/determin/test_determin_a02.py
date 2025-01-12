import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_a02_a(capfd, determin_indata_01, determin_instatus_01):
    """The key variable must be character variable.  When it is numeric in the 
    input data file, error messages should be printed in the log.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_01,
        instatus=determin_instatus_01,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="rec", # REVIEW: SAS used upper/lower case as used in dataset, not parameter (i.e. 'REC' not 'rec')

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'rec' in list 'unit_id' (indata dataset) must be character, but it is numeric."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()