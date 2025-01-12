import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_prorate_a03_a(capfd, prorate_indata_03, prorate_instatus_02):
    """The key variable must be a character variable.  
    When it is numeric in the input status data file, Prorate should not proceed.  
    Error messages should be printed in the log.
    
    NOTE: 
        this test differs from prorateA02 in that this one has a character 'ident' variable 
        on its *indata* dataset but numeric 'ident' on *instatus*.
        A message specific to the instatus dataset should appear
    """
    print(prorate_indata_03.to_json(orient='split'))

    banfftest.pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_03,
        instatus=prorate_instatus_02,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="IMPUTED",
        unit_id="ident",

        # Unit Test Parameters
        sas_log_name="prorate_a03.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ident' in list 'unit_id' (instatus dataset) must be character, but it is numeric."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()