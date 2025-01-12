import pytest
from banff.testing import pytest_determin

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_determin_h03(capfd, determin_indata_05, determin_instatus_02, determin_outdata_02, determin_outstatus_03):
    #### arrange
    determin_indata_05.loc[1, 'TOT1'] = 102.1
    determin_instatus_02.loc[12, 'STATUS'] = 'FTI'
    determin_instatus_02.loc[1, 'STATUS'] = 'I'


    #### act
    pytest_determin(
        # Outlier Parameters
        indata=determin_indata_05,
        instatus=determin_instatus_02,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        trace=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        expected_warning_count=1,
        rc_should_be_zero=True,
        expected_outdata=determin_outdata_02,
        expected_outstatus=determin_outstatus_03
    )
