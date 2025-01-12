import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_156
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_d02_a(capfd, donorimp_indata_02, donorimp_instatus_02):
    """If the NOTSORTED option in the BY statement is not used, then the observations 
    must be sorted by the BY variable(s).  If not, DonorImp should not proceed.  
    Error messages should be printed to the log.
    """
    # change AREA column to break sort order
    donorimp_indata_02.at[1,'area'] = "A2"
    donorimp_indata_02.at[3,'area'] = "A1"
    banfftest.pytest_donorimp(
        indata=donorimp_indata_02,
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
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=1,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = A2 and the next by group has area = A1."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()