"""This file will ideally provide a convenient means of running simple calls on each procedure, 
while exposing various options from the procedures pytest helper function.  

At its initial creation it is only accessible from the nls folder.  
It may become built into `banff.testing`, but its contents should be refined before doing that.  
"""

import banff.testing as banfftest
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

def run_determin_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    indata = pd.read_csv(StringIO("""
        ident TOTAL Q1 Q2 Q3 Q4 staff prov
        REC01 500 100 125 125 150 2000 24
        REC02 750 200 170 130 250 2500 24
        REC03 400 80 90 100 130 1500 24
        REC04 1000 150 250 350 250 3500 24
        REC05 3000 500 500 1000 1000 5000 24
        REC06 50 10 15 500 20 100 24
        REC07 600 110 140 230 45 2400 30
        REC08 900 175 999 999 300 3000 30
        REC09 2500 400 555 600 5000 89 30
        REC10 800 11 12 13 14 2800 30
        REC11 -25 -10 -5 -5 -10 3000 30
    """), sep=r'\s+')

    # create instatus
    determstat = pd.read_csv(StringIO("""
        fieldid status ident
        Q3 FTI REC06
        Q4 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        staff FTI REC09
        Q1 FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q4 FTI REC11
    """), sep=r'\s+')

    # run Banff procedure
    banff_call = banfftest.pytest_determin(
        indata=indata,
        instatus=determstat,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="ident",
        by="prov",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_donorimp_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    indata = pd.read_csv(StringIO("""
        IDENT TOTAL Q1 Q2 Q3 Q4 STAFF PROV
        REC01 500 120 150 80 150 50 24
        REC02 750 200 170 130 250 75 24
        REC03 400 80 90 100 130 40 24
        REC04 1000 150 250 350 250 100 24
        REC05 1050 200 225 325 300 100 24
        REC06 500 100 125 5000 130 45 24
        REC07 400 80 90 100 15 40 30
        REC08 950 150 999 999 200 90 30
        REC09 1025 200 225 300 10 10 30
        REC10 800 11 12 13 14 80 30
        REC11 -25 -10 -5 -5 -10 3000 30
        REC12 1000 150 250 350 250 100 30
        REC13 999 200 225 325 300 100 30
        REC14 -25 -10 -5 -10 -5 3000 30
    """), sep=r'\s+')

    # TIP: write a function and use the dataframe's `apply()` method to add TOEXCL
    def add_exclude(row):
        if row['TOTAL'] > 1000:
            return 'E'
        else:
            return ''

    # pandas.DataFrame objects have a method, `df.apply()` which can be used
    # to perform a function on each column (by default) or row (use `axis=1`)
    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)
    # we added a new column, "TOEXCL" to the indata dataframe 

    # create instatus
    donorstat = pd.read_csv(StringIO("""
        FIELDID STATUS IDENT
        Q3 IPR REC01
        Q4 FTE REC04
        Q3 FTI REC06
        Q2 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        STAFF FTI REC09
        Q1 FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q1 FTI REC11
        Q2 FTI REC11
        Q3 FTI REC11
        Q4 FTI REC11
        Q2 FTI REC13
        Q3 FTI REC13
    """), sep=r'\s+')

    # run Banff procedure
    banff_call = banfftest.pytest_donorimp(
        indata=indata,
        instatus=donorstat,
        outmatching_fields=True,
        unit_id="IDENT", 
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        post_edits="Q1 + Q2 + Q3 + Q4 - TOTAL <= 0;",
        min_donors=1,
        percent_donors=1,
        n=1,
        n_limit=1,
        mrl=0.5,
        random=True,
        eligdon="original",
        accept_negative=True,
        must_match="STAFF",
        data_excl_var="TOEXCL",
        by="prov",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )


def run_editstats_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    indata = pd.read_csv(StringIO("""
        x1 x2 x3 Group
        4 3 2 A
        -4 3 2 A
        6 3 2 B
        4 3 NaN A
        6 3 NaN B
    """), sep=r'\s+').sort_values(by=['Group'])

    # run Banff procedure
    banff_call = banfftest.pytest_editstat(
        indata=indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",
        accept_negative=True,

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_errorloc_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    example_indata = pd.read_csv(StringIO("""
        IDENT X1 X2 ZONE
        R03 10 40 B
        R02 -4 49 A
        R04 4 49 A
        R01 16 49 A
        R05 15 51 B
        R07 -4 29 B
        R06 30 70 B
    """), sep=r"\s+")

    # sort dataset
    example_indata = example_indata.sort_values(by=['ZONE', 'IDENT'])

    # run Banff procedure
    banff_call = banfftest.pytest_errorloc(
        indata=example_indata,
        edits="x1>=-5; x1<=15; x2>=30; x1+x2<=50;",
        weights="x1=1.5;",
        cardinality=2,
        time_per_obs=0.1,
        unit_id="IDENT",
        by="ZONE",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_estimator_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
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

    # add exclusion column to current data
    def add_exclude(row):
        if row['x'] > 1000:
            return 'E'
        else:
            return ''

    currentdata["CURRDATAEXCL"] = currentdata.apply(add_exclude, axis=1)

    # add exclusion column to historical data
    # TIP: alternatively we can use a "lambda function" to add the exclude column without creating a separate function
    histdata["HISTDATAEXCL"] = histdata.apply(lambda row: 'E' if row['x'] > 2000 else '' , axis=1)

    # sort datasets
    currentdata = currentdata.sort_values(by=['prov', 'ident'])
    histdata = histdata.sort_values(by=['prov', 'ident'])

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
    estimator[character_column_list] = estimator[character_column_list].astype("string")

    # run Banff procedure
    banff_call = banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        instatus_hist=histstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_massimp_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    massimpdata = pd.read_csv(StringIO("""
        IDENT TOTAL Q1 Q2 Q3 Q4 STAFF
        REC01 500 100 125 125 150 1000
        REC02 750 200 170 130 250 2000
        REC03 400 80 90 100 130 1000
        REC04 1000 150 250 350 250 2000
        REC05 3000 500 500 1000 1000 1000
        REC06 800 200 225 200 175 2000
        REC07 600 125 150 175 150 1000
        REC08 900 175 200 225 300 2000
        REC09 2500 500 650 600 750 1000
        REC10 800 150 175 225 250 2000
        REC21 3000 -45 -50 75 -234 2000
        REC11 575 NaN NaN NaN NaN 1000
        REC12 850 NaN NaN NaN NaN 2000
        REC13 375 NaN NaN NaN NaN 1000
        REC14 1100 NaN NaN NaN NaN 2000
        REC15 3100 NaN NaN NaN NaN 1000
        REC16 750 NaN NaN NaN NaN 2000
        REC17 675 NaN NaN NaN NaN 1000
        REC18 875 NaN NaN NaN NaN 2000
        REC19 4000 NaN NaN NaN NaN 1000
        REC20 NaN NaN NaN NaN NaN 2000
    """), sep=r'\s+')

    # sort dataset
    massimpdata = massimpdata.sort_values(by=["STAFF", "IDENT"])

    # run Banff procedure
    banff_call = banfftest.pytest_massimpu(
        indata=massimpdata,
        min_donors=1,
        percent_donors=1,
        random=True,
        accept_negative=True,
        unit_id="IDENT",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="TOTAL",
        by="STAFF",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_outlier_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
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
    banff_call = banfftest.pytest_outlier(
        indata=outlierdata,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_prorate_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # create indata
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['ident'] = "string"
    proratedata = pd.read_csv(StringIO("""
        ident Q1 Q2 Q3 Q4 YEAR
        R01 -25 42 27 25 40
        R02 -25 44 20 20 40
        R03 -18 44 15 5 40
        R04 -18 42 15 5 40
        R05 12 30 15 5 40
        R06 12 20 10 5 40
        """), 
        sep=r'\s+', 
        dtype=var_types
    )

    # run Banff procedure
    banff_call = banfftest.pytest_prorate(
        indata=proratedata,
        edits="Q1+4Q2+2Q3+Q4=YEAR;",
        method="BASIC",
        decimal=1,
        lower_bound=-100,
        upper_bound=100,
        modifier="always",
        accept_negative=True,
        unit_id="ident",

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )

def run_verifyedits_ex01(
    capfd,
    capture=False,
    expected_warning_count=None,
    expected_error_count=None,
    msg_list_contains_exact=None,
):
    # run Banff procedure
    banff_call = banfftest.pytest_verifyed(
        edits="""
            HEN_LT20+HEN_GE20+HEN_OTH=HEN_TOT;
            2*EGG_LAID<=HEN_GE20;
            HEN_GE20<=4*EGG_LAID;
            EGG_SOLD<=EGG_LAID;
            EGG_VALU<=2.75*EGG_SOLD;
            0.9*EGG_SOLD<=EGG_VALU;
            fail:HEN_GE20>300000;
        """,
        imply=50,
        extremal=10,

        capture=capture,

        pytest_capture=capfd,
        expected_warning_count=expected_warning_count,
        expected_error_count=expected_error_count,
        msg_list_contains_exact=msg_list_contains_exact,
    )