from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - Validate output dataset according to valid possible values
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorT03(capfd, JA_indata14, JA_instat15):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata14,
        instatus=JA_instat15,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 >=.8*total; Q1 + Q2 + Q3 + Q4 <=1.2*total;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True, 
        eligdon="any",
        unit_id="IDENT",
        must_match="STAFF",
        data_excl_var= "toexcl"
        ,

        # Unit Test Parameters
        sas_log_name="donorT03_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True
    )