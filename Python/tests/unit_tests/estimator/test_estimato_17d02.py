import pytest
from banff.testing import run_standard_assertions, assert_dataset_equal, assert_substr_count
import os
import banff

@pytest.mark.m_in_development
def test_estimato17d02(capfd, estimato_indata_17d, estimato_instatus_17d, estimato_indata_hist_17d, estimato_instatus_hist_17d, estimato_inestimator_17d, estimato_inalgorithm_17d, estimato_outdata_17d):
    #### arrange
    indata=estimato_indata_17d
    instatus = estimato_instatus_17d
    indata_hist=estimato_indata_hist_17d
    instatus_hist=estimato_instatus_hist_17d
    inestimator = estimato_inestimator_17d
    inalgorithm = estimato_inalgorithm_17d

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_17d02.txt"

    expect_zero_rc=True

    
    #### act
    #Create Banff call
    banff_call= banff.estimato(
        indata=indata,
        instatus=instatus,
        indata_hist=indata_hist,
        instatus_hist=instatus_hist,
        inestimator=inestimator,
        inalgorithm=inalgorithm,
        seed=3000,
        unit_id="IDENT",
        by="R1 R2")
    rc = banff_call.rc
    
    #### assert
    out_act, err = capfd.readouterr() # get console log for arrange and act

    run_standard_assertions(expect_zero_rc=expect_zero_rc, rc=rc, python_log=out_act,
                          sas_log_path=sas_log_path, msg_list_sas=msg_list_sas, msg_list_sas_exact=msg_list_sas_exact,
                          msg_list_contains=msg_list_contains, msg_list_contains_exact=msg_list_contains_exact)
    
    assert_dataset_equal(dataset_name="outdata", test_dataset=banff_call.outdata, control_dataset=estimato_outdata_17d)
    
    assert_substr_count(test_log=out_act, expected_count=0, substr_to_count="ERROR:")
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs

@pytest.mark.m_in_development
def test_estimato17d02_by_var(capfd, estimato_indata_17d, estimato_instatus_17d_by_vars, estimato_indata_hist_17d, estimato_instatus_hist_17d_by_vars, estimato_inestimator_17d, estimato_inalgorithm_17d, estimato_outdata_17d):
    #### arrange
    indata=estimato_indata_17d
    instatus = estimato_instatus_17d_by_vars
    indata_hist=estimato_indata_hist_17d
    instatus_hist=estimato_instatus_hist_17d_by_vars
    inestimator = estimato_inestimator_17d
    inalgorithm = estimato_inalgorithm_17d

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_17d02.txt"

    expect_zero_rc=True
    
    #### act
    #Create Banff call
    banff_call= banff.estimato(
        indata=indata,
        instatus=instatus,
        indata_hist=indata_hist,
        instatus_hist=instatus_hist,
        inestimator=inestimator,
        inalgorithm=inalgorithm,
        seed=3000,
        unit_id="IDENT",
        by="R1 R2")
    rc = banff_call.rc
    
    #### assert
    out_act, err = capfd.readouterr() # get console log for arrange and act

    run_standard_assertions(expect_zero_rc=expect_zero_rc, rc=rc, python_log=out_act,
                          sas_log_path=sas_log_path, msg_list_sas=msg_list_sas, msg_list_sas_exact=msg_list_sas_exact,
                          msg_list_contains=msg_list_contains, msg_list_contains_exact=msg_list_contains_exact)
    
    assert_dataset_equal(dataset_name="outdata", test_dataset=banff_call.outdata, control_dataset=estimato_outdata_17d)

    assert_substr_count(test_log=out_act)
    
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs