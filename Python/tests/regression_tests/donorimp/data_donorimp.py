"""datasets for donorimp regression tests
    The naming convention in this file is pretty loose
    `donorimp_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the donorimp procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `donorimp_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`
import banff.testing

##### INDATA #####
@pytest.fixture
def donorimp_indata_01():
    """`REC` is numeric when it should be character
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        01 A1 500 120 150 80 150 50 " "
        02 A1 750 200 170 130 250 75 " "
        04 A1 1000 150 250 350 250 100 " "
        05 A2 1050 200 225 325 300 100 E
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_02():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
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
def donorimp_indata_03():
    """valid indata dataset containing nonsense exclusion values
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   "."
        REC02 A1  750  200  170  130  250   75   "%"
        REC04 A1 1200  150  250  350  250  100   "."
        REC05 A2 1050  200  225  325  300  100   "5"
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

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
def donorimp_indata_presort():
    """dataset suitable for testing `presort` option"""
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
        REC05 A1 1000  150  250  350  250  100   "."
        REC06 A1 1000  150  250  350  250  100   "."
        REC07 A1 1000  150  250  350  250  100   "."
        REC08 A1 1000  150  250  350  250  100   "."
        REC09 A1 1000  150  250  350  250  100   "."
        REC10 A2 1050  200  225  325  300  100   "E"
        REC11 A2  500  120  150   80  150   50   "."
        REC12 A2  750  200  170  130  250  100   "."
        REC14 A2 1000  150  250  350  250  100   "."
        REC15 A1 1000  150  250  350  250  100   "."
        REC16 A1 1000  150  250  350  250  100   "."
        REC17 A1 1000  150  250  350  250  100   "."
        REC18 A1 1000  150  250  350  250  100   "."
        REC19 A1 1000  150  250  350  250  100   "."
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_prefill():
    """dataset suitable for testing `presort` option"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['iDent'] = str
    var_types['aRea'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        iDent aRea total Q1 Q2 Q3 Q4 staff toexcl
        REC00 A2  500  120  150   80  150   50   "."
        REC01 A1  500  120  150   80  150   50   "."
        REC02 A1  750  200  170  130  250  100   "."
        REC04 A1 1000  150  250  350  250  100   "."
        REC05 A1 1000  150  250  350  250  100   "."
        REC06 A1 1000  150  250  350  250  100   "."
        REC07 A1 1000  150  250  350  250  100   "."
        REC08 A1 1000  150  250  350  250  100   "."
        REC09 A1 1000  150  250  350  250  100   "."
        REC10 A2 1050  200  225  325  300  100   "E"
        REC11 A2  500  120  150   80  150   50   "."
        REC12 A2  750  200  170  130  250  100   "."
        REC14 A2 1000  150  250  350  250  100   "."
        REC15 A1 1000  150  250  350  250  100   "."
        REC16 A1 1000  150  250  350  250  100   "."
        REC17 A1 1000  150  250  350  250  100   "."
        REC18 A1 1000  150  250  350  250  100   "."
        REC19 A1 1000  150  250  350  250  100   "."
        """), 
        sep=r'\s+', 
        dtype=var_types
    )


@pytest.fixture
def donorimp_indata_05():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A2  500  120  150   80  150   50   "."
        REC04 A1  750  200  170  130  250   75   "."
        REC02 A1 1000  150  250  350  250  100   "."
        REC05 A2 1050  200  225  325  300  100   "E"
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_06():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   " "
        REC02 A1  750  200  170  130  250   75   " "
        REC04 A1  650  150  250  350  250  100   " "
        REC05 A2 1050  200  225  325  300  100   E
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_07():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   " "
        REC02 A1  750  200  170  360  250   75   " "
        REC04 A1 1000  150  250  350  250  100   " "
        REC05 A2 1050  200  225  325  300  100   E
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_08():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   " "
        REC02 A1  750  200  170  130  250   75   " "
        REC04 A1 1000  150  250  350  250  100   " "
        REC05 A2 1050  200  225  325  300  100   E
        REC06 A1  500  120  150   80  150   50   " "
        REC07 A1  750  200  170  130  250   75   " "
        REC08 A1 1000  150  250  350  250  100   " "
        REC09 A1  500  120  150   80  150   50   " "
        REC10 A1  750  200  170  130  250   75   " "
        REC11 A1 1000  150  250  350  250  100   " "
        REC12 A1  500  120  150   80  150   50   " "
        REC13 A1  750  200  170  130  250   75   " "
        REC14 A1 1000  150  250  350  250  100   " "
        REC15 A1  500  120  150   80  150   50   " "
        REC16 A1  750  200  170  130  250   75   " "
        REC17 A1 1000  150  250  350  250  100   " "
        REC18 A1  500  120  150   80  150   50   " "
        REC19 A1  750  200  170  130  250   75   " "
        REC20 A1 1000  150  250  350  250  100   " "
        REC21 A1  500  120  150   80  150   50   " "
        REC22 A1  750  200  170  130  250   75   " "
        REC23 A1 1000  150  250  350  250  100   " "
        REC24 A1  500  120  150   80  150   50   " "
        REC25 A1  750  200  170  130  250   75   " "
        REC26 A1 1000  150  250  350  250  100   " "
        REC27 A1  500  120  150   80  150   50   " "
        REC28 A1  750  200  170  130  250   75   " "
        REC29 A1 1000  150  250  350  250  100   " "
        REC30 A1  500  120  150   80  150   50   " "
        REC31 A1  750  200  170  130  250   75   " "
        REC32 A1 1000  150  250  350  250  100   " "
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_09():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1 500 120 150 80 150 50 " "
        REC02 A1 750 200 170 130 250 75 " "
        REC04 A1 1000 150 250 350 250 100 " "
        REC05 A2 1050 200 225 325 300 100 " "
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_10():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   " "
        REC02 A1  750  200  170  130  -250  75   " "
        REC04 A1 1000  150  250  350  250  100   " "
        REC05 A2 1050  200  225  325  300  100   E
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_11():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    var_types['toexcl'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1  500  120  150   80  150   50   " "
        REC02 A1  750  200  170  130  250   75   " "
        REC04 A1 1000  150  250  350  250  100   " "
        REC05 A2 1050  200  225  325  300  100   " "
        REC11 A1  500  120  150   80  150   50   " "
        REC12 A1  750  200  170  130  250   75   " "
        REC14 A1 1000  150  250  350  250  100   " "
        REC15 A2 1050  200  225  325  300  100   " "
        REC21 A1  500  120  150   80  150   50   " "
        REC22 A1  750  200  170  130  250   75   " "
        REC24 A1 1000  150  250  350  250  100   " "
        REC25 A2 1050  200  225  325  300  100   " "
        REC31 A1  500  120  150   80  150   50   " "
        REC32 A1  750  200  170  130  250   75   " "
        REC34 A1 1000  150  250  350  250  100   " "
        REC35 A2 1050  200  225  325  300  100   " "
        REC41 A1  500  120  150   80  150   50   " "
        REC42 A1  750  200  170  130  250   75   " "
        REC44 A1 1000  150  250  350  250  100   " "
        REC45 A2 1050  200  225  325  300  100   " "
        REC51 A1  500  120  150   80  150   50   " "
        REC52 A1  750  200  170  130  250   75   " "
        REC54 A1 1000  150  250  350  250  100   " "
        REC55 A2 1050  200  225  325  300  100   " "
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_12():
    """valid indata dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    var_types['SOME_CHAR'] = str
    var_types['EXCL_VAR'] = str
    
    return pd.read_csv(StringIO("""
        REC X Y randkey SOME_NUM SOME_CHAR EXCL_VAR
        A 1 2 0.1 1 abc .
        B 2 3 0.8 1 abc .
        C 3 4 0.3 1 abc .
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_13():
    """toexcl is numeric but should be character
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff toexcl
        REC01 A1 500 120 150 80 150 50 1
        REC02 A1 750 200 170 130 250 75 1
        REC04 A1 1000 150 250 350 250 100 1
        REC05 A2 1050 200 225 325 300 100 1
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_indata_14():
    """RANDKEY has invalid values
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    
    return pd.read_csv(StringIO("""
        REC X Y RANDKEY
        101  2  3   0.00
        102  2  3   0.01
        103  2  3   0.99
        104  2  3   1.00
        105  2  3  -0.01
        106  2  3   1.01
        107  2  3   NaN
        201  2  NaN   0.66
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### INSTATUS #####
@pytest.fixture
def donorimp_instatus_01():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

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
def donorimp_instatus_presort(donorimp_indata_presort):
    """dataset suitable for testing `presort` option"""
    var_types = defaultdict(lambda: str) # default to character

    dataset = pd.read_csv(StringIO("""
        ident fieldid status
        REC00 Q3 IPR 
        REC01 Q3 IPR 
        REC04 Q2 FTE 
        REC04 Q3 FTI 
        REC05 Q4 FTI 
        REC06 Q4 FTI 
        REC07 Q4 FTI 
        REC09 Q4 FTI 
        REC12 Q4 FTI 
        REC13 Q4 FTI 
        REC14 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    dataset = pd.merge(left=donorimp_indata_presort[['ident', 'area', 'staff']], right=dataset)
    return dataset

@pytest.fixture
def donorimp_instatus_prefill(donorimp_indata_prefill):
    """dataset suitable for testing `presort` option"""
    var_types = defaultdict(lambda: str) # default to character

    dataset = pd.read_csv(StringIO("""
        iDent fieldid status
        REC00 Q3 IPR 
        REC01 Q3 IPR 
        REC04 Q2 FTE 
        REC04 Q3 FTI 
        REC05 Q4 FTI 
        REC06 Q4 FTI 
        REC07 Q4 FTI 
        REC09 Q4 FTI 
        REC12 Q4 FTI 
        REC13 Q4 FTI 
        REC14 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    dataset = pd.merge(left=donorimp_indata_prefill[['iDent', 'aRea', 'staff']], right=dataset)
    return dataset

@pytest.fixture
def donorimp_instatus_03():
    """valid instatus dataset, NOT sorted by 'ident'
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC04 Q3 IPR 
        REC01 Q2 FTE 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_04():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 IPR 
        REC04 Q2 FTE
        REC04 Q3 FTI
        REC04 Q4 FTI
        REC04 Q1 FTI
        REC04 Q2 FTI
        REC04 total FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_05():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 FTI 
        REC04 Q2 FTE 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_06():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 FTI 
        REC04 Q2 FTE 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        REC05 Q4 FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_07():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 IDE 
        REC02 Q2 IPR 
        REC01 staff IDN 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_08():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 IDE 
        REC02 Q2 IPR 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_09():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC04 Q1 FTI 
        REC04 Q2 FTI 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        REC04 TOTAL FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_10():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 IPE 
        REC02 Q2 FTE 
        REC04 Q3 FTI 
        REC04 Q4 FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_11():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC02 Q3 IDE
        REC12 Q3 IDE
        REC22 Q3 IDE
        REC32 Q3 IDE
        REC42 Q3 IDE
        REC52 Q3 IDE
        REC04 Q3 FTI
        REC04 Q4 FTI
        REC14 Q3 FTI
        REC14 Q4 FTI
        REC24 Q3 FTI
        REC24 Q4 FTI
        REC34 Q3 FTI
        REC34 Q4 FTI
        REC44 Q3 FTI
        REC44 Q4 FTI
        REC54 Q3 FTI
        REC54 Q4 FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_12():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS
        B X FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_13():
    """ident should be character, but is numeric
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['ident'] = np.float64

    return pd.read_csv(StringIO("""
        ident fieldid status
        01 Q3 IPR
        04 Q2 FTE
        04 Q3 FTI
        04 Q4 FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_14():
    """fieldid should be character, but is numeric
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['fieldid'] = np.float64

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 3 IPR
        REC02 2 FTE
        REC04 3 FTI
        REC04 4 FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_15():
    """status should be character, but is numeric
    """
    var_types = defaultdict(lambda: str) # default to character
    var_types['status'] = np.float64

    return pd.read_csv(StringIO("""
        ident fieldid status
        REC01 Q3 1
        REC02 Q2 1
        REC04 Q3 1
        REC04 Q4 1
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_instatus_16():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS
        201 Y FTI 
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTDATA #####
@pytest.fixture
def donorimp_outdata_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str

    return pd.read_csv(StringIO("""
        ident total  Q1  Q2  Q3  Q4
        REC04  1200 150 250 325 300
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

    return pd.read_csv(StringIO("""
        ident total Q1  Q2  Q3  Q4
        REC04 1000  150 250 130 250
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdata_03():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str

    return pd.read_csv(StringIO("""
        ident total  Q1  Q2  Q3  Q4
        REC04  750 200 170 80 150
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTSTATUS #####
@pytest.fixture
def donorimp_outstatus_01():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO("""
        ident FIELDID STATUS  VALUE
        REC04 Q3 IDN  130.0
        REC04 Q4 IDN  250.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outmatching_fields_01():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident FIELDID STATUS
        REC04 Q3 IDN
        REC04 Q4 IDN
        REC04 staff MFU
        REC04 Q1 MFS
        REC04 Q2 MFS
        REC04 total MFS
        """), 
        sep=r'\s+', 
        dtype=var_types
    )
@pytest.fixture
def donorimp_outstatus_02():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO("""
        ident FIELDID STATUS  VALUE
        REC04 Q3 IDN  130.0
        REC04 Q4 IDN  250.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTDONORMAP #####
@pytest.fixture
def donorimp_outdonormap_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RECIPIENT'] = str
    var_types['DONOR'] = str

    return pd.read_csv(StringIO("""
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC01  2                  NaN
        """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdonormap_02():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RECIPIENT'] = str
    var_types['DONOR'] = str

    return pd.read_csv(StringIO("""
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC02  1                  NaN
        """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdonormap_03():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RECIPIENT'] = str
    var_types['DONOR'] = str

    return pd.read_csv(StringIO("""
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC01  1                  NaN
        """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdonormap_04():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RECIPIENT'] = str
    var_types['DONOR'] = str

    return pd.read_csv(StringIO("""
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC32  1                  NaN
        """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdonormap_05():
    """PARTIAL outdonormap dataset, DONOR column excluded"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RECIPIENT'] = str
    var_types['DONOR'] = str

    return pd.read_csv(StringIO("""
        RECIPIENT   NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04       7                  NaN
        REC14       7                  NaN
        REC24       7                  NaN
        REC34       7                  NaN
        REC44       7                  NaN
        REC54       7                  NaN
        """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def donorimp_outdonormap_empty():
    """empty outdonormap
    """

    return banff.testing.PAT_from_string("""
        s s n n
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    """)