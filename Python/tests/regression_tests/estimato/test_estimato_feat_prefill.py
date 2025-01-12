"""Proc estimator requires its `indata`, `instatus`, `indata_hist`, and `instatus_hist` 
files to be sorted in ascending order of `by` variables
- when `instatus` has no `by` variables, there is no sort requirement
- when `instatus_hist` has no `by` variables, there is no sort requirement

When `prefill_by_vars=True`, `by` variables should be automatically added to `instatus` and `instatus_hist`
and the datasets should automatically be sorted correctly.  

This tedious test file focuses on edge cases such as when
    - status has records which don't occur in data: leading to missing BY values on status
        - when data does not contain any missing by values
        - when data does contain missing by values

NOTE ABOUT DATA: 
Data for this test is pretty messy, so it's kept local to this file
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_prefill_a(
    capfd, 
    estimato_indata_prefill, estimato_instatus_prefill, 
    estimato_indata_hist_prefill, estimato_instatus_hist_prefill, 
    estimato_inalgorithm_03, estimato_inestimator_03,
    estimato_outdata_prefill, estimato_outstatus_prefill
):
    """
    All valid, should pass
    """

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_prefill,
        instatus=estimato_instatus_prefill,
        indata_hist=estimato_indata_hist_prefill,
        instatus_hist=estimato_instatus_hist_prefill,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        expected_error_count=0,
        expected_outdata=estimato_outdata_prefill,
        expected_outstatus=estimato_outstatus_prefill,
        round_data=14,
        #msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_prefill_b(
    capfd, 
    estimato_indata_prefill, estimato_instatus_prefill_noby, 
    estimato_indata_hist_prefill, estimato_instatus_hist_prefill_noby, 
    estimato_inalgorithm_03, estimato_inestimator_03,
    estimato_outdata_prefill, estimato_outstatus_prefill
):
    """
    All valid, no BY on status files
    """

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_prefill,
        instatus=estimato_instatus_prefill_noby,
        indata_hist=estimato_indata_hist_prefill,
        instatus_hist=estimato_instatus_hist_prefill_noby,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2+2, # +2 for by var missing from status, status_hist
        expected_error_count=0,
        expected_outdata=estimato_outdata_prefill,
        expected_outstatus=estimato_outstatus_prefill,
        round_data=14,
        msg_list_contains_exact=[
            "WARNING: BY variable area is not on input data set instatus.",
            "NOTE: This procedure can perform faster if the instatus data set contains the by variables.",
            "WARNING: BY variable area is not on input data set instatus_hist.",
            "NOTE: This procedure can perform faster if the instatus_hist data set contains the by variables.",
        ],
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_prefill_c(
    capfd, 
    estimato_indata_prefill, estimato_instatus_prefill_noby, 
    estimato_indata_hist_prefill, estimato_instatus_hist_prefill_noby, 
    estimato_inalgorithm_03, estimato_inestimator_03,
    estimato_outdata_prefill, estimato_outstatus_prefill
):
    """
    All valid, no BY on status files, `prefill_by_vars=True`
    """

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_prefill,
        instatus=estimato_instatus_prefill_noby,
        indata_hist=estimato_indata_hist_prefill,
        instatus_hist=estimato_instatus_hist_prefill_noby,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        expected_error_count=0,
        expected_outdata=estimato_outdata_prefill,
        expected_outstatus=estimato_outstatus_prefill,
        round_data=14,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_prefill_d(
    capfd, 
    estimato_indata_prefill, estimato_instatus_prefill_missing_fkey, 
    estimato_indata_hist_prefill, estimato_instatus_hist_prefill_missing_fkey, 
    estimato_inalgorithm_03, estimato_inestimator_03,
    estimato_outdata_prefill, estimato_outstatus_prefill
):
    """
    All valid, no BY on status files, `prefill_by_vars=True`
    Status contains records that doesn't exist on data, so their by values are missing
    That shouldn't cause any issues
    """

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_prefill,
        instatus=estimato_instatus_prefill_missing_fkey,
        indata_hist=estimato_indata_hist_prefill,
        instatus_hist=estimato_instatus_hist_prefill_missing_fkey,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        expected_error_count=0,
        expected_outdata=estimato_outdata_prefill,
        expected_outstatus=estimato_outstatus_prefill,
        round_data=14,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_prefill_e(
    capfd, 
    estimato_indata_prefill_missing_by, estimato_instatus_prefill_missing_fkey, 
    estimato_indata_hist_prefill, estimato_instatus_hist_prefill_missing_fkey, 
    estimato_inalgorithm_03, estimato_inestimator_03,
    estimato_outdata_prefill_2, estimato_outstatus_prefill_2
):
    """
    All valid, no BY on status files, `prefill_by_vars=True`
    Both data and status have record 'A0' which has <missing> 'area' (missing BY value).  
    Status contains a record that doesn't exist on data, so its 'area' will also end up as <missing>
    None of this should cause issues.  
    The by group "area=<missing>" should be examined during processing.  
    """

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_prefill_missing_by,
        instatus=estimato_instatus_prefill_missing_fkey,
        indata_hist=estimato_indata_hist_prefill,
        instatus_hist=estimato_instatus_hist_prefill_missing_fkey,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
        expected_error_count=0,
        expected_outdata=estimato_outdata_prefill_2,
        expected_outstatus=estimato_outstatus_prefill_2,
        round_data=14,
        msg_list_contains="NOTE: The above message was for the following by group: area=<missing>"
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()


#### DATASETS ####
import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`
import banff.testing

@pytest.fixture
def estimato_indata_prefill():
    """
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 NaN 1 1 1 A
        A3 3 3 NaN 3 A
        A4 4 4 4 NaN A
        A2 2 NaN 2 2 D
        A5 NaN 5 5 5 D
        A6 6 NaN 6 6 D
        A7 7 7 NaN 7 D
        A8 8 8 8 NaN D
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_indata_prefill_missing_by():
    """
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 NaN 1 1 1 A
        A3 3 3 NaN 3 A
        A4 4 4 4 NaN A
        A2 2 NaN 2 2 D
        A5 NaN 5 5 5 D
        A6 6 NaN 6 6 D
        A7 7 7 NaN 7 D
        A8 8 8 8 NaN D
        A0 NaN 1 1 1 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_outdata_prefill():
    """
    """
    return banff.testing.PAT_from_string("""
              s           n n n n                   
            ident         V    X     Y     Z
            A4  4.000000  4.0   4.0   5.0
            A1  2.6666666666666666666666667  1.0   1.0   1.0
            A3  3.000000  3.0   6.0   3.0
            A7  7.000000  7.0  14.0   7.0
            A8  8.000000  8.0   8.0  13.0
            A5  6.6666666666666666666666667  5.0   5.0   5.0
            A2  2.000000  6.5   2.0   2.0
            A6  6.000000  6.5   6.0   6.0
    """)

@pytest.fixture
def estimato_outdata_prefill_2():
    """
    """
    return banff.testing.PAT_from_string("""
                s n n n n
            ident         V    X     Y     Z
            A4  4.000000  4.0   4.0   5.0
            A1  2.6666666666666666666666667  1.0   1.0   1.0
            A3  3.000000  3.0   6.0   3.0
            A7  7.000000  7.0  14.0   7.0
            A8  8.000000  8.0   8.0  13.0
            A5  6.6666666666666666666666667  5.0   5.0   5.0
            A2  2.000000  6.5   2.0   2.0
            A6  6.000000  6.5   6.0   6.0
            A0  1.000000  1.0   1.0   1.0
    """)

@pytest.fixture
def estimato_indata_hist_prefill():
    """dataset suitable for testing `presort` option"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    dataset = pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 4 4 4 4 A
        A2 8 3 3 3 A
        A3 2 2 2 2 A
        A4 1 1 1 1 A
        A5 3 8 8 8 D
        A6 7 7 7 7 D
        A7 6 6 6 6 D
        A8 5 5 5 5 D
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    return dataset

@pytest.fixture
def estimato_instatus_prefill():
    """
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status area
        A1  V  FTI A
        A3  Y  FTI A
        A4  Z  FTI A
        A2  X  FTI D
        A5  V  FTI D
        A6  X  FTI D
        A7  Y  FTI D
        A8  Z  FTI D
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_prefill_noby(estimato_instatus_prefill):
    """
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A1  V  FTI
        A3  Y  FTI
        A4  Z  FTI
        A2  X  FTI
        A5  V  FTI
        A6  X  FTI
        A7  Y  FTI
        A8  Z  FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_prefill_missing_fkey(estimato_instatus_prefill):
    """
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A0  V  FTI
        A1  V  FTI
        A3  Y  FTI
        A4  Z  FTI
        A2  X  FTI
        A5  V  FTI
        A6  X  FTI
        A7  Y  FTI
        A8  Z  FTI
        A9  Z  FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_outstatus_prefill():
    """
    """
    return banff.testing.PAT_from_string("""
                                        s s s n
            ident FIELDID STATUS      VALUE
            A4       Z   IAL3   5.000000
            A1       V   ICAM   2.6666666666666666666666667
            A3       Y   IAL2   6.000000
            A7       Y   IAL2  14.000000
            A8       Z   IAL3  13.000000
            A5       V   ICAM   6.6666666666666666666666667
            A2       X    IPM   6.500000
            A6       X    IPM   6.500000
    """)

@pytest.fixture
def estimato_outstatus_prefill_2():
    """
    """
    return banff.testing.PAT_from_string("""
                                        s s s n
            ident FIELDID STATUS      VALUE
            A4       Z   IAL3   5.000000
            A1       V   ICAM   2.6666666666666666666666667
            A3       Y   IAL2   6.000000
            A7       Y   IAL2  14.000000
            A8       Z   IAL3  13.000000
            A5       V   ICAM   6.6666666666666666666666667
            A2       X    IPM   6.500000
            A6       X    IPM   6.500000
            A0       V   ICAM   1.000000
    """)

@pytest.fixture
def estimato_instatus_hist_prefill():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status area
        A2 X   IDN A
        A4 X   IDE A
        A6 X   IDN D
        A8 X   IDE D
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_hist_prefill_noby():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A2 X   IDN
        A4 X   IDE
        A6 X   IDN
        A8 X   IDE
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_hist_prefill_missing_fkey():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A0 X   IDN
        A2 X   IDN
        A4 X   IDE
        A6 X   IDN
        A8 X   IDE
        A9 X   IDE
        """), 
        sep=r'\s+', 
        dtype=var_types
    )
