"""Ensure the `verify_specs` option works as expected
"""
import pytest
import pandas as pd
import pyarrow as pa
import banff.testing as banfftest
import banff
import pandas as pd
from io import StringIO
from collections import defaultdict # helps with assigning variable types when making datasets
import numpy as np # for referencing `np.float64`

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_prorate_verify_edits_a(capfd, prorate_indata):
    # run Banff procedure
    banff_call = banff.testing.pytest_prorate(
        indata=prorate_indata,
        edits="Q1+4Q2+2Q3+Q4=YEAR;",
        method="BASIC",
        decimal=1,
        lower_bound=-100,
        upper_bound=100,
        modifier="always",
        accept_negative=True,
        unit_id="ident",
        verify_edits=True,

        pytest_capture=capfd,
        msg_list_contains_exact=[
            "NOTE: outdata: disabled",
            "NOTE: outstatus: disabled",
            "NOTE: outreject: disabled",
            "NOTE: verify_edits",
        ],
    )

@pytest.fixture
def prorate_indata():
    # create indata
    var_types = defaultdict(lambda: np.float64) # default to numeric
    var_types['ident'] = "string"
    return pd.read_csv(StringIO("""
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

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()
