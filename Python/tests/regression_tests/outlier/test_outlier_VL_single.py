"""The `VL_single` testcases check that certain variable lists disallow specification of 
more than one variable.  The validation only checks if MORE variables were specified than allowed.  
This previously automatic SAS validation is newly implemented in C code following SAS' removal.  """

import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_outlier_VL_single__unit_id(capfd, outlier_indata_01, outlier_indata_hist_01):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_01,
        indata_hist=outlier_indata_hist_01,
        method="R",
        mii=3,
        mei=2,
        outlier_stats=True,
        unit_id="ident SOME_CHAR",
        var="V1 V2",
        weight="weight",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_outlier_VL_single__weight(capfd, outlier_indata_01, outlier_indata_hist_01):
    """`weight` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_outlier(
        # Procedure Parameters
        indata=outlier_indata_01,
        indata_hist=outlier_indata_hist_01,
        method="R",
        mii=3,
        mei=2,
        outlier_stats=True,
        unit_id="ident",
        var="V1 V2",
        weight="weight SOME_CHAR",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'weight' accepts at most 1 numeric variable."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()