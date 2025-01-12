"""percent_donors must be a real number in the range [1,100).  If not, DonorImp should not continue.  
Error messages should be written to the log file.
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_m02_a(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `percent_donors=0.5`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        #min_donors=1, # intentionally commented out
        percent_donors=0.5,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="ERROR: Invalid percent_donors. percent_donors must be greater than or equal to 1 and less than 100."
    )

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_m02_b(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `percent_donors=-5`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        #min_donors=1, # intentionally commented out
        percent_donors=-5,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="ERROR: Invalid percent_donors. percent_donors must be greater than or equal to 1 and less than 100."
    )

@pytest.mark.m_fail_as_is
@pytest.mark.m_modified
@pytest.mark.j_ITSLMBANFF_157
@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_m02_c(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `percent_donors="7%"`"""
    try:
        banfftest.pytest_donorimp(
            # Procedure Parameters
            indata=donorimp_indata_02,
            instatus=donorimp_instatus_02,
            edits="Q1 + Q2 + Q3 + Q4 = total;",
            post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
            #min_donors=1, # intentionally commented out
            percent_donors="7%",
            n=3,
            outmatching_fields=True,
            eligdon="any",
            unit_id="ident",
            must_match="staff",
            data_excl_var="toexcl"
            ,

            # Test Parameters
            pytest_capture=capfd,
            rc_should_be_zero=False,
            expected_error_count=1,
            msg_list_contains='''ERROR: parameter 'percent_donors' incorrect type, expecting a REAL numeric constant, but received
        string: "7%"'''
        )
    except TypeError:
        assert True, "received expected `TypeError`"
    else:
        assert False, "did not receive expected `TypeError`"

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_m02_d(capfd, donorimp_indata_02, donorimp_instatus_02):
    """invalid `percent_donors=100`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        #min_donors=1, # intentionally commented out
        percent_donors=100,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains="ERROR: Invalid percent_donors. percent_donors must be greater than or equal to 1 and less than 100."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
@pytest.mark.m_modified # outdated spec, used warning message from Banff 2.8.1
def test_donorimp_m02_e(capfd, donorimp_indata_02, donorimp_instatus_02):
    """valid `percent_donors=99.5`"""

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_02,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        #min_donors=1, # intentionally commented out
        percent_donors=99.5,
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
        expected_warning_count=1,
        msg_list_contains=[
            "WARNING: There are not enough donors in the group processed. No imputation will be done.",
            "Number of donors ..................................:       2      66.67%",
            "Number of recipients ..............................:       1      33.33%",
            "    not imputed (insufficient number of donors) ...:       1      33.33%"
        ]
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()