import pytest
import pandas as pd
import banff.testing
from io import StringIO

@pytest.fixture
def indata_Donor_IT01():
    return pd.read_csv(StringIO("""ident area total Q1 Q2 Q3 Q4 staff toexcl
REC01 A1  NaN  120  150   80  150   NaN   .
REC02 A1  750  200  170  130  -250  NaN   .
REC04 A1 1000  150  250  350  250   NaN   .
REC05 A2 1050  200  225  325  300   NaN   .
REC11 A1  500  120  150   80  150   NaN   .
REC12 A1  750  200  170  130  250   NaN   .
REC14 A1 1000  150  250  350  250   NaN   .
REC24 A1 1000  150  250  350  250   NaN   .
REC22 A1  750  200  170  130  250   NaN   .
REC32 A1  750  200  170  130  250   NaN   .
"""), sep=r'\s+', dtype={'ident': str, 'area': str, 'total': float, 'Q1': float, 'Q2': float, 'Q3': float, 'Q4':float, 'staff': float, 'toexcl': str})

@pytest.fixture
def indataCharKey():
    # synthetic input dataset
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff toexcl
    REC01 A1 500 120 150 80 150 50
    REC02 A1 750 200 170 130 250 75
    REC04 A1 1000 150 250 350 250 100
    REC05 A2 1050 200 225 325 300 100
    """), sep=r'\s+', dtype={'ident': str, 'area': str})

@pytest.fixture
def indataIntKey():
    return pd.read_csv(StringIO("""
    ident area total Q1 Q2 Q3 Q4 staff toexcl
    01 A1 500 120 150 80 150 50 
    02 A1 750 200 170 130 250 75 
    04 A1 1000 150 250 350 250 100 
    05 A2 1050 200 225 325 300 100 
    """), sep=r'\s+', dtype={'ident': int, 'area': str})

@pytest.fixture
def instatCharKey():
    return pd.read_csv(StringIO("""
    ident fieldid status
    REC01 Q3 IPR
    REC04 Q2 FTE
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+', dtype={'ident': str, 'fieldid': str, 'status': str})

@pytest.fixture
def instatCharKey_by_var():
    """Originally taken from donorD03 test case, modified to add BY variables"""
    return pd.read_csv(StringIO("""
    ident fieldid status area staff
    REC01 Q3 IPR A1 50
    REC04 Q2 FTE A1 100
    REC04 Q3 FTI A1 100
    REC04 Q4 FTI A1 100
    """), sep=r'\s+', dtype={'ident': str, 'fieldid': str, 'status': str})

@pytest.fixture
def instatIntKey():
    return pd.read_csv(StringIO("""
    ident fieldid status
    01 Q3 IPR
    04 Q2 FTE
    04 Q3 FTI
    04 Q4 FTI
    """), sep=r'\s+', dtype={'ident': int, 'fieldid': str, 'status': str})

@pytest.fixture
def JA_indata():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    """), sep=r'\s+')

@pytest.fixture
def JA_instat3():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IDE
    REC02 Q2 IPR
    REC01 staff IDN
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_instat4():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IDE
    REC02 Q2 IPR
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_instat5():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC04 Q1 FTI
    REC04 Q2 FTI
    REC04 Q3 FTI
    REC04 Q4 FTI
    REC04 TOTAL FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_instat6():
    """NOTE: 'IPE' below is a typo made years ago when implementing test cases.  It should be "IPR" ... HOWEVER
                to remain consistent with SAS tests, we will continue to use this typo"""
    return  pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPE
    REC02 Q2 FTE
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata7():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    REC06 A1  500  120  150   80  150   50   .
    REC07 A1  750  200  170  130  250   75   .
    REC08 A1 1000  150  250  350  250  100   .
    REC09 A1  500  120  150   80  150   50   .
    REC10 A1  750  200  170  130  250   75   .
    REC11 A1 1000  150  250  350  250  100   .
    REC12 A1  500  120  150   80  150   50   .
    REC13 A1  750  200  170  130  250   75   .
    REC14 A1 1000  150  250  350  250  100   .
    REC15 A1  500  120  150   80  150   50   .
    REC16 A1  750  200  170  130  250   75   .
    REC17 A1 1000  150  250  350  250  100   .
    REC18 A1  500  120  150   80  150   50   .
    REC19 A1  750  200  170  130  250   75   .
    REC20 A1 1000  150  250  350  250  100   .
    REC21 A1  500  120  150   80  150   50   .
    REC22 A1  750  200  170  130  250   75   .
    REC23 A1 1000  150  250  350  250  100   .
    REC24 A1  500  120  150   80  150   50   .
    REC25 A1  750  200  170  130  250   75   .
    REC26 A1 1000  150  250  350  250  100   .
    REC27 A1  500  120  150   80  150   50   .
    REC28 A1  750  200  170  130  250   75   .
    REC29 A1 1000  150  250  350  250  100   .
    REC30 A1  500  120  150   80  150   50   .
    REC31 A1  750  200  170  130  250   75   .
    REC32 A1 1000  150  250  350  250  100   .
    """), sep=r'\s+')

@pytest.fixture
def indata_DonorT14():
    return pd.read_csv(StringIO("""ident area total Q1 Q2 Q3 Q4 staff toexcl
REC01 A1  NaN  120  150   80  150   50   .
REC02 A1  750   200 170  130  -250  75   .
REC04 A1 1000  150  250  350  250  100   .
REC05 A2 1050  200  225  325  300  NaN   .
REC11 A1  500  120  150   80  150  -50   .
REC12 A1  750  200  170  130  250   75   .
REC14 A1 1000  150  250  350  250 -100   .
REC24 A1 1000  150  250  350  250 -100   .
REC22 A1  750  200  170  130  250  NaN   .
REC32 A1  750  200  170  130  250  -75   .
"""), sep=r'\s+', dtype={'ident': str, 'area': str, 'total': float, 'Q1': float, 'Q2': float, 'Q3': float, 'Q4':float, 'staff': float, 'toexcl': str})

@pytest.fixture
def instatus_DonorT14():
    return pd.read_csv(StringIO("""ident fieldid status
REC01 Q3 IPR
REC02 Q3 IPR
REC14 Q3 FTI
REC14 Q4 FTI
REC04 Q3 FTI
REC04 Q4 FTI
REC24 Q3 FTI
REC24 Q4 FTI
"""), sep=r'\s+')

@pytest.fixture
def expected_outdata_01():
    return pd.read_csv(StringIO("""
    ident total  Q1  Q2  Q3  Q4
    REC04  1200 150 250 325 300
     """), sep=r'\s+')

@pytest.fixture
def expected_outdata_02():
    """CHANGES
     - In SAS this included the "area" and "staff" variables
        however these are absent in Python.  
        Why? SAS includes BY variables in outdata, unnecessarily
     - use "REC04" not "REC02", the .docx is wrong """
    return banff.testing.PAT_from_string("""
        s n
        ident total Q1  Q2  Q3  Q4
        REC04 1000  150 250 130 250
    """)

@pytest.fixture
def expected_outdata_03():
    return banff.testing.PAT_from_string("""
        s n
        ident total  Q1  Q2  Q3  Q4
        REC04  750 200 170 80 150
    """)

@pytest.fixture
def expected_outdata_04():
    return banff.testing.PAT_from_string("""
        s n
        ident total  Q1  Q2  Q3  Q4
        REC04 1000  150 250 130 250
    """)

@pytest.fixture
def expected_outstatus_01():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC04      Q3    IDN
    REC04      Q4    IDN
    REC04   STAFF    MFU
    REC04      Q1    MFS
    REC04      Q2    MFS
    REC04   TOTAL    MFS
     """), sep=r'\s+')

@pytest.fixture
def expected_outstatus_01b():
    """differs only in case of strings"""
    return pd.read_csv(StringIO("""
    ident FIELDID STATUS  VALUE
    REC04      Q3    IDN  130.0
    REC04      Q4    IDN  250.0
     """), sep=r'\s+')

@pytest.fixture
def expected_outmatching_fields_01b():
    """differs only in case of strings"""
    return pd.read_csv(StringIO("""
    ident FIELDID STATUS
    REC04      Q3    IDN
    REC04      Q4    IDN
    REC04   staff    MFU
    REC04      Q1    MFS
    REC04      Q2    MFS
    REC04   total    MFS
     """), sep=r'\s+')

@pytest.fixture
def expected_outstatus_02():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC04      Q3    IDN
    REC04      Q4    IDN
     """), sep=r'\s+')

@pytest.fixture
def expected_outdonormap_01():
    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    REC04      REC01  1                  NaN
     """), sep=r'\s+')

@pytest.fixture
def expected_outdonormap_02():
    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    REC04      REC02  1                  NaN
     """), sep=r'\s+')

@pytest.fixture
def expected_outdonormap_03():
    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
     """), sep=r'\s+')

@pytest.fixture
def expected_outdonormap_04():
    return pd.read_csv(StringIO("""
    RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
    REC04      REC32  1                  NaN
     """), sep=r'\s+')

@pytest.fixture
def expected_outdonormap_05():
    return banff.testing.PAT_from_string("""
        s s n n
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC01  2                  NaN
    """)

@pytest.fixture
def JA_indata2():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    """), sep=r'\s+')

@pytest.fixture
def JA_instat6():
    return  pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPE
    REC02 Q2 FTE
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata7():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
	REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    REC06 A1  500  120  150   80  150   50   .
    REC07 A1  750  200  170  130  250   75   .
    REC08 A1 1000  150  250  350  250  100   .
    REC09 A1  500  120  150   80  150   50   .
    REC10 A1  750  200  170  130  250   75   .
    REC11 A1 1000  150  250  350  250  100   .
    REC12 A1  500  120  150   80  150   50   .
    REC13 A1  750  200  170  130  250   75   .
    REC14 A1 1000  150  250  350  250  100   .
    REC15 A1  500  120  150   80  150   50   .
    REC16 A1  750  200  170  130  250   75   .
    REC17 A1 1000  150  250  350  250  100   .
    REC18 A1  500  120  150   80  150   50   .
    REC19 A1  750  200  170  130  250   75   .
    REC20 A1 1000  150  250  350  250  100   .
    REC21 A1  500  120  150   80  150   50   .
    REC22 A1  750  200  170  130  250   75   .
    REC23 A1 1000  150  250  350  250  100   .
    REC24 A1  500  120  150   80  150   50   .
    REC25 A1  750  200  170  130  250   75   .
    REC26 A1 1000  150  250  350  250  100   .
    REC27 A1  500  120  150   80  150   50   .
    REC28 A1  750  200  170  130  250   75   .
    REC29 A1 1000  150  250  350  250  100   .
    REC30 A1  500  120  150   80  150   50   .
    REC31 A1  750  200  170  130  250   75   .
    REC32 A1 1000  150  250  350  250  100   .
    """), sep=r'\s+')

@pytest.fixture
def JA_instat8():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPR 
    REC04 Q2 FTE 
    REC04 Q3 FTI 
    REC04 Q4 FTI 
    """), sep=r'\s+')

@pytest.fixture
def JA_indata9():
    return pd.read_csv(StringIO("""
	REC01 A1  500  120  150   80  150   50   .
	REC02 A1  750  200  170  130  250   75   .
	REC04 A1 1000  150  250  350  250  100   .
	REC05 A2 1050  200  225  325  300  100   E
	REC06 A1  500  120  150   80  150   50   .
	REC07 A1  750  200  170  130  250   75   .
	REC08 A1 1000  150  250  350  250  100   .
	REC09 A1  500  120  150   80  150   50   .
	REC10 A1  750  200  170  130  250   75   .
	REC11 A1 1000  150  250  350  250  100   .
	REC12 A1  500  120  150   80  150   50   .
	REC13 A1  750  200  170  130  250   75   .
	REC14 A1 1000  150  250  350  250  100   .
	REC15 A1  500  120  150   80  150   50   .
	REC16 A1  750  200  170  130  250   75   .
	REC17 A1 1000  150  250  350  250  100   .
	REC18 A1  500  120  150   80  150   50   .
	REC19 A1  750  200  170  130  250   75   .
	REC20 A1 1000  150  250  350  250  100   .
	REC21 A1  500  120  150   80  150   50   .
	REC22 A1  750  200  170  130  250   75   .
	REC23 A1 1000  150  250  350  250  100   .
	REC24 A1  500  120  150   80  150   50   .
	REC25 A1  750  200  170  130  250   75   .
	REC26 A1 1000  150  250  350  250  100   .
	REC27 A1  500  120  150   80  150   50   .
	REC28 A1  750  200  170  130  250   75   .
	REC29 A1 1000  150  250  350  250  100   .
	REC30 A1  500  120  150   80  150   50   .
	REC31 A1  750  200  170  130  250   75   .
	REC32 A1 1000  150  250  350  250  100   .
    """), sep=r'\s+')

@pytest.fixture
def JA_indata10():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
	REC01 A1  500  120  150   80  150   50   .
	REC02 A1  750  200  170  130  250   75   .
	REC04 A1 1000  150  250  350  250  100   .
	REC05 A2 1050  200  225  325  300  100   E
	REC06 A1  500  120  150   80  150   50   .
	REC07 A1  750  200  170  130  250   75   .
	REC08 A1 1000  150  250  350  250  100   .
	REC09 A1  500  120  150   80  150   50   .
	REC10 A1  750  200  170  130  250   75   .
	REC11 A1 1000  150  250  350  250  100   .
	REC12 A1  500  120  150   80  150   50   .
	REC13 A1  750  200  170  130  250   75   .
	REC14 A1 1000  150  250  350  250  100   .
	REC15 A1  500  120  150   80  150   50   .
	REC16 A1  750  200  170  130  250   75   .
	REC17 A1 1000  150  250  350  250  100   .
	REC18 A1  500  120  150   80  150   50   .
	REC19 A1  750  200  170  130  250   75   .
	REC20 A1 1000  150  250  350  250  100   .
	REC21 A1  500  120  150   80  150   50   .
	REC22 A1  750  200  170  130  250   75   .
	REC23 A1 1000  150  250  350  250  100   .
	REC24 A1  500  120  150   80  150   50   .
	REC25 A1  750  200  170  130  250   75   .
	REC26 A1 1000  150  250  350  250  100   .
	REC27 A1  500  120  150   80  150   50   .
	REC28 A1  750  200  170  130  250   75   .
	REC29 A1 1000  150  250  350  250  100   .
	REC30 A1  500  120  150   80  150   50   .
	REC31 A1  750  200  170  130  250   75   .
	REC32 A1 1000  150  250  350  250  100   .
    """), sep=r'\s+')

@pytest.fixture
def JA_indata11():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
	REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  -250  75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    """), sep=r'\s+')

@pytest.fixture
def JA_indata12():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   .
    REC11 A1  500  120  150   80  150   50   .
    REC12 A1  750  200  170  130  250   75   .
    REC14 A1 1000  150  250  350  250  100   .
    REC15 A2 1050  200  225  325  300  100   .
    REC21 A1  500  120  150   80  150   50   .
    REC22 A1  750  200  170  130  250   75   .
    REC24 A1 1000  150  250  350  250  100   .
    REC25 A2 1050  200  225  325  300  100   .
    REC31 A1  500  120  150   80  150   50   .
    REC32 A1  750  200  170  130  250   75   .
    REC34 A1 1000  150  250  350  250  100   .
    REC35 A2 1050  200  225  325  300  100   .
    REC41 A1  500  120  150   80  150   50   .
    REC42 A1  750  200  170  130  250   75   .
    REC44 A1 1000  150  250  350  250  100   .
    REC45 A2 1050  200  225  325  300  100   .
    REC51 A1  500  120  150   80  150   50   .
    REC52 A1  750  200  170  130  250   75   .
    REC54 A1 1000  150  250  350  250  100   .
    REC55 A2 1050  200  225  325  300  100   .
    """), sep=r'\s+')    

@pytest.fixture
def JA_instat13():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC02 Q3 IDE
    REC12 Q3 IDE
    REC22 Q3 IDE
    REC32 Q3 IDE
    REC42 Q3 IDE
    REC52 Q3 IDE
    REC04 Q3 FTI 
    REC04 Q4 FTI 
    REC14 Q3 FTI 
    REC14 Q4 FTI 
    REC24 Q3 FTI 
    REC24 Q4 FTI 
    REC34 Q3 FTI 
    REC34 Q4 FTI 
    REC44 Q3 FTI 
    REC44 Q4 FTI 
    REC54 Q3 FTI 
    REC54 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata14():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    RECD10 A1 996   146  246  346  246  96   .
    RECD20 A1 997   147  247  347  247  97   .
    RECD30 A1 1002  152  252  352  252  102  .
    RECD40 A1 999   149  249  349  249  99   .
    RECD50 A1 1000  150  250  350  250  100  .
    RECD60 A1 1001  151  251  351  251  101  .
    RECD70 A1 998   148  248  348  248  98   .
    RECD71 A1 998   148  248  348  248  98   .
    RECD72 A1 998   148  248  348  248  98   .
    RECD73 A1 998   148  248  348  248  98   .
    RECD74 A1 998   148  248  348  248  98   .
    RECD75 A1 998   148  248  348  248  98   .
    RECD76 A1 998   148  248  348  248  98   .
    RECD77 A1 998   148  248  348  248  98   .
    RECD78 A1 998   148  248  348  248  98   .
    RECD79 A1 998   148  248  348  248  98   .
    RECD80 A1 1003   153  253  353  253  103   .
    RECD81 A1 1003   153  253  353  253  103   .
    RECD82 A1 1003   153  253  353  253  103   .
    RECD83 A1 1003   153  253  353  253  103   .
    RECD84 A1 1003   153  253  353  253  103   .
    RECD85 A1 1003   153  253  353  253  103   .
    RECD86 A1 1003   153  253  353  253  103   .
    RECD87 A1 1003   153  253  353  253  103   .
    RECD88 A1 1003   153  253  353  253  103   .
    RECD89 A1 1003   153  253  353  253  103   .
    RECR57 A1 1000  150  250  350  250  100  .
    """), sep=r'\s+')

@pytest.fixture
def JA_instat15():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    RECD20 Q3 IPR
    RECD30 Q3 FTE
    RECD40 Q3 FTE
    RECD50 Q3 FTE
    RECD60 Q3 FTE
    RECR57 Q3 FTI
    RECR57 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata16():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC11 A1  500  120  150   80 -150  -50   .
    REC02 A1  750  200  170  130  250   75   E
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    """), sep=r'\s+')

@pytest.fixture
def JA_instat17():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC02 Q3 IPR
    REC04 Q3 FTI 
    REC04 Q4 FTI 
    REC11 Q3 IPR
    """), sep=r'\s+')

@pytest.fixture
def JA_indata18():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    " " A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   E
    """), sep=r'\s+')

@pytest.fixture
def JA_instat19():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPR
    REC02 Q3 IPR
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata20():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1   Q2   Q3   Q4    STAFF TOEXCL
    REC01 A1    NaN  120  150   80  150   50    .
    REC02 A1    750  200  170  130  -250  75    .
    REC04 A1   1000  150  250  350  250   100   .
    REC05 A2   1050  200  225  325  300   NaN   .
    REC11 A1    500  120  150   80  150   -50   .
    REC12 A1    750  200  170  130  250   75    .
    REC14 A1   1000  150  250  350  250   -100  .
    REC24 A1   1000  150  250  350  250   -100  .
    REC22 A1   750   200  170  130  250   NaN   .
    REC32 A1   750   200  170  130  250   -75   .
    """), sep=r'\s+')

@pytest.fixture
def JA_instat21():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPR
    REC02 Q3 IPR
    REC14 Q3 FTI
    REC14 Q4 FTI
    REC22 staff FTI
    REC32 staff FTI
    REC04 Q3 FTI
    REC04 Q4 FTI
    REC24 Q3 FTI
    REC24 Q4 FTI
    REC24 staff FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata22():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
    REC02 A1  750  200  170  130  250   75   .
    REC04 A1 1000  150  250  350  250  100   .
    REC05 A2 1050  200  225  325  300  100   .
    """), sep=r'\s+')

@pytest.fixture
def JA_instat23():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPR
    REC02 STAFF FTI
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_instat24():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    " " Q3 IPR
    REC02 " " IPR
    REC04 Q3 FTI
    REC04 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata27():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
    REC01 A1  500  120  150   80  150   50   .
	REC02 A1  750  200  170  130  250   75   .
	REC04 A1 1000  150  250  350  250  100   .
	REC05 A2 1050  200  170  325  300   75   .
	REC11 A1  500  120  150   80  150   50   .
	REC12 A1  750  200  170  130  250   75   .
	REC14 A1 1000  150  250  350  250  100   .
	REC15 A2 1050  200  170  325  300   75   .
	REC21 A1  500  120  150   80  150   50   .
	REC22 A1  750  200  170  130  250   75   .
	REC24 A1 1000  150  250  350  250  100   .
	REC25 A2 1050  200  225  325  300  100   .
	REC31 A1  500  120  150   80  150   50   .
	REC32 A1  750  200  170  130  250   75   .
	REC34 A1 1000  150  250  350  250  100   .
	REC35 A2 1050  200  225  325  300  100   .
	REC41 A1  500  120  150   80  150   50   .
	REC42 A1  750  200  170  130  250   75   .
	REC44 A1 1000  150  250  350  250  100   .
	REC45 A2 1050  200  225  325  300  100   .
	REC51 A1  500  120  150   80  150   50   .
	REC52 A1  750  200  170  130  250   75   .
	REC54 A1 1000  150  250  350  250  100   .
	REC55 A2 1050  200  225  325  300  100   .
    """), sep=r'\s+') 

@pytest.fixture
def JA_instat28():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC02 Q3 IDE
    REC12 Q3 IDE
    REC22 Q3 IDE
    REC32 Q3 IDE
    REC42 Q3 IDE
    REC52 Q3 IDE
    REC04 Q3 FTI 
    REC04 Q4 FTI 
    REC14 Q3 FTI 
    REC14 Q4 FTI 
    REC24 Q3 FTI 
    REC24 Q4 FTI 
    REC34 Q3 FTI 
    REC34 Q4 FTI 
    REC44 Q3 FTI 
    REC44 Q4 FTI 
    REC54 Q3 FTI 
    REC54 Q4 FTI 
    """), sep=r'\s+')

@pytest.fixture
def JA_indata29():
    return pd.read_csv(StringIO("""
    IDENT AREA TOTAL Q1 Q2 Q3 Q4 STAFF TOEXCL
	REC01 A1    .  120  150   80  150   50   .
	REC02 A1  750   200 170  130  -250  75   .
	REC04 A1 1000  150  250  350  250  100   .
	REC05 A2 1050  200  225  325  300    .   .
	REC11 A1  500  120  150   80  150  -50   .
	REC12 A1  750  200  170  130  250   75   .
	REC14 A1 1000  150  250  350  250 -100   .
	REC24 A1 1000  150  250  350  250 -100   .
	REC22 A1  750  200  170  130  250    .   .
	REC32 A1  750  200  170  130  250  -75   .
    """), sep=r'\s+')

@pytest.fixture
def JA_instat30():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3 IPR
    REC02 Q3 IPR
    REC14 Q3 FTI
    REC14 Q4 FTI
    REC04 Q3 FTI
    REC04 Q4 FTI
    REC24 Q3 FTI
    REC24 Q4 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata32():
    return pd.read_csv(StringIO("""
    IDENT AREA VAR1 VAR2
	R01 1 1 4
	R02 1 2 5
	R03 1 3 6
	R04 1 NaN 4
	R05 1 NaN 4
	R06 1 NaN 4
	R07 1 NaN 4
	R08 1 NaN 4
	R09 1 NaN 4
	R10 1 NaN 4
	R11 1 NaN 4
	R12 1 NaN 4
	R13 1 NaN 4
	R14 1 NaN 4
	R15 1 NaN 4
	R16 2 1 7
	R17 2 2 8
	R18 2 3 9
	R19 2 4 10
	R20 2 5 11
	R21 2 6 12
	R22 2 NaN 7
	R23 2 NaN 7
	R24 2 NaN 7
	R25 2 NaN 8
	R26 2 NaN 8
	R27 2 NaN 8
	R28 2 NaN 9
	R29 2 NaN 9
	R30 2 NaN 9
	R31 3 1 13
	R32 3 2 14
	R33 3 3 15
	R34 3 4 16
	R35 3 5 17
	R36 3 6 18
	R37 3 6 19
	R38 3 7 20
	R39 3 9 21
	R40 3 10 22
	R41 3 11 23
	R42 3 12 24
	R43 3 NaN 13
	R44 3 NaN 13
	R45 3 NaN 13
	R46 4 1 25
	R47 4 2 25
	R48 4 3 27
	R49 4 4 27
	R50 4 5 28
	R51 4 NaN 25
	R52 4 NaN 25
	R53 4 NaN 25
	R54 4 NaN 25
	R55 4 NaN 25
	R56 4 NaN 26
	R57 4 NaN 26
	R58 4 NaN 26
	R59 4 NaN 26
	R60 4 NaN 26
	R61 4 NaN 26
    """), sep=r'\s+')

@pytest.fixture
def JA_indata32_exclude():
    return pd.read_csv(StringIO("""
    IDENT AREA VAR1 VAR2 EXCLUDE
	R01 1 1 4 "E"
	R02 1 2 5 " "
	R03 1 3 6 " "
	R04 1 NaN 4 " "
	R05 1 NaN 4 " "
	R06 1 NaN 4 " "
	R07 1 NaN 4 " "
	R08 1 NaN 4 " "
	R09 1 NaN 4 " "
	R10 1 NaN 4 " "
	R11 1 NaN 4 " "
	R12 1 NaN 4 " "
	R13 1 NaN 4 " "
	R14 1 NaN 4 " "
	R15 1 NaN 4 " "
	R16 2 1 7 "E"
	R17 2 2 8 "E"
	R18 2 3 9 " "
	R19 2 4 10 " "
	R20 2 5 11 " "
	R21 2 6 12 " "
	R22 2 NaN 7 " "
	R23 2 NaN 7 " "
	R24 2 NaN 7 " "
	R25 2 NaN 8 " "
	R26 2 NaN 8 " "
	R27 2 NaN 8 " "
	R28 2 NaN 9 " "
	R29 2 NaN 9 " "
	R30 2 NaN 9 " "
	R31 3 1 13 " "
	R32 3 2 14 " "
	R33 3 3 15 " "
	R34 3 4 16 " "
	R35 3 5 17 " "
	R36 3 6 18 " "
	R37 3 6 19 " "
	R38 3 7 20 " "
	R39 3 9 21 " "
	R40 3 10 22 " "
	R41 3 11 23 " "
	R42 3 12 24 " "
	R43 3 NaN 13 " "
	R44 3 NaN 13 " "
	R45 3 NaN 13 " "
	R46 4 1 25 "E"
	R47 4 2 25 "E"
	R48 4 3 27 " "
	R49 4 4 27 " "
	R50 4 5 28 " "
	R51 4 NaN 25 " "
	R52 4 NaN 25 " "
	R53 4 NaN 25 " "
	R54 4 NaN 25 " "
	R55 4 NaN 25 " "
	R56 4 NaN 26 " "
	R57 4 NaN 26 " "
	R58 4 NaN 26 " "
	R59 4 NaN 26 " "
	R60 4 NaN 26 " "
	R61 4 NaN 26 " "
    """), sep=r'\s+')

@pytest.fixture
def JA_instat33():
    return pd.read_csv(StringIO("""
    IDENT AREA FIELDID STATUS
    R04 1 VAR1 FTI
    R05 1 VAR1 FTI
    R06 1 VAR1 FTI
    R07 1 VAR1 FTI
    R08 1 VAR1 FTI
    R09 1 VAR1 FTI
    R10 1 VAR1 FTI
    R11 1 VAR1 FTI
    R12 1 VAR1 FTI
    R13 1 VAR1 FTI
    R14 1 VAR1 FTI
    R15 1 VAR1 FTI
    R22 2 VAR1 FTI
    R23 2 VAR1 FTI
    R24 2 VAR1 FTI
    R25 2 VAR1 FTI
    R26 2 VAR1 FTI
    R27 2 VAR1 FTI
    R28 2 VAR1 FTI
    R29 2 VAR1 FTI
    R30 2 VAR1 FTI
    R43 3 VAR1 FTI
    R44 3 VAR1 FTI
    R45 3 VAR1 FTI
    R51 4 VAR1 FTI
    R52 4 VAR1 FTI
    R53 4 VAR1 FTI
    R54 4 VAR1 FTI
    R55 4 VAR1 FTI
    R56 4 VAR1 FTI
    R57 4 VAR1 FTI
    R58 4 VAR1 FTI
    R59 4 VAR1 FTI
    R60 4 VAR1 FTI
    R61 4 VAR1 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_instat33_b():
    return pd.read_csv(StringIO("""
    IDENT AREA FIELDID STATUS
    R04 1 VAR1 FTE
    R05 1 VAR1 FTE
    R06 1 VAR1 FTE
    R07 1 VAR1 FTI
    R08 1 VAR1 FTI
    R09 1 VAR1 FTI
    R10 1 VAR1 FTI
    R11 1 VAR1 FTI
    R12 1 VAR1 FTI
    R13 1 VAR1 FTI
    R14 1 VAR1 FTI
    R15 1 VAR1 FTI
    R22 2 VAR1 FTE
    R23 2 VAR1 FTE
    R24 2 VAR1 FTE
    R25 2 VAR1 FTI
    R26 2 VAR1 FTI
    R27 2 VAR1 FTI
    R28 2 VAR1 FTI
    R29 2 VAR1 FTI
    R30 2 VAR1 FTI
    R43 3 VAR1 FTI
    R44 3 VAR1 FTI
    R45 3 VAR1 FTI
    R51 4 VAR1 FTE
    R52 4 VAR1 FTI
    R53 4 VAR1 FTI
    R54 4 VAR1 FTI
    R55 4 VAR1 FTI
    R56 4 VAR1 FTI
    R57 4 VAR1 FTI
    R58 4 VAR1 FTI
    R59 4 VAR1 FTI
    R60 4 VAR1 FTI
    R61 4 VAR1 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata34():
    return pd.read_csv(StringIO("""
    IDENT AREA VAR1 VAR2
	R01 1 1 4
    R02 1 2 5
    R03 1 3 6
    R04 1 . 4
    R05 1 . 4
    R06 1 . 4
    R07 1 . 4
    R08 1 . 4
    R09 1 . 4
    R10 1 . 4
    R11 1 . 4
    R12 1 . 4
    R13 1 . 4
    R14 1 . 4
    R15 1 . 4
    R16 2 1 7
    R17 2 2 8
    R18 2 3 9
    R19 2 4 10
    R20 2 5 11
    R21 2 6 12
    R22 2 . 7
    R23 2 . 7
    R24 2 . 7
    R25 2 . 8
    R26 2 . 8
    R27 2 . 8
    R28 2 . 9
    R29 2 . 9
    R30 2 . 9
    R31 3 1 13
    R32 3 2 14
    R33 3 3 15
    R34 3 4 16
    R35 3 5 17
    R36 3 6 18
    R37 3 6 19
    R38 3 7 20
    R39 3 9 21
    R40 3 10 22
    R41 3 11 23
    R42 3 12 24
    R43 3 . 13
    R44 3 . 13
    R45 3 . 13
    R46 4 1 25
    R47 4 2 25
    R48 4 3 27
    R49 4 4 27
    R50 4 5 28
    R51 4 . 25
    R52 4 . 25
    R53 4 . 25
    R54 4 . 25
    R55 4 . 25
    R56 4 . 26
    R57 4 . 26
    R58 4 . 26
    R59 4 . 26
    R60 4 . 26
    R61 4 . 26
    """), sep=r'\s+')

@pytest.fixture
def JA_indata35():
    return pd.read_csv(StringIO("""
    IDENT AREA VAR1 VAR2
    R01 1 1 4
    R02 1 2 5
    R03 1 NaN 5
    R04 1 NaN 5
    R05 1 NaN 5
    R06 1 NaN 5
    R07 1 NaN 5
    R08 1 NaN 5
    R09 1 NaN 5
    R10 1 NaN 5
    R11 1 NaN 5
    R12 1 NaN 5
    R13 1 NaN 5
    R14 1 NaN 5
    R15 1 NaN 5
    """), sep=r'\s+')

@pytest.fixture
def JA_instat36():
    return pd.read_csv(StringIO("""
    IDENT AREA FIELDID STATUS
    R03 1 VAR1 FTI
    R04 1 VAR1 FTI
    R05 1 VAR1 FTI
    R06 1 VAR1 FTI
    R07 1 VAR1 FTI
    R08 1 VAR1 FTI
    R09 1 VAR1 FTI
    R10 1 VAR1 FTI
    R11 1 VAR1 FTI
    R12 1 VAR1 FTI
    R13 1 VAR1 FTI
    R14 1 VAR1 FTI
    R15 1 VAR1 FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata37():
    return pd.read_csv(StringIO("""
    IDENT TOTAL Q1 Q2 Q3 Q4 STAFF PROV
	REC01  500  120  150   80  150   50 24
	REC02  750  200  170  130  250   75 24
	REC03  400   80   90  100  130   40 24
	REC04 1000  150  250  350  250  100 24 
	REC05 1050  200  225  325  300  100 24
	REC06  500  100  125 5000  130   45 24
	REC07  400   80   90  100   15   40 30
	REC08  950  150  999  999  200   90 30
	REC09 1025  200  225  300   10   10 30
	REC10  800   11   12   13   14   80 30
	REC11  -25  -10   -5   -5  -10 3000 30
	REC12 1000  150  250  350  250  100 30
	REC13  999  200  225  325  300  100 30
	REC14  -25  -10   -5  -10   -5 3000 30
    """), sep=r'\s+')

@pytest.fixture
def JA_instat38():
    return pd.read_csv(StringIO("""
    IDENT FIELDID STATUS
    REC01 Q3    IPR
    REC04 Q4    FTE
    REC06 Q3    FTI
    REC07 Q2    FTI
    REC08 Q2    FTI
    REC08 Q3    FTI
    REC09 Q4    FTI
    REC09 STAFF FTI
    REC10 Q1    FTI
    REC10 Q2    FTI
    REC10 Q3    FTI
    REC10 Q4    FTI
    REC11 Q1    FTI
    REC11 Q2    FTI
    REC11 Q3    FTI
    REC11 Q4    FTI
    REC13 Q2    FTI
    REC13 Q3    FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata39():
    foo = pd.read_csv(StringIO("""
    REC X Y RANDKEY
    101  2  3   0.00
    102  2  3   0.01
    103  2  3   0.99
    104  2  3   1.00
    105  2  3  -0.01
    106  2  3   1.01
    107  2  3   NaN
    201  2  NaN   0.66
    """), sep=r'\s+')
    foo['REC'] = foo['REC'].astype(str)
    
    return foo

@pytest.fixture
def JA_instat40():
    foo = pd.read_csv(StringIO("""
    REC FIELDID STATUS
    201 Y FTI 
    """), sep=r'\s+')

    foo['REC'] = foo['REC'].astype(str)
    
    return foo

@pytest.fixture
def JA_indata41():
    return pd.read_csv(StringIO("""
    REC X Y RANDKEY
    101  2  3   0.00
    102  2  3   0.01
    201  2  .   0.49
    202  2  .   0.51
    """), sep=r'\s+')

@pytest.fixture
def JA_instat42():
    return pd.read_csv(StringIO("""
    REC FIELDID STATUS
    201 Y FTI 
    202 Y FTI
    """), sep=r'\s+')

@pytest.fixture
def JA_indata43():
    return pd.read_csv(StringIO("""
    REC X Y RANDKEY
    101  2  3   0.00
    102  2  3   0.01
    201  2  .   1.00
    """), sep=r'\s+')

@pytest.fixture
def JA_instat44():
    return pd.read_csv(StringIO("""
    REC FIELDID STATUS
    201 Y FTI 
    """), sep=r'\s+')

@pytest.fixture
def JA_indata45():
    foo = pd.read_csv(StringIO("""
            REC X Y RANDKEY
            106  2  3   1.01
            """), sep=r'\s+')
    
    foo['REC'] = foo['REC'].astype(str)

    return foo

@pytest.fixture
def JA_indata46():
    foo = pd.read_csv(StringIO("""
            REC X Y RANDKEY
            107  2  3   NaN
            """), sep=r'\s+')
    
    foo['REC'] = foo['REC'].astype(str)

    return foo

@pytest.fixture
def expected_outdonormap_06():
    return banff.testing.PAT_from_string("""
        s s n n
        RECIPIENT  DONOR  NUMBER_OF_ATTEMPTS DONORLIMIT
        REC04      REC02  1                  NaN
        REC14      REC32  1                  NaN
        REC24      REC32  1                  NaN
    """)