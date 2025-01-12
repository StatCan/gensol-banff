"""Validate that outstatus dataset conforms to standard
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

# `unit_id` parameter is NOT case sensitive, so the dataset and parameter can differ in case
unit_id_dataset="IDENT"
unit_id_parameter="IdEnT"

@pytest.mark.m_auto_pass
def test_standard_outstatus_estimato_a(capfd, indata, indata_hist, instatus, instatus_hist, outstatus):
    """Outstatus file's <unit-id> column should use capitalization from indata (IDENT)
    instead of capitalization in parameters (IdEnT).
    
    This test based on user guide example 1."""

    # create inalgorithm
    # TIP: For this single row dataframe, we can create it very simply 
    # create empty dataframe without any columns defined
    algorithm = pd.DataFrame()
    # add values; this magically adds the column, places the value in the first row, only really useful for a single row dataset
    algorithm['algorithmname'] = ['prevalue']
    algorithm['type'] = ['ef']
    algorithm['status'] = ['vp']
    algorithm['formula'] = ['fieldid(h,v)']
    algorithm['description'] = ['Previous value is imputed.']

    # create inestimator
    # TIP: for creating more complicated datasets including missing data, we can use a different approach
    # create dataless dataframe with column names pre-populated
    estimator = pd.DataFrame(columns= ["est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])

    # add data; `len(estimator) refers to the number of rows in the estimator DataFrame.  Given the 0-based index, this allows us to append rows`
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'algorithmname':'prevalue', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1, 'algorithmname':'curmean', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'n', 'excludeoutliers':'n', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'diftrend', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'histreg', 'fieldid':'x'}

    character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
    # convert column datatype
    estimator[character_column_list] = estimator[character_column_list].astype("string")
    assert unit_id_dataset != unit_id_parameter, "Test case flaw: unit_id_dataset should differ from unit_id_parameter"

    # run Banff procedure
    banff_call = banfftest.pytest_estimato(
        unit_id=unit_id_parameter,
        indata=indata,
        indata_hist=indata_hist,
        instatus=instatus,
        instatus_hist=instatus_hist,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        seed=1,

        pytest_capture=capfd,
        expected_warning_count=None,
        expected_outstatus=outstatus,
    )

##### DATA #####


@pytest.fixture
def indata():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types[f'{unit_id_dataset}'] = str

    indata = pd.read_csv(StringIO(f"""
        {unit_id_dataset} x prov
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
        """), 
        sep=r"\s+",
        dtype=var_types
    )

    # add exclusion column to current data
    def add_exclude(row):
        if row['x'] > 1000:
            return 'E'
        else:
            return ''

    indata["CURRDATAEXCL"] = indata.apply(add_exclude, axis=1)
    indata = indata.sort_values(by=['prov', unit_id_dataset])

    return indata

@pytest.fixture
def instatus():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} fieldid status
        REC01 x FTI
        REC03 x FTI
        REC10 x FTI
        REC16 x FTI
        REC19 x FTI
        REC21 x FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def indata_hist():
    var_types = defaultdict(lambda: np.float64) # default to numeric
    # explicitly set character values
    var_types[f'{unit_id_dataset}'] = str

    indata = pd.read_csv(StringIO(f"""
        {unit_id_dataset} x prov
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
        """), 
        sep=r"\s+",
        dtype=var_types
    )
    indata["HISTDATAEXCL"] = indata.apply(lambda row: 'E' if row['x'] > 2000 else '' , axis=1)
    indata = indata.sort_values(by=['prov', unit_id_dataset])

    return indata

@pytest.fixture
def instatus_hist():
    var_types = defaultdict(lambda: str) # default to character

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} fieldid status
        REC02 X FTI
        REC10 X FTI
        REC14 X FTI
        REC17 X FTI
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

@pytest.fixture
def outstatus():
    var_types = defaultdict(lambda: str) # default to character
    var_types['VALUE'] = np.float64

    return pd.read_csv(StringIO(f"""
        {unit_id_dataset} FIELDID STATUS   VALUE
        REC03       x    IVP   150.0
        REC01       x    IVP   250.0
        REC16       x    IVP    30.0
        REC10       x    ICM   400.0
        REC19       x    IVP  1100.0
        REC21       x    IVP   800.0
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()