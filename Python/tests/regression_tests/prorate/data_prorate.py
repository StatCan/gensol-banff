"""datasets for prorate regression tests
    The naming convention in this file is pretty loose
    `prorate_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the prorate procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `prorate_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def prorate_indata_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['SOME_CHAR'] = str
    
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1 SOME_NUM SOME_CHAR
        V001 2 4 0 5 22.22 1 abc
        V002 3 5 7 2 25.10 1 abc
        V003 5 0 1 3 15 1 abc
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_indata_02():
    """ident numeric, should be character"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        001 2 4 0 5 22
    """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_indata_03():
    """valid indata dataset"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        001 2 4 0 5 22
    """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_indata_04():
    """valid indata dataset"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        V001 2 4 0 5 22
    """),
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_indata_05():
    """valid indata dataset"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident prov area VA VB VC VD TOT1
        V001 2 B 12 8 0 5 22
        V002 2 B  13 5 7 9 25
        V003 2 A 5 0 5 3 15
        V004 2 A 12 8 0 5 18
        V005 1 A 13 5 7 9 20
        V006 1 A 5 0 5 3 100
    """),
        sep=r'\s+', 
        dtype=var_types
    )

##### INSTATUS #####
@pytest.fixture
def prorate_instatus_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IMP
        V001 VB FTI
        V001 VD O
        V002 VA FTI
        V002 VB HTI
        V002 VC IC2
        V003 VA I
        V003 VB I
        V003 VC HPM
        V003 VD " "
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_instatus_02():
    """ valid instatus dataset """
    var_types = defaultdict(lambda: str) # default to character
    return pd.read_csv(StringIO("""
        ident fieldid status
        001 VA IMPUTED
        001 VB FTI
        001 VD ORIGINAL
    """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_instatus_02():
    """ident is numeric, should be character"""
    var_types = defaultdict(lambda: str) # default to character
    var_types['ident'] = np.float64
    return pd.read_csv(StringIO("""
        ident fieldid status
        001 VA IMPUTED
        001 VB FTI
        001 VD ORIGINAL
    """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_instatus_03():
    """valid instatus"""
    var_types = defaultdict(lambda: str) # default to character
    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IMPUTED
        V001 VB FTI
        V001 VD ORIGINAL
    """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def prorate_instatus_04():
    """valid instatus"""
    var_types = defaultdict(lambda: str) # default to character
    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IDE
        V001 VB FTI
        V001 VD O
        V002 VA FTI
        V002 VB HTI
        V002 VC IC2
        V003 VA I
        V003 VB I
        V003 VC HPM
        V003 VD " "
        V004 VA IDE
        V004 VB FTI
        V004 VD O
        V005 VA FTI
        V005 VB HTI
        V005 VC IC2
        V006 VA I
        V006 VB I
        V006 VC HPM
        V006 VD " "
    """), 
        sep=r'\s+', 
        dtype=var_types
    )