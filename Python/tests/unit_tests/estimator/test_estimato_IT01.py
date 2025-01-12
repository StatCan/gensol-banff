"""
These tests validate the "exit_to_return" changes made to the estimator procedure after its initial conversion.  
Prior to that change, the procedure would exit from the C runtime upon error.  
Now, the procedure should return an error code up the call stack and ultimately
return a non-zero return code.  
"""
import pytest
from banff.testing import run_standard_assertions
import banff

@pytest.mark.m_validated
@pytest.mark.m_for_review # new JSON validation preceeds existing C code validation
#@pytest.mark.m_sasexit #tests an instance where `SAS_XEXIT()` was called in the SAS based version
def test_estimato_IT01a(capfd, estimato_indata_03p, estimato_instatus_03p, estimato_indata_hist_03p, estimato_instatus_hist_03p, estimato_inestimator_03p, estimato_inalgorithm_03p,):
    #### arrange
    indata=estimato_indata_03p
    instatus = estimato_instatus_03p
    indata_hist=estimato_indata_hist_03p
    instatus_hist=estimato_instatus_hist_03p
    inestimator = estimato_inestimator_03p
    inalgorithm = estimato_inalgorithm_03p

    # insert numeric data in character data column
    inalgorithm.at[0, 'description'] = 1

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path            = None

    expect_zero_rc=False
    msg_list_contains_exact.append("ERROR: Variable DESCRIPTION is not character in ALGORITHM data set.")

    #### act
    #Create Banff call
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

@pytest.mark.m_validated
@pytest.mark.m_for_review # Mixed datatype columns now causes an exception during packing: PyArrow doesn't know what to do
#@pytest.mark.m_sasexit #tests an instance where `SAS_XEXIT()` was called in the SAS based version
def test_estimato_IT01b(capfd, estimato_indata_03p, estimato_instatus_03p, estimato_indata_hist_03p, estimato_instatus_hist_03p, estimato_inestimator_03p, estimato_inalgorithm_03p,):
    #### arrange
    indata=estimato_indata_03p
    instatus = estimato_instatus_03p
    indata_hist=estimato_indata_hist_03p
    instatus_hist=estimato_instatus_hist_03p
    inestimator = estimato_inestimator_03p
    inalgorithm = estimato_inalgorithm_03p

    # insert character data in numeric data column
    indata.at[0, 'V'] = "."

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path            = None

    expect_zero_rc=False
    msg_list_contains_exact.append("ERROR: Variable V is not numeric in indata data set.")

    #### act
    #Create Banff call
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