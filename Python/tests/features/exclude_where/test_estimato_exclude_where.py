"""Basic test validates estimator's `exclude_where_indata` option

Tests derived from user guide example 1.
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa
import banff.exceptions

@pytest.mark.m_auto_pass
def test_estimato_excl_where_a(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 as-is, validate expected header and statistics"""

    # perform exclusions manually
    currentdata["CURRDATAEXCL"] = currentdata.apply(add_exclude, axis=1)
    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)

    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=4,
        msg_list_contains_exact=[
            "NOTE: data_excl_var = CURRDATAEXCL",
            "NOTE: hist_excl_var = HISTDATAEXCL",
        ],
        msg_list_contains=expected_log,
        
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_b(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, without_exclusions_log):
    """Run user guide example 1 without an exclusion variable, validate
     that header and statistics differ
     """
    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        #data_excl_var="CURRDATAEXCL",
        #hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=3,
        msg_list_contains_exact=[
            "NOTE: data_excl_var not specified.",
            "NOTE: hist_excl_var not specified.",
        ],
        msg_list_contains=without_exclusions_log,
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_c(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - pandas dataframe for input
        validate expected header, that stats match original stats
    """
    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        exclude_where_indata="x>2000",
        exclude_where_indata_hist="x>1000",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=4,
        msg_list_contains_exact=[
            "NOTE: data_excl_var = _exclude",
            "NOTE: hist_excl_var = _exclude",
        ],
        msg_list_contains=expected_log,
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_d(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - pyarrow table for input
        validate expected header, that stats match original stats
    """
    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=pa.Table.from_pandas(currentdata),
        indata_hist=pa.Table.from_pandas(histdata),
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        exclude_where_indata="x>2000",
        exclude_where_indata_hist="x>1000",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=4,
        msg_list_contains_exact=[
            "NOTE: data_excl_var = _exclude",
            "NOTE: hist_excl_var = _exclude",
        ],
        msg_list_contains=expected_log,
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_e(capfd, currentdata_with_exclude, histdata_with_exclude, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - default name ('_exclude') already exists, should add random suffix
        validate expected header, that stats match original stats
    """
    # sort datasets
    currentdata_with_exclude.sort_values(by=['prov', 'ident'], inplace=True)
    histdata_with_exclude.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=currentdata_with_exclude,
        indata_hist=histdata_with_exclude,
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        exclude_where_indata="x>2000",
        exclude_where_indata_hist="x>1000",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=4,
        msg_list_contains_exact=[
            "NOTE: data_excl_var = _exclude_",
            "NOTE: hist_excl_var = _exclude_",
        ],
        msg_list_contains=expected_log,
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_f(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 as-is, validate expected header and statistics
    
    Passing empty string for `exclude_where...` should be ignored"""

    # perform exclusions manually
    currentdata["CURRDATAEXCL"] = currentdata.apply(add_exclude, axis=1)
    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)

    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    banff_call= banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        #instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",
        exclude_where_indata=" ",
        exclude_where_indata_hist="",

        trace=True,  # verbose logging
        capture=True,  # so pytest can capture python logging statements

        pytest_capture=capfd,
        expected_warning_count=4,
        expected_error_count=None,  # don't count error messages
        msg_list_contains_exact=[
            "Ignoring option `exclude_where_indata=",
            "Ignoring option `exclude_where_indata_hist=",
            "NOTE: data_excl_var = CURRDATAEXCL",
            "NOTE: hist_excl_var = HISTDATAEXCL",
        ],
        msg_list_contains=expected_log,
        
    )

@pytest.mark.m_auto_pass
def test_estimato_excl_where_g(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """it is prohibited to specify a non-empty `exclude_where_indata` and `data_excl_var` together
    """

    # perform exclusions manually
    currentdata["CURRDATAEXCL"] = currentdata.apply(add_exclude, axis=1)
    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)

    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    try:
        banff_call= banfftest.pytest_estimato(
            indata=currentdata,
            indata_hist=histdata,
            instatus=curstatus,
            #instatus_hist=histstatus,
            inalgorithm=algorithm,
            inestimator=estimator.query('est_setid=="set1"'),
            accept_negative=True,
            data_excl_var="CURRDATAEXCL",
            hist_excl_var="HISTDATAEXCL",
            by="prov",
            unit_id="ident",
            exclude_where_indata="x>2000",
            exclude_where_indata_hist="",

            pytest_capture=capfd,
            
        )
    except banff.exceptions.ProcedureValidationError as e:
        assert e.args[0] == 'options `data_excl_var` and `exclude_where_indata` cannot be specified together'
    else:
        raise AssertionError("Did not receive expected exception, `ValueError`")

@pytest.mark.m_auto_pass
def test_estimato_excl_where_h(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """it is prohibited to specify a non-empty `exclude_where_indata_hist` and `hist_excl_var` together
    """

    # perform exclusions manually
    currentdata["CURRDATAEXCL"] = currentdata.apply(add_exclude, axis=1)
    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 1000 else '' , axis=1)

    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    try:
        banff_call= banfftest.pytest_estimato(
            indata=currentdata,
            indata_hist=histdata,
            instatus=curstatus,
            #instatus_hist=histstatus,
            inalgorithm=algorithm,
            inestimator=estimator.query('est_setid=="set1"'),
            accept_negative=True,
            data_excl_var="CURRDATAEXCL",
            hist_excl_var="HISTDATAEXCL",
            by="prov",
            unit_id="ident",
            exclude_where_indata_hist="x>1000",

            pytest_capture=capfd,
            
        )
    except banff.exceptions.ProcedureValidationError as e:
        assert e.args[0] == 'options `hist_excl_var` and `exclude_where_indata_hist` cannot be specified together'
    else:
        raise AssertionError("Did not receive expected exception, `ValueError`")

@pytest.mark.m_auto_pass
def test_estimato_excl_where_i(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 passing a `exclude_where_indata` with disallowed ';' character"""
    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    try:
        banff_call= banfftest.pytest_estimato(
            indata=currentdata,
            indata_hist=histdata,
            instatus=curstatus,
            #instatus_hist=histstatus,
            inalgorithm=algorithm,
            inestimator=estimator.query('est_setid=="set1"'),
            accept_negative=True,
            exclude_where_indata="x>2000;",
            exclude_where_indata_hist="x>1000",
            by="prov",
            unit_id="ident",

            pytest_capture=capfd,
            expected_warning_count=4,
            msg_list_contains_exact=[
                "NOTE: data_excl_var = _exclude",
                "NOTE: hist_excl_var = _exclude",
            ],
            msg_list_contains=expected_log,
        )
    except ValueError as e:
        assert e.args[0] == "Parameter `exclude_where_indata` cannot contain the substring ';'"
    else:
        raise AssertionError("Procedure should have raised a ValueError")

@pytest.mark.m_auto_pass
def test_estimato_excl_where_j(capfd, currentdata, histdata, curstatus, histstatus, algorithm, estimator, expected_log):
    """Run user guide example 1 passing a `exclude_where_indata_hist` with disallowed ';' character"""
    # sort datasets
    currentdata.sort_values(by=['prov', 'ident'], inplace=True)
    histdata.sort_values(by=['prov', 'ident'], inplace=True)
    try:
        banff_call= banfftest.pytest_estimato(
            indata=currentdata,
            indata_hist=histdata,
            instatus=curstatus,
            #instatus_hist=histstatus,
            inalgorithm=algorithm,
            inestimator=estimator.query('est_setid=="set1"'),
            accept_negative=True,
            exclude_where_indata="x>2000",
            exclude_where_indata_hist="x>1000;",
            by="prov",
            unit_id="ident",

            pytest_capture=capfd,
            expected_warning_count=4,
            msg_list_contains_exact=[
                "NOTE: data_excl_var = _exclude",
                "NOTE: hist_excl_var = _exclude",
            ],
            msg_list_contains=expected_log,
        )
    except ValueError as e:
        assert e.args[0] == "Parameter `exclude_where_indata_hist` cannot contain the substring ';'"
    else:
        raise AssertionError("Procedure should have raised a ValueError")

##### DATA #####

@pytest.fixture
def expected_log():
    return """Statistics on observations:
--------------------------

Number of observations in indata data set ..........:      12
Number of observations in indata_hist data set ..........:      10
Number of valid observations in indata data set ....:      12
Number of valid observations in indata_hist data set ....:       9

Statistics on estimators:
------------------------

Estimator 0.
No parameters.

Estimator 1.
Count = 6 Percent = 50.00 not weighted.
Average    Period              Variable
658.33333  CURR                X


NOTE: Estimator 0: Number of random error donors is 6.

NOTE: Estimator 1: Number of random error donors is 6.


Statistics on imputation:
------------------------

Est Variable                            FTI    Imp    */0
0   X                                     3      3      0
1   X                                     0      0      0

NOTE: The above message was for the following by group:
      prov=24

Statistics on observations:
--------------------------

Number of observations in indata data set ..........:      10
Number of observations in indata_hist data set ..........:      10
Number of valid observations in indata data set ....:      10
Number of valid observations in indata_hist data set ....:       7

Statistics on estimators:
------------------------

Estimator 0.
No parameters.

Estimator 1.
Count = 4 Percent = 40.00 not weighted.
Average    Period              Variable
831.25000  CURR                X


NOTE: Estimator 0: Number of random error donors is 4.

WARNING: Estimator: Estimator 0: Number of random error donors is smaller than 5. Variance of the imputed values will not be reliable.

NOTE: Estimator 1: Number of random error donors is 4.

WARNING: Estimator: Estimator 1: Number of random error donors is smaller than 5. Variance of the imputed values will not be reliable.


Statistics on imputation:
------------------------

Est Variable                            FTI    Imp    */0
0   X                                     3      3      0
1   X                                     0      0      0

NOTE: The above message was for the following by group:
      prov=30"""

@pytest.fixture
def without_exclusions_log():
    return """Statistics on observations:
--------------------------

Number of observations in indata data set ..........:      12
Number of observations in indata_hist data set ..........:      10
Number of valid observations in indata data set ....:      12
Number of valid observations in indata_hist data set ....:       9

Statistics on estimators:
------------------------

Estimator 0.
No parameters.

Estimator 1.
Count = 9 Percent = 75.00 not weighted.
Average    Period              Variable
722.22222  CURR                X


NOTE: Estimator 0: Number of random error donors is 6.

NOTE: Estimator 1: Number of random error donors is 9.


Statistics on imputation:
------------------------

Est Variable                            FTI    Imp    */0
0   X                                     3      3      0
1   X                                     0      0      0

NOTE: The above message was for the following by group:
      prov=24

Statistics on observations:
--------------------------

Number of observations in indata data set ..........:      10
Number of observations in indata_hist data set ..........:      10
Number of valid observations in indata data set ....:      10
Number of valid observations in indata_hist data set ....:       7

Statistics on estimators:
------------------------

Estimator 0.
No parameters.

Estimator 1.
Count = 7 Percent = 70.00 not weighted.
Average    Period              Variable
3446.42857 CURR                X


NOTE: Estimator 0: Number of random error donors is 4.

WARNING: Estimator: Estimator 0: Number of random error donors is smaller than 5. Variance of the imputed values will not be reliable.

NOTE: Estimator 1: Number of random error donors is 7.


Statistics on imputation:
------------------------

Est Variable                            FTI    Imp    */0
0   X                                     3      3      0
1   X                                     0      0      0

NOTE: The above message was for the following by group:
      prov=30"""

@pytest.fixture
def currentdata():
    return pd.read_csv(StringIO("""
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

@pytest.fixture
def currentdata_with_exclude():
    return pd.read_csv(StringIO("""
    ident x prov _exclude
    REC01 -500 24 E
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

@pytest.fixture
def histdata():
    return pd.read_csv(StringIO("""
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

@pytest.fixture
def histdata_with_exclude():
    return pd.read_csv(StringIO("""
    ident x prov _exclude
    REC01 500 24 E
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

@pytest.fixture
def curstatus():
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
def histstatus():
    return pd.read_csv(StringIO("""
    ident fieldid status
    REC02 X FTI
    REC10 X FTI
    REC14 X FTI
    REC17 X FTI
    """), sep=r'\s+')

# add exclusion column to current data
def add_exclude(row):
    if row['x'] > 2000:
        return 'E'
    else:
        return ''

@pytest.fixture
def algorithm():
    new_ds = pd.DataFrame()
    new_ds['algorithmname'] = ['prevalue']
    new_ds['type'] = ['ef']
    new_ds['status'] = ['vp']
    new_ds['formula'] = ['fieldid(h,v)']
    new_ds['description'] = ['Previous value is imputed.']
    return new_ds


@pytest.fixture
def estimator():
    new_ds = pd.DataFrame(columns= ["est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])
    new_ds.loc[len(new_ds)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'algorithmname':'prevalue', 'fieldid':'x'}
    new_ds.loc[len(new_ds)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1, 'algorithmname':'curmean', 'fieldid':'x'}
    new_ds.loc[len(new_ds)] = {'est_setid':'set2', 'excludeimputed':'n', 'excludeoutliers':'n', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'diftrend', 'fieldid':'x'}
    new_ds.loc[len(new_ds)] = {'est_setid':'set2', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'histreg', 'fieldid':'x'}
    character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
    new_ds[character_column_list] = new_ds[character_column_list].fillna('')
    return new_ds

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()