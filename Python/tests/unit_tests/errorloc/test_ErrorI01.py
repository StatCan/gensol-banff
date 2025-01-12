import pytest
from banff.testing import pytest_errorloc

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_errorI01_a(capfd, errorloc_indata_grp_i):
    #### arrange

    #### act
    pytest_errorloc(
        # Banff parameters
        indata=errorloc_indata_grp_i,
        edits="x1>=5; x1<=15; x2>=30; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=150; x1+x2+x3<=210;",
        seed=-1,
        unit_id="REC"
        ,

        # Unit Test Parameters
        sas_log_name="errorI01_a.log",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_sas="WARNING: The seed value specified is negative and cannot be used. It has been replaced with a generated number."
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_errorI01_b(capfd, errorloc_indata_grp_i):
    #### arrange

    #### act
    pytest_errorloc(
        # Banff parameters
        indata=errorloc_indata_grp_i,
        edits="x1>=5; x1<=15; x2>=30; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=150; x1+x2+x3<=210;",
        seed=0,
        unit_id="REC"
        ,

        # Unit Test Parameters
        sas_log_name="errorI01_b.log",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_sas="WARNING: The seed value specified is 0 and cannot be used. It has been replaced with a generated number."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_jira_64
def test_errorI01_c(capfd, errorloc_indata_grp_i):
    #### arrange

    #### act
    pytest_errorloc(
        # Banff parameters
        indata=errorloc_indata_grp_i,
        edits="x1>=5; x1<=15; x2>=30; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=150; x1+x2+x3<=210;",
        seed=3.3,
        unit_id="REC"
        ,

        # Unit Test Parameters
        sas_log_name="errorI01_c.log",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="expecting an INTEGER constant"
    )