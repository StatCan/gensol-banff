import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.j_ITSLMBANFF_86 # causes 2 extra warnings vs SAS
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato17d03_a(capfd, estimato_indata_03, estimato_instatus_03, estimato_indata_hist_03, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    #### arrange

    #### act
    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_03,
        instatus=estimato_instatus_03,
        indata_hist=estimato_indata_hist_03,
        instatus_hist=estimato_instatus_hist_03,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=2,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()