import pytest
import banff.testing as banfftest

import pandas as pd
@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_r04_a(capfd, donorimp_indata_08, donorimp_instatus_02, donorimp_outdonormap_04):
    """To verify that the output is the same for the same value of seed.
    """
    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_08,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        seed=371,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outdonormap=donorimp_outdonormap_04,
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()