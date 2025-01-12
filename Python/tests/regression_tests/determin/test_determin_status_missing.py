"""
when `by` groups are specified, input data (and data_hist) must be sorted in ascending order of
the `by` groups.  When input status file(s) (instatus, instatus_hist) include `by` variables, they
must be sorted as well.  

BY values can include "missing", which corresponds to `NAN` for floating point type in C, `null` in JSON, 
and `None` in Python.  When sorted in `pandas`, "missing" values go last.  

These tests validate that missing BY values are handled correctly in a variety of niche circumstances

NOTE ABOUT DATA: 
Data for this test is pretty messy, so it's kept local to this file
"""

import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_a(capfd, determin_indata_missing_fkey_01, determin_instatus_missing_fkey_01):
    """character BY group, status has missing value when `prefill_by_vars=True`
    should run without error
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_01,
        instatus=determin_instatus_missing_fkey_01,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=True,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=None,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_b(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_01):
    """integer BY group, status has missing value when `prefill_by_vars=True`
    should run without error
    NOTE: Due to extra record (REC12) in status, prefill will 
        - have a missing `null` value for REC12
        - convert INT to FLOAT for status' BY values
            - because INT has no NaN (Not a Number) value
        C code must handle mix of float and int for by, treating both equally as numeric
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_01,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=True,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=None,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_c(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_02):
    """Buggy Behaviour when some unsorted secondary datasets
    In this example, indata has by groups '24' and '30' and records 1 through 11.  
        Status has an additional record, 12, which has 'null' for its by group.  
        In Python, 'null' values appear last when sorted in ascending order.  
        In the status file, record 12 and its 'null' by group appear first.  
        C code sees `null` as the first BY group and assumes that there must be no data
        for groups '24' or '30', since those values would have appeared earlier.  
        As there is no 'null' by group in the indata file, the invalid sort order 
        of status is never detected.  
        The same is true in sas-based banff 2.8.2 and earlier.  
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_02,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=False,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=None,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_d(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_03):
    """
    Data has no 'REC12'.  Status has 'REC12' with a missing value for the BY group.  
    - REC12 appears second on status, right after a record with by group 24
    RESULT:
        Both SAS and Python assume that status has no more data for groups 24 or 30 (from indata).  
        Instatus' sort order violation isn't detected, since no data from the <missing> by group appears in indata
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_03,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=False,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=None,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_e(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_04):
    """
    data has no 'REC12'.  Status has 'REC12' with BY value 23.  
    - REC12 appears second on status, right after a record with group 24
        - this is invalid, it is detected when C searches for statuses for other records in group 24
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_04,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=False,
        presort=False,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set instatus is not sorted in ascending sequence. The current by group has pRoV = 24.000000 and the next by group has pRoV = 23.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_e_pf(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_04):
    """
    Same, but with `prefill_by_vars=True`.  
    should pass
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_04,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=True,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_f(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_05):
    """
    invalid sort order should be detected
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_05,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=False,
        presort=False,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set instatus is not sorted in ascending sequence. The current by group has pRoV = 25.000000 and the next by group has pRoV = 23.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_status_missing_f_pf(capfd, determin_indata_missing_fkey_02, determin_instatus_missing_fkey_05):
    """
    Same, but with `prefill_by_vars=True`.  
    should pass
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_missing_fkey_02,
        instatus=determin_instatus_missing_fkey_05,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="IdEnT",
        by="pRoV",
        prefill_by_vars=True,
        trace=True,
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()

#### DATA ####

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

@pytest.fixture
def determin_instatus_missing_fkey_01():
    """valid instatus dataset, contains 'REC12' which doesn't appear in corresponding
    data dataset.  This should be allowed
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        fieldid status IDent
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
        Q4 FTI REC12
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def determin_instatus_missing_fkey_02():
    """invalid instatus dataset, should be sorted by `prov`, however
    the missing (`null`) value for REC12 appears first when it should appear last.  
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['prov'] = np.float64

    return pd.read_csv(StringIO("""
        fieldid status IDent prov
        Q4 FTI REC12 
        Q3 FTI REC06 24
        Q4 FTI REC07 30
        Q2 FTI REC08 30
        Q3 FTI REC08 30
        Q4 FTI REC09 30
        staff FTI REC09 30
        Q1 FTI REC10 30
        Q2 FTI REC10 30
        Q3 FTI REC10 30
        Q4 FTI REC10 30
        Q4 FTI REC11 30
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def determin_instatus_missing_fkey_03():
    """invalid instatus dataset, should be sorted by `prov`, however
    the missing (`null`) value for REC12 appears second when it should appear last.  
    This leads procedures to believe that there are no further groups after record 12
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['prov'] = np.float64

    return pd.read_csv(StringIO("""
        fieldid status IDent prov
        Q3 FTI REC06 24
        Q4 FTI REC12 
        Q4 FTI REC07 30
        Q2 FTI REC08 30
        Q3 FTI REC08 30
        Q4 FTI REC09 30
        staff FTI REC09 30
        Q1 FTI REC10 30
        Q2 FTI REC10 30
        Q3 FTI REC10 30
        Q4 FTI REC10 30
        Q4 FTI REC11 30
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def determin_instatus_missing_fkey_04():
    """
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['prov'] = np.float64

    return pd.read_csv(StringIO("""
        fieldid status IDent prov
        Q3 FTI REC06 24
        Q4 FTI REC12 23
        Q4 FTI REC07 30
        Q2 FTI REC08 30
        Q3 FTI REC08 30
        Q4 FTI REC09 30
        staff FTI REC09 30
        Q1 FTI REC10 30
        Q2 FTI REC10 30
        Q3 FTI REC10 30
        Q4 FTI REC10 30
        Q4 FTI REC11 30
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def determin_instatus_missing_fkey_05():
    """
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['prov'] = np.float64

    return pd.read_csv(StringIO("""
        fieldid status IDent prov
        Q3 FTI REC06 24
        Q4 FTI REC13 25
        Q4 FTI REC12 23
        Q4 FTI REC07 30
        Q2 FTI REC08 30
        Q3 FTI REC08 30
        Q4 FTI REC09 30
        staff FTI REC09 30
        Q1 FTI REC10 30
        Q2 FTI REC10 30
        Q3 FTI REC10 30
        Q4 FTI REC10 30
        Q4 FTI REC11 30
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def determin_indata_missing_fkey_01():
    """valid indata dataset, character BY group
    """
    var_types = defaultdict(lambda: np.int64) # default to numeric
    var_types['ident'] = str
    var_types['prov'] = str

    return pd.read_csv(StringIO("""
        ident TOTAL Q1 Q2 Q3 Q4 staff prov
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
        sep=r'\s+',
        dtype=var_types
    )

@pytest.fixture
def determin_indata_missing_fkey_02():
    """valid indata dataset, character BY group
    """
    var_types = defaultdict(lambda: np.int64) # default to numeric
    var_types['ident'] = str

    return pd.read_csv(StringIO("""
        ident TOTAL Q1 Q2 Q3 Q4 staff prov
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
        sep=r'\s+',
        dtype=var_types
    )
