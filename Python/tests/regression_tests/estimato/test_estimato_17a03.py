import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_159
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_17a03_a(capfd, estimato_indata_02, estimato_indata_hist_02, estimato_instatus_02, estimato_instatus_hist_02, estimato_inalgorithm_02, estimato_inestimator_02):
    """Only one key variable is allowed, i.e. composite keys are not allowed.  
    If a composite key is specified, Estimator should not proceed.  
    Error messages should be printed to the log.
    """

    banfftest.pytest_estimato(
        # Procedure Parameters
        indata=estimato_indata_02,
        indata_hist=estimato_indata_hist_02,
        instatus=estimato_instatus_02,
        instatus_hist=estimato_instatus_hist_02,
        inalgorithm=estimato_inalgorithm_02,
        inestimator=estimato_inestimator_02,
        seed=30000,
        unit_id="IDENT V",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable check, the number of errors here matches number of datasets using `unit_id`
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()