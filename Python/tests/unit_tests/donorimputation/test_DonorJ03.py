# The edits must be enclosed in quotes.  If not, Donor should not proceed.
# Error messages should be printed to the log file. 

## DISABLED
## this tests that "edits must be inclosed in quotes".  It's essentially a unit test 
## for SAS' (Python's) interpreter, which we don't need to implement.  
## - use quotes when you need to use quotes

#import pytest
#from banff.testing import assert_log_consistent
#import os
#import banff
#
#@pytest.mark.m_validated
#def test_donorJ03(capfd, indataCharKey, instatCharKey):
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
#        donorimputation_j03= banff.donorimp(
#            indata=indataCharKey,
#            instatus=instatCharKey,
#            outdata=None,
#            outstatus=None,
#            outdonormap=None,
#            edits="Q1 + Q2 + Q3 + Q4 = total;", #Can't not quote in python
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
#        rc = donorimputation_j03.rc
#    except Exception as e:
#        print(e)
#
#    #### assert
#    out_act, err = capfd.readouterr()
#    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorJ03_sas.txt"
#    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR 22-322: Expecting a quoted string")
#    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR 76-322: Syntax error, statement will be ignored")
#    assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"