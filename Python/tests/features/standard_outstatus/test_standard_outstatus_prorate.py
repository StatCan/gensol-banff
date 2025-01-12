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
def test_standard_outstatus_prorate_a(capfd, indata, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    banff_call = banfftest.pytest_prorate(
        unit_id=unit_id_parameter,
        indata=indata,
        edits="Q1+4Q2+2Q3+Q4=YEAR;",
        method="BASIC",
        decimal=1,
        lower_bound=-100,
        upper_bound=100,
        modifier="always",
        accept_negative=True,

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
        {unit_id_dataset} Q1 Q2 Q3 Q4 YEAR
        R01 -25 42 27 25 40
        R02 -25 44 20 20 40
        R03 -18 44 15 5 40
        R04 -18 42 15 5 40
        R05 12 30 15 5 40
        R06 12 20 10 5 40
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    return example_indata

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS  VALUE
        R01      Q1    IPR    5.2
        R01      Q2    IPR   29.3
        R01      Q3    IPR   10.7
        R01      Q4    IPR   -5.2
        R02      Q1    IPR    4.7
        R02      Q2    IPR   30.9
        R02      Q3    IPR    8.2
        R02      Q4    IPR   -3.8
        R03      Q1    IPR    1.6
        R03      Q2    IPR   32.0
        R03      Q3    IPR    6.9
        R03      Q4    IPR   -0.5
        R04      Q1    IPR   -3.6
        R04      Q2    IPR   33.6
        R04      Q3    IPR    9.0
        R04      Q4    IPR    1.0
        R05      Q1    IPR    3.8
        R05      Q2    IPR   24.8
        R05      Q3    IPR    9.8
        R05      Q4    IPR    1.6
        R06      Q1    IPR    8.9
        R06      Q2    IPR   18.7
        R06      Q3    IPR    8.7
        R06      Q4    IPR    3.7
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()