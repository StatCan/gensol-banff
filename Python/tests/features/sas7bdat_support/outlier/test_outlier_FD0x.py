import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_outlier_FD0x_a(capfd):
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata="outlier_fd0x_a_indata.sas7bdat",
        method="C",
        mii=6,
        mei=4,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="V1",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus="outlier_fd0x_a_outstatus.sas7bdat",
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_outlier_FD0x_b(capfd):
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata="outlier_fd0x_b_indata.sas7bdat",
        method="s",
        beta_i=3.0,
        beta_e=1.5,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="V1",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus="outlier_fd0x_b_outstatus.sas7bdat",
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_outlier_FD0x_c(capfd):
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata="outlier_fd0x_c_indata.sas7bdat",
        method="C",
        mii=6,
        mei=3,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="V2",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        expected_outstatus="outlier_fd0x_c_outstatus.sas7bdat",
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_outlier_FD0x_d(capfd):
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata="outlier_fd0x_d_indata.sas7bdat",
        method="s",
        beta_i=3.0,
        beta_e=1.0,
        start_centile=75,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="V2",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        expected_outstatus="outlier_fd0x_d_outstatus.sas7bdat",
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_outlier_FD0x_e(capfd):
    banfftest.pytest_outlier(
        # Procedure Parameters
        indata="outlier_fd0x_e_indata.sas7bdat",
        method="s",
        side="RIGHT",
        beta_i=3.0,
        beta_e=1.5,
        start_centile=80,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="V1",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus="outlier_fd0x_e_outstatus.sas7bdat",
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()