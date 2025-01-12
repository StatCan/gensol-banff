import pytest
from banff.testing import run_standard_assertions
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
#@pytest.mark.m_sasexit
def test_estimato12b01(capfd, estimato_indata_12b, estimato_instatus_12b, estimato_inestimator_12b, estimato_inalgorithm_12b,):
    #### arrange
    indata=estimato_indata_12b
    instatus = estimato_instatus_12b
    indata_hist=None
    instatus_hist=None
    inestimator = estimato_inestimator_12b
    inalgorithm = estimato_inalgorithm_12b

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_12b01.txt"

    expect_zero_rc=False
    msg_list_sas.append("ERROR: Estimator: Estimator 0: key 1A has a status for the current WEIGHTVARIABLE.")

    
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
            seed=30000,
            unit_id="ID",
        data_excl_var="EXC")
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