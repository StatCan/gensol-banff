"""Verify that consecutive spaces (NOT TECHNICALLY "WHITESPACE") in variable lists properly ignored"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_varlist_whitespace_a(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    """Nominal whitespace: 
    leading whitespace: none
    delimiting whitespace: single space
    trailing whitespace: none
    """
    _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01,
                        must_match="total staff")

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_varlist_whitespace_b(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    """Leading whitespace: 
    leading whitespace: multiple spaces
    delimiting whitespace: single space
    trailing whitespace: none
    """
    _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01,
                        must_match="   total staff")

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_varlist_whitespace_c(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    """multiple delimiting whitespace: 
    leading whitespace: none
    delimiting whitespace: multiple spaces
    trailing whitespace: none
    """
    _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01,
                        must_match="total   staff")

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_varlist_whitespace_d(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    """trailing whitespace: 
    leading whitespace: none
    delimiting whitespace: single space
    trailing whitespace: multiple spaces
    """
    _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01,
                        must_match="total staff  ")

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_varlist_whitespace_e(capfd, massimpu_indata_04, massimpu_outdonormap_01):
    """terrible whitespace: 
    leading whitespace: multiple spaces
    delimiting whitespace: multiple spaces
    trailing whitespace: multiple spaces
    """
    _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01,
                        must_match="    total      staff                ")

def test_varlist_whitespace_f(capfd, massimpu_indata_04):
    """only whitespace"""
    banfftest.pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_04,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="    ",
        random=True,
        accept_negative=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains_exact="NOTE: must_match specified without a variable list.",
    )

def _varlist_whitespace(capfd, massimpu_indata_04, massimpu_outdonormap_01, must_match):
    """Helper function for running calls differing only in `must_match` argument.  """
    banfftest.pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_04,
        min_donors=2,
        percent_donors=10,
        unit_id="ident",
        must_impute="Q1 Q2 Q3 Q4",
        must_match=must_match,
        random=True,
        accept_negative=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains_exact="NOTE: must_match = total staff",
        expected_outdonormap=massimpu_outdonormap_01
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()

import pandas as pd
from io import StringIO
from collections import defaultdict
import numpy as np

@pytest.fixture
def massimpu_indata_04():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
    01   1   500   120   150    80   150    50
    02   1   750   200   170   130   250    -75
    03   1   900   NaN  NaN  NaN  NaN   -90
    04   2  1000 150   250   350   250   100
    05   2  1050   NaN  NaN  NaN  NaN  NaN
    """), sep=r'\s+', dtype={'ident': str})

@pytest.fixture
def massimpu_outdonormap_01():
    d = defaultdict(lambda: np.float64)
    d['RECIPIENT'] = str
    d['DONOR'] = str

    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    03         02     1                  NaN
    05         04     1                  NaN
     """), sep=r'\s+', dtype=d)