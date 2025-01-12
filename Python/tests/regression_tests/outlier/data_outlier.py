"""datasets for outlier regression tests
    The naming convention in this file is pretty loose
    `outlier_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the outlier procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `outlier_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`
import pyarrow as pa

##### INDATA #####
@pytest.fixture
def outlier_indata_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['SOME_CHAR'] = str
    
    return pd.read_csv(StringIO("""
        ident area V1 V2 weight SOME_NUM SOME_CHAR
        R01   1   4   19  1 1 abc
        R02   1   14   5  1 1 abc
        R03   1   18   19  1 1 abc
        R04   1   7   6  1 1 abc
        R05   1   1   1  1 1 abc
        R06   1   5   2  1 1 abc
        R07   1   3   8  1 1 abc
        R08   1   16   14  1 1 abc
        R09   1   2   3  1 1 abc
        R10   1   6   2  1 1 abc
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outlier_indata_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['SOME_CHAR'] = str
    
    return pd.read_csv(StringIO("""
        ident area V1 V2
        R01   1   4   19
        R02   1   14   5
        R03   1   18   19
        R04   1   7   6
        R05   1   1   1
        R06   1   5   2
        R07   1   3   8
        R08   1   16   14
        R09   1   2   3
        R10   1   6   2
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outlier_indata_03():
    """valid input dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident area V1  V2
        R00   2   4   19
        R01   1   4   19
        R02   1   14   5
        R03   1   18   19
        R04   1   7   6
        R05   1   1   1
        R06   1   5   2
        R07   1   3   8
        R08   1   16   14
        R09   1   2   3
        R10   1   6   2
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outlier_indata_04():
    """valid input dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident area V1  V2
        R01   1   4   99
        R02   1   14   5
        R03   1   18   19
        R04   1   7   6
        R05   1   1   1
        R06   1   5   2
        R07   1   3   8
        R08   1   16   14
        R09   1   2   3
        R10   1   6   2
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### INDATA_HIST #####
@pytest.fixture
def outlier_indata_hist_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident area V1 V2
        R01   1   19   4
        R02   1   5   14
        R03   1   19   18
        R04   1   6   7
        R05   1   1   1
        R06   1   2   5
        R07   1   8   3
        R08   1   14   16
        R09   1   3   2
        R10   1   2   6
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outlier_indata_hist_02():
    """dataset with correct columns but 0 rows"""
    sch = pa.schema([
        pa.field('ident', pa.string()),
        pa.field('area', pa.float64()),
        pa.field('V1', pa.float64()),
        pa.field('V2', pa.float64()),
    ])
    # create indata_hist
    indata_hist = pa.Table.from_pylist([], schema=sch)
    return indata_hist