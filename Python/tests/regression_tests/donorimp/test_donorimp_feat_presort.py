"""Proc donorimputation requires its `indata` and `instatus` files to be sorted
in ascending order of `by` variables.  
- when `instatus` has no `by` variables it can be unsorted

When `presort=True`, unsorted files will be sorted automatically
- with by vars
- without by vars
- with a mix of by vars and no by vars"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_a(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Catch unsorted indata when instatus sorted.  
        BY vars: both files
    """
    # unsort indata
    donorimp_indata_presort.sort_values(by=['Q3', 'area', 'staff'], inplace=True)

    # sort instatus
    donorimp_instatus_presort.sort_values(by=['area', 'staff'], inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=1,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has Area = A2 staff = 50.000000 and the next by group has Area = A1 staff = 100.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_b(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Catch unsorted instatus when indata sorted.  
        BY vars: both files
    """
    # sort indata
    donorimp_indata_presort.sort_values(by=['area', 'staff'], inplace=True)

    # unsort instatus
    donorimp_instatus_presort.sort_values(by='fieldid', inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=False,
        prefill_by_vars=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=1,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set instatus is not sorted in ascending sequence. The current by group has Area = A1 staff = 100.000000 and the next by group has Area = A1 staff = 50.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_c(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Catch that neither instatus or indata are sorted.  
        BY vars: both files
    """
    # unsort indata
    donorimp_indata_presort.sort_values(by=['Q3', 'area', 'staff'], inplace=True)

    # unsort instatus
    donorimp_instatus_presort.sort_values(by='fieldid', inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=1,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has Area = A2 staff = 50.000000 and the next by group has Area = A1 staff = 100.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_d(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Allow unsorted instatus when indata sorted.  
        BY vars: indata only
    """
    # sort indata
    donorimp_indata_presort.sort_values(by=['area', 'staff'], inplace=True)

    # unsort instatus
    donorimp_instatus_presort = donorimp_instatus_presort[['ident', 'fieldid', 'status']]

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=False,
        prefill_by_vars=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=5,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_e(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: both files
    """
    # unsort indata
    donorimp_indata_presort.sort_values(by='Q3', inplace=True)

    # unsort instatus
    donorimp_instatus_presort.sort_values(by='fieldid', inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_f(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: not used
    """
    # unsort indata
    donorimp_indata_presort.sort_values(by='Q3', inplace=True)

    # unsort instatus
    donorimp_instatus_presort.sort_values(by='fieldid', inplace=True)

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        presort=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_presort_g(capfd, donorimp_indata_presort, donorimp_instatus_presort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: both files
    """
    # unsort indata
    donorimp_indata_presort.sort_values(by='Q3', inplace=True)

    # remove BY vars from instatus
    donorimp_instatus_presort = donorimp_instatus_presort[['ident', 'fieldid', 'status']]

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_presort,
        instatus=donorimp_instatus_presort,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="ident",
        data_excl_var="toexcl",
        by="Area staff",
        presort=True,
        prefill_by_vars=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=5,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()