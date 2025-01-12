import banff.testing as banfftest
import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

@pytest.fixture
def donorimp_indata_02():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = 'string'
    var_types['area'] = 'string'
    var_types['toexcl'] = 'string'

    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1 500 120 150 80 150 50 " "
        REC02 A1 750 200 170 130 250 75 " "
        REC04 A1 1000 150 250 350 250 100 " "
        REC05 A2 1050 200 225 325 300 100 E
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_01():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: 'string') # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3    IPR 
        REC04 Q2    FTE 
        REC04 Q4    FTI 
        REC04 STAFF FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.mark.m_pass_as_is
def test_donorimp_a03_a(capfd, donorimp_indata_02, donorimp_instatus_01):
    """The key variable is mandatory. If not specified, DonorImp should not proceed.
    Error messages should be printed to the log file.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_01,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        # unit_id="ident", # intentionally commented out
        must_match="staff",
        data_excl_var="toexcl",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: unit_id is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()