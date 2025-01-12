import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_k02_a(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        random=True,
        seed=-5,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",

        # Unit Test Parameters
        sas_log_name="massimpu_k02_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_sas=["(value chosen by the system)",
                      "WARNING: The seed value specified is negative and cannot be used. It has been replaced with a generated number."]
    )
