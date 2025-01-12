import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
@pytest.mark.m_modified # expected outdata: BY variables removed, use "REC04" - .docx is wrong
def test_donorimp_d03_a_SAS(capfd):
    """To verify that more than one BY variable can be used.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata="donorimp_d03_a_indata.sas7bdat",
        instatus="donorimp_d03_a_instatus.sas7bdat",
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="area staff"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=4,
        expected_outdata="donorimp_d03_a_outdata.sas7bdat",
        expected_outstatus="donorimp_d03_a_outstatus.sas7bdat",
        expected_outdonormap="donorimp_d03_a_donormap.sas7bdat"
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()