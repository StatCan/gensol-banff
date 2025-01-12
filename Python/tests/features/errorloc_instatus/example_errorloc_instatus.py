"""Test errorloc's python only instatus dataset
"""
import banff
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

##### DATA #####

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



seed=3212024

banff.errorloc(
    # Procedure Parameters
    indata=indata(),
    instatus=instatus(),
    edits="Var1+Var2+Var3=Total;",
    weights="Var3=3; Total=10;",
    accept_negative=False,
    seed=seed,
    unit_id="RecordID",
    trace=True,
    prefill_by_vars=True,
    presort=True,
)