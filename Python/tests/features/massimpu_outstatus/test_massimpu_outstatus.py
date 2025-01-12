"""Basic test that outstatus dataset is created, contains some values

Based on user guide example 1
Outstatus contents NOT VALIDATED by methodology, it's just a check for consistent values.  
"""

# import necessary Python packages
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest

@pytest.fixture
def massimpdata():
    # create indata dataframe
    massimpdata = pd.read_csv(StringIO("""
        IDENT TOTAL Q1 Q2 Q3 Q4 STAFF
        REC01 500 100 125 125 150 1000
        REC02 750 200 170 130 250 2000
        REC03 400 80 90 100 130 1000
        REC04 1000 150 250 350 250 2000
        REC05 3000 500 500 1000 1000 1000
        REC06 800 200 225 200 175 2000
        REC07 600 125 150 175 150 1000
        REC08 900 175 200 225 300 2000
        REC09 2500 500 650 600 750 1000
        REC10 800 150 175 225 250 2000
        REC21 3000 -45 -50 75 -234 2000
        REC11 575 NaN NaN NaN NaN 1000
        REC12 850 NaN NaN NaN NaN 2000
        REC13 375 NaN NaN NaN NaN 1000
        REC14 1100 NaN NaN NaN NaN 2000
        REC15 3100 NaN NaN NaN NaN 1000
        REC16 750 NaN NaN NaN NaN 2000
        REC17 675 NaN NaN NaN NaN 1000
        REC18 875 NaN NaN NaN NaN 2000
        REC19 4000 NaN NaN NaN NaN 1000
        REC20 NaN NaN NaN NaN NaN 2000
    """), sep=r'\s+')

    # sort the input data set by the STAFF and IDENT variables.
    return massimpdata.sort_values(by=["STAFF", "IDENT"])

@pytest.fixture
def totally_unvalidated_outstatus():
    """This dataset hasn't been validated at all, it could be nonsense
    We're mostly just making sure it consistently produces an output at this point
    """
    # create indata dataframe
    return pd.read_csv(StringIO("""
        IDENT FIELDID STATUS   VALUE
        REC11      Q4   IMAS   150.0
        REC11      Q3   IMAS   175.0
        REC11      Q2   IMAS   150.0
        REC11      Q1   IMAS   125.0
        REC13      Q4   IMAS   130.0
        REC13      Q3   IMAS   100.0
        REC13      Q2   IMAS    90.0
        REC13      Q1   IMAS    80.0
        REC15      Q4   IMAS  1000.0
        REC15      Q3   IMAS  1000.0
        REC15      Q2   IMAS   500.0
        REC15      Q1   IMAS   500.0
        REC17      Q4   IMAS   750.0
        REC17      Q3   IMAS   600.0
        REC17      Q2   IMAS   650.0
        REC17      Q1   IMAS   500.0
        REC19      Q4   IMAS  1000.0
        REC19      Q3   IMAS  1000.0
        REC19      Q2   IMAS   500.0
        REC19      Q1   IMAS   500.0
        REC12      Q4   IMAS   175.0
        REC12      Q3   IMAS   200.0
        REC12      Q2   IMAS   225.0
        REC12      Q1   IMAS   200.0
        REC14      Q4   IMAS   250.0
        REC14      Q3   IMAS   350.0
        REC14      Q2   IMAS   250.0
        REC14      Q1   IMAS   150.0
        REC16      Q4   IMAS   250.0
        REC16      Q3   IMAS   130.0
        REC16      Q2   IMAS   170.0
        REC16      Q1   IMAS   200.0
        REC18      Q4   IMAS   300.0
        REC18      Q3   IMAS   225.0
        REC18      Q2   IMAS   200.0
        REC18      Q1   IMAS   175.0
        REC20      Q4   IMAS   250.0
        REC20      Q3   IMAS   350.0
        REC20      Q2   IMAS   250.0
        REC20      Q1   IMAS   150.0
    """), sep=r'\s+')

@pytest.mark.m_auto_pass
def test_massimpu_outstatus_a(capfd, massimpdata, totally_unvalidated_outstatus):
    # create Banff call
    banff_call = banfftest.pytest_massimpu(
        indata=massimpdata,
        min_donors=1,
        percent_donors=1,
        random=True,
        accept_negative=True,
        unit_id="IDENT",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="TOTAL",
        by="STAFF",
        seed=1,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=totally_unvalidated_outstatus,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()