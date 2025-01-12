import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_g01(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_g01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: edits is mandatory."
    )
