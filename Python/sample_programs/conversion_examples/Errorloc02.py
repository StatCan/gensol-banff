# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata
data = pd.read_csv(StringIO("""
    REC X Y randkey
    A 3 4 0.1
    B 2 3 0.6
    C 4 1 0.3
    D 5 6 0.4
"""), sep=r'\s+')

# run Banff procedure
# TIP: Assign the "Banff call" to any variable of your choosing
name_this_variable_whatever_you_want = banff.errorloc(
    indata=data,
    edits="X+Y>=6; X<=4; Y<=5;",
    unit_id="REC",
    rand_num_var="randkey",
)

# TIP:
print("You can access the output datasets like this")
print(name_this_variable_whatever_you_want.outstatus)