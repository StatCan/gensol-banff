import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_h02_a(capfd, prorate_indata_04, prorate_instatus_02, prorate_outreject_01):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_04,
        instatus=prorate_instatus_02,
        edits="VA + VB + VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outreject=prorate_outreject_01
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_h02_b(capfd, prorate_indata_04, prorate_instatus_02, prorate_outreject_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_04,
        instatus=prorate_instatus_02,
        edits="VA + VB + VC + VD = TOT1;",
        decimal=1,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outreject=prorate_outreject_02
    )
