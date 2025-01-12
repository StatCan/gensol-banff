"""To verify the correctness of output from Editstats"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_c02_a(capfd, editstat_indata_02, 
                        editstat_outedits_reduced_01,
                        editstat_outedit_status_01,
                        editstat_outk_edits_status_01,
                        editstat_outglobal_status_01,
                        editstat_outedit_applic_01,
                        editstat_outvars_role_01,
                        ):
    """With accept_negative=False"""
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_02,
        edits="""
            A + B + C + D = TOT1; A + E <= 20;
            B + C + F + H <= 50; E + F + G + H = TOT2;
            TOT1 + TOT2 = TOT; D >= 1; G <= 30;
        """,
        accept_negative=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outedits_reduced=editstat_outedits_reduced_01,
        expected_outedit_status=editstat_outedit_status_01,
        expected_outk_edits_status=editstat_outk_edits_status_01,
        expected_outglobal_status=editstat_outglobal_status_01,
        expected_outedit_applic=editstat_outedit_applic_01,
        expected_outvars_role=editstat_outvars_role_01,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_c02_b(capfd, editstat_indata_02, 
                        editstat_outedits_reduced_01,
                        editstat_outedit_status_01,
                        editstat_outk_edits_status_01,
                        editstat_outglobal_status_01,
                        editstat_outedit_applic_01,
                        editstat_outvars_role_01,
                        ):
    """Without accept_negative=False (results should be identical to accept_negative=False)"""
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_02,
        edits="""
            A + B + C + D = TOT1; A + E <= 20;
            B + C + F + H <= 50; E + F + G + H = TOT2;
            TOT1 + TOT2 = TOT; D >= 1; G <= 30;
        """,
        #accept_negative=False,  # intentionally commented out

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outedits_reduced=editstat_outedits_reduced_01,
        expected_outedit_status=editstat_outedit_status_01,
        expected_outk_edits_status=editstat_outk_edits_status_01,
        expected_outglobal_status=editstat_outglobal_status_01,
        expected_outedit_applic=editstat_outedit_applic_01,
        expected_outvars_role=editstat_outvars_role_01,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()