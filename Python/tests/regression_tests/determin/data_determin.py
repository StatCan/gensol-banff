"""datasets for determin regression tests
    The naming convention in this file is pretty loose
    `determin_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the determin procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `determin_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### INDATA #####
@pytest.fixture
def determin_indata_empty():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = "string[pyarrow]"

    return pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        """), 
        sep=r'\s+',
        dtype=var_types
    ).drop(0)

@pytest.fixture
def determin_indata_01():
    """`REC` is numeric when it should be character
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric

    return pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        7  50.60 45.24 97.48 45.96 297.0  NaN  86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

@pytest.fixture
def determin_indata_02():
    """duplicate value in `REC`
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = str

    dataset = pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        3  50.60 45.24 97.48 45.96 297.0  NaN  86.40  0.05  89.6 434.2
        7  50.60 45.24 97.48 45.96 297.0  NaN  86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

    return dataset.sort_values(by="REC")

@pytest.fixture
def determin_indata_03():
    """`REC` has a 'missing' value
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = str

    return pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        " "  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        7  50.60 45.24 97.48 45.96 297.0  NaN  86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

@pytest.fixture
def determin_indata_04():
    """data is NOT sorted by `AREA REC`, but it should be
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = str

    return pd.read_csv(StringIO("""
        REC AREA VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  1 16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        6  2 76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        7  1 50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
        1  2 16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        2  1 76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        4  2 50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

@pytest.fixture
def determin_indata_presort(determin_indata_04):
    """dataset suitable for testing `presort` option"""
    return determin_indata_04

@pytest.fixture
def determin_indata_05():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = str

    return pd.read_csv(StringIO("""
        REC VA VB VC VD TOT1 VE VF VG TOT2 GT
        3  16.00 40.23 36.21 10.54 176.7 49.21 88.23 38.95 178.2 390.0
        6  76.21 20.54 38.99 33.63 102.1 53.58 16.73 95.63 115.8 470.5
        7  50.60 45.24 97.48 45.96 297.0 NaN   86.40  0.05  89.6 434.2
        """), 
        sep=r'\s+',
        dtype=var_types
    )

@pytest.fixture
def determin_indata_06():
    """valid indata dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['ident'] = str
    var_types['SOME_CHAR'] = str

    return pd.read_csv(StringIO("""
        ident TOTAL Q1 Q2 Q3 Q4 staff prov SOME_NUM SOME_CHAR
        REC01 500 100 125 125 150 2000 24 1 abc
        REC02 750 200 170 130 250 2500 24 1 abc
        REC03 400 80 90 100 130 1500 24 1 abc
        REC04 1000 150 250 350 250 3500 24 1 abc
        REC05 3000 500 500 1000 1000 5000 24 1 abc
        REC06 50 10 15 500 20 100 24 1 abc
        REC07 600 110 140 230 45 2400 30 1 abc
        REC08 900 175 999 999 300 3000 30 1 abc
        REC09 2500 400 555 600 5000 89 30 1 abc
        REC10 800 11 12 13 14 2800 30 1 abc
        REC11 -25 -10 -5 -5 -10 3000 30 1 abc
        """), 
        sep=r'\s+',
        dtype=var_types
    )

##### INSTATUS #####
@pytest.fixture
def determin_instatus_empty():
    """valid instatus dataset
    """
    import pyarrow as pa
    var_types = defaultdict(lambda: "string[pyarrow]") # default to character
    #var_types['REC'] = "string[pyarrow]"

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS
        3 VA IPR 
        """), 
        sep=r'\s+', 
        dtype=var_types,
       # dtype_backend="pyarrow",
    ).drop(0)

@pytest.fixture
def determin_instatus_01():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

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

@pytest.fixture
def determin_instatus_02():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    dataset = pd.read_csv(StringIO("""
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
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    return dataset.sort_values(by="REC")

@pytest.fixture
def determin_instatus_persort(determin_instatus_02, determin_indata_presort):
    """dataset suitable for testing `presort` option"""
    dataset = pd.merge(left=determin_indata_presort[['REC', 'AREA']], right=determin_instatus_02)
    return dataset

@pytest.fixture
def determin_instatus_03():
    """valid instatus dataset, for testing single-variable variable-lists
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        fieldid status ident
        Q3 FTI REC06
        Q4 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        staff FTI REC09
        Q1 FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q4 FTI REC11
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

##### OUTDATA #####
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
        
        This isn't a bug, it's just how computers work.  
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['REC'] = str

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
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO("""
        REC FIELDID STATUS   VALUE
        3 VB IDE  113.94999999999998863132
        """),
        sep=r'\s+', 
        dtype=var_types, 
        float_precision='round_trip' # `float_precision` only works in this case when 'round_trip' or 'legacy'
    )