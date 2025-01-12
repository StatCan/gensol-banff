"""To ensure that the correct implied edits are printed out
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_b05_a(capfd):
    banfftest.pytest_verifyed(
        edits="X >= Y; X - 2*Y <= 10; X + Y <= 20;",
        imply=10,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains=[
            """Implied edits found:
                - X  <= 0
                    X  <= 16.66667
                    Y  <= 10""",
            """
            Submitted maximum number of implied edits ...................:  10
            Number of implied edits generated ...........................:  3
                Number of implied equalities ............................:  0
                Number of implied inequalities ..........................:  3""",
        ]
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_verifyed_b05_b(capfd):
    banfftest.pytest_verifyed(
        edits="X >= Y; X - 2*Y <= 10; X + Y <= 20;",
        imply=10,
        accept_negative=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        msg_list_contains=[
            """Implied edits found:
                - X  <= 10
                    X  <= 16.66667
                - Y  <= 10
                    Y  <= 10""",
            """
            Submitted maximum number of implied edits ...................:  10
            Number of implied edits generated ...........................:  4
                Number of implied equalities ............................:  0
                Number of implied inequalities ..........................:  4
""",
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()