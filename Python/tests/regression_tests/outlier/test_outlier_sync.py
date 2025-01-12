"""Ensure proper syncing of `indata` and `indata_hist` datasets with `by` variables.  """

import pytest
import banff.testing as banfftest

@pytest.mark.m_auto_pass
def test_outlier_sync_a(capfd, outlier_indata_04, outlier_indata_hist_02):
    """When indata_hist has the correct columns but 0 rows, an fatal access violation should NOT occur.  """
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_04,
        indata_hist=outlier_indata_hist_02,
        outstatus_detailed=True,
        method="H",
        mii=3,
        mei=2,
        unit_id="ident",
        var="V1 V2",
        by="area",
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=None,  # ignore warning count
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()