#############################################################################################################
## Description: Conversion of SAS test case to Python: donorR01
## Command to run in CMD: python -m pytest test_donorR01.py
## Notes:
## Last Updated: 2023-apr-20
#############################################################################################################
import pytest
from banff.testing import assert_log_consistent
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorR01(capfd, JA_indata7, instatCharKey):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata7
    
    #synthetic input dataset - instatus
    instat = instatCharKey

    #### act
    try:
        #create Banff call
        donorimp_R01 = banff.donorimp(
                indata=indata,
                instatus=instat,
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
                unit_id="IDENT",
                must_match="STAFF",
                data_excl_var= "toexcl"
                )     
        #execute Banff call
        rc = donorimp_R01.rc
    except Exception as e:
        print("The R01 call had an unhandled exception")
        print(e)
        

    #### act
    out_act, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorR01_sas.txt"
    
    # this is pretty good, but we should run multiple times and compare the value chosen by the system, to ensure it's random
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="value chosen by the system")
    assert rc==0, f"Banff returned non-zero value: {rc}"
    
    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)