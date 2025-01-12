import pytest
from banff.testing import run_standard_assertions, assert_dataset_equal
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_precision # SAS&Python dataset only equal when rounded to 9 decimal places
def test_outlierC03_a(capfd, outlier_indata_01, outlier_inhist_01, outlier_expected_outstatus_c03, outlier_expected_outstatus_detailed_c03):
    #### arrange
    indata=outlier_indata_01
    indata_hist = outlier_inhist_01
    expected_outstatus = outlier_expected_outstatus_c03

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierC03_a.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=True
    expected_error_count    = 0
    expected_warning_count  = 0
    
    #### act
    #Create Banff call
    banff_call= banff.outlier(
        indata=indata,
        indata_hist=indata_hist,
        method="H",
        mii=3,
        mei=2,
        outlier_stats=True,
        unit_id="ident",
        var="V1 V2",
        with_var="V1 V2",
        trace=True
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
    
    assert_dataset_equal(test_dataset=banff_call.outstatus, control_dataset=expected_outstatus, dataset_name="OUTSTATUS", round_data=9)
    assert_dataset_equal(test_dataset=banff_call.outstatus_detailed, control_dataset=outlier_expected_outstatus_detailed_c03, dataset_name="OUTSTATUS_DETAILED", round_data=9)

    out_assert, err2 = capfd.readouterr() # get console log for assert
    print(out_act+out_assert) # print all captured logs


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_precision # SAS&Python dataset only equal when rounded to 9 decimal places
def test_outlierC03_b(capfd, outlier_indata_01, outlier_inhist_01, outlier_expected_outstatus_c03, outlier_expected_outstatus_detailed_c03_b):
    #### arrange
    indata=outlier_indata_01
    indata_hist = outlier_inhist_01
    expected_outstatus = outlier_expected_outstatus_c03

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierC03_b.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=True
    expected_error_count    = 0
    expected_warning_count  = 0
    
    #### act
    #Create Banff call
    banff_call= banff.outlier(
        indata=indata,
        #indata_hist=indata_hist,
        method="H",
        mii=3,
        mei=2,
        outlier_stats=True,
        unit_id="ident",
        var="V1 V2",
        with_var="V2 V1"
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
    try:
        assert_dataset_equal(test_dataset=banff_call.outstatus, control_dataset=expected_outstatus, dataset_name="OUTSTATUS", round_data=9)
        assert_dataset_equal(test_dataset=banff_call.outstatus_detailed, control_dataset=outlier_expected_outstatus_detailed_c03_b, dataset_name="OUTSTATUS_DETAILED", round_data=9)
    finally:
        out_assert, err2 = capfd.readouterr() # get console log for assert
        print(out_act+out_assert) # print all captured logs



@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_precision # SAS&Python dataset only equal when rounded to 9 decimal places
def test_outlierC03_c(capfd, outlier_indata_01, outlier_inhist_01, outlier_expected_outstatus_c03_c, outlier_expected_outstatus_detailed_c03_c):
    #### arrange
    indata=outlier_indata_01
    indata_hist = outlier_inhist_01
    expected_outstatus = outlier_expected_outstatus_c03_c

    # arrange assertions
    sas_log_path = os.path.dirname(__file__) + "/output-sas/outlierC03_c.txt"
    msg_list_sas            = []
    msg_list_sas_exact      = []
    msg_list_contains       = []
    msg_list_contains_exact = []
    expected_error_count    = None
    expected_warning_count  = None

    expect_zero_rc=True
    expected_error_count    = 0
    expected_warning_count  = 0
    
    #### act
    #Create Banff call
    banff_call= banff.outlier(
        indata=indata,
        indata_hist=indata_hist,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        outlier_stats=True,
        unit_id="ident",
        var="V1 V2",
        with_var="V1 V2"
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
    
    try:
        assert_dataset_equal(test_dataset=banff_call.outstatus, control_dataset=expected_outstatus, dataset_name="OUTSTATUS", round_data=9)
        assert_dataset_equal(test_dataset=banff_call.outstatus_detailed, control_dataset=outlier_expected_outstatus_detailed_c03_c, dataset_name="OUTSTATUS_DETAILED", round_data=9)
    finally:
        out_assert, err2 = capfd.readouterr() # get console log for assert
        print(out_act+out_assert) # print all captured logs