import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_outlier_IT01(capfd, outlier_indata_b05_v2_char, outlier_expected_outstatus_B05_1, outlier_expected_outstatus_detailed_B05_1):
    """ When a "NAME8" type parameter is specified in lowercase, the Banff procedure
    should "upcase" it, no error should occur if the upcased value is valid
    'current' is invalid, but 'CURRENT' is valid, however 'current' should be 
    upcased and thus ultimately be valid"""
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

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_error_count=0,
        expected_warning_count=0,
        expected_outstatus=outlier_expected_outstatus_B05_1,
        expected_outstatus_detailed=outlier_expected_outstatus_detailed_B05_1,
    )