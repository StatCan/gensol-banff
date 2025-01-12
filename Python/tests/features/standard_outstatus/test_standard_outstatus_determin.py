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
def test_standard_outstatus_determin_a(capfd, indata, instatus, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    # run Banff procedure
    banff_call = banfftest.pytest_determin(
        unit_id=unit_id_parameter,
        indata=indata,
        instatus=instatus,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        by="prov",

        pytest_capture=capfd,
        expected_warning_count=None,
        expected_outstatus=outstatus,
    )

##### DATA #####

@pytest.fixture
def indata():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types[f'{unit_id_dataset}'] = str

    indata = pd.read_csv(StringIO(f"""
        {unit_id_dataset} TOTAL Q1 Q2 Q3 Q4 staff prov
        REC01 500 100 125 125 150 2000 24
        REC02 750 200 170 130 250 2500 24
        REC03 400 80 90 100 130 1500 24
        REC04 1000 150 250 350 250 3500 24
        REC05 3000 500 500 1000 1000 5000 24
        REC06 50 10 15 500 20 100 24
        REC07 600 110 140 230 45 2400 30
        REC08 900 175 999 999 300 3000 30
        REC09 2500 400 555 600 5000 89 30
        REC10 800 11 12 13 14 2800 30
        REC11 -25 -10 -5 -5 -10 3000 30
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    return indata

@pytest.fixture
def instatus():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO(f"""
        fieldid status {unit_id_dataset}
        Q3 FTI REC06
        Q4 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        staff FTI REC09
        Q1 FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q4 FTI REC11
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS VALUE
        REC06      Q3    IDE   5.0
        REC07      Q4    IDE 120.0
        REC09      Q4    IDE 945.0
        REC11      Q4    IDE  -5.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()