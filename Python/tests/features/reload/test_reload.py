"""Test the ability to unload and reload procedures, enabling or disabling debug statistics"""
# import necessary Python packages
import banff
import pandas as pd
from io import StringIO
import pytest
import banff.testing

@pytest.mark.m_auto_pass
def test_reload(capfd, indata, determstat):
    """Run procedure call without enabling C debug output, assure no debug output produced, 
    then enable debug output, run again, check that debug output is produced"""

    expected_debug_outputs=[
        "[C,TIME,CPU",
        "MEMORY USAGE:",
    ]

    banff.testing.pytest_determin(
        indata=indata,
        instatus=determstat,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="ident",
        by="prov",

        pytest_capture=capfd
    )
    output_non_debug = capfd.readouterr()[0]  # clear captured contents
    for msg in expected_debug_outputs:
        assert msg not in output_non_debug, "debug output found in non-debug mode"

    try:
        # enable debug output
        banff.proc.BanffProcedure._reload_all_procs(debug=True)
        banff.testing.pytest_determin(
            indata=indata,
            instatus=determstat,
            edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
            accept_negative=True,
            unit_id="ident",
            by="prov",

            pytest_capture=capfd,
            msg_list_contains_exact=expected_debug_outputs
        )
    finally:
        # disable debug output
        banff.proc.BanffProcedure._reload_all_procs(debug=False)

@pytest.fixture
def indata():
# create indata dataframe
    return pd.read_csv(StringIO("""
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

@pytest.fixture
def determstat():
    # create donorstat dataset
    return pd.read_csv(StringIO("""
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


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()