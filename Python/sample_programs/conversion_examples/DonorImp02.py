# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata
data = pd.read_csv(StringIO("""
    REC X Y randkey
    A 1 2 0.1
    B 2 3 0.8
    C 3 4 0.3
"""), sep=r'\s+')

# create instatus
instatus = pd.read_csv(StringIO("""
    REC FIELDID STATUS
    B X FTI
"""), sep=r'\s+')

# run Banff procedure
banff_call = banff.donorimp(
    trace=banff.log_level.INFO, # by setting `trace` to the INFO log level, extra info will be printed to the log
    indata=data,
    instatus=instatus,
    outmatching_fields=True,
    unit_id="REC", 
    edits="x=y;",
    post_edits="x<=y;",
    min_donors=1,
    percent_donors=1,
    n=1,
    display_level=1,
    n_limit=1,
    rand_num_var="randkey",
)