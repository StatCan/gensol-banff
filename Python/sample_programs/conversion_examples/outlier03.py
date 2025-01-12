# import necessary Python packages
import banff
import pandas as pd

# create indata, define columnms but add no data
outlierdata = pd.DataFrame(columns= ["IDENT", "X01", "X02", "Prov"])

# populate dataframe using for loop
import random
random.seed(8)
for n in range(1, 31):
    current_label = len(outlierdata)
    outlierdata.loc[current_label] = {
        'IDENT': 'R{:07d}'.format(n),
        'X01': int(random.uniform(0,20)),
        'X02': int(random.uniform(0,20)),
        'Prov': 10 if (n)%2==0 else 11}
    if n%5==0:
        outlierdata.loc[current_label, 'X01'] *= -1
        outlierdata.loc[current_label, 'X02'] *= -1

# sort dataset
outlierdata = outlierdata.sort_values(by=["Prov"])

# create indata_hist, define columnms but add no data
outlierhist = pd.DataFrame(columns= ["IDENT", "X01", "X02", "Prov"])

# populate dataframe using for loop
import random
random.seed(8)
for n in range(1, 31):
    current_label = len(outlierhist)
    outlierhist.loc[current_label] = {
        'IDENT': 'R{:07d}'.format(n),
        'X01': int(random.uniform(0,30)),
        'X02': int(random.uniform(0,30)),
        'Prov': 10 if (n)%2==0 else 11}
    if n%7==0:
        outlierhist.loc[current_label, 'X01'] *= -1
        outlierhist.loc[current_label, 'X02'] *= -1

# sort dataset
outlierhist = outlierhist.sort_values(by=["Prov"])

# run Banff procedure
banff_call = banff.outlier(
    indata=outlierdata,
    indata_hist=outlierhist,
    method="historic",
    mii=1.5,
    mei=1.3,
    mdm=0.05,
    outlier_stats=True,
    unit_id="ident",
    var="X01 X02",
    by="Prov",
)