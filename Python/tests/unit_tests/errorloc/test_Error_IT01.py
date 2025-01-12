## Purpose: Test case should run without error and sumulates user giving 
##              - only MISSING values for numeric variable
##              - some blank '' values for character variables
## Comment: This is based on ErrorK02
## Date written: 2023-April-28
## Written by: Stephen Arsenault
import pytest
from banff.testing import assert_log_consistent
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_bug # numeric variable with all missing values causes fatal error for Banff <= 3.01.001a2.dev3
@pytest.mark.m_jira_69
def test_error_IT01(capfd, errorloc_indata_IT01):
    #### arrange
    indata=errorloc_indata_IT01
    indata[['IDENT']]=indata[['IDENT']].fillna('')
    
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

    sas_log_path = os.path.dirname(__file__) + "/output-sas/errorK02.log" # refers to errorK02 on purpose
    
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="Number of observations dropped because the key is missing ..:       2      33.33%")
    assert rc==0, f"Banff returned non-zero value: {rc}"
   
    out_assert, err2 = capfd.readouterr()
    print(out_act)
    print(out_assert)