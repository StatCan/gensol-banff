from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorT10(capfd, JA_indata22, JA_instat19):
    
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata22,
        instatus=JA_instat19,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True, 
        eligdon="any",
        unit_id="IDENT",
        must_match="TOTAL",
        data_excl_var= "IDENT",
        by = "IDENT"
        ,

        # Unit Test Parameters
        sas_log_name="donorT10_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=3,
        msg_list_sas=[
            "ERROR: The lists of variables of the unit_id and by statements are mutually exclusive",
            "ERROR: The lists of variables of the unit_id and data_excl_var statements are mutually exclusive",
            "ERROR: The lists of variables of the data_excl_var and by statements are mutually exclusive"
        ]
    )

