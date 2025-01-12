import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_a(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_1, outlier_expected_outstatus_detailed_B05_1):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v2_char,
        method="C",
        mii=3,
        mei=2,
        accept_negative=True,
        unit_id="ident",
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=0,
        expected_outstatus=outlier_expected_outstatus_B05_1,
        expected_outstatus_detailed=outlier_expected_outstatus_detailed_B05_1,
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_b(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_1, outlier_expected_outstatus_detailed_B05_1):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v2_char,
        method="C",
        mii=3,
        mei=2,
        accept_negative=True,
        unit_id="ident",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=0,
        expected_outstatus=outlier_expected_outstatus_B05_1,
        expected_outstatus_detailed=outlier_expected_outstatus_detailed_B05_1,
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_c(capfd, outlier_indata_b05_v1_v2_char):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v1_v2_char,
        method="C",
        mii=3,
        mei=2,
        accept_negative=True,
        unit_id="ident",
        by="area",

        # Unit Test Parameters
        sas_log_name="outlierB05_c.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_sas_exact="ERROR: No numeric variable in indata data set to use as default list for var statement"
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_d(capfd, outlier_indata_b05_v1_v2_char):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v1_v2_char,
        method="C",
        mii=3,
        mei=2,
        accept_negative=True,
        unit_id="ident",
        by="area",
        var=""
        ,

        # Unit Test Parameters
        sas_log_name="outlierB05_d.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_sas_exact="ERROR: At least one variable must be specified with a var statement"
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_e(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_2, outlier_expected_outstatus_detailed_B05_2):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v2_char,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        accept_negative=True,
        unit_id="ident",
        by="area"
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=0,
        expected_outstatus=outlier_expected_outstatus_B05_2,
        expected_outstatus_detailed=outlier_expected_outstatus_detailed_B05_2,
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_f(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_2, outlier_expected_outstatus_detailed_B05_2):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v2_char,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        accept_negative=True,
        unit_id="ident"
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=0,
        expected_outstatus=outlier_expected_outstatus_B05_2,
        expected_outstatus_detailed=outlier_expected_outstatus_detailed_B05_2,
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_g(capfd, outlier_indata_b05_v1_v2_char):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v1_v2_char,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        accept_negative=True,
        unit_id="ident",
        by="area"
        ,

        # Unit Test Parameters
        sas_log_name="outlierB05_g.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_sas_exact="ERROR: No numeric variable in indata data set to use as default list for var statement"
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlierB05_h(capfd, outlier_indata_b05_v1_v2_char):
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v1_v2_char,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        accept_negative=True,
        unit_id="ident",
        var=""
        ,

        # Unit Test Parameters
        sas_log_name="outlierB05_h.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_sas_exact="ERROR: At least one variable must be specified with a var statement"
    )