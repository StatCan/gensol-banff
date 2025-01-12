from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - compare outdonormap with control dataset
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorT07(capfd,  JA_indata22, JA_instat23):
    # we want to search for more than one message, so lets build a `list`
    message_list = [] # empty list
    message_list.append("WARNING: There were 1 mixed observations dropped from indata data set because one or more")

    # now we'll add a second message which contains multiple lines of text
    # TIP: use """ (triple quotes) """ for multi-line messages
    message_list.append("""
Number of observations ............................:       4
Number of observations dropped ....................:       1
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       1
    excluded donors ...............................:       0
    non-eligible donors ...........................:       0
    """)

    # supply `message_list` for the `msg_list_sas` argument
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata22,
        instatus=JA_instat23,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
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
        sas_log_name="donorT07_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=message_list,
        expected_warning_count=1
    )

