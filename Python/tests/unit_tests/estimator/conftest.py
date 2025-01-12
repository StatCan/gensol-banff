import pytest
import pandas as pd
import banff.testing
from io import StringIO

@pytest.fixture
def estimato_indata_01a():
    return pd.read_csv(StringIO("""
        ID X Y Z
        1A 1 1 1
        2A 2 2 2
        3A 3 3 3
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_04a():
    return pd.read_csv(StringIO("""
        ID U V W X Y Z
        1A -1 1 2 2 2 2
        2A 0 2 2 2 2 2
        3A 1 2 NaN NaN NaN NaN
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_03p():
    return pd.read_csv(StringIO("""
        ident V X Y Z
        A1 NaN 1 1 1
        A2 2 NaN 2 2
        A3 3 3 NaN 3
        A4 4 4 4 NaN
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_12b():
    return pd.read_csv(StringIO("""
        ID X Y Z EXC
        1A 1 1 1 E
        2A 2 2 NaN C
        3A 3 4 5 c
        4A 5 6 7 g
        5A 7 8 9 " "
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_17d():
    return pd.read_csv(StringIO("""
        ident V X Y Z R1 R2
        A1 NaN 1 1 1 A C
        A2 2 NaN 2 2 A C
        A3 3 3 NaN 3 A D
        A4 4 4 4 NaN A D
        A5 NaN 5 5 5 B C
        A6 6 NaN 6 6 B C
        A7 7 7 NaN 7 B D
        A8 8 8 8 NaN B D
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_17d03():
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 NaN 1 1 1 A
        A2 2 NaN 2 2 B
        A3 3 3 NaN 3 A
        A4 4 4 4 NaN A
        A5 NaN 5 5 5 B
        A6 6 NaN 6 6 B
        A7 7 7 NaN 7 B
        A8 8 8 8 NaN B
    """), sep=r'\s+')

@pytest.fixture
def estimato_outdata_17d():
    return pd.read_csv(StringIO("""
        ident V X Y Z 
        A1 1 1   1  1
        A2 2 3.5 2  2
        A4 4 4   4  5
        A3 3 3   6  3
        A5 5 5   5  5
        A6 6 7.5 6  6
        A7 7 7   14 7
        A8 8 8   8  13
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_01a():
    return pd.read_csv(StringIO("""
        fieldid status id
        Z FTI 1A
        Z FTI 2A
        Z FTI 3A
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_04a():
    return pd.read_csv(StringIO("""
        fieldid status id
        W       " "       3A
        X       " "       3A
        Y       " "       3A
        Z       " "       3A
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_03p():
    return pd.read_csv(StringIO("""
        ident fieldid status
        A1  V  FTI
        A2  X  FTI
        A3  Y  FTI
        A4  Z  FTI
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_12b():
    return pd.read_csv(StringIO("""
        fieldid status id
        Z FTI 2A
        Y FTI 1A
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_17d():
    return pd.read_csv(StringIO("""
        ident fieldid status
        A1  V  FTI
        A2  X  FTI
        A3  Y  FTI
        A4  Z  FTI
        A5  V  FTI
        A6  X  FTI
        A7  Y  FTI
        A8  Z  FTI
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_17d_by_vars():
    return pd.read_csv(StringIO("""
        ident fieldid status R1 R2
        A1  V  FTI A C
        A2  X  FTI A C
        A3  Y  FTI A D
        A4  Z  FTI A D
        A5  V  FTI B C
        A6  X  FTI B C
        A7  Y  FTI B D
        A8  Z  FTI B D
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_hist_03p():
    return pd.read_csv(StringIO("""
        ident V X Y Z
        A1 4 0 -4 4
        A2 3 3 3 3
        A3 2 2 2 2
        A4 1 1 1 1
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_hist_17d():
    return pd.read_csv(StringIO("""
        ident V X Y Z R1 R2
        A1 4 4 4 4 A C
        A2 3 3 3 3 A C
        A3 2 2 2 2 A D
        A4 1 1 1 1 A D
        A5 8 8 8 8 B C
        A6 7 7 7 7 B C
        A7 6 6 6 6 B D
        A8 5 5 5 5 B D
    """), sep=r'\s+')

@pytest.fixture
def estimato_indata_hist_17d03():
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 4 4 4 4 A
        A2 3 3 3 3 A
        A3 2 2 2 2 A
        A4 1 1 1 1 A
        A5 8 8 8 8 B
        A6 7 7 7 7 B
        A7 6 6 6 6 B
        A8 5 5 5 5 B
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_hist_03p():
    return pd.read_csv(StringIO("""
        ident fieldid status
        A1 Z   IDN
        A4 X   IDE
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_hist_17d():
    return pd.read_csv(StringIO("""
        ident fieldid status
        A2 X   IDN
        A4 X   IDE
        A6 X   IDN
        A8 X   IDE
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_hist_17d_by_vars():
    return pd.read_csv(StringIO("""
        ident fieldid status R1 R2
        A2 X   IDN A C
        A4 X   IDE A D
        A6 X   IDN B C
        A8 X   IDE B D
    """), sep=r'\s+')

@pytest.fixture
def estimato_inalgorithm_04a():
    return pd.read_csv(StringIO("""
        algorithmname type status formula description
        "UDZERO"    "EF" "3J1"  1                  "constant"
        "DIVZERO"   "EF" "3J2"  5                  "constant"
    """), sep=r'\s+', dtype={'formula': str}) # dtype necessary since values can be a digit

@pytest.fixture
def estimato_inalgorithm_03p():
    return pd.read_csv(StringIO("""
        algorithmname type status formula description
        "HISTREG2"    "LR" "HR2"  INTERCEPT,AUX1(H) " "
    """), sep=r'\s+', dtype={'formula': str}) # dtype necessary since values can be a digit

@pytest.fixture
def estimato_inalgorithm_12b():
    return pd.read_csv(StringIO("""
        algorithmname type status formula description
        CRAUX        EF  CAE  AUX1(A)+10        " "
        LRCRI        LR  LRC  AUX1^2            " "
        CRAUX        EF  CAE  AUX1/(2-2)        " "
        ZEROBETA     LR  ZBT  INTERCEPT,AUX1    " "
    """), sep=r'\s+', dtype={'formula': str}) # dtype necessary since values can be a digit

@pytest.fixture
def estimato_inalgorithm_17d():
    return pd.read_csv(StringIO("""
        algorithmname type status formula description
        ALG2 EF AL2  AUX1+AUX2     SumCrAux1CrAux2
        ALG3 EF AL3  AUX1(H)+AUX2  SumHsAux1CrAux2
    """), sep=r'\s+', dtype={'formula': str}) # dtype necessary since values can be a digit

# NOTE: value " " resolves to a single space character, consistent with SAS' "missing" character value
@pytest.fixture
def estimato_inestimator_01a():
    return pd.read_csv(StringIO("""
        active excludeimputed excludeoutliers countcriteria percentcriteria algorithmname weightvariable variancevariable varianceperiod varianceexponent fieldid auxvariables RANDOMERROR
        2 " " " " NaN NaN "CURSUM2" " " " " " " NaN "Z" "X,Y" "n"
    """), sep=r'\s+')

@pytest.fixture
def estimato_inestimator_01b():
    return pd.read_csv(StringIO("""
        active excludeimputed excludeoutliers countcriteria percentcriteria algorithmname weightvariable variancevariable varianceperiod varianceexponent fieldid auxvariables RANDOMERROR
        2  "N"  "N"  1  30  "CURMEAN"  " "  " "  " "  NaN  "Z"  " "  "N"
    """), sep=r'\s+')

@pytest.fixture
def estimato_inestimator_04a():
    return pd.read_csv(StringIO("""
        active excludeimputed excludeoutliers countcriteria percentcriteria algorithmname weightvariable variancevariable varianceperiod varianceexponent fieldid auxvariables RANDOMERROR
        1  " "  " "  NaN    NaN     "UDZERO"        " "  " "  " "  NaN  "Z"  " "  "N"
        1  " "  " "  NaN    NaN     "DIVZERO"       " "  " "  " "  NaN  "Y"  " "  "N"
        2  "N"  "N"  1      30      "CURAUXMEAN"    " "  " "  " "  NaN  "W"  "V"  "N"
    """), sep=r'\s+')

@pytest.fixture
def estimato_inestimator_03p():
    return pd.read_csv(StringIO("""
        active excludeimputed excludeoutliers randomerror countcriteria percentcriteria weightvariable variancevariable varianceperiod varianceexponent algorithmname fieldid auxvariables
        1 N N N 1 1 " " X H 1 HISTREG     V   " "
        2 N N N 1 1 " " Y H 2 HISTREG2    X   V
        3 N N N 1 1 " " Z H 3 HISTREG2    Y   V
        4 N N N 1 1 " " V C 1 HISTREG2    Z   Y
    """), sep=r'\s+')

@pytest.fixture
def estimato_inestimator_12b():
    return pd.read_csv(StringIO("""
        active excludeimputed excludeoutliers countcriteria percentcriteria algorithmname weightvariable variancevariable varianceperiod varianceexponent fieldid auxvariables RANDOMERROR
        1 N N 1 10 CURAUXMEAN  Y  " "  " "  NaN Z X n  
    """), sep=r'\s+')

@pytest.fixture
def estimato_inestimator_17d():
    return pd.read_csv(StringIO("""
        active  excludeimputed  excludeoutliers     randomerror     countcriteria   percentcriteria     weightvariable  variancevariable    varianceperiod  varianceexponent    algorithmname   fieldid     auxvariables
        1       N               N                   N               1               1                   " "             " "                 " "             NaN                 CURAUXMEAN      V           X
        2       N               N                   N               1               1                   " "             " "                 " "             NaN                 PREMEAN         X           " "
        3       " "             " "                 N               NaN             NaN                 " "             " "                 " "             NaN                 ALG2            Y           X,Z
        4       " "             " "                 N               NaN             NaN                 " "             " "                 " "             NaN                 ALG3            Z           X,Y
    """), sep=r'\s+')

@pytest.fixture
def estimato_outdata_01b():
    return pd.read_csv(StringIO("""
        ID Z
        3A 1.5
    """), sep=r'\s+')

@pytest.fixture
def estimato_outstatus_01b():
    return pd.read_csv(StringIO("""
        FIELDID STATUS ID  VALUE
        Z ICM 3A    1.5
    """), sep=r'\s+')

@pytest.fixture
def estimato_outacceptable_04a():
    return banff.testing.PAT_from_string("""
        n s s
        ESTIMID    ALGORITHMNAME    ID
        2        CURAUXMEAN      2A
        2        CURAUXMEAN      1A
    """)

@pytest.fixture
def estimato_outest_ef_04a():
    return banff.testing.PAT_from_string("""
        n s s s n n
        ESTIMID    ALGORITHMNAME    FIELDID    PERIOD      AVERAGE_VALUE     COUNT
        2        CURAUXMEAN         V         C          1.5        2
    """)

@pytest.fixture
def estimato_outest_parm_04a():
    return banff.testing.PAT_from_string("""
        n s s n n n n
        ESTIMID    ALGORITHMNAME    FIELDID    FTI    IMP    DIVISIONBYZERO    NEGATIVE
        0        UDZERO             Z        0      0            0              0
        1        DIVZERO            Y        0      0            0              0
        2        CURAUXMEAN         W        0      0            0              0
    """)