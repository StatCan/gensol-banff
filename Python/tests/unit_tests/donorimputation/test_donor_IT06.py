from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donor_IT06(capfd, JA_indata22, JA_instat19):
    """When a proper subset of variables listed in `must_match` also appear in `edits`
        1) no error should occur
        2) the `must_match` variables NOT listed in `edits` should
          remain on the re-initialized `must_match` variable list.  
        This test ONLY validates point #1 above.  Use a live debugger to validate #2 
            - reinitialization occurs just after the call to `CleanMustMatchVariables`

        In this case, `TOTAL`, listed in `edits`, should be removed from `must_match`, 
        `STAFF` should remain on the `must_match` list
        """
    
    pytest_donorimp(
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
        data_excl_var= "toexcl"
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )

