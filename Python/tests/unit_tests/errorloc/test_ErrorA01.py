import pytest
from banff.testing import assert_log_consistent, pytest_errorloc
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_missing_validation
def test_errorA01_b(capfd, errorloc_indata_grp_a):
    #### arrange
    
    #### act
    #Create Banff call
    errorloc_call= banff.errorloc(
        indata=errorloc_indata_grp_a,
        outstatus=None,
        outreject=None,
        edits="x1+x2<=50; x3>=100;",
        seed=1
    )

    rc = errorloc_call.rc

    #### assert
    out_act, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/errorA01.log"
    
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR 22-322: Expecting a name.")
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR: Variable NAME not found.")
    assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act)
    print(out_assert)