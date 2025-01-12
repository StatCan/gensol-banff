"""When the edits are enclosed in quotes, Editstats should proceed.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_b06_a(capfd, editstat_indata_01):
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_01,
        edits='x1+1>=x2; x1<=5; x2>=x3; x1+x2+x3<=9;',

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_editstat_b06_b(capfd, editstat_indata_01):
    banfftest.pytest_editstat(
        # Procedure Parameters
        indata=editstat_indata_01,
        edits="x1+1>=x2; x1<=5; x2>=x3; x1+x2+x3<=9;",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()