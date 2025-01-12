"""If the NOTSORTED option is not used with the BY statement, the current data set must be
 sorted by the BY variable(s) first.  If not, Prorate should not proceed.  
 Error messages should be printed to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_prorate_o02_a(capfd, prorate_indata_05, prorate_instatus_04):
    """ the 'notsorted' option is NOT specified
    """
    banfftest.pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_05,
        instatus=prorate_instatus_04,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",
        method="s",
        unit_id="ident",
        by="prov area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has prov = 2.000000 area = B and the next by group has prov = 2.000000 area = A."
    )


@pytest.mark.m_fail_as_is
@pytest.mark.m_missing_feature # "notsorted" option for BY variables not implemented 
def test_prorate_o02_b(capfd, prorate_indata_05, prorate_instatus_04):
    banfftest.pytest_prorate(
        # Procedure Parameters
        indata=prorate_indata_05,
        instatus=prorate_instatus_04,
        edits="VA + VB+ VC + VD = TOT1;",
        decimal=0,
        lower_bound=0,
        upper_bound=999,
        modifier="A",
        method="s",
        unit_id="ident",
        by="prov area notsorted",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()