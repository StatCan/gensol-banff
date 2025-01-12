"""datasets for estimato regression tests
    The naming convention in this file is pretty loose
    `estimato_<dataset-name>_<##>`
        where <dataset-name> is a input or output parameter name for 
        the estimato procedure
    !!There is no relationship between the numbering of these datasets and any unit test numbering!!

        Example: `estimato_indata_01`
"""

import pytest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`
import pyarrow as pa

##### INDATA #####
@pytest.fixture
def estimato_indata_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    currentdata = pd.read_csv(StringIO("""
    ident x prov SOME_NUM SOME_CHAR
    REC01 -500 24 1 abc
    REC02 750 24 1 abc
    REC03 -400 24 1 abc
    REC04 1000 24 1 abc
    REC05 1050 24 1 abc
    REC06 500 24 1 abc
    REC07 400 30 1 abc
    REC08 950 30 1 abc
    REC09 1025 30 1 abc
    REC10 -800 30 1 abc
    REC12 10000 30 1 abc
    REC13 500 24 1 abc
    REC14 750 24 1 abc
    REC15 400 24 1 abc
    REC16 -1000 24 1 abc
    REC17 1050 24 1 abc
    REC18 500 24 1 abc
    REC19 -400 30 1 abc
    REC20 950 30 1 abc
    REC21 -1025 30 1 abc
    REC22 800 30 1 abc
    REC23 10000 30 1 abc
    """), sep=r'\s+')

    currentdata["CURRDATAEXCL"] = currentdata.apply(lambda row: 'E' if row['x'] > 2000 else '' , axis=1)
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)

    return currentdata

@pytest.fixture
def estimato_indata_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    currentdata = pd.read_csv(StringIO("""
    IDENT V X Y Z
    A1 NaN 1 1 1
    A2 2 NaN 2 2
    A3 3 3 NaN 3
    A4 4 4 4 NaN
    """), sep=r'\s+')

    currentdata.sort_values(by=['IDENT'], inplace=True)

    return currentdata

@pytest.fixture
def estimato_indata_03():
    """invalid 'area' sort order
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 NaN 1 1 1 A
        A2 2 NaN 2 2 B
        A3 3 3 NaN 3 A
        A4 4 4 4 NaN A
        A5 NaN 5 5 5 B
        A6 6 NaN 6 6 B
        A7 7 7 NaN 7 B
        A8 8 8 8 NaN B
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_indata_04():
    """invalid 'area' sort order
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 NaN 1 1 1 1
        A2 2 NaN 2 2 2
        A3 3 3 NaN 3 1
        A4 4 4 4 NaN 1
        A5 NaN 5 5 5 2
        A6 6 NaN 6 6 2
        A7 7 7 NaN 7 2
        A8 8 8 8 NaN 2
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_indata_presort(estimato_indata_03):
    """dataset suitable for testing `presort` option"""
    return estimato_indata_03

##### INDATA_HIST #####
@pytest.fixture
def estimato_indata_hist_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    histdata = pd.read_csv(StringIO("""
    ident x prov SOME_NUM SOME_CHAR
    REC01 500 24 1 abc
    REC02 -750 24 1 abc
    REC03 400 24 1 abc
    REC05 870 24 1 abc
    REC06 500 24 1 abc
    REC07 400 30 1 abc
    REC08 950 30 1 abc
    REC09 950 30 1 abc
    REC10 -800 30 1 abc
    REC11 800 30 1 abc
    REC12 500 24 1 abc
    REC13 750 24 1 abc
    REC14 -400 24 1 abc
    REC15 870 24 1 abc
    REC16 500 24 1 abc
    REC17 -400 30 1 abc
    REC18 950 30 1 abc
    REC19 950 30 1 abc
    REC20 800 30 1 abc
    REC21 800 30 1 abc
    """), sep=r'\s+')

    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)

    return histdata

@pytest.fixture
def estimato_indata_hist_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    currentdata = pd.read_csv(StringIO("""
    IDENT V X Y Z
    A1 4 4 4 4
    A2 3 3 3 3
    A3 2 2 2 2
    A4 1 1 1 1
    """), sep=r'\s+')

    currentdata.sort_values(by=['IDENT'], inplace=True)

    return currentdata

@pytest.fixture
def estimato_indata_hist_03():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 4 4 4 4 A
        A2 3 3 3 3 A
        A3 2 2 2 2 A
        A4 1 1 1 1 A
        A5 8 8 8 8 B
        A6 7 7 7 7 B
        A7 6 6 6 6 B
        A8 5 5 5 5 B
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_indata_hist_04():
    """valid indata dataset
    """
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    
    return pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 4 4 4 4 1
        A2 3 3 3 3 1
        A3 2 2 2 2 1
        A4 1 1 1 1 1
        A5 8 8 8 8 2
        A6 7 7 7 7 2
        A7 6 6 6 6 2
        A8 5 5 5 5 2
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_indata_hist_05():
    """dataset with correct columns but 0 rows"""
    sch = pa.schema([
        pa.field('ident', pa.string()),
        pa.field('V', pa.float64()),
        pa.field('X', pa.float64()),
        pa.field('Y', pa.float64()),
        pa.field('Z', pa.float64()),
        pa.field('area', pa.float64()),
    ])
    # create indata_hist
    indata_hist = pa.Table.from_pylist([], schema=sch)
    return indata_hist

@pytest.fixture
def estimato_indata_hist_presort():
    """dataset suitable for testing `presort` option"""
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types['ident'] = str
    var_types['area'] = str
    
    dataset = pd.read_csv(StringIO("""
        ident V X Y Z area
        A1 4 4 4 4 A
        A2 8 3 3 3 A
        A3 2 2 2 2 A
        A4 1 1 1 1 A
        A5 3 8 8 8 B
        A6 7 7 7 7 B
        A7 6 6 6 6 B
        A8 5 5 5 5 B
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    dataset.sort_values(by='Z', inplace=True)
    return dataset

##### INSTATUS #####
@pytest.fixture
def estimato_instatus_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    return pd.read_csv(StringIO("""
    ident fieldid status
    REC01 x FTI
    REC03 x FTI
    REC10 x FTI
    REC16 x FTI
    REC19 x FTI
    REC21 x FTI
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    return pd.read_csv(StringIO("""
    ident fieldid status
    A1  V  FTI
    A2  X  FTI
    A3  Y  FTI
    A4  Z  FTI
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_03():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A1  V  FTI
        A2  X  FTI
        A3  Y  FTI
        A4  Z  FTI
        A5  V  FTI
        A6  X  FTI
        A7  Y  FTI
        A8  Z  FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_presort(estimato_indata_presort, estimato_instatus_03):
    """dataset suitable for testing `presort` option"""
    dataset = pd.merge(left=estimato_indata_presort[['ident', 'area']], right=estimato_instatus_03)
    return dataset

##### INSTATUS_HIST #####
@pytest.fixture
def estimato_instatus_hist_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    return pd.read_csv(StringIO("""
    ident fieldid status
    REC02 X FTI
    REC10 X FTI
    REC14 X FTI
    REC17 X FTI
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_hist_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    return pd.read_csv(StringIO("""
    ident fieldid status
    A1 X   IDN
    A3 Y   IDT
    """), sep=r'\s+')

@pytest.fixture
def estimato_instatus_hist_03():
    """valid instatus dataset
    """
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO("""
        ident fieldid status
        A2 X   IDN
        A4 X   IDE
        A6 X   IDN
        A8 X   IDE
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def estimato_instatus_hist_presort(estimato_indata_hist_presort, estimato_instatus_hist_03):
    """dataset suitable for testing `presort` option"""
    dataset = pd.merge(left=estimato_indata_hist_presort[['ident', 'area']], right=estimato_instatus_hist_03)
    return dataset

##### INESTIMATOR #####
@pytest.fixture
def estimato_inestimator_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    # create dataless dataframe with column names pre-populated
    estimator = pd.DataFrame(columns= ["est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])

    # append data
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'algorithmname':'prevalue', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1, 'algorithmname':'curmean', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'n', 'excludeoutliers':'n', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'diftrend', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'histreg', 'fieldid':'x'}
    # use the `.fillna` method to replace NaN with '' in character columns
    character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
    estimator[character_column_list] = estimator[character_column_list].fillna('')

    return estimator

@pytest.fixture
def estimato_inestimator_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    # create dataless dataframe with column names pre-populated
    estimator = pd.DataFrame(columns= ["active", "est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])

    # append data
    estimator.loc[len(estimator)] = {'active' : 1, 'randomerror':'N','algorithmname':'ALG1', 'fieldid':'V', 'auxvariables' : 'Y'}
    estimator.loc[len(estimator)] = {'active' : 2, 'randomerror':'N','algorithmname':'PREVALUE', 'fieldid':'X'}
    estimator.loc[len(estimator)] = {'active' : 3, 'randomerror':'N','algorithmname':'ALG2', 'fieldid':'Y', 'auxvariables' : 'X,Z'}
    estimator.loc[len(estimator)] = {'active' : 4, 'randomerror':'N','algorithmname':'ALG3', 'fieldid':'Z', 'auxvariables' : 'X,Y'}
    character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
    estimator[character_column_list] = estimator[character_column_list].fillna('')

    return estimator

@pytest.fixture
def estimato_inestimator_03():
    return pd.read_csv(StringIO("""
        active  excludeimputed  excludeoutliers     randomerror     countcriteria   percentcriteria     weightvariable  variancevariable    varianceperiod  varianceexponent    algorithmname   fieldid     auxvariables
        1       N               N                   N               1               1                   " "             " "                 " "             NaN                 CURAUXMEAN      V           X
        2       N               N                   N               1               1                   " "             " "                 " "             NaN                 PREMEAN         X           " "
        3       " "             " "                 N               NaN             NaN                 " "             " "                 " "             NaN                 ALG2            Y           X,Z
        4       " "             " "                 N               NaN             NaN                 " "             " "                 " "             NaN                 ALG3            Z           X,Y
    """), sep=r'\s+')

##### INALGORITHM #####
@pytest.fixture
def estimato_inalgorithm_01():
    """valid input dataset, for testing single-variable variable-lists
    """
    # create empty dataframe
    algorithm = pd.DataFrame()
    # add values
    algorithm['algorithmname'] = ['prevalue']
    algorithm['type'] = ['ef']
    algorithm['status'] = ['vp']
    algorithm['formula'] = ['fieldid(h,v)']
    algorithm['description'] = ['Previous value is imputed.']

    return algorithm

@pytest.fixture
def estimato_inalgorithm_02():
    """valid input dataset, for testing single-variable variable-lists
    """
    # create dataless dataframe with column names pre-populated
    algorithm = pd.DataFrame(columns= ["algorithmname", "type", "status", 'formula', "description"])

    # append data
    algorithm.loc[len(algorithm)] = {'algorithmname':'ALG1', 'type':'EF', 'status':'AL1', 'formula':'AUX1`', 'description':'CrAux1'}
    algorithm.loc[len(algorithm)] = {'algorithmname':'ALG2', 'type':'EF', 'status':'AL2', 'formula':'AUX1+AUX2`', 'description':'SumCrAux1CrAux2'}
    algorithm.loc[len(algorithm)] = {'algorithmname':'ALG3', 'type':'EF', 'status':'AL3', 'formula':'AUX1 (H) +AUX2`', 'description':'SumHsAux1CrAux2'}
    # create empty dataframe

    return algorithm

@pytest.fixture
def estimato_inalgorithm_03():
    return pd.read_csv(StringIO("""
        algorithmname type status formula description
        ALG2 EF AL2  AUX1+AUX2     SumCrAux1CrAux2
        ALG3 EF AL3  AUX1(H)+AUX2  SumHsAux1CrAux2
    """), sep=r'\s+', dtype={'formula': str}) # dtype necessary since values can be a digit