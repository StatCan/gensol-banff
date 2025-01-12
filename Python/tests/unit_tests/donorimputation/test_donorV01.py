from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorV01_b(capfd, JA_indata39, JA_instat40):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata39,
        instatus=JA_instat40,
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        unit_id="REC",
        rand_num_var="randkey"
        ,

        # Unit Test Parameters
        sas_log_name="donorV01b_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas=[
            #NOTE: the messages below split into 2 since redesigned code prints
            # more decimal places than SAS code did (i.e. "...record -0.0100000...")
            "ERROR: DONOR: The random number from the recipient record -0.01",
            "is less than 0. Please fix the random number and rerun the procedure."
        ]
    )
    

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorV01_c(capfd, JA_instat40, JA_indata45):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata45,
        instatus=JA_instat40,
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        unit_id="REC",
        rand_num_var="randkey"
        ,

        # Unit Test Parameters
        sas_log_name="donorV01c_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas=[
            #NOTE: the messages below split into 2 since redesigned code prints
            # more decimal places than SAS code did (i.e. "...record -0.0100000...")
            "ERROR: DONOR: The random number from the recipient record 1.01",
            "is greater than 1. Please fix the random number and rerun the procedure."
        ]
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorV01_d(capfd, JA_instat40, JA_indata46):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata46,
        instatus=JA_instat40,
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        unit_id="REC",
        rand_num_var="randkey"
        ,

        # Unit Test Parameters
        sas_log_name="donorV01d_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: The variable specified in rand_num_var contains one or more missing values. Please verify the contents of the rand_num_var variable."
    )