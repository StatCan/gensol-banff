"""Three character variables must be present in the input status data set.  
If one is not a character variable, Donor should not proceed.
Error messages should be printed to the log.
"""

import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f04_a(capfd, donorimp_indata_02, donorimp_instatus_13):
    """ IDENT not character
    """
    banfftest.pytest_donorimp(
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_13,
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
        unit_id="ident",
        must_match="STAFF",
        data_excl_var="toexcl",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ident' in list 'unit_id' (instatus dataset) must be character, but it is numeric."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f04_b(capfd, donorimp_indata_02, donorimp_instatus_14):
    """ FIELDID not character
    """
    banfftest.pytest_donorimp(
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_14,
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
        unit_id="ident",
        must_match="STAFF",
        data_excl_var="toexcl",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'FIELDID' in list 'instatus variables' (instatus dataset) must be character, but it is numeric."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_154
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f04_c(capfd, donorimp_indata_02, donorimp_instatus_15):
    """ STATUS not character
    """
    #Create Banff call
    banfftest.pytest_donorimp(
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_15,
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
        unit_id="ident",
        must_match="STAFF",
        data_excl_var="toexcl",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'STATUS' in list 'instatus variables' (instatus dataset) must be character, but it is numeric."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()