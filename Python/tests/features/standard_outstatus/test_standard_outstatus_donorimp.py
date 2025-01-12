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
def test_standard_outstatus_donorimp_a(capfd, indata, instatus, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    # run Banff procedure
    banff_call = banfftest.pytest_donorimp(
        unit_id=unit_id_parameter,
        indata=indata,
        instatus=instatus,
        outmatching_fields=True,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        post_edits="Q1 + Q2 + Q3 + Q4 - TOTAL <= 0;",
        min_donors=1,
        percent_donors=1,
        n=1,
        n_limit=1,
        mrl=0.5,
        random=True,
        eligdon="original",
        accept_negative=True,
        must_match="STAFF",
        data_excl_var="TOEXCL",
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
        {unit_id_dataset} TOTAL Q1 Q2 Q3 Q4 STAFF PROV
        REC01 500 120 150 80 150 50 24
        REC02 750 200 170 130 250 75 24
        REC03 400 80 90 100 130 40 24
        REC04 1000 150 250 350 250 100 24
        REC05 1050 200 225 325 300 100 24
        REC06 500 100 125 5000 130 45 24
        REC07 400 80 90 100 15 40 30
        REC08 950 150 999 999 200 90 30
        REC09 1025 200 225 300 10 10 30
        REC10 800 11 12 13 14 80 30
        REC11 -25 -10 -5 -5 -10 3000 30
        REC12 1000 150 250 350 250 100 30
        REC13 999 200 225 325 300 100 30
        REC14 -25 -10 -5 -10 -5 3000 30
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    def add_exclude(row):
        if row['TOTAL'] > 1000:
            return 'E'
        else:
            return ''

    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)

    return indata

@pytest.fixture
def instatus():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO(f"""
        FIELDID STATUS {unit_id_dataset}
        Q3 IPR REC01
        Q4 FTE REC04
        Q3 FTI REC06
        Q2 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        STAFF FTI REC09
        Q1 FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q1 FTI REC11
        Q2 FTI REC11
        Q3 FTI REC11
        Q4 FTI REC11
        Q2 FTI REC13
        Q3 FTI REC13
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS  VALUE
        REC06      Q3    IDN  100.0
        REC08      Q2    IDN  250.0
        REC08      Q3    IDN  350.0
        REC09      Q4    IDN  250.0
        REC10      Q1    IDN  -10.0
        REC10      Q2    IDN   -5.0
        REC10      Q3    IDN  -10.0
        REC10      Q4    IDN   -5.0
        REC11      Q1    IDN  -10.0
        REC11      Q2    IDN   -5.0
        REC11      Q3    IDN  -10.0
        REC11      Q4    IDN   -5.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()