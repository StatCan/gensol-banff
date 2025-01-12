import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
#@pytest.mark.m_for_review # message differs from SAS, hence the `msg_list_contains` usage, not `msg_list_sas`
def test_massimpu_k03(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        random=True,
        seed=2.2,

        # Unit Test Parameters
        sas_log_name="massimpu_k03.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="expecting an INTEGER constant"
    )