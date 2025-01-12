import pytest
from banff.testing import run_standard_assertions, assert_dataset_equal
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato04a01(capfd, estimato_indata_04a, estimato_instatus_04a, estimato_inestimator_04a, estimato_inalgorithm_04a,
                       estimato_outacceptable_04a, estimato_outest_ef_04a, estimato_outest_parm_04a):
    #### arrange
    indata=estimato_indata_04a
    instatus = estimato_instatus_04a
    inestimator = estimato_inestimator_04a
    inalgorithm = estimato_inalgorithm_04a

    # arrange assertions
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    sas_log_path = os.path.dirname(__file__) + "/output-sas/estimato_04a01.txt"

    expect_zero_rc=True
    msg_list_sas.append("""
    Statistics on imputation:
    ------------------------
    Est Variable                            FTI    Imp    */0    Neg
    0   Z                                     0      0      0      0
    1   Y                                     0      0      0      0
    2   W                                     0      0      0      0
    """)

    
    #### act
    #Create Banff call
    banff_call= banff.estimato(
        indata=indata,
        instatus=instatus,
        inestimator=inestimator,
        inalgorithm=inalgorithm,
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
    
    assert_dataset_equal(dataset_name="outacceptable", test_dataset=banff_call.outacceptable, control_dataset=estimato_outacceptable_04a)
    assert_dataset_equal(dataset_name="outest_ef", test_dataset=banff_call.outest_ef, control_dataset=estimato_outest_ef_04a)
    assert_dataset_equal(dataset_name="outest_parm", test_dataset=banff_call.outest_parm, control_dataset=estimato_outest_parm_04a)
    
    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs