# If DATA is omitted, the most recently created SAS data set should be used.
#import pytest
#from banff.testing import assert_log_consistent
#import os
#import banff

## DISABLED
## this checks that if you omit the indata parameter that it somehow uses magic
## to select an input dataset for you.  This SAS logic will not be implemented

#
#@pytest.mark.m_validated
#@pytest.mark.m_missing_feature # Probably won't be implementing this
#def test_donorE02(capfd, indataCharKey, instatCharKey):
#    #### arrange
#    for i, row in indataCharKey.iterrows():
#        val = "."
#        if indataCharKey.at[i,'total'] > 1000:
#            val = "E"
#        indataCharKey.at[i,'toexcl'] = val
#
#    #### act
#    try:
#        #Create Banff call
#        donorimputation_e02= banff.donorimp(
#            indata=indataCharKey,
#            instatus=instatCharKey,
#            outdata=None,
#            outstatus=None,
#            outdonormap=None,
#            edits="Q1 + Q2 + Q3 + Q4 = total;",
#            post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
#            min_donors=1,
#            percent_donors=1,
#            n=3,
#            outmatching_fields=True,
#            eligdon="any",
#            unit_id="ident",
#            must_match="STAFF",
#            data_excl_var="toexcl"
#        )
#
#        rc = donorimputation_e02.rc
#    except Exception as e:
#        print(e)
#
#    #### assert
#    out_act, err = capfd.readouterr()
#    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorE02_sas.txt"
#    
#    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR: Variable TOTAL specified in edits is not in indata data set")
#    assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"
#
#    out_assert, err2 = capfd.readouterr()
#    print(out_act+out_assert)