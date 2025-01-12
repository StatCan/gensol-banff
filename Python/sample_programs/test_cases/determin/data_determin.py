"""datasets for determin regression tests"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def determin_indata_05():
    """valid indata dataset"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = 'string'

    return pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        7  50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

##### INSTATUS #####
@pytest.fixture
def determin_instatus_01():
    """valid instatus dataset"""
    var_types = defaultdict(lambda: 'string') # default to character

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS
        3 VA IPR 
        3 VB FTI
        3 VC I
        3 VD I
        3 TOT1 O
        3 VE IC2
        3 VF O
        3 VG IPM
        3 TOT2 ICM
        6 VB IPV
        6 VC IAT
        6 VD O
        6 TOT1 I
        6 VE O
        6 VF IDN
        6 TOT2 IPV
        7 VB IPV
        7 VC ODE
        7 VD IDE
        7 VF O
        7 VG I
        7 TOT2 ICR
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTDATA #####
@pytest.fixture
def determin_outdata_01():
    """Expected outdata"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = 'string'

    return pd.read_csv(StringIO("""
        REC VG VF VE VD VC VB VA TOT2 TOT1 GT
        "3" 38.95 88.23 49.21 10.54 36.21 113.94999999999998863132 16 178.2 176.7 390
        """), 
        sep=r'\s+', 
        dtype=var_types, 
        float_precision='round_trip' # `float_precision` only works in this case when 'round_trip' or 'legacy'
    )

##### OUTSTATUS #####
@pytest.fixture
def determin_outstatus_01():
    """Expected outstatus"""
    var_types = defaultdict(lambda: 'string') # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS   VALUE
        3 VB IDE  113.94999999999998863132
        """),
        sep=r'\s+', 
        dtype=var_types, 
        float_precision='round_trip' # `float_precision` only works in this case when 'round_trip' or 'legacy'
    )