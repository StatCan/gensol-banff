import pytest
from banff.testing import pytest_errorloc

@pytest.mark.m_validated
@pytest.mark.m_for_review # Mixed datatype columns now causes an exception during packing: PyArrow doesn't know what to do
def test_error_IT03(capfd, errorloc_indata_k02, errorloc_expected_outstatus_01):
    """Similar to `test_donor_IT05`
        When invalid typed data (i.e. character data in numeric data column) is detected
        during read operations, an error message should be printed to the log
        and the procedure should terminate, returning a non-zero RC.  
        This test validates that the correct error message is printed.  
        This test failed prior to implementing the proper initialization of
        errorloc's `VL_in_var` variable list.  """
    #### arrange
    errorloc_indata_k02=errorloc_indata_k02
    errorloc_indata_k02[['IDENT']]=errorloc_indata_k02[['IDENT']].fillna('')
    errorloc_indata_k02.at[5,'X1'] = '.'
    
    pytest_errorloc(
        # Outlier Parameters
        indata=errorloc_indata_k02,
        edits="x1>=5; x1<=10; x2>=-1; x1+x2<=50; x3>=100; x3<=150; x1+x2+x3>=135; x1+x2+x3<=210;",
        accept_negative=False,
        seed=1,
        unit_id="IDENT"
        ,

        # Unit Test Parameters
        sas_log_name="errorK02.log",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        msg_list_contains="ERROR: invalid data detected while reading numeric variable 'X1'",
        expected_error_count=1
    )
