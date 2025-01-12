import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_m01(capfd, prorate_indata_05, prorate_instatus_03, prorate_outdata_01, prorate_outstatus_01):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_05,
        instatus=prorate_instatus_03,
        edits="""0.5VA + 1.5VB + 3VC + VD = TOT1; 
      .5VE + VF + 10VG + 0.25VH = TOT2; 
      TOT1 + .9TOT2 = GT;""",
        decimal=0,
        lower_bound=-999,
        upper_bound=999,
        modifier="A",
        unit_id="ident",
        accept_negative=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outdata=prorate_outdata_01,
        expected_outstatus=prorate_outstatus_01
    )