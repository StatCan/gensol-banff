import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_g02_a(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        edits="A + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_g02_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="""
ERROR: Prorating edits parser: Looking for 'Variable' but found 
       'Modifier' instead.
       
       Error at or before:
       A + VB+ VC + VD = TOT1;
        ^
       """
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_g02_b(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        edits="VA + I + VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_g02_b.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="""
ERROR: Prorating edits parser: Looking for 'Variable' but found 
       'Modifier' instead.
       
       Error at or before:
       VA + I + VC + VD = TOT1;
             ^
       """
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_g02_c(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        edits="VA + N + VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_g02_c.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="""
ERROR: Prorating edits parser: Looking for 'Variable' but found 
       'Modifier' instead.
       
       Error at or before:
       VA + N + VC + VD = TOT1;
             ^
       """
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_g02_d(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        edits="VA + VB+ VC + VD = O;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="I",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_g02_d.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="""
ERROR: Prorating edits parser: Looking for 'Variable' but found 
       'Modifier' instead.
       
       Error at or before:
       VA + VB+ VC + VD = O;
                           ^
       """
    )
