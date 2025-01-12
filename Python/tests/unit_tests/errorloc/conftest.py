import pytest
import pandas as pd
from io import StringIO

@pytest.fixture
def errorloc_indata_IT01():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
IDENT X1  X2   X3
R01   10  NaN NaN
""    NaN NaN 110
R03   14  NaN 128
R04   -9  NaN 99
""    10  NaN -98
R06   -1  NaN -10
    """), sep=r'\s+')

@pytest.fixture
def errorloc_indata_grp_a():
    # synthetic input dataset
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
    """), sep=r'\s+', dtype={'ident': str, 'area': str})

@pytest.fixture
def errorloc_indata_grp_a_02():
    # synthetic input dataset
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
    """), sep=r'\s+', dtype={'REC': str, 'AREA': str})

@pytest.fixture
def errorloc_indata_grp_b():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
REC A1 A2 X1 X2 X3
'101'   1   1   10   40   100
'102'   1   1   4   40   110
'103'   1   1   16   40   128
'104'   1   2   10   29   152
'105'   1   2   10   51   98
'106'   1   2   4   51   136
'201'   2   1   10   40   100
'202'   2   1   4   40   110
'203'   2   1   16   40   128
'204'   2   2   10   29   152
'205'   2   2   10   51   98
'206'   2   2   4   51   136
    """), sep=r'\s+', dtype={'ident': str, 'A1': str, 'A2': str})

@pytest.fixture
def errorloc_indata_grp_c():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
REC A1 A2 X1 X2 X3
'102'   1   10   40   100
'101'   1   4   40   110
'103'   1   16   40   128
'104'   1   10   29   152
'105'   1   10   51   98
'106'   1   4   51   136
'201'   2   10   40   100
'202'   2   4   40   110
'203'   2   16   40   128
'204'   2   10   29   152
'205'   2   10   51   98
'206'   2   4   51   136
    """), sep=r'\s+', dtype={'ident': str, 'A1': str, 'A2': str})

@pytest.fixture
def errorloc_indata_grp_i():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
REC X1 X2 X3
'101'   10   35   100
'102'   NaN   35   110
'103'   14   40   128
'104'   10   5   100
'105'   10   44   98
'106'   10   31   100
"""), sep=r'\s+', dtype={'ident': str, 'A1': str, 'A2': str})

@pytest.fixture
def errorloc_indata_k02():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
IDENT X1  X2   X3
R01   10  35   NaN
""    NaN 35   110
R03   14  40   128
R04   -9  5    99
""    10  44   -98
R06   -1  -1.5 -10
    """), sep=r'\s+')

@pytest.fixture
def errorloc_expected_outstatus_01():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS  VALUE
    R01      X3    FTI    NaN
    R03      X1    FTI   14.0
    R04      X3    FTI   99.0
    R04      X1    FTI   -9.0
    R06      X3    FTI  -10.0
    R06      X2    FTI   -1.5
    R06      X1    FTI   -1.0
     """), sep=r'\s+')