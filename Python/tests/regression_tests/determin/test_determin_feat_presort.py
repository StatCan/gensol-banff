"""Proc deterministic requires its `indata` and `instatus` files to be sorted
in ascending order of `by` variables and `unit_id`.  
- when `instatus` has no `by` variables, sort it by `unit_id`

When `presort=True`, unsorted files will be sorted automatically
- with by vars
- without by vars
- with a mix of by vars and no by vars 
"""
import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_a(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Catch unsorted indata when instatus sorted.  
        BY vars: both files
    """
    # sort
    determin_instatus_persort.sort_values(by=['AREA', 'REC'], inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Data set indata is not sorted in ascending sequence. The current by group has AREA = 2.000000 and the next by group has AREA = 1.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_b(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Catch unsorted instatus when indata sorted.  
        BY vars: both files
    """
    # sort
    determin_indata_presort.sort_values(by=['AREA', 'REC'], inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        prefill_by_vars=False,
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=1,
        msg_list_contains_exact="ERROR: Data set instatus is not sorted in ascending sequence. The current by group has AREA = 2.000000 and the next by group has AREA = 1.000000."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_c(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Catch that neither instatus or indata are sorted.  
        BY vars: both files
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_d(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Catch unsorted instatus when indata sorted.  
        BY vars: indata only
    """
    # drop BY vars from instatus and ruin sort order
    import pandas as pd
    with pd.option_context("mode.copy_on_write", True):
        determin_instatus_persort = determin_instatus_persort[['REC', 'FIELDID', 'STATUS']]
        determin_instatus_persort.sort_values(by='REC', ascending=False, inplace=True)

        # sort indata correctly
        determin_indata_presort.sort_values(by=['AREA', 'REC'], inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        prefill_by_vars=False,
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=1,
        msg_list_contains_exact="ERROR: instatus data set is not sorted in ascending order. The current key is 6 and the previous key was 7."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_e(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Catch unsorted indata when instatus sorted.  
        BY vars: not used
    """
    # drop BY vars from instatus and sort correctly
    import pandas as pd
    with pd.option_context("mode.copy_on_write", True):
        determin_instatus_persort = determin_instatus_persort[['REC', 'FIELDID', 'STATUS']]
        determin_instatus_persort.sort_values(by='REC', inplace=True)

        # ruin sort order
        determin_indata_presort.sort_values(by=['REC'], ascending=False, inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        presort=False,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: indata data set is not sorted in ascending order. The current key is 6 and the previous key was 7."
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_f(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: both files
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        presort=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_g(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: not used
    """
    # drop BY vars from instatus and ruin sort order
    import pandas as pd
    with pd.option_context("mode.copy_on_write", True):
        determin_instatus_persort = determin_instatus_persort[['REC', 'FIELDID', 'STATUS']]
        determin_instatus_persort.sort_values(by='REC', ascending=False, inplace=True)

        # ruin sort order
        determin_indata_presort.sort_values(by=['REC'], ascending=False, inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        presort=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
    )

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_determin_feat_presort_h(capfd, determin_indata_presort, determin_instatus_persort):
    """
    Validate that `presort` sorts all input datasets correctly 
        BY vars: indata only
    """
    # drop BY vars from instatus and ruin sort order
    import pandas as pd
    with pd.option_context("mode.copy_on_write", True):
        determin_instatus_persort = determin_instatus_persort[['REC', 'FIELDID', 'STATUS']]
        determin_instatus_persort.sort_values(by='REC', ascending=False, inplace=True)

        # ruin sort order
        determin_indata_presort.sort_values(by=['REC'], ascending=False, inplace=True)

    banfftest.pytest_determin(
        # Procedure Parameters
        indata=determin_indata_presort,
        instatus=determin_instatus_persort,
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        accept_negative=True,
        unit_id="REC",
        by="AREA",
        presort=True,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=2,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()