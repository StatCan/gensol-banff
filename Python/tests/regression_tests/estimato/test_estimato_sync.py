"""Ensure proper syncing of `indata` and `indata_hist` datasets with `by` variables.  

When `by` variables are included on both these datasets their types (numeric or text) should match.  
Multiple subtypes exist for these, such as int and float for numeric types
For text data `pyarrow` has the 'string' and 'large_string' datatypes.  

Syncing should succeed as long as each of the variables on both datasets are both numeric or text.  
Slight differences in datatypes (eg 'int' and 'double', 'string' and 'large_string') should not cause any errors.  
"""
import pytest
import pandas as pd
import pyarrow as pa
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_sync_a(capfd, estimato_indata_03, estimato_instatus_03, estimato_indata_hist_03, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    """Successfully sync datasets when 'area' (by variable) has 'string' type for indata, 'large_string' type for indata_hist.  """
    #### arrange

    # use `string` in `indata`, `large_string` in `indata_hist`
    indata = pa.Table.from_pandas(estimato_indata_03)
    indata_hist = pa.Table.from_pandas(estimato_indata_hist_03.astype({'area': pd.StringDtype(storage="pyarrow")}))

    # ensure `indata` uses 'string' datatype
    assert indata.schema.field('area').type.equals(pa.string()), 'indata has wrong datatype for area variable'

    # ensure `indata_hist` uses 'large_string' datatype
    assert indata_hist.schema.field('area').type.equals(pa.large_string()), 'indata_hist has wrong datatype for area variable'

    #### act
    banfftest.pytest_estimato(
        # Banff parameters
        indata=indata,
        instatus=estimato_instatus_03,
        indata_hist=indata_hist,
        instatus_hist=estimato_instatus_hist_03,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        expected_warning_count=2,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_sync_b(capfd, estimato_indata_04, estimato_instatus_03, estimato_indata_hist_04, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    """Successfully sync datasets when 'area' has 'float' type for indata, 'int' type for indata_hist.  """
    #### arrange

    # use `string` in `indata`, `large_string` in `indata_hist`
    indata = pa.Table.from_pandas(estimato_indata_04)
    indata_hist = pa.Table.from_pandas(estimato_indata_hist_04.astype({'area': pd.Int64Dtype()}))

    # ensure `indata` uses 'string' datatype
    assert indata.schema.field('area').type.equals(pa.float64()), 'indata has wrong datatype for area variable'

    # ensure `indata_hist` uses 'large_string' datatype
    assert indata_hist.schema.field('area').type.equals(pa.int64()), 'indata_hist has wrong datatype for area variable'

    #### act
    banfftest.pytest_estimato(
        # Banff parameters
        indata=indata,
        instatus=estimato_instatus_03,
        indata_hist=indata_hist,
        instatus_hist=estimato_instatus_hist_03,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        expected_warning_count=2,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_sync_c(capfd, estimato_indata_04, estimato_instatus_03, estimato_indata_hist_03, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    """Failure during sync when 'area' has 'float' type for indata, 'string' type for indata_hist.  """
    #### arrange

    # use `string` in `indata`, `large_string` in `indata_hist`
    indata = pa.Table.from_pandas(estimato_indata_04)
    indata_hist = pa.Table.from_pandas(estimato_indata_hist_03)

    # ensure `indata` uses 'string' datatype
    assert indata.schema.field('area').type.equals(pa.float64()), 'indata has wrong datatype for area variable'

    # ensure `indata_hist` uses 'large_string' datatype
    assert indata_hist.schema.field('area').type.equals(pa.string()), 'indata_hist has wrong datatype for area variable'

    #### act
    banfftest.pytest_estimato(
        # Banff parameters
        indata=indata,
        instatus=estimato_instatus_03,
        indata_hist=indata_hist,
        instatus_hist=estimato_instatus_hist_03,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",

        # Unit Test Parameters
        rc_should_be_zero=False,
        pytest_capture=capfd,
        expected_error_count=1,
        expected_warning_count=3,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_sync_d(capfd, estimato_indata_04, estimato_instatus_03, estimato_indata_hist_05, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    """When indata_hist has the correct columns but 0 rows, an fatal access violation should NOT occur.  """
    #### arrange

    indata = pa.Table.from_pandas(estimato_indata_04)

    #### act
    banfftest.pytest_estimato(
        # Banff parameters
        indata=indata,
        instatus=estimato_instatus_03,
        indata_hist=estimato_indata_hist_05,
        instatus_hist=estimato_instatus_hist_03,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        expected_warning_count=2,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()