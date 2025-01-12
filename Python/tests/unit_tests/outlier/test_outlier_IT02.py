import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_outlier_IT02(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_1):
    """When a non-mandatory varlist is specified as an empty string, no fatal runtime error should occur
        In this case, errorloc differs from other procedures
         should run to completion without error
        Passing "" for a varlist is NOT ALLOWED, so a specific error message should appear
        """
    #### arrange
    
    #### act
    #Create Banff call
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_b05_v2_char,
        method="current",
        mii=3,
        mei=2,
        accept_negative=True,
        unit_id="ident",
        var="",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=0,
        msg_list_contains_exact="ERROR: At least one variable must be specified with a var statement."
    )