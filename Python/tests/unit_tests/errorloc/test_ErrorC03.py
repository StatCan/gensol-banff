import pytest
from banff.testing import pytest_errorloc

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_errorC03(capfd, errorloc_indata_grp_c):
    """When using BY variables, datasets not sorted by the BY variables should trigger
    an error message"""
    #### arrange
    
    pytest_errorloc(
        # Banff Parameters
        indata=errorloc_indata_grp_c,
        edits="x1+x2<=50; x3>=100;",
        seed=1,
        unit_id="REC",
        presort=False,

        # Unit Test Parameters
        sas_log_name="errorC03.log",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        msg_list_sas="data set is not sorted in ascending order. The ",
        expected_error_count=1
    )
