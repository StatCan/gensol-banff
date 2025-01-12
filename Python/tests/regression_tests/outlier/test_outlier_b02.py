"""With historical trends and ratios, if the var statement is present without 
WITH and the VAR variable(s) specified are not in the hist/aux input data set, 
Outlier should not proceed.  Error messages should be printed to the log.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_b02_b(capfd, outlier_indata_02, outlier_indata_hist_01):
    """V2 missing, historic method
    """
    outlier_indata_hist_01.rename(columns={"V2":"V3"}, inplace=True)
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        unit_id="ident",
        method="H",
        mii=3,
        mei=2,
        var="V1 V2",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'V2' in list 'with_var' (indata_hist dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_b02_c(capfd, outlier_indata_02, outlier_indata_hist_01):
    """V2 missing, sigma gap method
    """
    outlier_indata_hist_01.rename(columns={"V2":"V3"}, inplace=True)
    banfftest.pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        unit_id="ident",
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        var="V1 V2",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'V2' in list 'with_var' (indata_hist dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()