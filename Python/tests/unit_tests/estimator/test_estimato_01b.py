import pytest
from banff.testing import run_standard_assertions, assert_dataset_equal
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01b01(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01b, estimato_outdata_01b, estimato_outstatus_01b):
    #### arrange
    indata=estimato_indata_01a
    instatus = estimato_instatus_01a
    inestimator = estimato_inestimator_01b

    indata.at[2, 'Z'] = None # set value to missing
    instatus.drop([0,1], inplace=True) # drop first 2 rows

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_01b01.txt"

    expect_zero_rc=True
    msg_list_sas.append("""
        Estimator 0.
        Count = 2 Percent = 66.67 not weighted.
        Average    Period              Variable
        1.50000    CURR                Z
    """)
    
    #### act
    #Create Banff call
    banff_call= banff.estimato(
        indata=indata,
        instatus=instatus,
        inestimator=inestimator,
        unit_id="ID",
        seed=30000
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
    
    assert_dataset_equal(dataset_name="outdata", test_dataset=banff_call.outdata, control_dataset=estimato_outdata_01b)
    assert_dataset_equal(dataset_name="outstatus", test_dataset=banff_call.outstatus, control_dataset=estimato_outstatus_01b)
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs