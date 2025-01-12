import pytest
from banff.testing import run_standard_assertions
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_missing_feature # AUX|HIST alias not implemented, HIST only supported name
def test_outlierB02_a(capfd, outlier_indata_01, outlier_inhist_01):
    #### arrange
    indata=outlier_indata_01
    indata_hist = outlier_inhist_01

    indata_hist.rename(columns={"V2":"V3"}, inplace=True)

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierB02_a.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=False
    msg_list_sas_exact.append("ERROR: Variable V2 is not in AUX data set")
    expected_error_count = 1
    
    #### act
    #Create Banff call
    banff_call= banff.outlier(
        indata=indata,
        indata_hist=indata_hist,
        unit_id="ident",
        method="R",
        mii=3,
        mei=2,
        var="V1 V2"
    )
    rc = banff_call.rc
    
    #### assert
    out_act, err = capfd.readouterr() # get console log for arrange and act

    run_standard_assertions(expect_zero_rc=expect_zero_rc, rc=rc, python_log=out_act,
                          sas_log_path=sas_log_path, msg_list_sas=msg_list_sas, msg_list_sas_exact=msg_list_sas_exact,
                          msg_list_contains=msg_list_contains, msg_list_contains_exact=msg_list_contains_exact,
                          expect_error_count=expected_error_count, expect_warning_count=expected_warning_count)
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs
