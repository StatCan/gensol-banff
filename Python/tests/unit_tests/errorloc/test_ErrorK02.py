import pytest
from banff.testing import pytest_errorloc

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_errorK02(capfd, errorloc_indata_k02, errorloc_expected_outstatus_01):
    #### arrange
    errorloc_indata_k02=errorloc_indata_k02
    errorloc_indata_k02[['IDENT']]=errorloc_indata_k02[['IDENT']].fillna('')
    
    pytest_errorloc(
        # Banff Parameters
        indata=errorloc_indata_k02,
        edits="x1>=5; x1<=10; x2>=-1; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=135; x1+x2+x3<=210;",
        accept_negative=False,
        seed=1,
        unit_id="IDENT"
        ,

        # Unit Test Parameters
        sas_log_name="errorK02.log",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=[
            "Number of observations .....................................:       6     100.00%",
            "Number of observations dropped because the key is missing ..:       2      33.33%"
        ],
        expected_outstatus=errorloc_expected_outstatus_01
    )
