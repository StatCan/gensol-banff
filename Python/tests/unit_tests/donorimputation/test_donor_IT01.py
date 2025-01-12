## Purpose: Test case should run without error and sumulates user giving 
##              - only MISSING values for numeric variable
##              - only '.' values for character variable
## Comment: This is based on DonorT14
## Date written: 2023-April-27
## Written by: Stephen Arsenault
import pytest
from banff.testing import assert_log_consistent
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_bug # numeric variable with all missing values causes fatal error for Banff <= 3.01.001a2.dev3
@pytest.mark.m_jira_69
def test_donor_IT01(capfd, indata_Donor_IT01, instatus_DonorT14):
    #### arrange
    indata=indata_Donor_IT01
    instatus=instatus_DonorT14
    
    #### act
    #Create Banff call
    donor_call= banff.donorimp(
        indata=indata,
        instatus=instatus,
        outdata=None,
        outstatus=None,
        outdonormap=None,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        accept_negative=True,
        seed=1,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
    )

    rc = donor_call.rc

    #### assert
    out_act, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donor_IT01_sas.txt"

    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="WARNING: There were 10 observations dropped from indata data set because one/more edits variables or one/more must_match variables outside the edits group are missing in indata data set and there is no corresponding FTI in instatus data set.")
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="Number of observations dropped ....................:      10")
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="Number of valid observations ......................:       0     100.00%")
    assert rc==0, f"Banff returned non-zero value: {rc}"
    
    # print all captured output
    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)