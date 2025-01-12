import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_o04(capfd, prorate_indata_07, prorate_instatus_05,
                     prorate_outdata_02, prorate_outstatus_02, prorate_outreject_03):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_07,
        instatus=prorate_instatus_05,
        edits="VA + VB+ VC + VD = TOT1; TOT1+TOT2=GT;",
        decimal=0,
        lower_bound=-100,
        upper_bound=999,
        modifier="I",
        method="B",
        accept_negative=True,
        unit_id="REC",
        by="AREA",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outdata=prorate_outdata_02,
        expected_outstatus=prorate_outstatus_02,
        expected_outreject=prorate_outreject_03
    )

