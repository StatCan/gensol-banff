## Purpose: Test case should terminate early due to mixed datatype in column
##              - 'IDENT' column is character, but has a non-character value
## Comment: In SAS mixing datatypes is impossible, so this test is unique to Python
##          - based on Error_IT01
## Comment: This test fails for Banff <= 3.01.001a2.dev3
## Date written: 2023-April-28
## Written by: Stephen Arsenault
import pytest
from banff.testing import assert_log_contains
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_for_review # "missing" values in Character columns now supported... so this test case no longer relevant
@pytest.mark.m_bug # numeric variable with all missing values causes fatal error for Banff <= 3.01.001a2.dev3
@pytest.mark.m_jira_69
def test_error_IT02(capfd, errorloc_indata_IT01):
    #### arrange
    indata=errorloc_indata_IT01
    
    #### act
    #Create Banff call
    errorloc_call= banff.errorloc(
        indata=indata,
        outstatus=None,
        outreject=None,
        edits="x1>=5; x1<=10; x2>=-1; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=135; x1+x2+x3<=210;",
        accept_negative=False,
        seed=1,
        unit_id="IDENT"
    )

    rc = errorloc_call.rc

    #### assert
    out_act, err = capfd.readouterr()
    print(out_act)
    sas_log_path = os.path.dirname(__file__) + "/output-sas/errorK02.log" # refers to errorK02 on purpose
    
    assert_log_contains(test_log=out_act, msg="ERROR: invalid data detected while reading character variable 'IDENT' on row 1")
    assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"
   
    out_assert, err2 = capfd.readouterr()
    print(out_act)
    print(out_assert)

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()