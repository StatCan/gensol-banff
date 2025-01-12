from banff.testing import pytest_donorimp
import pytest

#disabled @pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU05(capfd, JA_indata32_exclude, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32_exclude,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        #n_limit=1,
        mrl=1.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        data_excl_var="EXCLUDE",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU05_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_sas="""NOTE: There were 1 observations dropped from indata data set because they are potential donors but EXCLUDE value is 'E'
      for these observations.

NOTE: DONORLIMIT = 9

Number of observations ............................:      15
Number of observations dropped ....................:       1
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       1
    non-eligible donors ...........................:       0

Number of valid observations ......................:      14     100.00%
Number of donors ..................................:       2      14.29%
Number of donors to reach DONORLIMIT ..............:       1      50.00%
Number of recipients ..............................:      12      85.71%
    with matching fields ..........................:      12      85.71%
        imputed ...................................:      12      85.71%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=1

NOTE: There were 2 observations dropped from indata data set because they are potential donors but EXCLUDE value is 'E'
      for these observations.

NOTE: DONORLIMIT = 5

Number of observations ............................:      15
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       2
    non-eligible donors ...........................:       0

Number of valid observations ......................:      13     100.00%
Number of donors ..................................:       4      30.77%
Number of donors to reach DONORLIMIT ..............:       1      25.00%
Number of recipients ..............................:       9      69.23%
    with matching fields ..........................:       9      69.23%
        imputed ...................................:       9      69.23%
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

NOTE: There were 2 observations dropped from indata data set because they are potential donors but EXCLUDE value is 'E'
      for these observations.

NOTE: DONORLIMIT = 6

Number of observations ............................:      16
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       2
    non-eligible donors ...........................:       0

Number of valid observations ......................:      14     100.00%
Number of donors ..................................:       3      21.43%
Number of donors to reach DONORLIMIT ..............:       1      33.33%
Number of recipients ..............................:      11      78.57%
    with matching fields ..........................:      11      78.57%
        imputed ...................................:      11      78.57%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the following by group:
      AREA=4


Number of observations ............................:      61
Number of observations dropped ....................:       5
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       5
    non-eligible donors ...........................:       0

Number of valid observations ......................:      56     100.00%
Number of donors ..................................:      21      37.50%
Number of donors to reach DONORLIMIT ..............:       4      19.05%
Number of recipients ..............................:      35      62.50%
    with matching fields ..........................:      35      62.50%
        imputed ...................................:      35      62.50%
        not imputed ...............................:       0       0.00%
    without matching fields .......................:       0       0.00%

NOTE: The above message was for the total of all by-groups."""
    )