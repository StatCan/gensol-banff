import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_l06_a(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        by="ident",

        # Unit Test Parameters
        sas_log_name="massimpu_l06_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: The lists of variables of the unit_id and by statements are mutually exclusive."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_l06_b(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="Q2 staff",

        # Unit Test Parameters
        sas_log_name="massimpu_l06_b.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: The lists of variables of the must_impute and must_match statements are mutually exclusive."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_l06_c(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        by="Q2",

        # Unit Test Parameters
        sas_log_name="massimpu_l06_c.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: The lists of variables of the must_impute and by statements are mutually exclusive."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_l06_d(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        by="staff",

        # Unit Test Parameters
        sas_log_name="massimpu_l06_d.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: The lists of variables of the must_match and by statements are mutually exclusive."
    )

@pytest.mark.m_validated
@pytest.mark.m_performance_test
def test_massimpu_l11(capfd, massimpu_indata_05):
    """
    WARNING: this implementation of the L11 test case does not actually validate the 
    output values, it is instead used only for performance measurement purposes
    """
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_05,
        min_donors=1,
        percent_donors=1,
        accept_negative=True,
        random=True,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        by="area",
        trace=True,
        no_by_stats=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )

@pytest.mark.m_validated
@pytest.mark.m_performance_test
def test_massimpu_l12(capfd, massimpu_indata_05):
    """
    WARNING: this implementation of the L12 test case does not actually validate the 
    output values, it is instead used only for performance measurement purposes
    """
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_05,
        min_donors=1,
        percent_donors=1,
        accept_negative=True,
        random=True,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
       #by="area",
        trace=True,
        no_by_stats=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )