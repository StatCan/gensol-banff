"""Ensure the `verify_specs` option works as expected
"""
import pytest
import pandas as pd
import pyarrow as pa
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_verify_specs_a(capfd, estimato_indata_03, estimato_instatus_03, estimato_indata_hist_03, estimato_instatus_hist_03, estimato_inalgorithm_03, estimato_inestimator_03):
    """When `verify_specs=True` is specified, the procedure should return without error and without producing output data.  """
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
        verify_specs=True,

        # Unit Test Parameters
        pytest_capture=capfd,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()