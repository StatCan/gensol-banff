"""If the NOTSORTED option is not used with the BY statement, the current data set must be
 sorted by the BY variable(s) first.  If not, Outlier should not proceed.  
 Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m__default_var_list
def test_outlier_d03_a(capfd, outlier_indata_03):
    """method='c'
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_03,
        method="C",
        mii=3,
        mei=2,
        unit_id="ident",
        var="V1",
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = 2.000000 and the next by group has area = 1.000000."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_d03_b(capfd, outlier_indata_03, outlier_indata_hist_01):
    """method='h'
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_03,
        indata_hist=outlier_indata_hist_01,
        method="H",
        mii=3,
        mei=2,
        unit_id="ident",
        var="V1",
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = 2.000000 and the next by group has area = 1.000000."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_d03_c(capfd, outlier_indata_03, outlier_indata_hist_01):
    """method='s'
    """
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_03,
        indata_hist=outlier_indata_hist_01,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        unit_id="ident",
        var="V1",
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = 2.000000 and the next by group has area = 1.000000."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()