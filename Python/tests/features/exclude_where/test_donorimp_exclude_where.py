"""Basic test validates donorimp's `exclude_where_indata` option

Tests derived from user guide example 1.
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_a(capfd, indata, donorstat, expected_stats_with_exclude):
    """Run user guide example 1 as-is, validate expected header and statistics"""

    # we added a new column, "TOEXCL" to the indata dataframe 
    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)

    banff_call= banfftest.pytest_donorimp(
        indata=indata,
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        data_excl_var="TOEXCL",
        by="prov",
        #exclude_where_indata="total < 0",

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact=[
            "NOTE: data_excl_var = TOEXCL",
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but TOEXCL value is 'E' for these observations.",
        ],
        msg_list_contains=expected_stats_with_exclude,
    )

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_b(capfd, indata, donorstat, expected_stats_without_exclude):
    """Run user guide example 1 without an exclusion variable, validate
     that header and statistics differ
     """
    banff_call= banfftest.pytest_donorimp(
        indata=indata,
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        by="prov",
        #exclude_where_indata="total < 0",

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact="NOTE: data_excl_var not specified",
        msg_list_contains=expected_stats_without_exclude
    )

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_c(capfd, indata, donorstat, expected_stats_with_exclude):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - pandas dataframe for input
        validate expected header, that stats match original stats
    """
    banff_call= banfftest.pytest_donorimp(
        indata=indata,
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        by="prov",
        exclude_where_indata="total > 1000",

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact=[
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but _exclude value is 'E' for these observations.",
            "NOTE: data_excl_var = _exclude",
        ],
        msg_list_contains=expected_stats_with_exclude
    )

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_d(capfd, indata, donorstat, expected_stats_with_exclude):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - pyarrow table for input
        validate expected header, that stats match original stats
    """
    banff_call= banfftest.pytest_donorimp(
        indata=pa.Table.from_pandas(indata),
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        by="prov",
        exclude_where_indata="total > 1000",

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact=[
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but _exclude value is 'E' for these observations.",
            "NOTE: data_excl_var = _exclude",
        ],
        msg_list_contains=expected_stats_with_exclude
    )

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_e(capfd, indata_with_exclude, donorstat, expected_stats_with_exclude):
    """Run user guide example 1 using `exclude_where_indata` instead of original exclusion scheme
        - default name ('_exclude') already exists, should add random suffix
        validate expected header, that stats match original stats
    """
    banff_call= banfftest.pytest_donorimp(
        indata=indata_with_exclude,
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        by="prov",
        exclude_where_indata="total > 1000",

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact=[
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but _exclude_",
            " value is 'E' for these observations.",
            "NOTE: data_excl_var = _exclude_",
        ],
        msg_list_contains=expected_stats_with_exclude
    )

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_f(capfd, indata, donorstat, expected_stats_with_exclude):
    """Run user guide example 1 as-is, validate expected header and statistics
    
    Passing empty string for `exclude_where...` should be ignored
    """

    # we added a new column, "TOEXCL" to the indata dataframe 
    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)

    banff_call= banfftest.pytest_donorimp(
        indata=indata,
        instatus=donorstat,
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
        outmatching_fields=True,
        accept_negative=True,
        must_match="STAFF",
        data_excl_var="TOEXCL",
        by="prov",
        exclude_where_indata=" ",

        trace=True,  # verbose logging
        capture=True,  # so pytest can capture python logging statements

        pytest_capture=capfd,
        expected_warning_count=1,
        msg_list_contains_exact=[
            "Ignoring option `exclude_where_indata=",
            "NOTE: data_excl_var = TOEXCL",
            "NOTE: There were 1 observations dropped from indata data set because they are potential donors but TOEXCL value is 'E' for these observations.",
        ],
        msg_list_contains=expected_stats_with_exclude,
    )

def test_donorimp_exclude_where_indata_g(capfd, indata, donorstat, expected_stats_with_exclude):
    """it is prohibited to specify a non-empty `exclude_where_indata` and `data_excl_var` together
    """

    # we added a new column, "TOEXCL" to the indata dataframe 
    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)

    try:
        banff_call= banfftest.pytest_donorimp(
            indata=indata,
            instatus=donorstat,
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
            outmatching_fields=True,
            accept_negative=True,
            must_match="STAFF",
            data_excl_var="TOEXCL",
            by="prov",
            exclude_where_indata="total > 1000",

            pytest_capture=capfd,
        )
    except ValueError as e:
        assert e.args[0] == 'options `data_excl_var` and `exclude_where_indata` cannot be specified together'
    else:
        raise AssertionError("Did not receive expected exception, `ValueError`")

@pytest.mark.m_auto_pass
def test_donorimp_exclude_where_indata_h(capfd, indata, donorstat, expected_stats_with_exclude):
    """Run user guide example 1  passing a `exclude_where_indata` with disallowed ';' character"""
    try:
        banff_call= banfftest.pytest_donorimp(
            indata=indata,
            instatus=donorstat,
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
            outmatching_fields=True,
            accept_negative=True,
            must_match="STAFF",
            by="prov",
            exclude_where_indata="total > 1000;",

            pytest_capture=capfd,
            expected_warning_count=1,
            msg_list_contains_exact=[
                "NOTE: There were 1 observations dropped from indata data set because they are potential donors but _exclude value is 'E' for these observations.",
                "NOTE: data_excl_var = _exclude",
            ],
            msg_list_contains=expected_stats_with_exclude
        )
    except ValueError as e:
        assert e.args[0] == "Parameter `exclude_where_indata` cannot contain the substring ';'"
    else:
        raise AssertionError("Procedure should have raised a ValueError")

##### DATA #####

@pytest.fixture
def expected_stats_with_exclude():
    return """
Number of observations ............................:      14
Number of observations dropped ....................:       2
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       1
    non-eligible donors ...........................:       1

Number of valid observations ......................:      12     100.00%
Number of donors ..................................:       5      41.67%
Number of donors to reach DONORLIMIT ..............:       3      60.00%
Number of recipients ..............................:       7      58.33%
    with matching fields ..........................:       7      58.33%
        imputed ...................................:       5      41.67%
        not imputed ...............................:       2      16.67%
    without matching fields .......................:       0       0.00%
        imputed ...................................:       0       0.00%
        not imputed ...............................:       0       0.00%


NOTE: The above message was for the total of all by-groups."""

@pytest.fixture
def expected_stats_without_exclude():
    return """
Number of observations ............................:      14
Number of observations dropped ....................:       1
    missing key ...................................:       0
    missing data ..................................:       0
    negative data .................................:       0
    mixed observations ............................:       0
    excluded donors ...............................:       0
    non-eligible donors ...........................:       1

Number of valid observations ......................:      13     100.00%
Number of donors ..................................:       6      46.15%
Number of donors to reach DONORLIMIT ..............:       3      50.00%
Number of recipients ..............................:       7      53.85%
    with matching fields ..........................:       7      53.85%
        imputed ...................................:       5      38.46%
        not imputed ...............................:       2      15.38%
    without matching fields .......................:       0       0.00%
        imputed ...................................:       0       0.00%
        not imputed ...............................:       0       0.00%


NOTE: The above message was for the total of all by-groups.
"""

@pytest.fixture
def indata():
    return pd.read_csv(StringIO("""
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

@pytest.fixture
def indata_with_exclude():
    return pd.read_csv(StringIO("""
    IDENT TOTAL Q1 Q2 Q3 Q4 STAFF PROV _exclude
    REC01 500 120 150 80 150 50 24 E
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

def add_exclude(row):
    if row['TOTAL'] > 1000:
        return 'E'
    else:
        return ''

@pytest.fixture
def donorstat():
    return pd.read_csv(StringIO("""
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

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()