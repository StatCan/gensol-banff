import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_c01_a(capfd, donorimp_indata_01, donorimp_instatus_02):
    """If the data_excl_var statement is present but the variable specified is not in the 
    input data set, DonorImp should not proceed.  Error messages should be printed to the log.
    """
    banfftest.pytest_donorimp(
        # Banff Parameters
        indata=donorimp_indata_01,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="EXCL",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'EXCL' in list 'data_excl_var' (indata dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()