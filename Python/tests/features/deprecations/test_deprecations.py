"""Basic test validates donorimp's `exclude_where_indata` option

Tests derived from user guide example 1.
"""
import banff
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa

@pytest.mark.m_auto_pass
def test_deprecation_donorimp_match_field_stat(capfd, indata, donorstat):
    """Run user guide example 1 providing now-deprecated `match_field_stat` option, 
    expect a `DeprecationWarning`"""

    # we added a new column, "TOEXCL" to the indata dataframe 
    indata["TOEXCL"] = indata.apply(add_exclude, axis=1)

    try:
        banff_call= banff.donorimp(
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
            match_field_stat=True,
            accept_negative=True,
            must_match="STAFF",
            data_excl_var="TOEXCL",
            by="prov",
            #exclude_where_indata="total < 0",

        )
    except DeprecationWarning as e:
        assert e.args[0] == 'Option `match_field_stat` is deprecated, use `outmatching_fields` instead'
    else:
        raise AssertionError("Did not receive expected exception, `DeprecationWarning`")

##### DATA #####
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