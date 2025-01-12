"""Test the optional BY variables on status file.  
Results should be identical regardless of BY variable presence on status input file.  
However, performance and console-logged WARNING: and NOTE: messages will differ"""
import pytest
from banff.testing import assert_dataset_equal, assert_substr_count
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donor_IT09(capfd):# import necessary Python packages
    import banff
    import pandas as pd
    from io import StringIO

    # create indata dataframe
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

    # create donorstat dataset
    donorstat = pd.read_csv(StringIO("""
    FIELDID STATUS IDENT prov
    Q3 IPR REC01 24
    Q4 FTE REC04 24
    Q3 FTI REC06 24
    Q2 FTI REC07 30
    Q2 FTI REC08 30
    Q3 FTI REC08 30
    Q4 FTI REC09 30
    STAFF FTI REC09 30
    Q1 FTI REC10 30
    Q2 FTI REC10 30
    Q3 FTI REC10 30
    Q4 FTI REC10 30
    Q1 FTI REC11 30
    Q2 FTI REC11 30
    Q3 FTI REC11 30
    Q4 FTI REC11 30
    Q2 FTI REC13 30
    Q3 FTI REC13 30
    """), sep=r'\s+')

    # create Banff call 
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
                    outmatching_fields=True,
                    accept_negative=True,
                    must_match="STAFF",
                    data_excl_var="TOEXCL",
                    by="prov"
                    )

    # TIP: this is how you access the ouput datasets
    print("OUTDATA dataset \n", banff_call.outdata, "\n")
    print("OUTSTATUS dataset\n", banff_call.outstatus, "\n")
    print("outdonormap dataset\n", banff_call.outdonormap, "\n")

import sys
if __name__ == "__main__":
    pytest.main(sys.argv)