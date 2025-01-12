import banff.exceptions
import pytest
from banff.testing import run_standard_assertions
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
#@pytest.mark.m_sasexit
def test_estimato03p01(capfd, estimato_indata_03p, estimato_instatus_03p, estimato_indata_hist_03p, estimato_instatus_hist_03p, estimato_inestimator_03p, estimato_inalgorithm_03p,):
    #### arrange
    indata=estimato_indata_03p
    instatus = estimato_instatus_03p
    indata_hist=estimato_indata_hist_03p
    instatus_hist=estimato_instatus_hist_03p
    inestimator = estimato_inestimator_03p
    inalgorithm = estimato_inalgorithm_03p

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_03p01.txt"

    expect_zero_rc=False
    msg_list_sas.append("ERROR: Estimator: Estimator 0: key A1 has a 0.0 value for the historical VARIANCEVARIABLE.")
    msg_list_sas.append("ERROR: Estimator: Estimator 1: key A1 has a negative historical VARIANCEVARIABLE.")
    msg_list_sas.append("ERROR: Estimator: Estimator 2: key A1 has a status for the historical VARIANCEVARIABLE.")
    msg_list_sas.append("ERROR: Estimator: Estimator 3: key A1 has a status for the current VARIANCEVARIABLE.")

    
    #### act
    #Create Banff call
    try:
        banff_call= banff.estimato(
            indata=indata,
            instatus=instatus,
            indata_hist=indata_hist,
            instatus_hist=instatus_hist,
            inestimator=inestimator,
            inalgorithm=inalgorithm,
            unit_id="IDENT",
            accept_negative=True
        )
    except banff.exceptions.ProcedureCError as e:
        rc = e.return_code
    else:
        rc = banff_call.rc
    
    #### assert
    out_act, err = capfd.readouterr() # get console log for arrange and act
    msg_list_contains +=msg_list_sas
    msg_list_sas=[]
    msg_list_contains_exact += msg_list_sas_exact
    msg_list_sas_exact=[]
    sas_log_path=None
    run_standard_assertions(expect_zero_rc=expect_zero_rc, rc=rc, python_log=out_act,
                          sas_log_path=sas_log_path, msg_list_sas=msg_list_sas, msg_list_sas_exact=msg_list_sas_exact,
                          msg_list_contains=msg_list_contains, msg_list_contains_exact=msg_list_contains_exact,
                          pytest_capture=capfd)
    
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs