from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - compare outdonormap with control dataset
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorT13(capfd, JA_indata27, JA_instat28):

    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata27,
        instatus=JA_instat28,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=5,
        outmatching_fields=True, 
        eligdon="o",
        unit_id="IDENT",
        must_match="STAFF",
        data_excl_var= "TOEXCL"
        ,

        # Unit Test Parameters
        sas_log_name="donorT13_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True
    )