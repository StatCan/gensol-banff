"""
Demonstrate diagnostics functions when repeatedly running a nontrivial sized testcase.  
There may be slight variations in the amount of memory allocated or freed during a given
portion of the process when running the same call multiple times.  
"""

import os
os.environ['BANFF_DEBUG_STATS'] = "TRUE"  
import banff
import pandas as pd

# generate input dataset
list_data = []
for m in range(100000):
    list_data.append(["R{:07d}".format(10*m + 1), "A{:06d}".format(m+1),  4, 19])
    list_data.append(["R{:07d}".format(10*m + 2), "A{:06d}".format(m+1), 14,  5])
    list_data.append(["R{:07d}".format(10*m + 3), "A{:06d}".format(m+1), 18, 19])
    list_data.append(["R{:07d}".format(10*m + 4), "A{:06d}".format(m+1),  7,  6])
    list_data.append(["R{:07d}".format(10*m + 5), "A{:06d}".format(m+1),  1,  1])
    list_data.append(["R{:07d}".format(10*m + 6), "A{:06d}".format(m+1),  5,  2])
    list_data.append(["R{:07d}".format(10*m + 7), "A{:06d}".format(m+1),  3,  8])
    list_data.append(["R{:07d}".format(10*m + 8), "A{:06d}".format(m+1), 16, 14])
    list_data.append(["R{:07d}".format(10*m + 9), "A{:06d}".format(m+1),  2,  3])
    list_data.append(["R{:07d}".format(10*m + 10), "A{:06d}".format(m+1),  6,  2])

df_indata = pd.DataFrame(list_data, columns=['ident', 'area', 'V1', 'V2'])

# generate input historical dataset

list_data = []
for m in range(100000):
    list_data.append(["R{:07d}".format(10*m + 1), "A{:06d}".format(m+1), 19,  4])
    list_data.append(["R{:07d}".format(10*m + 2), "A{:06d}".format(m+1),  5, 14])
    list_data.append(["R{:07d}".format(10*m + 3), "A{:06d}".format(m+1), 19, 18])
    list_data.append(["R{:07d}".format(10*m + 4), "A{:06d}".format(m+1),  6,  7])
    list_data.append(["R{:07d}".format(10*m + 5), "A{:06d}".format(m+1),  1,  1])
    list_data.append(["R{:07d}".format(10*m + 6), "A{:06d}".format(m+1),  2,  5])
    list_data.append(["R{:07d}".format(10*m + 7), "A{:06d}".format(m+1),  8,  3])
    list_data.append(["R{:07d}".format(10*m + 8), "A{:06d}".format(m+1), 14, 16])
    list_data.append(["R{:07d}".format(10*m + 9), "A{:06d}".format(m+1),  3,  2])
    list_data.append(["R{:07d}".format(10*m + 10), "A{:06d}".format(m+1), 2,  6])

df_indata_hist = pd.DataFrame(list_data, columns=['ident', 'area', 'V1', 'V2'])

banff.diagnostics.enable_all()

banff_call = banff.outlier(
        #trace=banff.log_level_INFO, # only displays high-level memory usage statistics
        trace=True, # display all log levels
        indata=df_indata,
        indata_hist=df_indata_hist,
        method="H",
        mii=3,
        mei=2,
        outlier_stats=True,
        no_by_stats=True,
        unit_id="ident",
        var="V1 V2",
        by="area"
        )

# delete the in-memory output datasets before running again
for ds in banff_call._output_datasets:
    ds.ds_output = None

banff_call.rerun()

for ds in banff_call._output_datasets:
    ds.ds_output = None

banff_call.rerun()

