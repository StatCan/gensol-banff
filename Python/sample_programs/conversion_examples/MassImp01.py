# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata
massimpdata = pd.read_csv(StringIO("""
    IDENT TOTAL Q1 Q2 Q3 Q4 STAFF
    REC01 500 100 125 125 150 1000
    REC02 750 200 170 130 250 2000
    REC03 400 80 90 100 130 1000
    REC04 1000 150 250 350 250 2000
    REC05 3000 500 500 1000 1000 1000
    REC06 800 200 225 200 175 2000
    REC07 600 125 150 175 150 1000
    REC08 900 175 200 225 300 2000
    REC09 2500 500 650 600 750 1000
    REC10 800 150 175 225 250 2000
    REC21 3000 -45 -50 75 -234 2000
    REC11 575 NaN NaN NaN NaN 1000
    REC12 850 NaN NaN NaN NaN 2000
    REC13 375 NaN NaN NaN NaN 1000
    REC14 1100 NaN NaN NaN NaN 2000
    REC15 3100 NaN NaN NaN NaN 1000
    REC16 750 NaN NaN NaN NaN 2000
    REC17 675 NaN NaN NaN NaN 1000
    REC18 875 NaN NaN NaN NaN 2000
    REC19 4000 NaN NaN NaN NaN 1000
    REC20 NaN NaN NaN NaN NaN 2000
"""), sep=r'\s+')

# sort dataset
massimpdata = massimpdata.sort_values(by=["STAFF", "IDENT"])

# run Banff call
banff_call = banff.massimp(
    indata=massimpdata,
    min_donors=1,
    percent_donors=1,
    random=True,
    accept_negative=True,
    unit_id="IDENT",
    must_impute="Q1 Q2 Q3 Q4",
    must_match="TOTAL",
    by="STAFF",
)

# TIP: this is how you access the ouput datasets
print("OUTDATA dataset \n", banff_call.outdata, "\n")
print("OUTSTATUS dataset \n", banff_call.outstatus, "\n")
print("OUTDONORMAP dataset\n", banff_call.outdonormap, "\n")