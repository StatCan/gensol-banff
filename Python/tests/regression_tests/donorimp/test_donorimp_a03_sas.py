import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_donorimp_a03_a_sas(capfd):
    """The key variable is mandatory. If not specified, DonorImp should not proceed.
    Error messages should be printed to the log file.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata="donorimp_a03_a_indata.sas7bdat",
        instatus="donorimp_a03_a_instatus.sas7bdat",
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        # unit_id="ident", # intentionally commented out
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: unit_id is mandatory."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()