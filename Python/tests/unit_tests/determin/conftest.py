"""datasets for determin unit tests
    The naming convention in this file is pretty loose
    `determin_<dataset-name>_<##>[__<describe-minor-change>]`
        where <dataset-name> is a input or output parameter name for 
        the determin procedure
        
        Example: `determin_indata_01`
        
        There is no relationship between the numbering of these datasets and any unit test numbering"""
import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict
import numpy as np
import banff.testing

@pytest.fixture
def determin_indata_01():
    return pd.read_csv(StringIO("""
    REC VA VB VC VD TOT1 VE VF VG TOT2 GT
    3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    7  50.60 45.24 97.48 45.96 297.0  NaN    86.40  0.05  89.6 434.2
    """), sep=r'\s+')

@pytest.fixture
def determin_indata_02():
    return pd.read_csv(StringIO("""
    REC VA VB VC VD TOT1 VE VF VG TOT2 GT
    3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    3  50.60 45.24 97.48 45.96 297.0 NaN 86.40  0.05  89.6 434.2
    7  50.60 45.24 97.48 45.96 297.0 NaN 86.40  0.05  89.6 434.2
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_indata_03():
    return pd.read_csv(StringIO("""
    REC VA VB VC VD TOT1 VE VF VG TOT2 GT
    3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    " "  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    7  50.60 45.24 97.48 45.96 297.0  NaN    86.40  0.05  89.6 434.2
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_indata_04():
    foo = pd.read_csv(StringIO("""
    REC AREA VA VB VC VD TOT1 VE VF VG TOT2 GT
    3  1 16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    6  2 76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    7  1 50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
    1  2 16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    2  1 76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    4  2 50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
    """), sep=r'\s+', dtype={'REC': str})
    
    return foo.sort_values(by="REC")

@pytest.fixture
def determin_indata_05():
    return pd.read_csv(StringIO("""
    REC VA VB VC VD TOT1 VE VF VG TOT2 GT
    3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
    6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
    7  50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_instatus_01():
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
    """), sep=r'\s+')

@pytest.fixture
def determin_instatus_02():
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
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_instatus_03():
    foo= pd.read_csv(StringIO("""
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
        1 VA IPR 
        1 VB FTI
        1 VC I
        1 VD I
        1 TOT1 O
        1 VE IC2
        1 VF O
        1 VG IPM
        1 TOT2 ICM
        2 VB IPV
        2 VC IAT
        2 VD O
        2 TOT1 I
        2 VE O
        2 VF IDN
        2 TOT2 IPV
        4 VB IPV
        4 VC ODE
        4 VD IDE
        4 VF O
        4 VG I
        4 TOT2 ICR
    """), sep=r'\s+', dtype={'REC': str})

    return foo.sort_values(by="REC")


@pytest.fixture
def determin_outdata_01():
    """
    SAS Test claims value of `VB` should be `113.95`, but comparing the actual result against
    that number shows differences.  This is due to numeric precision of floating point numbers. 
        for a 32 bit float, the actual value (13.9499999999999886313) and 113.95 are both stored
        as 113.9499969482421875.
        However, for 64-bit double, The actual value is stored as 
            113.94999999999998863131622783839702606201171875
        while 113.95 is stored as 
            113.9500000000000028421709430404007434844970703125
        
        This isn't a bug, it's just how computers work.  """
    # defaultdict allows us to specify character columns and make others float by default
    d = defaultdict(lambda: np.float64)
    d['REC'] = str
    return pd.read_csv(StringIO("""
        REC VG VF VE VD VC VB VA TOT2 TOT1 GT
        "3" 38.95 88.23 49.21 10.54 36.21 113.94999999999998863132 16 178.2 176.7 390
    """), sep=r'\s+', dtype=d, float_precision='round_trip') # `float_precision` only works in this case when 'round_trip' or 'legacy'


@pytest.fixture
def determin_outdata_02():
    return banff.testing.PAT_from_string("""
        s n
        REC VG VF VE VD VC VB VA TOT2 TOT1 GT
    """)

@pytest.fixture
def determin_outstatus_01():
    return pd.read_csv(StringIO("""
        REC FIELDID STATUS
        3 VB IDE
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_outstatus_02():
    return pd.read_csv(StringIO("""
        REC FIELDID STATUS   VALUE
        3 VB IDE  149.05
        3 TOT1 IDE  211.80
    """), sep=r'\s+', dtype={'REC': str})

@pytest.fixture
def determin_outstatus_03():
    return banff.testing.PAT_from_string("""
        s s s n
        REC FIELDID STATUS VALUE
    """)

@pytest.fixture
def determin_empty_dataset():
    return pd.DataFrame()

