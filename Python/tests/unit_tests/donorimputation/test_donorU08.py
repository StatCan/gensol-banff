from banff.testing import pytest_donorimp
import pytest

#@pytest.mark.m_in_development
@pytest.mark.m_incomplete
#required to complete test case
#   - compare outdonormap with control dataset
#disabled @pytest.mark.m_validated
def test_donorU08(capfd, JA_indata37, JA_instat38):

    
    for i, row in JA_indata37.iterrows():
        val = " "
        if JA_indata37.at[i,'TOTAL'] > 1000:
            val = "E"
        JA_indata37.at[i,'toexcl'] = val

    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata37,
        instatus=JA_instat38,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=1,
        n_limit=1,
        mrl=0.5,
        eligdon="original",
        outmatching_fields = True,
        accept_negative=True,
        unit_id="IDENT",
        must_match="STAFF",
        data_excl_var="toexcl",
        by="prov"
        ,

        # Unit Test Parameters
        sas_log_name="donorU08_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        msg_list_sas="""NOTE: There were 1 observations dropped from indata data set because they are potential donors but 
      toexcl value is 'E' for these observations.
NOTE: There were 1 observations dropped from indata data set because they are potential donors but they 
      dont meet criteria related to eligdon.

NOTE: DONORLIMIT = 1

Number of observations ............................:       6
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       1
    non-eligible donors ...........................:       1

Number of valid observations ......................:       4     100.00%
Number of donors ..................................:       3      75.00%
Number of donors to reach DONORLIMIT ..............:       1      33.33%
Number of recipients ..............................:       1      25.00%
    with matching fields ..........................:       1      25.00%
        imputed ...................................:       1      25.00%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      prov=24

NOTE: DONORLIMIT = 2

WARNING: DONORLIMIT parameters might prevent recipients from being imputed.

Number of observations ............................:       8
Number of observations dropped ....................:       0

Number of valid observations ......................:       8     100.00%
Number of donors ..................................:       2      25.00%
Number of donors to reach DONORLIMIT ..............:       2     100.00%
Number of recipients ..............................:       6      75.00%
    with matching fields ..........................:       6      75.00%
        imputed ...................................:       4      50.00%
        not imputed ...............................:       2      25.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      prov=30


Number of observations ............................:      14
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       1
    non-eligible donors ...........................:       1

Number of valid observations ......................:      12     100.00%
Number of donors ..................................:       5      41.67%
Number of donors to reach DONORLIMIT ..............:       3      60.00%
Number of recipients ..............................:       7      58.33%
    with matching fields ..........................:       7      58.33%
        imputed ...................................:       5      41.67%
        not imputed ...............................:       2      16.67%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the total of all by-groups."""
    )

