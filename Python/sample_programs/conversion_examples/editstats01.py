# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata
indata = pd.read_csv(StringIO("""
    x1 x2 x3 Group
    4 3 2 A
    -4 3 2 A
    6 3 2 B
    4 3 NaN A
    6 3 NaN B
"""), sep=r'\s+').sort_values(by=['Group'])

banff_call = banff.editstats(
    indata=indata,
    edits="""
        x1+1>=x2;
        x1<=5;
        x2>=x3;
        x1+x2+x3<=9;
    """,
    by="Group",
    accept_negative=True,
)