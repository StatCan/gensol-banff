"""datasets for editstat regression tests
    The naming convention in this file is pretty loose
    `editstat_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the editstat procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `editstat_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def editstat_indata_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    #var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        x1 x2 x3
        4 3 2
        4 3 NaN
        6 3 2
        6 3 NaN
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_indata_02():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    #var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
            A B C D E F G H TOT1 TOT2 TOT
            10 0 -4 0 5 6 15 0 6 26 30
            0 10 20 1 5 7 0 12 31 24 55
            25 55 0 -2 -2 20 14 30 80 62 NaN
            -5 15 12 NaN 12 17 -3 19 NaN 45 71
            15 -4 -5 2 4 -1 12 -20 8 -5 3
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outedits_reduced_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['EDIT_EQUATION'] = 'string'
    
    return pd.read_csv(StringIO("""EDITID,EDIT_EQUATION
7,- A <= 0 
8,- B <= 0 
9,- C <= 0 
11,- E <= 0 
12,- F <= 0 
13,- G <= 0 
14,- H <= 0 
0,A + E <= 20 
1,B + C + F + H <= 50 
2,- D <= -1 
3,G <= 30 
4,- TOT + TOT1 + TOT2  = 0 
5,E + F + G + H - TOT2  = 0 
6,A + B + C + D - TOT1  = 0 """), 
        #sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outedit_status_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric

    return pd.read_csv(StringIO("""EDITID,OBS_PASSED,OBS_MISSED,OBS_FAILED
7,4,0,1
8,4,0,1
9,3,0,2
11,4,0,1
12,4,0,1
13,4,0,1
14,4,0,1
0,4,0,1
1,3,0,2
2,2,1,2
3,5,0,0
4,2,2,1
5,5,0,0
6,3,1,1
"""), 
        #sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outk_edits_status_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric

    return pd.read_csv(StringIO("""K_EDITS,OBS_PASSED,OBS_MISSED,OBS_FAILED
0,0,3,1
1,0,1,0
2,0,0,0
3,0,1,2
4,0,0,1
5,0,0,1
6,0,0,0
7,0,0,0
8,2,0,0
9,0,0,0
10,1,0,0
11,1,0,0
12,0,0,0
13,0,0,0
14,1,0,0
"""), 
        #sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outglobal_status_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric

    return pd.read_csv(StringIO("""OBS_PASSED,OBS_MISSED,OBS_FAILED,OBS_TOTAL
1,0,4,5
"""), 
        #sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outedit_applic_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['FIELDID'] = 'string'

    return pd.read_csv(StringIO("""FIELDID,EDIT_APPLIC_PASSED,EDIT_APPLIC_MISSED,EDIT_APPLIC_FAILED,EDIT_APPLIC_NOTINVOLVED,EDITS_INVOLVED
A,11,1,3,55,3
B,10,1,4,55,3
C,9,1,5,55,3
D,5,2,3,60,2
E,13,0,2,55,3
F,12,0,3,55,3
G,14,0,1,55,3
H,12,0,3,55,3
TOT,2,2,1,65,1
TOT1,5,3,2,60,2
TOT2,7,2,1,60,2
"""), 
        #sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def editstat_outvars_role_01():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['FIELDID'] = 'string'

    return pd.read_csv(StringIO("""FIELDID,OBS_PASSED,OBS_MISSED,OBS_FAILED,OBS_NOT_APPLICABLE
A,1,0,2,2
B,1,0,3,1
C,1,0,4,0
D,1,0,2,2
E,1,0,1,3
F,1,0,3,1
G,1,0,1,3
H,1,0,3,1
TOT,1,0,1,3
TOT1,1,0,2,2
TOT2,1,0,1,3
"""), 
        #sep=r'\s+', 
        dtype=var_types
    )