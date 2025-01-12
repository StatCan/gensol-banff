import pytest
from banff.testing import pytest_massimpu

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_c01(capfd, massimpu_indata_03_char_ident):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        #must_match="total staff",
        #random=True,

        # Unit Test Parameters
        sas_log_name="massimpu_c01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: Invalid options. At least one of must_match and random must be specified"
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_c02(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_04,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        random=True,
        accept_negative=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outdonormap=massimpu_outdonormap_01
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_massimpu_c03(capfd, massimpu_indata_03_char_ident, massimpu_outdonormap_01):
    test_call = pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_03_char_ident,
        outdonormap='pandas',
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        #must_match="total staff",
        random=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )

    assert 1==1
    # 1 row
    assert len(test_call.banff_call.outdonormap) == 1
    # recipient #3
    assert test_call.banff_call.outdonormap['RECIPIENT'][0] == "03"
    # #attempts
    assert test_call.banff_call.outdonormap['NUMBER_OF_ATTEMPTS'][0] == 0
    # donor
    donor=test_call.banff_call.outdonormap['DONOR'][0]
    assert (
        donor == "01" 
        or donor == "02"
        or donor == "04"
        or donor == "05"
    )