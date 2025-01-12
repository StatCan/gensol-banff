"""Only one key variable is allowed, i.e. no composite keys are permitted.  If a composite key is specified, 
Outlier should not proceed. Error messages should be printed to the log file."""

import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_159
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_a02_a(capfd, outlier_indata_02, outlier_indata_hist_01):

    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        method="C",
        mii=3,
        mei=2,
        unit_id="ident area",
        var="V1 V2",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_159
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_a02_b(capfd, outlier_indata_02, outlier_indata_hist_01):

    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        method="H",
        mii=3,
        mei=2,
        unit_id="ident area",
        var="V1 V2",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_159
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_a02_c(capfd, outlier_indata_02, outlier_indata_hist_01):

    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_02,
        indata_hist=outlier_indata_hist_01,
        method="S",
        beta_i=1.5,
        beta_e=0.5,
        unit_id="ident area",
        var="V1 V2",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()