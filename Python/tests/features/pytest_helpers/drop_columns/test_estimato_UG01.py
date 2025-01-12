# import necessary Python packages
import banff.testing as banfftest

def test_estimato_UG01_a(capfd):
    """Pytest helper functions should automatically drop `by` variables from SOME control datasets 
       before comparing with actual output.  
        - if SAS included BY variables but Python does not
        as implemented for https://jira.statcan.ca/browse/ITSLMBANFF-203
       In the case where BY variables are expected in the actual output dataset, they should NOT
       be dropped from the control dataset.  
       This test case will fail if there is any inconsistency with the listed `expected` datasets.  """
    # create Banff call which will succeed
    banff_test= banfftest.pytest_estimato(
        indata="donorimp_ug01_a_indata.sas7bdat",
        instatus="donorimp_ug01_a_instatus.sas7bdat",
        indata_hist="donorimp_ug01_a_indata_hist.sas7bdat",
        inalgorithm="donorimp_ug01_a_inalgorithm.sas7bdat",
        inestimator="donorimp_ug01_a_inestimator.sas7bdat",
        seed=1,
        accept_negative=True,
        unit_id="ident",
        data_excl_var="CURRDATAEXCL",
        hist_excl_var="HISTDATAEXCL",
        by="prov",
        
        pytest_capture=capfd,
        expected_warning_count=None, # ignore warning count
        expected_outstatus="donorimp_ug01_a_outstatus.sas7bdat", 
        expected_outdata="donorimp_ug01_a_outdata.sas7bdat",
        expected_outrand_err="donorimp_ug01_a_outrand_err.sas7bdat",
        expected_outest_parm="donorimp_ug01_a_outest_parm.sas7bdat",
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()