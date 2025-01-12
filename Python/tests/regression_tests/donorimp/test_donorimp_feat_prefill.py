"""Proc donorimputation's instatus file can optionally include `by` variables
when `by` is specified in the procedure call.  

When `prefill_by_vars=True`, instatus will have `by` variables added from `indata`"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_prefill_a(capfd, donorimp_indata_prefill, donorimp_instatus_prefill):
    """
    Expect warnings about missing BY variables, note regarding performance.  
    by variables on instatus: None
    prefill_by_vars=False
    """
    # sort indata
    donorimp_indata_prefill.sort_values(by=['aRea', 'staff'], inplace=True)

    # drop by variables from instatus
    donorimp_instatus_prefill = donorimp_instatus_prefill[['iDent', 'fieldid', 'status']]

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_prefill,
        instatus=donorimp_instatus_prefill,
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
        prefill_by_vars=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=5,
        expected_error_count=0,
        msg_list_contains_exact=[
            "NOTE: This procedure can perform faster if the instatus data set contains the by variables.",
            "WARNING: BY variable Area is not on input data set instatus.",
            "WARNING: BY variable staff is not on input data set instatus.",
        ],
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_prefill_b(capfd, donorimp_indata_prefill, donorimp_instatus_prefill):
    """
    When `by` variables are present on instatus, NOTE regarding performance should NOT be found.  
    The way this is implemented is hacky.  An option should be added to `assert_helper` which
    validates that messages do not appear in the log.  

    prefill_by_vars=False
    """
    # sort indata
    donorimp_indata_prefill.sort_values(by=['aRea', 'staff'], inplace=True)

    # drop by variables from instatus
    donorimp_instatus_prefill.sort_values(by=['aRea', 'staff'], inplace=True)

    # try:except: is a hacky way to ensure message is NOT on the log
    try:
        banfftest.pytest_donorimp(
            # Procedure Parameters
            indata=donorimp_indata_prefill,
            instatus=donorimp_instatus_prefill,
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
            prefill_by_vars=False,

            # Test Parameters
            pytest_capture=capfd,
            rc_should_be_zero=True,
            expected_warning_count=5,
            expected_error_count=0,
            msg_list_contains_exact=[
                "NOTE: This procedure can perform faster if the instatus data set contains the by variables.",
            ],
        )
    except AssertionError as e:
        expected_exception_string = "could not find 'NOTE: This procedure can perform faster if the instatus data set contains the by variables.'"
        assert expected_exception_string in e.args[0]
    else:
        raise AssertionError("message appeared in log when it shouldn't")

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_donorimp_prefill_c(capfd, donorimp_indata_prefill, donorimp_instatus_prefill):
    """
    When `prefill_by_vars=True`, there should be no warnings relating to missing `by` variables, 
    and no note regarding performance.  

    prefill_by_vars=True
    """
    # sort indata
    donorimp_indata_prefill.sort_values(by=['aRea', 'staff'], inplace=True)

    # drop by variables from instatus
    donorimp_instatus_prefill = donorimp_instatus_prefill[['iDent', 'fieldid', 'status']]

    banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_prefill,
        instatus=donorimp_instatus_prefill,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        unit_id="Ident",
        data_excl_var="toexcl",
        by="Area STaff",
        prefill_by_vars=True,
        presort=True,
        trace=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
        expected_error_count=0,
        msg_list_contains_exact=[
        ],
    )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()