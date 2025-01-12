from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_for_review # Mixed datatype columns now causes an exception during packing: PyArrow doesn't know what to do
def test_donor_IT05(capfd, JA_indata22, JA_instat19):
    """When a variable in the edits contains character data, an error should occur
        and a message should indicate which variable had invalid data"""
    
    # insert character value in numeric Q2 column, used in edits
    JA_indata22.at[3,'Q2'] = '.'
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
        rc_should_be_zero=False,
        msg_list_contains="ERROR: invalid data detected while reading numeric variable 'Q2'",
        expected_error_count=1
    )

