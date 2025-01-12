import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_donorimp_t14_a_sas(capfd, donorimp_indata_11, donorimp_instatus_11, donorimp_outdonormap_05):
    """To verify that variables with IDE status in the input data set are considered original/not imputed.
    """

    banff_call = banfftest.pytest_donorimp(
        # Procedure Parameters
        indata="donorimp_t14_a_indata.sas7bdat",
        instatus="donorimp_t14_a_instatus.sas7bdat",
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        accept_negative=True,
        seed=1,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        expected_outdata="donorimp_t14_a_outdata.sas7bdat",
        expected_outstatus="donorimp_t14_a_outstatus.sas7bdat",
        expected_outdonormap="donorimp_t14_a_donormap.sas7bdat",
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()