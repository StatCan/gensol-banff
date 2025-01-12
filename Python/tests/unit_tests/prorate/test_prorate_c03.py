import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_c03(capfd, prorate_indata_03, prorate_instatus_02__bad_sort):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02__bad_sort,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        sas_log_name="prorate_c03.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: instatus data set is not sorted in ascending order. The current key is V001 and the previous key was V003."
    )
