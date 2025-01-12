"""Tests in this file are for demonstration purposes.  See each test function's docstring for details
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_auto_pass
def test_auto_remove_by_vars(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_02, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """Demonstrate automatic removal of BY variables from expected output datasets
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff",
        capture=None, # this suppresses procedure console output

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=None, # disable this check
        expected_error_count=None, # disable this check
        expected_warning_count=None, # disable this check
        expected_outdata=donorimp_outdata_02,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

def test_find_value_difference(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_02, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """Demonstrate identification of differing values in otherwise equal datasets
    """

    donorimp_outdata_02.loc[0, 'Q4'] = 100

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff",
        capture=None, # this suppresses procedure console output

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=None, # disable this check
        expected_error_count=None, # disable this check
        expected_warning_count=None, # disable this check
        expected_outdata=donorimp_outdata_02,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

def test_find_size_difference(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_02, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """Demonstrate identification of missing column or row
    """

    donorimp_outmatching_fields_01.drop(columns=['STATUS'], inplace=True)
    donorimp_outmatching_fields_01.drop(index=0, inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff",
        capture=None, # this suppresses procedure console output

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=None, # disable this check
        expected_error_count=None, # disable this check
        expected_warning_count=None, # disable this check
        expected_outdata=donorimp_outdata_02,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

def test_case_difference(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_02, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """Demonstrate case sensitivity for column names
    """

    donorimp_outstatus_01.rename(columns={'STATUS': 'status'}, inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff",
        capture=None, # this suppresses procedure console output

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=None, # disable this check
        expected_error_count=None, # disable this check
        expected_warning_count=None, # disable this check
        expected_outdata=donorimp_outdata_02,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

def test_type_difference(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_03, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """Demonstrate detection of datatype difference 
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff",
        capture=None, # this suppresses procedure console output

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=None, # disable this check
        expected_error_count=None, # disable this check
        expected_warning_count=None, # disable this check
        expected_outdata=donorimp_outdata_03,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`


@pytest.fixture
def donorimp_indata_04():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   "."
        REC02 A1  750  200  170  130  250  100   "."
        REC04 A1 1000  150  250  350  250  100   "."
        REC05 A2 1050  200  225  325  300  100   "E"
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_02():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 IPR 
        REC04 Q2 FTE 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdata_02():
    """CHANGES
     - In SAS this included the "area" and "staff" variables
        however these are absent in Python.  
        Why? SAS includes BY variables in outdata, unnecessarily
     - use "REC04" not "REC02", the .docx is wrong 
     """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str

    return pd.read_csv(StringIO("""
        ident total Q1  Q2  Q3  Q4 area staff
        REC04 1000  150 250 130 250 A1 100
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdata_03():
    """CHANGES
     - In SAS this included the "area" and "staff" variables
        however these are absent in Python.  
        Why? SAS includes BY variables in outdata, unnecessarily
     - use "REC04" not "REC02", the .docx is wrong 
     """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['Q1'] = str

    return pd.read_csv(StringIO("""
        ident total Q1  Q2  Q3  Q4 area staff
        REC04 1000  150.0 250 130 250 A1 100
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outstatus_01():
    var_types = defaultdict(lambda: str) # default to character
    var_types['staff'] = np.float64
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO("""
        area staff ident FIELDID STATUS  VALUE
        A1 100 REC04 Q3 IDN   130.0
        A1 100 REC04 Q4 IDN   250.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outmatching_fields_01():
    var_types = defaultdict(lambda: str) # default to character
    var_types['staff'] = np.float64

    return pd.read_csv(StringIO("""
        area staff ident FIELDID STATUS
        A1 100 REC04 Q3 IDN 
        A1 100 REC04 Q4 IDN 
        A1 100 REC04 Q1 MFS 
        A1 100 REC04 Q2 MFS 
        A1 100 REC04 total MFS 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )