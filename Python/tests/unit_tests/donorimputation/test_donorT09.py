from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - compare outdonormap with control dataset
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorT09(capfd, JA_indata22, JA_instat19):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata22,
        instatus=JA_instat19,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True, 
        eligdon="any",
        unit_id="IDENT",
        must_match="TOTAL",
        data_excl_var= "toexcl"
        ,

        # Unit Test Parameters
        sas_log_name="donorT09_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=0
    )

