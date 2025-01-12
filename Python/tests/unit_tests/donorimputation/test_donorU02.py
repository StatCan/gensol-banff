from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU02_a(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        n_limit=2,
        #mrl=1.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU02a_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        msg_list_sas=[
            "NOTE: DONORLIMIT = 2",
            "WARNING: DONORLIMIT parameters might prevent recipients from being imputed."
        ]
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU02_b(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        #n_limit=2,
        mrl=1.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU02b_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=[
            "NOTE: DONORLIMIT = 6",
            "NOTE: DONORLIMIT = 3",
            "NOTE: DONORLIMIT = 2",
            "NOTE: DONORLIMIT = 5"
        ]
    )

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU02_c(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        n_limit=3,
        mrl=1.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU02c_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=[
            "NOTE: DONORLIMIT = 6",
            "NOTE: DONORLIMIT = 3",
            "NOTE: DONORLIMIT = 5"
        ]
    )