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

# run Banff procedure
banff_call = banff.outlier(
    indata=outlierdata,
    method="sg",
    beta_e=1,
    beta_i=1.75,
    sigma="MAD",
    start_centile=60,
    outlier_stats=True,
    accept_negative=True,
    unit_id="ident",
    var="X01 X02",
    by="Prov",
)