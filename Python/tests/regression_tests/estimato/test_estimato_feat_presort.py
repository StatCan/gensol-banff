"""Proc estimator requires its `indata`, `instatus`, `indata_hist`, and `instatus_hist` 
files to be sorted in ascending order of `by` variables
- when `instatus` has no `by` variables, there is no sort requirement
- when `instatus_hist` has no `by` variables, there is no sort requirement

When `presort=True`, unsorted files will be sorted automatically
- with by vars
- with a mix of by vars and no by vars
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_a(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    Catch unsorted indata
        unsorted:   indata
        sorted:     others
        by vars:    all
    """
    # unsort
    estimato_indata_presort.sort_values(by='V', inplace=True)

    # sort
    estimato_instatus_presort.sort_values(by='area', inplace=True)
    estimato_indata_hist_presort.sort_values(by='area', inplace=True)
    estimato_instatus_hist_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=2,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_b(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    Catch unsorted indata_hist
        unsorted:   indata_hist
        sorted:     others
        by vars:    all
    """
    # unsort
    estimato_indata_hist_presort.sort_values(by='V', inplace=True)

    # sort
    estimato_instatus_presort.sort_values(by='area', inplace=True)
    estimato_indata_presort.sort_values(by='area', inplace=True)
    estimato_instatus_hist_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=2,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata_hist is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_c(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    Catch unsorted instatus
        unsorted:   instatus
        sorted:     others
        by vars:    all
    """
    # unsort
    estimato_instatus_presort.sort_values(by='fieldid', inplace=True)

    # sort
    estimato_indata_hist_presort.sort_values(by='area', inplace=True)
    estimato_indata_presort.sort_values(by='area', inplace=True)
    estimato_instatus_hist_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=3,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set instatus is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_d(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    Catch unsorted instatus_hist
        unsorted:   instatus_hist
        sorted:     others
        by vars:    all
    """
    # unsort
    estimato_instatus_hist_presort.sort_values(by='status', inplace=True)

    # sort
    estimato_indata_hist_presort.sort_values(by='area', inplace=True)
    estimato_indata_presort.sort_values(by='area', inplace=True)
    estimato_instatus_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_warning_count=2,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set instatus_hist is not sorted in ascending sequence. The current by group has area = B and the next by group has area = A."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_e(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    allow unsorted instatus when no by vars present
        unsorted:   instatus
        sorted:     others
        by vars:    all except instatus
    """
    # unsort and strip by variables
    estimato_instatus_presort.sort_values(by='fieldid', inplace=True)
    estimato_instatus_presort = estimato_instatus_presort[['ident', 'fieldid', 'status']]

    # sort
    estimato_indata_hist_presort.sort_values(by='area', inplace=True)
    estimato_indata_presort.sort_values(by='area', inplace=True)
    estimato_instatus_hist_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
        msg_list_contains_exact="NOTE: This procedure can perform faster if the instatus data set contains the by variables."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_f(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    allow unsorted instatus_hist when no by vars present
        unsorted:   instatus_hist
        sorted:     others
        by vars:    all except instatus_hist
    """
    # unsort and strip by variables
    estimato_instatus_hist_presort.sort_values(by='status', inplace=True)
    estimato_instatus_hist_presort = estimato_instatus_hist_presort[['ident', 'fieldid', 'status']]

    # sort
    estimato_indata_hist_presort.sort_values(by='area', inplace=True)
    estimato_indata_presort.sort_values(by='area', inplace=True)
    estimato_instatus_presort.sort_values(by='area', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=False,
        prefill_by_vars=False,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=3,
        msg_list_contains_exact="NOTE: This procedure can perform faster if the instatus_hist data set contains the by variables."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_estimato_presort_g(capfd, estimato_indata_presort, estimato_instatus_presort, estimato_indata_hist_presort, estimato_instatus_hist_presort, estimato_inalgorithm_03, estimato_inestimator_03):
    """
    Validate that `presort` sorts all input datasets correctly 
        unsorted:   all
        by vars:    all
    """
    # unsort
    estimato_indata_presort.sort_values(by='V', inplace=True)
    estimato_indata_hist_presort.sort_values(by='V', inplace=True)
    estimato_instatus_presort.sort_values(by='fieldid', inplace=True)
    estimato_instatus_hist_presort.sort_values(by='status', inplace=True)

    banfftest.pytest_estimato(
        # Banff parameters
        indata=estimato_indata_presort,
        instatus=estimato_instatus_presort,
        indata_hist=estimato_indata_hist_presort,
        instatus_hist=estimato_instatus_hist_presort,
        inalgorithm=estimato_inalgorithm_03,
        inestimator=estimato_inestimator_03,
        unit_id="ident",
        seed=3000,
        by="area",
        presort=True,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()