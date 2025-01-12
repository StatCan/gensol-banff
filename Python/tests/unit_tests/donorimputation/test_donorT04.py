from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - compare outdonormap with control dataset
@pytest.mark.m_validated
@pytest.mark.m_auto_pass # complete enough to be valuable at this point
def test_donorT04(capfd, JA_indata16, JA_instat17):
    # we want to search for more than one message, so lets build a `list`
    message_list = [] # empty list
    message_list.append("NOTE: There were 2 observations dropped from indata data set because they are potential donors but toexcl value is 'E' for these observations.")

    # now we'll add a second message which contains multiple lines of text
    # TIP: use """ (triple quotes) """ for multi-line messages
    message_list.append("""
Number of observations ............................:       4
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       2
    non-eligible donors ...........................:       0
    """)

    # supply `message_list` for the `msg_list_sas` argument
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata16,
        instatus=JA_instat17,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True, 
        eligdon="any",
        accept_negative = True,
        unit_id="IDENT",
        must_match="STAFF",
        data_excl_var= "toexcl"
        ,

        # Unit Test Parameters
        sas_log_name="donorT04_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas=message_list
    )

