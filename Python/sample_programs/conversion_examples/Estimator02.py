# import necessary Python packages
import banff
import pandas as pd
from io import StringIO

# create indata
currentdata = pd.read_csv(StringIO("""
    ident x prov
    REC01 -500 24
    REC02 750 24
    REC03 -400 24
    REC04 1000 24
    REC05 1050 24
    REC06 500 24
    REC07 400 30
    REC08 950 30
    REC09 1025 30
    REC10 -800 30
    REC12 10000 30
    REC13 500 24
    REC14 750 24
    REC15 400 24
    REC16 -1000 24
    REC17 1050 24
    REC18 500 24
    REC19 -400 30
    REC20 950 30
    REC21 -1025 30
    REC22 800 30
    REC23 10000 30
"""), sep=r'\s+')

currentdata["CURRDATAEXCL"] = currentdata.apply(lambda row: 'E' if row['x'] > 2000 else '' , axis=1)

# create indata_hist
histdata = pd.read_csv(StringIO("""
    ident x prov
    REC01 500 24
    REC02 -750 24
    REC03 400 24
    REC05 870 24
    REC06 500 24
    REC07 400 30
    REC08 950 30
    REC09 950 30
    REC10 -800 30
    REC11 800 30
    REC12 500 24
    REC13 750 24
    REC14 -400 24
    REC15 870 24
    REC16 500 24
    REC17 -400 30
    REC18 950 30
    REC19 950 30
    REC20 800 30
    REC21 800 30
"""), sep=r'\s+')

histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)

# create instatus
curstatus = pd.read_csv(StringIO("""
    ident fieldid status
    REC01 x FTI
    REC03 x FTI
    REC10 x FTI
    REC16 x FTI
    REC19 x FTI
    REC21 x FTI
"""), sep=r'\s+')

# create instatus_hist
histstatus = pd.read_csv(StringIO("""
    ident fieldid status
    REC02 X FTI
    REC10 X FTI
    REC14 X FTI
    REC17 X FTI
"""), sep=r'\s+')

# sort datasets
currentdata = currentdata.sort_values(by=['prov', 'ident'])
histdata = histdata.sort_values(by=['prov', 'ident'])

# create inalgorithm
# create empty dataframe
algorithm = pd.DataFrame()
# add values
algorithm['algorithmname'] = ['prevalue']
algorithm['type'] = ['ef']
algorithm['status'] = ['vp']
algorithm['formula'] = ['fieldid(h,v)']
algorithm['description'] = ['Previous value is imputed.']

# create inestimator
# create dataless dataframe with column names pre-populated
estimator = pd.DataFrame(columns= ["est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])

# append data
estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'algorithmname':'prevalue', 'fieldid':'x'}
estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1, 'algorithmname':'curmean', 'fieldid':'x'}
estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'n', 'excludeoutliers':'n', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'diftrend', 'fieldid':'x'}
estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'histreg', 'fieldid':'x'}
# create a list of columns to convert
character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
# use the `.astype()` method to set the column's datatype
estimator[character_column_list] = estimator[character_column_list].astype('string')

# run Banff procedure
banff_call = banff.estimator(
    indata=currentdata,
    indata_hist=histdata,
    instatus=curstatus,
    # instatus_hist=histstatus,  # created, but not used in actual user guide example
    inalgorithm=algorithm,
    inestimator=estimator.query('est_setid=="set2"'),
    data_excl_var="CURRDATAEXCL",
    hist_excl_var="HISTDATAEXCL",
    by="prov",
    unit_id="ident",
)