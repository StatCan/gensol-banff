import pytest
from banff.testing import pytest_prorate

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_c01_a(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        #instatus=prorate_instatus_02,
        edits="VA:I + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_c01_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1, # original test case only produced 1 error, now proc flags indata and instatus missing "idnt"
        msg_list_sas="ERROR: instatus data set is mandatory: one or more modifiers are ORIGINAL or IMPUTED."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_prorate_c01_b(capfd, prorate_indata_03, prorate_instatus_02):
    pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        #instatus=prorate_instatus_02,
        edits="VA:O + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_c01_b.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1, # original test case only produced 1 error, now proc flags indata and instatus missing "idnt"
        msg_list_sas="ERROR: instatus data set is mandatory: one or more modifiers are ORIGINAL or IMPUTED."
    )
