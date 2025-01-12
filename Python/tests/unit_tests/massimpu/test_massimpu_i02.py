import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_i02_a(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=0,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",

        # Unit Test Parameters
        sas_log_name="massimpu_i02_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Invalid percent_donors. percent_donors must be greater than or equal to 1 and less than 100."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_i02_b(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=-5,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",

        # Unit Test Parameters
        sas_log_name="massimpu_i02_b.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Invalid percent_donors. percent_donors must be greater than or equal to 1 and less than 100."
    )

@pytest.mark.m_validated
@pytest.mark.m_for_review # SAS validation, caught by new validation now
def test_massimpu_i02_c(capfd, massimpu_indata_03_char_ident):
    """
    SAS performed the validation being checked here.  
    A new validation exists, we must review it
    """
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors="7%",
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",

        # Unit Test Parameters
        sas_log_name="massimpu_i02_c.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
    )



@pytest.mark.m_validated
@pytest.mark.m_for_review # SAS validation, caught by new validation now
def test_massimpu_i02_d(capfd, massimpu_indata_03_char_ident):
    """
    SAS performed the validation being checked here.  
    A new validation exists, we must review it
    """
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors="six",
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",

        # Unit Test Parameters
        sas_log_name="massimpu_i02_d.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="""ERROR 22-322: Expecting a numeric constant.
ERROR 202-322: The option or parameter is not recognized and will be ignored."""
    )

