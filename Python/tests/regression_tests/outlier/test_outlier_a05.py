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
def test_outlier_a05_b(capfd, outlier_indata_02, outlier_indata_hist_01):
    """ID is not on indata
    """
    #### arrange
    # change 'ident' to 'id', causing mismatching key variable
    outlier_indata_hist_01.rename(columns={"ident":"id"},inplace=True)

    #### act
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        outlier_stats=True,
        method="H",
        mii=3,
        mei=2,
        unit_id="ID",
        var="V1 V2",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ID' in list 'unit_id' (indata dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_a05_c(capfd, outlier_indata_02, outlier_indata_hist_01):
    """ident is not on indata_hist
    """
    #### arrange
    # change 'ident' to 'id', causing mismatching key variable
    outlier_indata_hist_01.rename(columns={"ident":"id"},inplace=True)

    #### act
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        outlier_stats=True,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        unit_id="ident",
        var="V1 V2",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ident' in list 'unit_id' (indata_hist dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_a05_d(capfd, outlier_indata_02, outlier_indata_hist_01):
    """ID is not on indata
    """
    #### arrange
    # change 'ident' to 'id', causing mismatching key variable
    outlier_indata_hist_01.rename(columns={"ident":"id"},inplace=True)

    #### act
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        outlier_stats=True,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        unit_id="ID",
        var="V1 V2",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ID' in list 'unit_id' (indata dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()