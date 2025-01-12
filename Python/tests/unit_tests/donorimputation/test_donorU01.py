from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_incomplete 
# INCOMPLETE because
# - last message (donorlimit = no limit) should occur 1x per BY group
# - should count occurances of this message, as specified in the test case document
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorU01(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU01_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=[
            "NOTE: n_limit not specified.",
            "NOTE: mrl not specified.",
            "NOTE: DONORLIMIT = No limit"
        ]
    )