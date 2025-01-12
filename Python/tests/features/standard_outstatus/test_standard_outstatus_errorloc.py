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
def test_standard_outstatus_errorloc_a(capfd, indata, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    banff_call = banfftest.pytest_errorloc(
        unit_id=unit_id_parameter,
        indata=indata,
        edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
        weights="x1=1.5;",
        cardinality=2,
        time_per_obs=0.1,
        by="ZONE",

        pytest_capture=capfd,
        expected_outstatus=outstatus,
    )

##### DATA #####

@pytest.fixture
def indata():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types[f'{unit_id_dataset}'] = str
    var_types['ZONE'] = str

    example_indata = pd.read_csv(StringIO(f"""
        {unit_id_dataset} X1 X2 ZONE
        R03 10 40 B
        R02 -4 49 A
        R04 4 49 A
        R01 16 49 A
        R05 15 51 B
        R07 -4 29 B
        R06 30 70 B
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    return example_indata.sort_values(by=['ZONE', unit_id_dataset])

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS VALUE
        R01      X1    FTI  16.0
        R02      X1    FTI  -4.0
        R04      X2    FTI  49.0
        R05      X2    FTI  51.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()