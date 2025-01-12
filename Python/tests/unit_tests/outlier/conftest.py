"""
Banff Outlier `conftest.py`
Conventions:
- create "base" datasets in functions named `ds_<proc-name>_<dataset-name>_<2-digit-number>`
  - for example `ds_outlier_indata_01`
- perform small "base" dataset modifications inside pytest "fixtures"
  - facilitates avoiding duplication of code in `ds_` functions
  - for example, if one or two values, or one column must be modified, call the `ds_` function
    and perform modifications in the fixture
    ```
    @pytest.fixture
    def outlierA_indata_01():
        dataset = ds_outlier_indata_01()
        dataset.at['V1', 0] = -2
    ```
"""
import pytest
import pandas as pd
import banff.testing
from io import StringIO

def ds_outlier_indata_01():
    return pd.read_csv(StringIO("""
        ident area V1 V2
        R01   1    4  19
        R02   1    14 5
        R03   1    18 19
        R04   1    7  6
        R05   1    1  1
        R06   1    5  2
        R07   1    3  8
        R08   1    16 14
        R09   1    2  3
        R10   1    6  2
    """), sep=r'\s+')

def ds_outlier_indata_02():
    return pd.read_csv(StringIO("""
        ident area V1  V2
        R01   1    4   19
        R02   1    14  5
        R03   1    18  19
        R04   1    7   6
        R05   1    -1  0
        R06   1    5   -2
        R07   1    3   8
        R08   1    -16 -14
        R09   1    2   3
        R10   1    6   2
    """), sep=r'\s+')

def ds_outlier_indata_03():
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
    """), sep=r'\s+')

def ds_outlier_indata_hist_01():
    return pd.read_csv(StringIO("""
        ident area V1 V2
        R01   1    19 4
        R02   1    5  14
        R03   1    19 18
        R04   1    6  7
        R05   1    1  1
        R06   1    2  5
        R07   1    8  3
        R08   1    14 16
        R09   1    3  2
        R10   1    2  6
    """), sep=r'\s+')

def ds_outlier_expected_outstatus_01():
    return banff.testing.PAT_from_string("""
s s s n
ident,FIELDID,STATUS,VALUE
R01,V1,FTI,4.0
R07,V1,FTE,3.0
R07,V2,FTE,8.0
R01,V2,FTI,19.0
    """, sep=',')

def ds_outlier_expected_outstatus_detailed_01():
    return banff.testing.PAT_from_string("""
s s s s n s n
ident,FIELDID,OUTLIER_STATUS,METHOD,CURRENT_VALUE,HIST_AUX,HIST_AUX_VALUE,EFFECT,IMP_BND_L,EXCL_BND_L,EXCL_BND_R,IMP_BND_R
R01,V1,ODIL,HB,4,V1,19,-4.089285714,-2.754166667,-1.836904762,2.809047619,4.2147619048
R07,V1,ODEL,HB,3,V1,8,-1.857142857,-2.754166667,-1.836904762,2.809047619,4.2147619048
R07,V2,ODER,HB,8,V2,3,1.8444444444,-4.237425595,-2.825744048,1.8246031746,2.7380952381
R01,V2,ODIR,HB,19,V2,4,4.0666666667,-4.237425595,-2.825744048,1.8246031746,2.7380952381
    """, sep=',')

def ds_outlier_expected_outstatus_detailed_01b():
    return banff.testing.PAT_from_string("""
s s s s n s n
ident,FIELDID,OUTLIER_STATUS,METHOD,CURRENT_VALUE,HIST_AUX,HIST_AUX_VALUE,EFFECT,IMP_BND_L,EXCL_BND_L,EXCL_BND_R,IMP_BND_R
R01,V1,ODIL,HB,4,V2,19,-4.089285714,-2.754166667,-1.836904762,2.809047619,4.2147619048
R07,V1,ODEL,HB,3,V2,8,-1.857142857,-2.754166667,-1.836904762,2.809047619,4.2147619048
R07,V2,ODER,HB,8,V1,3,1.8444444444,-4.237425595,-2.825744048,1.8246031746,2.7380952381
R01,V2,ODIR,HB,19,V1,4,4.0666666667,-4.237425595,-2.825744048,1.8246031746,2.7380952381
    """, sep=',')

def ds_outlier_expected_outstatus_02():
    return pd.read_csv(StringIO("""
ident,FIELDID,STATUS,VALUE
R08,V1,FTI,-16.0
R02,V1,FTE,14.0
R03,V1,FTI,18.0
    """))

def ds_outlier_expected_outstatus_detailed_02():
    return pd.read_csv(StringIO("""
ident,FIELDID,OUTLIER_STATUS
R08,V1,ODIL
R02,V1,ODER
R03,V1,ODIR
    """))

def ds_outlier_expected_outstatus_03():
    return pd.read_csv(StringIO("""
ident,FIELDID,STATUS,VALUE
R07,V2,FTE,8.0
R01,V2,FTI,19.0
"""))

def ds_outlier_expected_outstatus_detailed_03():
    return banff.testing.PAT_from_string("""
s s s s n s n
ident,FIELDID,OUTLIER_STATUS,METHOD,CURRENT_VALUE,HIST_AUX,HIST_AUX_VALUE,GAP,IMP_SIGMAGAP,EXCL_SIGMAGAP,IMP_BND_L,EXCL_BND_L,EXCL_BND_R,IMP_BND_R
R07,V2,ODER,SG,8,V2,3,1.1666666667,1.223145,0.407715,,,1.1666666667,2.0833333333
R01,V2,ODIR,SG,19,V2,4,2.0833333333,1.223145,0.407715,,,1.1666666667,2.0833333333
    """, sep=',')

def ds_outlier_expected_outstatus_04():
    return pd.read_csv(StringIO("""
ident,FIELDID,STATUS,VALUE
R05,V1,FTE,-1.0
R08,V1,FTI,-16.0
R02,V1,FTI,14.0
R03,V1,FTI,18.0
    """))

def ds_outlier_expected_outstatus_detailed_04():
    return pd.read_csv(StringIO("""
ident,FIELDID,OUTLIER_STATUS
R05,V1,ODEL
R08,V1,ODIL
R02,V1,ODIR
R03,V1,ODIR
    """))

@pytest.fixture
def outlier_indata_01():
    return ds_outlier_indata_01()

@pytest.fixture
def outlier_indata_03():
    return ds_outlier_indata_03()

@pytest.fixture
def outlier_indata_d07():
    list_data = []
    for m in range(100000):
        list_data.append(["R{:07d}".format(10*m + 1), "A{:06d}".format(m+1),  4, 19])
        list_data.append(["R{:07d}".format(10*m + 2), "A{:06d}".format(m+1), 14,  5])
        list_data.append(["R{:07d}".format(10*m + 3), "A{:06d}".format(m+1), 18, 19])
        list_data.append(["R{:07d}".format(10*m + 4), "A{:06d}".format(m+1),  7,  6])
        list_data.append(["R{:07d}".format(10*m + 5), "A{:06d}".format(m+1),  1,  1])
        list_data.append(["R{:07d}".format(10*m + 6), "A{:06d}".format(m+1),  5,  2])
        list_data.append(["R{:07d}".format(10*m + 7), "A{:06d}".format(m+1),  3,  8])
        list_data.append(["R{:07d}".format(10*m + 8), "A{:06d}".format(m+1), 16, 14])
        list_data.append(["R{:07d}".format(10*m + 9), "A{:06d}".format(m+1),  2,  3])
        list_data.append(["R{:07d}".format(10*m + 10), "A{:06d}".format(m+1),  6,  2])
    
    df = pd.DataFrame(list_data, columns=['ident', 'area', 'V1', 'V2'])
    return df

@pytest.fixture
def outlier_indata_hist_d07():
    list_data = []
    for m in range(100000):
        list_data.append(["R{:07d}".format(10*m + 1), "A{:06d}".format(m+1), 19,  4])
        list_data.append(["R{:07d}".format(10*m + 2), "A{:06d}".format(m+1),  5, 14])
        list_data.append(["R{:07d}".format(10*m + 3), "A{:06d}".format(m+1), 19, 18])
        list_data.append(["R{:07d}".format(10*m + 4), "A{:06d}".format(m+1),  6,  7])
        list_data.append(["R{:07d}".format(10*m + 5), "A{:06d}".format(m+1),  1,  1])
        list_data.append(["R{:07d}".format(10*m + 6), "A{:06d}".format(m+1),  2,  5])
        list_data.append(["R{:07d}".format(10*m + 7), "A{:06d}".format(m+1),  8,  3])
        list_data.append(["R{:07d}".format(10*m + 8), "A{:06d}".format(m+1), 14, 16])
        list_data.append(["R{:07d}".format(10*m + 9), "A{:06d}".format(m+1),  3,  2])
        list_data.append(["R{:07d}".format(10*m + 10), "A{:06d}".format(m+1), 2,  6])
    
    df = pd.DataFrame(list_data, columns=['ident', 'area', 'V1', 'V2'])
    return df

@pytest.fixture
def outlier_inhist_01():
    return ds_outlier_indata_hist_01()

@pytest.fixture
def outlier_indata_b05_v2_char():
    foo = ds_outlier_indata_02()
    foo['V2'] = foo['V2'].apply(str)
    
    return foo

@pytest.fixture
def outlier_indata_b05_v1_v2_char():
    """'V1' and 'V2' converted to character data """
    foo = ds_outlier_indata_02()
    foo['V2'] = foo['V2'].apply(str)
    foo['V1'] = foo['V1'].apply(str)
    
    return foo

@pytest.fixture
def outlier_expected_outstatus_c03():
    return ds_outlier_expected_outstatus_01()

@pytest.fixture
def outlier_expected_outstatus_detailed_c03():
    return ds_outlier_expected_outstatus_detailed_01()

@pytest.fixture
def outlier_expected_outstatus_detailed_c03_b():
    return ds_outlier_expected_outstatus_detailed_01b()
@pytest.fixture
def outlier_expected_outstatus_B05_1():
    return ds_outlier_expected_outstatus_02()

@pytest.fixture
def outlier_expected_outstatus_detailed_B05_1():
    return ds_outlier_expected_outstatus_detailed_02()

@pytest.fixture
def outlier_expected_outstatus_B05_2():
    return ds_outlier_expected_outstatus_04()

@pytest.fixture
def outlier_expected_outstatus_detailed_B05_2():
    return ds_outlier_expected_outstatus_detailed_04()

@pytest.fixture
def outlier_expected_outstatus_c03_c():
    return ds_outlier_expected_outstatus_03()

@pytest.fixture
def outlier_expected_outstatus_detailed_c03_c():
    return ds_outlier_expected_outstatus_detailed_03()