"""datasets for prorate unit tests
    The naming convention in this file is pretty loose
    `prorate_<dataset-name>_<##>[__<describe-minor-change>]`
        where <dataset-name> is a input or output parameter name for 
        the prorate procedure
        
        Example: `prorate_indata_01`
        
        There is no relationship between the numbering of these datasets and any unit test numbering"""
import pytest
import pandas as pd
import banff.testing
from io import StringIO

@pytest.fixture
def prorate_indata_01():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        V001 2 4 0 5 22
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_01__int_key():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        001 2 4 0 5 22
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_01__int_str_key():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        001 2 4 0 5 22
    """), sep=r'\s+', dtype={'ident':str}) # `dtype` ensures that numeric looking 'ident' values are typed as character data

@pytest.fixture
def prorate_indata_02():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        V001 2 4 0 5 22
        V003 3 5 7 2 25
        V002 5 0 1 3 15
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_03():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        V001 2 4 0 5 22
        V002 3 5 7 2 25
        V003 5 0 1 3 15
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_04():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1
        V001 2 4 0 5 22.22
        V002 3 5 7 2 25.10
        V003 5 0 1 3 15
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_05():
    return pd.read_csv(StringIO("""
        ident VA VB VC VD TOT1 VE VF VG VH TOT2 GT
        V001 2 4 0 5 22 3 10 7 2 25 35
        V002 -3 5 -7 2 25 5 0 1 3 15 60
    """), sep=r'\s+')

@pytest.fixture
def prorate_indata_06():
    return pd.read_csv(StringIO("""
        ident prov area VA VB VC VD TOT1
        V001 2 B 12 8 0 5 22
        V002 2 B  13 5 7 9 25
        V003 2 A 5 0 5 3 15
        V004 2 A 12 8 0 5 18
        V005 1 A 13 5 7 9 20
        V006 1 A 5 0 5 3 100
    """), sep=r'\s+', dtype={'prov':str, 'area':str})

@pytest.fixture
def prorate_indata_07():
    return pd.read_csv(StringIO("""
        REC AREA VA VB VC VD TOT1 VE VF VG TOT2 GT
        1 1 1 2 3 4 5 6 7 8 9 10
        2 1 1 2 3 4 5 6 7 8 9 10
        3 1 1 2 3 4 5 6 7 8 9 10
        4 1 1 2 3 4 5 6 7 8 9 10
        5 1 1 2 3 4 5 6 7 8 9 10
        6 2 1 2 3 4 5 6 7 8 9 10
        7 2 1 2 3 4 5 6 7 8 9 10
        8 2 1 2 3 4 5 6 7 8 9 10
        9 3 1 2 3 4 5 6 7 8 9 10
    """), sep=r'\s+', dtype={'REC':str})

@pytest.fixture
def prorate_instatus_01():
    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IMPUTED
        V001 VB FTI
        V001 VD ORIGINAL
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_01__int_key():
    return pd.read_csv(StringIO("""
        ident fieldid status
        001 VA IMPUTED
        001 VB FTI
        001 VD ORIGINAL
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_02():
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
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_02__bad_sort():
    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IMP
        V003 VB FTI
        V001 VD O
        V002 VA FTI
        V002 VB HTI
        V002 VC IC2
        V003 VA I
        V001 VB I
        V003 VC HPM
        V003 VD " "
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_03():
    return pd.read_csv(StringIO("""
        ident fieldid status
        V001 VA IMP
        V001 VB IDE
        V001 VD O
        V001 VE FTI
        V001 VF ILR2
        V001 VG IC2
        V002 VA FTI
        V002 VB ILR2
        V002 VC IC2
        V002 VE I
        V002 VF I
        V002 VG ILR1
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_04():
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
    """), sep=r'\s+')

@pytest.fixture
def prorate_instatus_05():
    return pd.read_csv(StringIO("""
        REC AREA FIELDID STATUS
        2 1 TOT1 IDN
        2 1 TOT2 IDN
        2 1 VA IDN
        3 1 TOT1 IDN
        3 1 TOT2 IDN
        3 1 VA IDN
        6 2 TOT1 IDN
        6 2 TOT2 IDN
        6 2 VA IDN
        7 2 TOT1 IDN
        7 2 TOT2 IDN
        7 2 VA IDN
        9 3 TOT1 IDN
        9 3 TOT2 IDN
        9 3 VA IDN
    """), sep=r'\s+', dtype={'REC':str})

@pytest.fixture
def prorate_outdata_01():
    return banff.testing.PAT_from_string("""
s n
ident,GT,TOT1,TOT2,VA,VB,VC,VD,VE,VF,VG,VH
V001,35,17,18,4,6,0,7,2,8,7,1
V002,60,37,23,77,-39,24,-25,11,0,1,11
    """, sep=',')

@pytest.fixture
def prorate_outdata_02():
    return banff.testing.PAT_from_string("""
s   n
REC,GT,TOT1,TOT2,VA,VB,VC,VD
2,10,4,6,-5,2,3,4
3,10,4,6,-5,2,3,4
6,10,4,6,-5,2,3,4
7,10,4,6,-5,2,3,4
9,10,4,6,-5,2,3,4
    """, sep=',')

@pytest.fixture
def prorate_outstatus_01():
    return banff.testing.PAT_from_string("""
s s s n
ident,FIELDID,STATUS,VALUE
V001,TOT1,IPR,17.0
V001,TOT2,IPR,18.0
V001,VA,IPR,4.0
V001,VB,IPR,6.0
V001,VD,IPR,7.0
V001,VE,IPR,2.0
V001,VF,IPR,8.0
V001,VH,IPR,1.0
V002,TOT1,IPR,37.0
V002,TOT2,IPR,23.0
V002,VA,IPR,77.0
V002,VB,IPR,-39.0
V002,VC,IPR,24.0
V002,VD,IPR,-25.0
V002,VE,IPR,11.0
V002,VH,IPR,11.0
    """, sep=',')

@pytest.fixture
def prorate_outstatus_02():
    return banff.testing.PAT_from_string("""
s s s n
REC,FIELDID,STATUS,VALUE
2,TOT1,IPR,4.0
2,TOT2,IPR,6.0
2,VA,IPR,-5.0
3,TOT1,IPR,4.0
3,TOT2,IPR,6.0
3,VA,IPR,-5.0
6,TOT1,IPR,4.0
6,TOT2,IPR,6.0
6,VA,IPR,-5.0
7,TOT1,IPR,4.0
7,TOT2,IPR,6.0
7,VA,IPR,-5.0
9,TOT1,IPR,4.0
9,TOT2,IPR,6.0
9,VA,IPR,-5.0
    """, sep=',')

@pytest.fixture
def prorate_outreject_01():
    return pd.read_csv(StringIO("""
ident,NAME_ERROR,TOTAL_NAME,FIELDID,RATIO_ERROR
V001,DECIMAL ERROR,TOT1," ",
V002,DECIMAL ERROR,TOT1," ",
    """), dtype={'FIELDID': str})

@pytest.fixture
def prorate_outreject_02():
    return pd.read_csv(StringIO("""
ident,NAME_ERROR,TOTAL_NAME,FIELDID,RATIO_ERROR
V001,DECIMAL ERROR,TOT1," ",
    """), dtype={'FIELDID': str})

@pytest.fixture
def prorate_outreject_03():
    return banff.testing.PAT_from_string("""
s s s s n
REC,NAME_ERROR,TOTAL_NAME,FIELDID,RATIO_ERROR
1,NOTHING TO PRORATE,GT, ,NaN
4,NOTHING TO PRORATE,GT, ,NaN
5,NOTHING TO PRORATE,GT, ,NaN
8,NOTHING TO PRORATE,GT, ,NaN
    """, sep=',')
