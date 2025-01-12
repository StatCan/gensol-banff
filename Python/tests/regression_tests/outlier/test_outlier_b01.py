"""For the HB and SG methods using two data sets, the key variable must be on both the current and the 
historical/ratio data set.  If it is not, Outlier should not proceed.  
Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_b01_a(capfd, outlier_indata_02, outlier_indata_hist_01):
    """V3 missing, current method
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        #indata_hist=indata_hist,
        unit_id="ident",
        method="C",
        mii=3,
        mei=2,
        var="V1 V3",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'V3' in list 'var' (indata dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_b01_b(capfd, outlier_indata_02, outlier_indata_hist_01):
    """V3 missing, historic method
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        #indata_hist=indata_hist,
        unit_id="ident",
        method="H",
        mii=3,
        mei=2,
        var="V1 V3",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'V3' in list 'var' (indata dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_b01_c(capfd, outlier_indata_02, outlier_indata_hist_01):
    """V3 missing, sigma gap method
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        #indata_hist=indata_hist,
        unit_id="ident",
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        var="V1 V3",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'V3' in list 'var' (indata dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()