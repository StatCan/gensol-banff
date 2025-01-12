import pytest
from banff.testing import pytest_estimato
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_for_review # Mixed datatype columns now causes an exception during packing: PyArrow doesn't know what to do
def test_estimato_IT02(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01b):
    #### arrange
    estimato_indata_01a.at[2, 'Z'] = None # set value to missing - it will be imputed
    estimato_instatus_01a.drop([0,1], inplace=True) # drop first 2 rows - don't need them
    estimato_indata_01a.at[1, 'Z'] = '.' # insert invalid data (character value in numeric column)

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01b,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        expected_warning_count=2,
        msg_list_contains="ERROR: invalid data detected while reading numeric variable 'Z'"
    )