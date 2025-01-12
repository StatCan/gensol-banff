"""datasets for errorloc regression tests
    The naming convention in this file is pretty loose
    `errorloc_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the errorloc procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `errorloc_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def errorloc_indata_01():
    """seemingly valid indata datasets
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    
    return pd.read_csv(StringIO("""
        REC AREA X1 X2 X3
        101   1   10   40   100
        102   1   4   40   110
        103   1   16   40   128
        104   1   10   29   152
        105   1   10   51   98
        106   1   4   51   136
        201   2   10   40   100
        202   2   4   40   110
        203   2   16   40   128
        204   2   10   29   152
        205   2   10   51   98
        206   2   4   51   136
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_02():
    """seemingly valid indata datasets
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    var_types['AREA'] = str
    
    return pd.read_csv(StringIO("""
        REC AREA X1 X2 X3
        101   1   10   40   100
        102   1   4   40   110
        103   1   16   40   128
        104   1   10   29   152
        105   1   10   51   98
        106   1   4   51   136
        201   2   10   40   100
        202   2   4   40   110
        203   2   16   40   128
        204   2   10   29   152
        205   2   10   51   98
        206   2   4   51   136
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_03():
    """Simple valid indata file, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    var_types['SOME_CHAR'] = str
    
    return pd.read_csv(StringIO("""
        REC X Y randkey SOME_NUM SOME_CHAR
        1  1  1  0.00  1 abc
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_05():
    """REC numeric, should be character
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    
    return pd.read_csv(StringIO("""
        REC AREA X1 X2 X3
        101   1   10   40   100
        102   1   4   40   110
        103   1   16   40   128
        104   1   10   29   152
        105   1   10   51   98
        106   1   4   51   136
        201   2   10   40   100
        202   2   4   40   110
        203   2   16   40   128
        204   2   10   29   152
        205   2   10   51   98
        206   2   4   51   136
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_06():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['REC'] = str
    var_types['A1'] = str
    var_types['A2'] = str
    
    return pd.read_csv(StringIO("""
        REC A1 A2 X1 X2 X3
        101   1   1   10   40   100
        102   1   1   4   40   110
        103   1   1   16   40   128
        104   1   2   10   29   152
        105   1   2   10   51   98
        106   1   2   4   51   136
        201   2   1   10   40   100
        202   2   1   4   40   110
        203   2   1   16   40   128
        204   2   2   10   29   152
        205   2   2   10   51   98
        206   2   2   4   51   136
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_07():
    indata = pd.DataFrame({
        'REC': ['1', '2', '3', '4'],
        'X1': [10, 10, 10, 10],
        'X2': [20, 10, 30, 400],
        'TOT': [30, 20, 40, 50]
    })

    return indata

##### OUTSTATUS #####
@pytest.fixture
def errorloc_outstatus_01():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    # BEWARE: values in `VALUE` differ from the associated value in `indata` due to 
    # pre-C implementation of `instatus`
    return pd.read_csv(StringIO("""
        REC FIELDID STATUS  VALUE
        4 X2 FTI 400
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_outstatus_02():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    # BEWARE: values in `VALUE` differ from the associated value in `indata` due to 
    # pre-C implementation of `instatus`
    return pd.read_csv(StringIO("""
        REC FIELDID STATUS  VALUE
        4 TOT FTI 50
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTREJECT #####