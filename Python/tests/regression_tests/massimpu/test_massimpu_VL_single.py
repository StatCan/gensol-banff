"""The `VL_single` testcases check that certain variable lists disallow specification of 
more than one variable.  The validation only checks if MORE variables were specified than allowed.  
This previously automatic SAS validation is newly implemented in C code following SAS' removal.  """

import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_new
@pytest.mark.j_ITSLMBANFF_159
def test_massimpu_VL_single__unit_id(capfd, massimpu_indata_02):
    """`unit_id` should allow at most one variable.  
    An error message should be printed and the procedure should 
    terminate early with a non-zero return code.  
    """

    banfftest.pytest_massimpu(
        # Procedure Parameters
        indata=massimpu_indata_02,
        min_donors=2,
        percent_donors=10,
        unit_id="ident SOME_CHAR",
        must_impute="Q1 Q2 Q3 Q4",
        must_match="total staff",
        
        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: List 'unit_id' accepts at most 1 character variable."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()