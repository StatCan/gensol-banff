"""The `VL_single` testcases check that certain variable lists disallow specification of 
more than one variable.  The validation only checks if MORE variables were specified than allowed.  
This previously automatic SAS validation is newly implemented in C code following SAS' removal.  """

import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_donorimp_VL_single__unit_id(capfd, donorimp_indata_12, donorimp_instatus_12):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_12,
        instatus=donorimp_instatus_12,
        unit_id="REC SOME_CHAR", 
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        n_limit=1,
        outmatching_fields=True,
        rand_num_var="randkey",
        data_excl_var="EXCL_VAR",
        
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
def test_donorimp_VL_single__rand_num_var(capfd, donorimp_indata_12, donorimp_instatus_12):
    """`rand_num_var` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_12,
        instatus=donorimp_instatus_12,
        unit_id="REC", 
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        n_limit=1,
        outmatching_fields=True,
        rand_num_var="randkey SOME_CHAR",
        data_excl_var="EXCL_VAR",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'rand_num_var' accepts at most 1 numeric variable."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_donorimp_VL_single__data_excl_var(capfd, donorimp_indata_12, donorimp_instatus_12):
    """`data_excl_var` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_12,
        instatus=donorimp_instatus_12,
        unit_id="REC", 
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        n_limit=1,
        outmatching_fields=True,
        rand_num_var="randkey",
        data_excl_var="EXCL_VAR SOME_CHAR",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=None, # disable: # of errors depends on number of datasets associated with varlist
        msg_list_contains_exact="ERROR: List 'data_excl_var' accepts at most 1 character variable."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()