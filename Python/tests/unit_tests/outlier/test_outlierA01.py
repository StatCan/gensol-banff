import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
# `m__default_var_list` because originally this test case triggered a call to `SetDefaultVarList()`
@pytest.mark.m__default_var_list
def test_outlierA01_a(capfd, outlier_indata_01):
    #### arrange

    #### act
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_01,
        method="C",
        mii=3,
        mei=2,
        var="V1 V2",

        # Unit Test Parameters
        sas_log_name="outlierA01_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: unit_id statement is mandatory."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_outlierA01_b(capfd, outlier_indata_01, outlier_inhist_01):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_01,
        indata_hist=outlier_inhist_01,
        method="H",
        mii=3,
        mei=2,
        var="V1 V2",

        # Unit Test Parameters
        sas_log_name="outlierA01_b.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: unit_id statement is mandatory."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_outlierA01_c(capfd, outlier_indata_01, outlier_inhist_01):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_01,
        indata_hist=outlier_inhist_01,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        var="V1 V2",

        # Unit Test Parameters
        sas_log_name="outlierA01_c.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: unit_id statement is mandatory."
    )