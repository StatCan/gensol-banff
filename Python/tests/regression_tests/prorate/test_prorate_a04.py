import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_prorate_a04_a(capfd, prorate_indata_04, prorate_instatus_03):
    banfftest.pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_04,
        instatus=prorate_instatus_03,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",
        #unit_id="idnt", # original test case uses lowercase
        unit_id="IDNT",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1, # original test case only produced 1 error, now proc flags indata and instatus missing "idnt"
        msg_list_contains_exact="ERROR: Variable 'IDNT' in list 'unit_id' (indata dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()