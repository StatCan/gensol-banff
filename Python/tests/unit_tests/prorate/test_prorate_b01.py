import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_b01(capfd, prorate_indata_02, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_02,
        instatus=prorate_instatus_02,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="O",
        unit_id="ident",
        presort=False,

        # Unit Test Parameters
        sas_log_name="prorate_b01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1, # original test case only produced 1 error, now proc flags indata and instatus missing "idnt"
        msg_list_sas_exact="data set is not sorted in ascending order"
    )
