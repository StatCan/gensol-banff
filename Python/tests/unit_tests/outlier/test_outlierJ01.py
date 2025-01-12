import pytest
from banff.testing import run_standard_assertions
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierJ01_a(capfd, outlier_indata_01):
    #### arrange
    indata=outlier_indata_01
    indata_hist = None

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierJ01_a.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=False
    expected_error_count    = 1
    expected_warning_count  = 0
    msg_list_sas_exact.append("ERROR: Invalid mei. The value must be greater than 0")
    
    #### act
    #Create Banff call
    try:
        banff_call= banff.outlier(
            indata=indata,
            indata_hist=indata_hist,
            method="C",
            mei=-2,
            accept_negative=True,
            unit_id="ident",
            var="V1 V2"
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


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierJ01_b(capfd, outlier_indata_01):
    #### arrange
    indata=outlier_indata_01
    indata_hist = None

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierJ01_b.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=False
    expected_error_count    = 1
    expected_warning_count  = 0
    msg_list_sas_exact.append("ERROR: Invalid mei. The value must be greater than 0")
    
    #### act
    #Create Banff call
    try:
        banff_call= banff.outlier(
            indata=indata,
            indata_hist=indata_hist,
            method="C",
            mei=0,
            accept_negative=True,
            unit_id="ident",
            var="V1 V2"
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


@pytest.mark.m_validated
@pytest.mark.m_for_review # tests SAS validation; new message for review
def test_outlierJ01_c(capfd, outlier_indata_01):
    #### arrange
    indata=outlier_indata_01
    indata_hist = None

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierJ01_c.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=False
    expected_error_count    = 1
    expected_warning_count  = 0
    msg_list_sas_exact.append("Expecting a numeric constant")
    
    #### act
    #Create Banff call
    banff_call= banff.outlier(
        indata=indata,
        indata_hist=indata_hist,
        method="C",
        mei="two",
        accept_negative=True,
        unit_id="ident",
        var="V1 V2"
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