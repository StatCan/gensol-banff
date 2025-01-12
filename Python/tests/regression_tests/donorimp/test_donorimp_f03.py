"""Three character variables must be present in the input status data set.  
If one is not present, Donor should not proceed.  
Error messages should be printed to the log.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f03_a(capfd, donorimp_indata_01, donorimp_instatus_02):
    """Instatus' ident is missing
    """
    donorimp_instatus_02.rename(columns={'ident':'iden'}, inplace = True)

    banfftest.pytest_donorimp(
        indata=donorimp_indata_01,
        instatus=donorimp_instatus_02,
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

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'ident' in list 'unit_id' (instatus dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f03_b(capfd, donorimp_indata_01, donorimp_instatus_02):
    """Instatus' fieldid is missing
    """
    donorimp_instatus_02.rename(columns={'fieldid':'field'}, inplace = True)

    banfftest.pytest_donorimp(
        indata=donorimp_indata_01,
        instatus=donorimp_instatus_02,
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

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'FIELDID' in list 'instatus variables' (instatus dataset) not found."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_f03_c(capfd, donorimp_indata_01, donorimp_instatus_02):
    """Instatus' status is missing
    """
    donorimp_instatus_02.rename(columns={'status':'stat'}, inplace = True)

    banfftest.pytest_donorimp(
        indata=donorimp_indata_01,
        instatus=donorimp_instatus_02,
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

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'STATUS' in list 'instatus variables' (instatus dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()