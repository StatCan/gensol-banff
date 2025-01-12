import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_q01_a(capfd, donorimp_indata_02, donorimp_instatus_10, donorimp_outstatus_01, donorimp_outmatching_fields_01):
    """When MATCHFIELDSTAT is specified, the matching fields should be 
    added to the output status data set with statuses MFS, MFU, or MFB. 
    """

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_10,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_outstatus=donorimp_outstatus_01,
        expected_outmatching_fields=donorimp_outmatching_fields_01,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()