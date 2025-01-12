from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU04(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=4,
        percent_donors=1,
        n=25,
        n_limit=1,
        #mrl=-.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU04_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
        msg_list_sas=[
            "WARNING: There are not enough donors in the group processed. No imputation will be done.",
            "NOTE: DONORLIMIT = 1",
            "WARNING: DONORLIMIT parameters might prevent recipients from being imputed."
        ]
    )
