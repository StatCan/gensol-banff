"""Various tests of rand_num_var
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_155
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_v01_a(capfd, donorimp_indata_14, donorimp_instatus_16):
    """RANDMISS does not exist
    """

    banfftest.pytest_donorimp(
        # Banff Parameters
        indata=donorimp_indata_14,
        instatus=donorimp_instatus_16,
        edits="x=y;",
        post_edits="x<=y;",
        min_donors=1,
        percent_donors=1,
        n=1,
        display_level=1,
        unit_id="REC",
        rand_num_var="RANDMISS",

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Variable 'RANDMISS' in list 'rand_num_var' (indata dataset) not found."
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()