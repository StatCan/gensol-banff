import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_performance_test
def test_outlierD07_a(capfd, outlier_indata_d07, outlier_indata_hist_d07):
    """When using BY variables, datasets not sorted by the BY variables should trigger
    an error message"""
    #### arrange

    #### act
    pytest_outlier(
        # Outlier Parameters
        trace=True,
        indata=outlier_indata_d07,
        indata_hist=outlier_indata_hist_d07,
        method="H",
        mii=3,
        mei=2,
        outlier_stats=True,
        no_by_stats=True,
        unit_id="ident",
        var="V1 V2",
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )

@pytest.mark.m_performance_test
def test_outlierD07_b(capfd, outlier_indata_d07, outlier_indata_hist_d07):
    """When using BY variables, datasets not sorted by the BY variables should trigger
    an error message"""
    #### arrange

    #### act
    pytest_outlier(
        # Outlier Parameters
        trace=True,
        indata=outlier_indata_d07,
        indata_hist=outlier_indata_hist_d07,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        outlier_stats=True,
        no_by_stats=True,
        unit_id="ident",
        var="V1 V2",
        by="area",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )