#  # If the specified DATA set does not exist, DonorImp should terminate.
#  # Error messages should be printed to the log file.
#  import pytest
#  from banff.testing import assert_log_consistent
#  import os
#  import banff
#  

### DISABLED: SAS test case DonorE04 doesn't really translate well as we now support multiple input TYPES per input dataset
###             Consider this replaced with new test_donor_IT03

#  @pytest.mark.m_validated
#  @pytest.mark.m_missing_validation # see https://jira.statcan.ca/browse/ITSLMBANFF-45
#  def test_donorE04(capfd, instatCharKey):
#      #### arrange
#  
#      #### act
#      #Create Banff call
#      donorimputation_e04= banff.donorimp(
#          indata="V:/hopefully/this/does/not/does/not/does/not/does/not/does/not/does/not/does/not/does/not/does/not/does/not/does/not/does/not/exist.csv",
#          instatus=instatCharKey,
#          outdata=None,
#          outstatus=None,
#          outdonormap=None,
#          edits="Q1 + Q2 + Q3 + Q4 = total;",
#          post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
#          min_donors=1,
#          percent_donors=1,
#          n=3,
#          outmatching_fields=True,
#          eligdon="any",
#          unit_id="ident"
#          #must_match="STAFF",
#          #data_excl_var="toexcl"
#      )
#  
#      rc = donorimputation_e04.rc
#  
#      #### assert
#      out_act, err = capfd.readouterr()
#      sas_log_path = os.path.dirname(__file__) + "/output-sas/donorE04_sas.txt"
#      
#      assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="ERROR: File WORK.INDAT.DATA does not exist")
#      assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"
#  
#      out_assert, err2 = capfd.readouterr()
#      print(out_act+out_assert)