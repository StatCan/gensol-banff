"""Basic test of `banff.set_default_output_spec()`

Ensure that the default output spec is as expected, and that modifications are effective.
"""
import banff.io_util
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa
import banff

@pytest.mark.m_auto_pass
def test_editstat_default_output_spec_a(capfd, editstat_indata):
    """Check for correct default output type"""
    banff_call = banfftest.pytest_editstat(
        indata=editstat_indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",

        pytest_capture=capfd,
    )

    assert isinstance(banff_call.banff_call.outedit_status, pa.Table)

@pytest.mark.m_auto_pass
def test_editstat_default_output_spec_b(capfd, editstat_indata):
    """Try modifying default spec"""
    banff.set_default_output_spec("pandas")

    banff_call = banfftest.pytest_editstat(
        indata=editstat_indata,
        edits="""
            x1+1>=x2;
            x1<=5;
            x2>=x3;
            x1+x2+x3<=9;
        """,
        by="Group",

        pytest_capture=capfd,
    )

    assert isinstance(banff_call.banff_call.outedit_status, pd.DataFrame)

@pytest.mark.m_auto_pass
def test_editstat_default_output_spec_c(capfd, editstat_indata):
    """Try setting to invalid specification"""
    try:
        banff.set_default_output_spec("fake spec")
    except ValueError:
        pass
    else:
        raise AssertionError("Exception should occur when invalid spec provided")

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

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()