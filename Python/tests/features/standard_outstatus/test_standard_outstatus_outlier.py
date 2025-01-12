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
def test_standard_outstatus_outlier_a(capfd, indata, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    banff_call = banfftest.pytest_outlier(
        unit_id=unit_id_parameter,
        indata=indata,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        var="X01 X02",
        by="Prov",

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
        {unit_id_dataset} X01  X02  Prov
        R0000030   -1  -18    10
        R0000016    9   12    10
        R0000022   17    5    10
        R0000014    7   13    10
        R0000012    5   19    10
        R0000024    2   15    10
        R0000018    2    1    10
        R0000010   -8  -18    10
        R0000026   14   18    10
        R0000006    9    9    10
        R0000004   19    4    10
        R0000028    3    3    10
        R0000002    2   14    10
        R0000008    1    4    10
        R0000020   -3  -18    10
        R0000025   -1  -13    11
        R0000027   16   10    11
        R0000023    7   17    11
        R0000021   11   14    11
        R0000001    4   19    11
        R0000017   18   11    11
        R0000029   10   10    11
        R0000013    1   12    11
        R0000011    7    2    11
        R0000009    0    5    11
        R0000007    3   16    11
        R0000005  -12   -9    11
        R0000003    1    4    11
        R0000019   18    9    11
        R0000015   -6  -13    11
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    return example_indata.sort_values(by=['Prov'])

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS VALUE
        R0000010     X01    FTI  -8.0
        R0000020     X01    FTE  -3.0
        R0000004     X01    FTE  19.0
        R0000030     X02    FTI -18.0
        R0000010     X02    FTI -18.0
        R0000020     X02    FTI -18.0
        R0000005     X01    FTE -12.0
        R0000025     X02    FTE -13.0
        R0000015     X02    FTE -13.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()