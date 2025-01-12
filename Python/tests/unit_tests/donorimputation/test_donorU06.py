from banff.testing import pytest_donorimp
import pytest

#disabled @pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU06(capfd, JA_indata32, JA_instat33_b):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33_b,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        #n_limit=1,
        mrl=1.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU06_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
        msg_list_sas="""WARNING: There were 3 observations dropped from indata data set because one/more edits variables or
         one/more must_match variables outside the edits group are missing in indata data set and there
         is no corresponding FTI in instatus data set.

NOTE: DONORLIMIT = 5

Number of observations ............................:      15
Number of observations dropped ....................:       3
    missing key ...................................:       0
    missing data ..................................:       3
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       0
    non-eligible donors ...........................:       0

Number of valid observations ......................:      12     100.00%
Number of donors ..................................:       3      25.00%
Number of donors to reach DONORLIMIT ..............:       1      33.33%
Number of recipients ..............................:       9      75.00%
    with matching fields ..........................:       9      75.00%
        imputed ...................................:       9      75.00%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=1

WARNING: There were 3 observations dropped from indata data set because one/more edits variables or
         one/more must_match variables outside the edits group are missing in indata data set and there
         is no corresponding FTI in instatus data set.

NOTE: DONORLIMIT = 2

Number of observations ............................:      15
Number of observations dropped ....................:       3
    missing key ...................................:       0
    missing data ..................................:       3
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       0
    non-eligible donors ...........................:       0

Number of valid observations ......................:      12     100.00%
Number of donors ..................................:       6      50.00%
Number of donors to reach DONORLIMIT ..............:       2      33.33%
Number of recipients ..............................:       6      50.00%
    with matching fields ..........................:       6      50.00%
        imputed ...................................:       6      50.00%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=2

NOTE: DONORLIMIT = 2

Number of observations ............................:      15
Number of observations dropped ....................:       0

Number of valid observations ......................:      15     100.00%
Number of donors ..................................:      12      80.00%
Number of donors to reach DONORLIMIT ..............:       1       8.33%
Number of recipients ..............................:       3      20.00%
    with matching fields ..........................:       3      20.00%
        imputed ...................................:       3      20.00%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=3

WARNING: There were 1 observations dropped from indata data set because one/more edits variables or
         one/more must_match variables outside the edits group are missing in indata data set and there
         is no corresponding FTI in instatus data set.

NOTE: DONORLIMIT = 3

Number of observations ............................:      16
Number of observations dropped ....................:       1
    missing key ...................................:       0
    missing data ..................................:       1
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       0
    non-eligible donors ...........................:       0

Number of valid observations ......................:      15     100.00%
Number of donors ..................................:       5      33.33%
Number of donors to reach DONORLIMIT ..............:       3      60.00%
Number of recipients ..............................:      10      66.67%
    with matching fields ..........................:      10      66.67%
        imputed ...................................:      10      66.67%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=4


Number of observations ............................:      61
Number of observations dropped ....................:       7
    missing key ...................................:       0
    missing data ..................................:       7
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       0
    non-eligible donors ...........................:       0

Number of valid observations ......................:      54     100.00%
Number of donors ..................................:      26      48.15%
Number of donors to reach DONORLIMIT ..............:       7      26.92%
Number of recipients ..............................:      28      51.85%
    with matching fields ..........................:      28      51.85%
        imputed ...................................:      28      51.85%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the total of all by-groups."""
    )