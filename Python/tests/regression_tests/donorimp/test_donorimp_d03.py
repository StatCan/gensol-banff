import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_modified # expected outdata: BY variables removed, use "REC04" - .docx is wrong
def test_donorimp_d03_a(capfd, donorimp_indata_04, donorimp_instatus_02, donorimp_outdata_02):
    """To verify that more than one BY variable can be used.
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_04,
        instatus=donorimp_instatus_02,
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
        expected_warning_count=2,
        expected_outdata=donorimp_outdata_02
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()