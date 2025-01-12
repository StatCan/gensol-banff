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

@pytest.fixture
def massimpu_indata_01():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
01   1   500   120   150    80   150    50
02   1   750   200   170   130   250    75
03   1   900   NaN  NaN  NaN  NaN   90
04   2  1000 150   250   350   250   100
05   2  1050   200   225   325   300   100
    """), sep=r'\s+')

@pytest.fixture
def massimpu_indata_01_char_ident_area():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
01   1   500   120   150    80   150    50
02   1   750   200   170   130   250    75
03   1   900   NaN  NaN  NaN  NaN   90
04   2  1000 150   250   350   250   100
05   2  1050   200   225   325   300   100
    """), sep=r'\s+', dtype={'area': str, 'ident': str})

@pytest.fixture
def massimpu_indata_03_char_ident():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
01   1   500   120   150    80   150    50
02   1   750   200   170   130   250    75
03   1   900   NaN  NaN  NaN  NaN   90
04   2  1000 150   250   350   250   100
05   2  1050   200   225   325   300   100
    """), sep=r'\s+', dtype={'ident': str})

@pytest.fixture
def massimpu_indata_03_char_ident_Q1():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
01   1   500   120   150    80   150    50
02   1   750   200   170   130   250    75
03   1   900   NaN  NaN  NaN  NaN   90
04   2  1000 150   250   350   250   100
05   2  1050   200   225   325   300   100
    """), sep=r'\s+', dtype={'ident': str, 'Q1': str})

@pytest.fixture
def massimpu_indata_04():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff
    01   1   500   120   150    80   150    50
    02   1   750   200   170   130   250    -75
    03   1   900   NaN  NaN  NaN  NaN   -90
    04   2  1000 150   250   350   250   100
    05   2  1050   NaN  NaN  NaN  NaN  NaN
    """), sep=r'\s+', dtype={'ident': str})

@pytest.fixture
def massimpu_indata_05():
    list_data = []
    for m in range(5000):
        list_data.append([ 1,   500,      100,     125,     125,     150,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m +  1)])
        list_data.append([ 2,   750,      200,     170,     130,     250,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m +  2)])
        list_data.append([ 3,   -400,     80,      90,      100,     130,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m +  3)])
        list_data.append([ 4,   1000,     150,     250,     350,     250,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m +  4)])
        list_data.append([ 5,   3000,     500,     500,     1000,    1000,  1000, "A{:04d}".format(m), "B{:06d}".format(100*m +  5)])
        list_data.append([ 6,   800,      200,     225,     200,     175,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m +  6)])
        list_data.append([ 7,   -600,     125,     150,     175,     150,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m +  7)])
        list_data.append([ 8,   900,      175,     200,     225,     300,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m +  8)])
        list_data.append([ 9,   2500,     500,     650,     600,     750,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m +  9)])
        list_data.append([10,   800,      150,     175,     225,     250,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m + 10)])
        list_data.append([11,   5750,  np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 11)])
        list_data.append([12,   850,   np.nan,  np.nan,  np.nan,  np.nan,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m + 12)])
        list_data.append([13,   -375,  np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 13)])
        list_data.append([14,   1100,  np.nan,  np.nan,  np.nan,  np.nan,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m + 14)])
        list_data.append([15,   -3100, np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 15)])
        list_data.append([16,   750,   np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 16)])
        list_data.append([17,   -675,  np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 17)])
        list_data.append([18,   875,   np.nan,  np.nan,  np.nan,  np.nan,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m + 18)])
        list_data.append([19,   4000,  np.nan,  np.nan,  np.nan,  np.nan,   1000, "A{:04d}".format(m), "B{:06d}".format(100*m + 19)])
        list_data.append([20,   -700,  np.nan,  np.nan,  np.nan,  np.nan,   2000, "A{:04d}".format(m), "B{:06d}".format(100*m + 20)])
    d = defaultdict(lambda: np.float64)
    d['AREA'] = str
    d['IDENT'] = str

    df = pd.DataFrame(list_data, columns=['i', 'total', 'Q1', 'Q2', 'Q3', 'Q4', 'staff', 'AREA', 'IDENT'])
    df.sort_values(by=['AREA', 'IDENT'], inplace=True)
    return df

@pytest.fixture
def massimpu_outdonormap_01():
    d = defaultdict(lambda: np.float64)
    d['RECIPIENT'] = str
    d['DONOR'] = str

    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    03         02     1                  NaN
    05         04     1                  NaN
     """), sep=r'\s+', dtype=d)