from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_incomplete # need to check output dataset for set of possible valid values
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorU07(capfd, JA_indata35, JA_instat36):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata35,
        instatus=JA_instat36,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=1,
        n_limit=1,
        #mrl=1.5,
        outmatching_fields = True,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU07_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1
    )



