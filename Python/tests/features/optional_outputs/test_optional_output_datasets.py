"""Basic test validates donorimp's `exclude_where_indata` option

Tests derived from user guide example 1.
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa

@pytest.mark.m_auto_pass
def test_editstat_optional_output_a(capfd, editstat_indata):
    """disable some optional datasets"""
    banff_call = banfftest.pytest_editstat(
        indata=editstat_indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",
        accept_negative=True,
        outedits_reduced=False,
        outglobal_status=False,
        #outk_edits_status=False,
        #outvars_role=False,
        #outedit_applic=False,
        #outedit_status=False,

        pytest_capture=capfd,
        msg_list_contains_exact="""
NOTE: indata: 4 column(s), 5 row(s)
NOTE: outedits_reduced: disabled
NOTE: outedit_status: enabled
NOTE: outk_edits_status: enabled
NOTE: outglobal_status: disabled
NOTE: outedit_applic: enabled
NOTE: outvars_role: enabled""",
    )

@pytest.mark.m_auto_pass
def test_editstat_optional_output_b(capfd, editstat_indata):
    """disable all optional datasets"""
    banff_call = banfftest.pytest_editstat(
        indata=editstat_indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",
        accept_negative=True,
        outedits_reduced=False,
        outglobal_status=False,
        outk_edits_status=False,
        outvars_role=False,
        outedit_applic=False,
        outedit_status=False,

        pytest_capture=capfd,
        msg_list_contains_exact="NOTE: No analysis is done by the procedure. _NULL_ has been specified for each output data sets.",
    )

@pytest.mark.m_auto_pass
def test_editstat_optional_output_c(capfd, editstat_indata):
    """default value for all optional datasets"""
    banff_call = banfftest.pytest_editstat(
        indata=editstat_indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",
        accept_negative=True,

        pytest_capture=capfd,
        msg_list_contains_exact="""
NOTE: indata: 4 column(s), 5 row(s)
NOTE: outedits_reduced: enabled
NOTE: outedit_status: enabled
NOTE: outk_edits_status: enabled
NOTE: outglobal_status: enabled
NOTE: outedit_applic: enabled
NOTE: outvars_role: enabled""",
    )

@pytest.mark.m_auto_pass
def test_estimato_optional_output_a(capfd, estimato_data_dict):
    """default value for all optional datasets"""
    d = estimato_data_dict
    banff_call = banfftest.pytest_estimato(
        indata=d['indata'],
        indata_hist=d['indata_hist'],
        instatus=d['instatus'],
        # instatus_hist=histstatus,  # created, but not used in actual user guide example
        inalgorithm=d['algorithm'],
        inestimator=d['estimator'].query('est_setid=="set2"'),
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=7,
        msg_list_contains_exact="""
NOTE: outdata: enabled
NOTE: outstatus: enabled
NOTE: outrand_err: enabled
NOTE: outacceptable: enabled
NOTE: outest_parm: enabled
NOTE: outest_ef: enabled
NOTE: outest_lr: enabled""",
    )

@pytest.mark.m_auto_pass
def test_estimato_optional_output_b(capfd, estimato_data_dict):
    """disable some optional datasets"""
    d = estimato_data_dict
    banff_call = banfftest.pytest_estimato(
        indata=d['indata'],
        indata_hist=d['indata_hist'],
        instatus=d['instatus'],
        # instatus_hist=histstatus,  # created, but not used in actual user guide example
        inalgorithm=d['algorithm'],
        inestimator=d['estimator'].query('est_setid=="set2"'),
        outrand_err=False,
        outacceptable=False,
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",

        pytest_capture=capfd,
        expected_warning_count=7,
        msg_list_contains_exact="""
NOTE: outdata: enabled
NOTE: outstatus: enabled
NOTE: outrand_err: disabled
NOTE: outacceptable: disabled
NOTE: outest_parm: enabled
NOTE: outest_ef: enabled
NOTE: outest_lr: enabled""",
    )

@pytest.mark.m_auto_pass
def test_estimato_optional_output_c(capfd, estimato_data_dict):
    """try disabling mandatory dataset"""
    d = estimato_data_dict
    try:
        banff_call = banfftest.pytest_estimato(
            indata=d['indata'],
            indata_hist=d['indata_hist'],
            instatus=d['instatus'],
            # instatus_hist=histstatus,  # created, but not used in actual user guide example
            inalgorithm=d['algorithm'],
            inestimator=d['estimator'].query('est_setid=="set2"'),
            outdata=False,
            data_excl_var="CURRDATAEXCL",
            hist_excl_var="HISTDATAEXCL",
            by="prov",
            unit_id="ident",

            pytest_capture=capfd,
        )
    except ValueError:
        assert True, "As expected"
    else:
        assert False, "Should have received a `ValueError`"

@pytest.mark.m_auto_pass
def test_outlier_optional_output_a(capfd, outlier_indata):
    """default value for optional dataset"""
    banff_call = banfftest.pytest_outlier(
        indata=outlier_indata,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        msg_list_contains_exact="""
NOTE: outstatus: enabled
NOTE: outstatus_detailed: enabled
NOTE: outsummary: enabled""",
    )

@pytest.mark.m_auto_pass
def test_outlier_optional_output_b(capfd, outlier_indata):
    """disable optional dataset"""
    banff_call = banfftest.pytest_outlier(
        indata=outlier_indata,
        outsummary=False,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        msg_list_contains_exact="""
NOTE: outstatus: enabled
NOTE: outstatus_detailed: enabled
NOTE: outsummary: disabled""",
    )

@pytest.mark.m_auto_pass
def test_outlier_optional_output_c(capfd, outlier_indata):
    """attempt to disable mandatory dataset"""
    try:
        banff_call = banfftest.pytest_outlier(
            indata=outlier_indata,
            outstatus=False,
            method="current",
            mii=4,
            mei=2.5,
            mdm=0.05,
            outlier_stats=True,
            accept_negative=True,
            unit_id="ident",
            var="X01 X02",
            by="Prov",

            pytest_capture=capfd,
        )
    except ValueError:
        assert True, "As expected"
    else:
        assert False, "Should have received a `ValueError`"

@pytest.mark.m_auto_pass
def test_outlier_optional_output_b(capfd, outlier_indata):
    """disable optional dataset"""
    banff_call = banfftest.pytest_outlier(
        indata=outlier_indata,
        outstatus_detailed=False,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        msg_list_contains_exact="""
NOTE: outstatus: enabled
NOTE: outstatus_detailed: disabled
NOTE: outsummary: enabled""",
    )

##### DATA #####

@pytest.fixture
def editstat_indata():
    return pd.read_csv(StringIO("""
    x1 x2 x3 Group
    4 3 2 A
    -4 3 2 A
    6 3 2 B
    4 3 NaN A
    6 3 NaN B
"""), sep=r'\s+').sort_values(by=['Group'])

@pytest.fixture
def outlier_indata():
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
    return outlierdata

@pytest.fixture
def estimato_data_dict():
        
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

    return {
        'indata': currentdata,
        'indata_hist': histdata,
        'instatus': curstatus,
        'instatus_hist': histstatus,
        'algorithm': algorithm,
        'estimator': estimator
    }


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()