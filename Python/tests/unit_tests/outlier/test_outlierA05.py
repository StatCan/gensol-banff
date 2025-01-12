import pytest
from banff.testing import pytest_outlier

@pytest.mark.m_validated
@pytest.mark.m_missing_feature # AIX|HIST aliases ignored, HIST used in all cases
def test_outlierA05_a(capfd, outlier_indata_01, outlier_inhist_01):
    #### arrange
    # change 'ident' to 'id', causing mismatching key variable
    outlier_indata_01.rename(columns={"ident":"id"},inplace=True)

    #### act
    pytest_outlier(
        # Outlier Parameters
        indata=outlier_indata_01,
        indata_hist=outlier_inhist_01,
        outlier_stats=True,
        method="R",
        mii=3,
        mei=2,
        unit_id="ident",
        var="V1 V2",

        # Unit Test Parameters
        sas_log_name="outlierA05_a.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: Variable ID is not in AUX data set."
    )