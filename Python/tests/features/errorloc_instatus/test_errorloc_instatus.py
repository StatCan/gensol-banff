"""Test errorloc's python only instatus dataset
"""
import banff.testing as banfftest
import banff.testing
import pandas as pd
import pyarrow as pa
from io import StringIO
import pytest
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

seed=3212024

@pytest.mark.m_auto_pass
def test_errorloc_instatus_a(capfd, indata, outstatus_no_instat):
    """Call errorloc without an instatus dataset, validate output as expected"""

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=indata,
        edits="Var1+Var2+Var3=Total;",
        weights="Var3=3; Total=10;",
        accept_negative=False,
        seed=seed,
        unit_id="RecordID",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=outstatus_no_instat,
    )

@pytest.mark.m_auto_pass
def test_errorloc_instatus_b(capfd, indata, instatus, outstatus_with_instat):
    """Call errorloc WITH an instatus dataset, validate output as expected"""

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=indata,
        instatus=instatus,
        edits="Var1+Var2+Var3=Total;",
        weights="Var3=3; Total=10;",
        accept_negative=False,
        seed=seed,
        unit_id="RecordID",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=outstatus_with_instat,
    )

@pytest.mark.m_auto_pass
def test_errorloc_instatus_c(capfd, indata, instatus, outstatus_with_instat):
    """Using Pyarrow Tables, Call errorloc WITH an instatus dataset, validate output as expected"""

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=pa.Table.from_pandas(indata),
        instatus=pa.Table.from_pandas(instatus),
        edits="Var1+Var2+Var3=Total;",
        weights="Var3=3; Total=10;",
        accept_negative=False,
        seed=seed,
        unit_id="RecordID",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=outstatus_with_instat,
    )

@pytest.mark.m_auto_pass
def test_errorloc_instatus_d(capfd, indata, instatus_different_case, outstatus_with_instat):
    """Call errorloc with an instatus dataset that has different case
    for some variables compared with indata (i.e. Var1, ToTaL instead of var1, total).  
    Validate output as expected"""

    banfftest.pytest_errorloc(
        # Procedure Parameters
        indata=indata,
        instatus=instatus_different_case,
        edits="Var1+Var2+Var3=Total;",
        weights="Var3=3; Total=10;",
        accept_negative=False,
        seed=seed,
        unit_id="RecordID",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=outstatus_with_instat,
    )

@pytest.mark.m_auto_pass
def test_errorloc_instatus_e(capfd, errorloc_indata_01, errorloc_outstatus_01):
    """Test for expected output from errorloc procedure

    This particular case triggered a bug related to status codes not being reset in C code.  
    This bug was introduced during the initial implementation of C-code instatus for errorloc.  
    """
    banff.testing.pytest_errorloc(
        indata=errorloc_indata_01,
        accept_negative=True,
        by='area',
        cardinality=13,
        edits='HEN_LT20 + HEN_GE20 + HEN_OTH = HEN_TOT; 2*EGG_LAID <= HEN_GE20; HEN_GE20 <= 4*EGG_LAID; EGG_SOLD <= EGG_LAID; EGG_VALU <= 2.75*EGG_SOLD; 0.9*EGG_SOLD <= EGG_VALU; HEN_GE20 <= 300000; QR_REV = QR_EXP + QR_PROF; 0.5*QR_REV <= 23*EGG_VALU; 23*EGG_VALU <= 1.5*QR_REV; HEN_LT20 >= 0; HEN_OTH >= 0; QR_EXP >= 0;',
        seed=1,
        time_per_obs=20,
        unit_id='ident',
        weights='HEN_TOT=5.0; HEN_LT20=2.0; HEN_GE20=2.0; HEN_OTH=2.0;',

        pytest_capture=capfd,
        expected_outstatus=errorloc_outstatus_01,
    )

##### DATA #####

@pytest.fixture
def indata():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['RecordID'] = str
    
    return pd.read_csv(StringIO("""
        RecordID	var1	var2	var3	total
        R0001	58	88	93	NaN
        R0002	40	89	85	215
        R0003	22	39	50	112
        R0004	-1	2	61	81
        R0005	73	21	83	177
        R0006	17	23	21	61
        R0007	47	40	39	126
        R0008	5	58	47	110
        R0009	65	30	42	137
        R0010	29	7	71	107
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def instatus():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        FieldID	RecordID	Status
        var3	R0002	FTI
        total	R0003	FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def instatus_different_case():
    """instatus file with capitalization for FieldID values which
    differs from indata.  """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        FieldID	RecorDID	Status
        Var3	R0002	FTI
        ToTaL	R0003	FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outstatus_no_instat():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    # BEWARE: values in `VALUE` differ from the associated value in `indata` due to 
    # pre-C implementation of `instatus`
    return pd.read_csv(StringIO("""
        RecordID FIELDID STATUS  VALUE
        R0001 total FTI    NaN
        R0002 var1 FTI   40
        R0003 var2 FTI   39
        R0004 var1 FTI   -1.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outstatus_with_instat():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    # BEWARE: values in `VALUE` differ from the associated value in `indata` due to 
    # pre-C implementation of `instatus`
    return pd.read_csv(StringIO("""
        RecordID FIELDID STATUS   VALUE
        R0001 total FTI     NaN
        R0002 var3 FTI     85
        R0003 total FTI     112
        R0004 var1 FTI    -1.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def errorloc_indata_01():
    """valid indata dataset
    """

    return banff.testing.PAT_from_string("""
        s n
        IDENT  AREA  STRAT  HEN_LT20  HEN_GE20  HEN_OTH   HEN_TOT  EGG_LAID   EGG_SOLD  EGG_VALU   QR_REV   QR_EXP  QR_PROF  
        R001    90      1       0.0       480      0.0     480.0       226        225       338     7843      749     7094  
        R002    90      1       0.0       500      0.0     500.0       230        227       374     8397      973     7424  
        R003    90      1       0.0       490      0.0     490.0       220        219       278     6123     3838     2285  
        R004    90      1       0.0       165      0.0     165.0        35         34        59     1448     1306      142  
        R005    91      2     100.0       175      0.0     275.0        78         75       101     2352     1864      595  
        R006    91      1       0.0        55      0.0      55.0        15         10        15      311      443     -132  
        R007    91      2     173.0       150      0.0     323.0        64         61        67     1533     2603     -929  
        R008    91      3       0.0       300    100.0     400.0       105        102       155     3384     3351       33  
        R009    91      1       0.0        75      0.0      75.0        35         33        41      827      301      526  
        R010    91      2       0.0       198      0.0     198.0        76         74       104     2563     1059     1504  
        R011    91      2     100.0         0      0.0     100.0         0          0         0        0      695     -695  
        R012    91      2       0.0       260      0.0     260.0       112        107       136     2858     1371     1487  
        R013    91      2       0.0        24      0.0      24.0       -12        -11        14      295       46      249  
        R014    91      1       0.0        80     25.0     155.0        23         21        26      587      301      286  
        R015    91      1     140.0       200      0.0     340.0         0         78        78     1771     1302      469  
        R016    91      3       0.0       300      0.0     300.0       120        119       139     3700     1165     2535  
        R017    91      2     100.0       230      0.0     330.0        76         75        75     1731     1238      493  
        R018    91      1       0.0        70      0.0      70.0        35         33        33      897      561      336  
        R019    91      1       0.0       100      0.0     100.0        20         18        22      476      273      203  
        R020    91      2     300.0       300      0.0     600.0       128        125       150     3689     3294      395  
        R021    91      2       0.0       165      0.0     165.0        73         71        89     2243     1377      866  
        R022    91      1       0.0        50      0.0      50.0        20         17        24      616      224      392  
        R023    91      1       0.0       125      0.0     125.0        35         30        38      852      845       36  
        R024    92      2     135.0        90      0.0     225.0        35         33        40     1025      872      153  
        R025    92      1      60.0        65      1.0     126.0        25         24        31      649      807     -158  
        R026    92      1       0.0        40      0.0      40.0        10          5        88     2037      218     1819  
        R027    92      1       0.0        60      0.0      60.0        25         21        32      789      121      668  
        R028    92      3       0.0       500      0.0     500.0       262        260       247     5739     3044     2695  
        R029    92      1       NaN       150      NaN     150.0        53         50        60     1280      709      571  
        R030    92      3       0.0       420      0.0     420.0       175        172       272     5789     1235     4554  
        R031    92      1       0.0        50     50.0     100.0        14         12        15      320      285       35  
        R032    92      1       0.0        95      0.0      95.0        42         36        50     1064      149      915  
        R033    92      3       0.0       475      0.0     475.0       210        205       324     7315     1346     5716  
        R034    92      2       0.0       280      0.0     280.0       135        130       169     3452     1161     2291  
        R035    92      3       0.0       490      6.0     496.0       210        208       318     6972     2986     4257  
        R036    92      1      59.0       141     12.0     212.0        58         54        81     2054     1323      731  
        R037    92      1       6.0        18      0.0      24.0         5          3         5      125      109       16  
        R038    92      1       0.0        20      0.0      20.0        10          0         0        0       90      -90  
        R039    92      1       0.0        48      0.0      48.0         5          3         5      107      125      -18  
        R040    92      1     170.0        28      0.0     198.0         4          2         2       42      307     -265  
        R041    92      1       0.0       100      0.0     100.0        29          0         0        0      359     -359  
        R042    93      2       0.0        30      0.0      30.0         8          0         0        0       60      -60  
        R043    93      2     100.0         0      0.0     100.0         0          0         0        0      177     -177  
        R044    93      2     100.0        62      0.0     162.0        30         28        35      788      723       65  
        R045    93      3       0.0        11     24.0      35.0         3          0         0        0      176     -176  
        R046    93      2      -1.0        -1     -1.0      -1.0        -1         -1        -1      -27       -2      -24  
        R047    93      2      86.0        70      0.0     156.0        43         41        66     1547      611      936  
        R048    93      3       0.0       180      0.0     180.0        12         11        17      430      408       22  
        R049    93      3       0.0       300      6.0     306.0       105        102       153     3965     1925     2040  
        R050    93      3     100.0       100     20.0     220.0       102         98       147     2421     1801     1620  
        R051    93      3       0.0        50      0.0      50.0        10          8        12      284      113      171  
        R052    93      2       0.0       100      0.0     100.0        10          5         8      180      708     -528  
        R053    93      2       0.0       130      0.0     130.0        25         24        36      790      959     -169  
        R054    93      3     125.0       130      0.0     255.0        80         78       117     2433      823     1610  
        R055    93      2       0.0       165      0.0     165.0        77         75        94     2184      256     1928  
        R056    93      1      50.0        40      0.0      98.0        20          6         9      213      705     -492  
        R057    93      3     185.0       150      0.0     335.0        56         54        81     2055     1459      596  
        R058    93      3       0.0       160      0.0     160.0        80         77       110     2666      485     2181  
        R059    93      3       0.0       195     30.0     225.0        80         77        98     2308      421     1887  
        R060    93      2       0.0        75      0.0      75.0        33         30        38      893      300      593  
        R061    93      2     100.0        90      0.0     190.0        50         47        59     1514      845      669  
        R062    93      2       0.0       100      0.0     100.0        35         34        46     1029      418      611  
        R063    93      2       0.0       100      0.0     100.0        56          1         1       27      569     -542  
        R064    93      3       0.0       200     25.0     225.0       105        100       150     3897     1201     2696  
        R065    93      3       0.0        85      0.0      85.0        24         26        34      731      265      466  
        R066    93      2       0.0        40      0.0      40.0        10          6        10      229      216       13  
        R067    93      3      25.0        70      0.0      95.0        35         28        46      971      702      269  
        R068    94      1       0.0       125      0.0     125.0        40         35        53     1138      880      258  
        R069    94      2       0.0         0      0.0       0.0         0          0         0        0        0        0  
        R070    94      3       0.0       175      0.0     175.0        60         60        84     1871      584     1287  
        R071    94      3       0.0        50      0.0      50.0        21         20        30      755      283      472  
        R072    94      1       0.0       150      0.0     150.0        65         60        90     2308      505     1803  
        R073    94      3       0.0       245     70.0     315.0       110        110       121     2934     1717     1217  
        R074    94      1       0.0        50     25.0      75.0        15          0         0        0      366     -366  
        R075    94      1       0.0        50     25.0      75.0        22         20        30      704      264      440  
        R076    94      3       3.0       250      0.0       NaN        50         90        90     1738        0     1738  
        R077    94      4       0.0      6000   5700.0   11700.0      1500       1500       600    69985    67876   -59095  
        R078    94      3       0.0       225     25.0     250.0        75         75       112     2659      994     1665  
        R079    94      2       0.0       220      0.0     220.0        56         30        51     1198      977      221  
        R080    94      1       0.0         0     24.0      24.0         0          0         0        0       92      -92  
        R081    94      5       0.0      8500      0.0    8500.0      4230       4230      4400   101649    37732    63917  
        R082    94      2     100.0       150      0.0     250.0        45         45        58     1399      624      775  
        R083    94      3       0.0       175     60.0     235.0        85         82       103     2451     1329     1122  
        R084    94      5       0.0      7900      0.0    7900.0      3640       3640      3822   103082    23437    79645  
        R085    94      5    9970.0     12630   2880.0   25480.0      6195       6195      7446   180369   134438    45931  
        R086    94      5       0.0         0      0.0       0.0         0          0         0        0        0        0  
        R087    94      7       0.0     54700  34000.0   88700.0     19210      19210     17300   317197   357110   -39913  
        R088    94      1       0.0        25      0.0      25.0         8          0         0        0      184     -184  
        R089    94      1       0.0        50      0.0      50.0        24         22        31      617      357      260  
        R090    94      1       0.0        30      0.0      30.0        12          0         0        0       72      -72  
        R091    94      1      60.0        55      0.0     115.0        21         18        25      610      789     -179  
        R092    94      2      -1.0        -1     -1.0      -1.0        -1         -1        -1      -25       -3      -22  
        R093    94      3     200.0       150      5.0     355.0        65         50        63     1365     2086    -1026  
        R094    94      3       0.0       250      0.0     250.0       105        103       144     3088     1577     1653  
        R095    94      2       0.0        60      0.0      60.0        23         21        32      816      381      435  
        R096    94      4       0.0      2800   2000.0    4800.0      1545       1545      1622    43323    29626    13697  
        R097    94      7       0.0     38000      0.0   38000.0     16800      16800     17136   395395   318503    76892  
        R098    94      5       0.0     43500  13600.0   57100.0     20000      20000     20000   423091   437048   -13957  
        R099    94      7   14000.0     34000    800.0   48800.0     15750      15750     16225   440348   313547   126775  
        R100    94      3      10.0       250     11.0     271.0       115        115       161     3748     1563     2185  
        R101    94      5    9000.0      7500  12100.0   25600.0      2680       2680      8000   185277   174006    11271  
        R102    94      4       0.0      7500      0.0    7500.0      2575       2575      2630    49107    53196    -4089  
        R103    94      3       NaN       200      NaN     200.0        30         30        45      938     1621     -683  
        R104    94      1      40.0        40      0.0      80.0        15         10        15      379      458      -79  
        R105    94      2       0.0       130      0.0     130.0        40         40        40      749      439      310  
        R106    94      2      50.0       150     12.0     212.0        52         50        84     2062     1642      420  
        R107    94      6       0.0     28400      0.0   28400.0     14100      14100     14664   366614   144400   222214  
        R108    94      6    6600.0     26000   9000.0   41600.0     12250      12250     12250   266556   131435   135121  
        R109    94      6       0.0         0  22000.0   22000.0         0          0         0        0    90377   -90377  
        R110    94      4       0.0      4929      0.0    4929.0      2457       2457      2457    48230    10525    43413  
        R111    94      3       0.0       250    140.0     390.0       120        120       180     4485     2256     2229  
        R112    94      1       0.0        15      0.0      15.0         7          0         0        0       48      -48  
        R113    94      6       0.0     23800  11600.0   35400.0     10938      10938     11249   243958   117437   126521  
        R114    94      7    9000.0     36348   7888.0   53236.0     14000          0         0        0   392290  -392290  
        R115    94      7       0.0     47000      0.0   47000.0     22050      22050     26500   549236   172953   376283  
        R116    94      7   20000.0     42000      0.0   62000.0     19950      19950     21150   502434    91451   405345  
        R117    94      2       0.0       190      0.0     190.0        90         90       112     2578      670     1908  
        R118    94      4    1700.0      3700      0.0    5400.0      1500       1500      2400    60804    28653    30683  
        R119    94      4       0.0      2200      0.0    2200.0       825        825      2195    52191    14983    37208  
        R120    94      1       0.0       110      0.0     110.0        38         35        56     1175      542      633  
        R121    94      3       0.0       250      0.0     250.0       120        120       150     3546      886     2299  
        R122    94      6       0.0     22000   8000.0   30000.0      9000       8500      8075   194524   111765   119472  
        R123    94      5       0.0     11400   2700.0   14100.0      5220       5220      5220   101009    33287    54440  
        R124    94      8       0.0    262500  65000.0  327500.0    122500     122500    112700  2513036  1414462  1098574  
        R125    94      4       0.0       700    600.0    1300.0       320        320       320     7746     8975    -1229  
        R126    94      3     250.0       500      0.0     750.0       245        245       318     7067     4702     2365  
        R127    94      2       0.0       150    140.0     330.0        70         70        95     2131     1903      228  
        R128    94      1       0.0        10      0.0      10.0         4          0         0        0       54      -54  
        R129    94      5       0.0      8900   4200.0   13100.0      2766       2766      3595    71576    37213    34363  
        R130    94      1       0.0        25      0.0      25.0        12          0         0        0      140     -140  
        R131    94      3      60.0       110      0.0     170.0        40         40        50     1200      853      347  
        R132    94      8       0.0     78110  36823.0  -38250.0     38250      38250     32813   694261  -185331   879592  
        R133    94      5       0.0     15173      0.0   15173.0      6180       6180      8358   182697    91655    91042  
        R134    94      2       0.0       110      0.0     110.0        50         48        70     1597      307     1290  
        R135    94      7   23700.0     43500  13727.0   80927.0     18900      18900     25893   601989   175109   426880  
        R136    94      3       0.0        40      0.0      40.0        15         14        21      428      132      296  
        R137    94      2       0.0       200      NaN       NaN        80         80       210    10355     1201     4912  
        R138    94      1       0.0        60     30.0      90.0        25         23        40     1016      244      772  
        R139    94      2       0.0        38      0.0      38.0        15         10        15      367      193      174  
        R140    94      3      10.0        60      5.0      75.0        25         25        38      924      130      794  
        R141    94      2      10.0        80     17.0      78.0        35         25        50     1248      528      720  
        R142    94      8   67000.0    122000  45000.0  234000.0     46250      46250    126500  3030416  1333900  1696516  
        R143    94      1     700.0        55      0.0     755.0        25         22        33      778     5931    -5153  
        R144    94      2       0.0        45      0.0      45.0        15         13        26      626      367      259  
        R145    94      2       0.0        50     25.0      75.0        15          5         8      189      171       18  
        R146    94      2       0.0       100     10.0      98.0        52         50        60     1406      218     1188  
        R147    94      3     260.0       210     40.0     510.0       100        100       130     2974     3408     -434  
        R148    94      6   11000.0     22000  10700.0   43700.0     10500      10500     12075   312410   171484   140926  
        R149    94      5       0.0     11600      0.0   11630.0      4725       4725      1420   105598    75601    29997  
        R150    94      1      18.0        40      0.0      58.0        15         15        23      555      420      135  
        R151    94      1       0.0        25      0.0      25.0         7          5         8      201      139       62  
        R152    94      1       0.0         6      6.0      16.0         2          0         0        0       30      -30  
        R153    94      1      -1.0       299     -1.0     325.0        70         67       100     2333      485     1848  
        R154    94      2       0.0        20      0.0      20.0         7          0         0        0       53      -53  
        R155    94      2       0.0        20      0.0      20.0         8          0         0        0       69      -69  
        R156    94      3       3.0       290     20.0     300.0        90         90       126     3111     2358      753  
        R157    94      9       0.0    100128      0.0  100128.0     46739      45290     45290  1023977   422354   601623  
        R158    94      9       0.0     83107  16858.0   99965.0     20973      20973     54364  1207131   745477   461654  
        R159    94      9       0.0     14448      0.0   14448.0      7200       7200     18523   416704    65666   351038  
        R160    94      9   28175.0     30875  13300.0   72350.0      7770       7770     20202   473186   108002   365184  
        R161    94      9   10000.0    150000  30000.0  190000.0     60000      60000     60000  1432196   279729  1152467  
        R162    94      6      -1.0        -1     -1.0   42500.0     15500         -1        -1      -25   117011   196150  


    """)

@pytest.fixture
def errorloc_outstatus_01():
    """Expected outstatus
    """

    return banff.testing.PAT_from_string("""
        s s s n
        IDENT   FIELDID STATUS     VALUE
        R004  EGG_LAID    FTI      35.0
        R005    QR_EXP    FTI    1864.0
        R007    QR_REV    FTI    1533.0
        R013  EGG_SOLD    FTI     -11.0
        R013  EGG_LAID    FTI     -12.0
        R014   HEN_OTH    FTI      25.0
        R015  EGG_LAID    FTI       0.0
        R019  EGG_LAID    FTI      20.0
        R023   QR_PROF    FTI      36.0
        R026    QR_REV    FTI    2037.0
        R026   QR_PROF    FTI    1819.0
        R026  EGG_VALU    FTI      88.0
        R028  EGG_SOLD    FTI     260.0
        R028  EGG_LAID    FTI     262.0
        R029   HEN_OTH    FTI       NaN
        R029  HEN_LT20    FTI       NaN
        R033    QR_REV    FTI    7315.0
        R035   QR_PROF    FTI    4257.0
        R039  EGG_LAID    FTI       5.0
        R040  EGG_LAID    FTI       4.0
        R047  EGG_SOLD    FTI      41.0
        R047  EGG_LAID    FTI      43.0
        R048  EGG_LAID    FTI      12.0
        R050   QR_PROF    FTI    1620.0
        R050  EGG_VALU    FTI     147.0
        R050  EGG_SOLD    FTI      98.0
        R050  EGG_LAID    FTI     102.0
        R051  EGG_LAID    FTI      10.0
        R052  EGG_LAID    FTI      10.0
        R053  EGG_LAID    FTI      25.0
        R054  EGG_SOLD    FTI      78.0
        R054  EGG_LAID    FTI      80.0
        R056   HEN_OTH    FTI       0.0
        R061  EGG_SOLD    FTI      47.0
        R061  EGG_LAID    FTI      50.0
        R063  EGG_LAID    FTI      56.0
        R064  EGG_LAID    FTI     105.0
        R065  EGG_SOLD    FTI      26.0
        R076   HEN_TOT    FTI       NaN
        R076  EGG_LAID    FTI      50.0
        R077   QR_PROF    FTI  -59095.0
        R077  EGG_VALU    FTI     600.0
        R093   QR_PROF    FTI   -1026.0
        R094   QR_PROF    FTI    1653.0
        R096  EGG_SOLD    FTI    1545.0
        R096  EGG_LAID    FTI    1545.0
        R099    QR_REV    FTI  440348.0
        R101  HEN_LT20    FTI    9000.0
        R101  EGG_VALU    FTI    8000.0
        R103   HEN_OTH    FTI       NaN
        R103  HEN_LT20    FTI       NaN
        R103  EGG_LAID    FTI      30.0
        R110    QR_REV    FTI   48230.0
        R116    QR_REV    FTI  502434.0
        R118    QR_EXP    FTI   28653.0
        R121   QR_PROF    FTI    2299.0
        R122    QR_REV    FTI  194524.0
        R123   QR_PROF    FTI   54440.0
        R127   HEN_OTH    FTI     140.0
        R132   QR_PROF    FTI  879592.0
        R132    QR_EXP    FTI -185331.0
        R132   HEN_TOT    FTI  -38250.0
        R132  EGG_VALU    FTI   32813.0
        R137    QR_REV    FTI   10355.0
        R137   HEN_TOT    FTI       NaN
        R137   HEN_OTH    FTI       NaN
        R141  HEN_GE20    FTI      80.0
        R141  EGG_LAID    FTI      35.0
        R146  HEN_GE20    FTI     100.0
        R146  EGG_SOLD    FTI      50.0
        R146  EGG_LAID    FTI      52.0
        R149  HEN_LT20    FTI       0.0
        R149  EGG_VALU    FTI    1420.0
        R152   HEN_OTH    FTI       6.0
        R153   HEN_OTH    FTI      -1.0
        R153  HEN_LT20    FTI      -1.0
        R153  EGG_LAID    FTI      70.0
        R156   HEN_OTH    FTI      20.0
        R162    QR_REV    FTI     -25.0
        R162   HEN_OTH    FTI      -1.0
        R162  HEN_LT20    FTI      -1.0
        R162  HEN_GE20    FTI      -1.0
        R162  EGG_VALU    FTI      -1.0
        R162  EGG_SOLD    FTI      -1.0
    """)

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()