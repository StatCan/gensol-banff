import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_b01(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        # must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",

        # Unit Test Parameters
        sas_log_name="massimpu_b01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: must_impute is mandatory"
    )