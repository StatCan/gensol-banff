"""The `VL_single` testcases check that certain variable lists disallow specification of 
more than one variable.  The validation only checks if MORE variables were specified than allowed.  
This previously automatic SAS validation is newly implemented in C code following SAS' removal.  """

import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_estimato_VL_single__unit_id(capfd, estimato_indata_01, estimato_indata_hist_01, estimato_instatus_01, estimato_instatus_hist_01, estimato_inalgorithm_01, estimato_inestimator_01):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_estimato(
        # Procedure Parameters
        indata=estimato_indata_01,
        indata_hist=estimato_indata_hist_01,
        instatus=estimato_instatus_01,
        instatus_hist=estimato_instatus_hist_01,
        inalgorithm=estimato_inalgorithm_01,
        inestimator=estimato_inestimator_01.query('est_setid=="set2"'),
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident SOME_CHAR",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable check, the number of errors here matches number of datasets using `unit_id`
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_estimato_VL_single__data_excl_var(capfd, estimato_indata_01, estimato_indata_hist_01, estimato_instatus_01, estimato_instatus_hist_01, estimato_inalgorithm_01, estimato_inestimator_01):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_estimato(
        # Procedure Parameters
        indata=estimato_indata_01,
        indata_hist=estimato_indata_hist_01,
        instatus=estimato_instatus_01,
        instatus_hist=estimato_instatus_hist_01,
        inalgorithm=estimato_inalgorithm_01,
        inestimator=estimato_inestimator_01.query('est_setid=="set2"'),
        data_excl_var="CURRDATAEXCL SOME_CHAR",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        unit_id="ident",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: List 'data_excl_var' accepts at most 1 character variable."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_estimato_VL_single__hist_excl_var(capfd, estimato_indata_01, estimato_indata_hist_01, estimato_instatus_01, estimato_instatus_hist_01, estimato_inalgorithm_01, estimato_inestimator_01):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_estimato(
        # Procedure Parameters
        indata=estimato_indata_01,
        indata_hist=estimato_indata_hist_01,
        instatus=estimato_instatus_01,
        instatus_hist=estimato_instatus_hist_01,
        inalgorithm=estimato_inalgorithm_01,
        inestimator=estimato_inestimator_01.query('est_setid=="set2"'),
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL SOME_CHAR",
        by="prov",
        unit_id="ident",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: List 'hist_excl_var' accepts at most 1 character variable."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()