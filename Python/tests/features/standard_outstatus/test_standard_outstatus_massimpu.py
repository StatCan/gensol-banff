"""Validate that outstatus dataset conforms to standard
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

# `unit_id` parameter is NOT case sensitive, so the dataset and parameter can differ in case
unit_id_dataset="IDENT"
unit_id_parameter="IdEnT"

@pytest.mark.m_auto_pass
def test_standard_outstatus_massimpu_a(capfd, indata, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    banff_call = banfftest.pytest_massimpu(
        unit_id=unit_id_parameter,
        indata=indata,
        min_donors=1,
        percent_donors=1,
        random=True,
        accept_negative=True,
        must_impute="Q1 Q2 Q3 Q4",
        must_match="TOTAL",
        by="STAFF",
        seed=1,

        pytest_capture=capfd,
        expected_outstatus=outstatus,
    )

##### DATA #####

@pytest.fixture
def indata():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types[f'{unit_id_dataset}'] = str

    example_indata = pd.read_csv(StringIO(f"""
        {unit_id_dataset} TOTAL Q1 Q2 Q3 Q4 STAFF
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
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    return example_indata.sort_values(by=['STAFF', unit_id_dataset])

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset}  FIELDID STATUS  VALUE
        REC11      Q4   IMAS  150.0
        REC11      Q3   IMAS  175.0
        REC11      Q2   IMAS  150.0
        REC11      Q1   IMAS  125.0
        REC13      Q4   IMAS  130.0
        REC13      Q3   IMAS  100.0
        REC13      Q2   IMAS   90.0
        REC13      Q1   IMAS   80.0
        REC15      Q4   IMAS 1000.0
        REC15      Q3   IMAS 1000.0
        REC15      Q2   IMAS  500.0
        REC15      Q1   IMAS  500.0
        REC17      Q4   IMAS  750.0
        REC17      Q3   IMAS  600.0
        REC17      Q2   IMAS  650.0
        REC17      Q1   IMAS  500.0
        REC19      Q4   IMAS 1000.0
        REC19      Q3   IMAS 1000.0
        REC19      Q2   IMAS  500.0
        REC19      Q1   IMAS  500.0
        REC12      Q4   IMAS  175.0
        REC12      Q3   IMAS  200.0
        REC12      Q2   IMAS  225.0
        REC12      Q1   IMAS  200.0
        REC14      Q4   IMAS  250.0
        REC14      Q3   IMAS  350.0
        REC14      Q2   IMAS  250.0
        REC14      Q1   IMAS  150.0
        REC16      Q4   IMAS  250.0
        REC16      Q3   IMAS  130.0
        REC16      Q2   IMAS  170.0
        REC16      Q1   IMAS  200.0
        REC18      Q4   IMAS  300.0
        REC18      Q3   IMAS  225.0
        REC18      Q2   IMAS  200.0
        REC18      Q1   IMAS  175.0
        REC20      Q4   IMAS  250.0
        REC20      Q3   IMAS  350.0
        REC20      Q2   IMAS  250.0
        REC20      Q1   IMAS  150.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()