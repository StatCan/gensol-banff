"""datasets for massimpu regression tests
    The naming convention in this file is pretty loose
    `massimpu_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the massimpu procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `massimpu_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def massimpu_indata_01():
    """`ident` is numeric when it should be character
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff
        01   1   500   120   150    80   150    50
        02   1   750   200   170   130   250    75
        03   1   900   NaN  NaN  NaN  NaN   90
        04   2  1000 150   250   350   250   100
        05   2  1050   200   225   325   300   100
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def massimpu_indata_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['SOME_CHAR'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff SOME_NUM SOME_CHAR
        01   1   500   120   150    80   150    50 1 abc
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def massimpu_indata_03():
    """valid looking input dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff
        01   1   500   120   150    80   150    50
        02   1   750   200   170   130   250    75
        03   1   900   NaN  NaN  NaN  NaN   90
        04   2  1000 150   250   350   250   100
        05   2  1050   200   225   325   300   100
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def massimpu_indata_04():
    """`Q1` is character when it should be numeric
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['Q1'] = str
    
    return pd.read_csv(StringIO("""
        ident area total Q1 Q2 Q3 Q4 staff
        01   1   500   120   150    80   150    50
        02   1   750   200   170   130   250    75
        03   1   900   NaN  NaN  NaN  NaN   90
        04   2  1000 150   250   350   250   100
        05   2  1050   200   225   325   300   100
        """), 
        sep=r'\s+', 
        dtype=var_types
    )
##### OUTSTATUS #####

##### OUTREJECT #####