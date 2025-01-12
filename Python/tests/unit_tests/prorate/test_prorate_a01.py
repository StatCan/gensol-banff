import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_a01(capfd, prorate_indata_01, prorate_instatus_01):
    #### arrange

    #### act
    pytest_prorate(
        # Outlier Parameters
        indata=prorate_indata_01,
        instatus=prorate_instatus_01,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",

        # Unit Test Parameters
        sas_log_name="prorate_a01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: unit_id is mandatory."
    )
