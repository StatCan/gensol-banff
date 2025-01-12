# import necessary Python packages
import banff
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

# create indata
var_types = defaultdict(lambda: np.float64) # default to numeric
var_types['ident'] = "string"
proratedata = pd.read_csv(StringIO("""
    ident Q1 Q2 Q3 Q4 YEAR
    R01 -25 42 27 25 40
    R02 -25 44 20 20 40
    R03 -18 44 15 5 40
    R04 -18 42 15 5 40
    R05 12 30 15 5 40
    R06 12 20 10 5 40
    """), 
    sep=r'\s+', 
    dtype=var_types
)

# run Banff procedure
banff_call = banff.prorate(
    indata=proratedata,
    edits="Q1+4Q2+2Q3+Q4=YEAR;",
    method="BASIC",
    decimal=1,
    lower_bound=-100,
    upper_bound=100,
    modifier="always",
    accept_negative=True,
    unit_id="ident",
)

# TIP: this is how you access the ouput datasets
print("OUTDATA dataset \n", banff_call.outdata, "\n")
print("OUTSTATUS dataset \n", banff_call.outstatus, "\n")