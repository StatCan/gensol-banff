import banff.testing as banfftest
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donor_IT08(capfd, JA_indata22, JA_instat19):
    """When a non-mandatory varlist is specified as an empty string, no fatal runtime error should occur
        In this case, donorimp should run to completion without error
        Passing "" for a varlist is treated as not specifying the varlist
        """
    
    banfftest.pytest_donorimp(
        # Banff Parameters
        indata=JA_indata22,
        instatus=JA_instat19,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True, 
        eligdon="any",
        unit_id="IDENT",
        must_match="TOTAL STAFF",
        data_excl_var= "",
        rand_num_var=""
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )


import sys
if __name__ == "__main__":
    banfftest.run_pytest()