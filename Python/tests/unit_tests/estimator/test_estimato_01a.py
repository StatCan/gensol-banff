import pytest
from banff.testing import pytest_estimato

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01a01(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01a):
    #### arrange
    # set countcriteria to non-missing value when missing value expected
    estimato_inestimator_01a.at[0,'countcriteria']=1

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01a,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        sas_log_name="estimato_01a01.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas_exact="ERROR: Estimator: Estimator 0: COUNTCRITERIA cannot be specified for an EF estimator that does not calculate an average."
    )



@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01a02(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01a):
    #### arrange
    # set percent criteria to invalid value
    estimato_inestimator_01a.at[0,'percentcriteria']=10

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01a,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        sas_log_name="estimato_01a02.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Estimator: Estimator 0: PERCENTCRITERIA cannot be specified for an EF estimator that does not calculate an average."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01a03(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01a):
    #### arrange
    # specify excludeimputed when we shouldn't
    estimato_inestimator_01a.at[0,'excludeimputed']="Y"

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01a,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        sas_log_name="estimato_01a03.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Estimator: Estimator 0: EXCLUDEIMPUTED cannot be specified for an EF estimator that does not calculate an average or does not have a random error to calculate."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01a04(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01a):
    #### arrange
    # specify excludeoutliers when we shouldn't
    estimato_inestimator_01a.at[0,'excludeoutliers']="Y"

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01a,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        sas_log_name="estimato_01a04.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Estimator: Estimator 0: EXCLUDEOUTLIERS cannot be specified for an EF estimator that does not calculate an average or does not have a random error to calculate."
    )


@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_estimato01a05(capfd, estimato_indata_01a, estimato_instatus_01a, estimato_inestimator_01a):
    #### arrange
    # make some things invalid
    estimato_inestimator_01a.at[0,'excludeimputed']="N"
    estimato_inestimator_01a.at[0,'excludeoutliers']="N"
    estimato_inestimator_01a.at[0,'countcriteria']=5
    estimato_inestimator_01a.at[0,'percentcriteria']=10
    estimato_inestimator_01a.at[0,'weightvariable']="W"

    #### act
    pytest_estimato(
        # Banff parameters
        indata=estimato_indata_01a,
        instatus=estimato_instatus_01a,
        inestimator=estimato_inestimator_01a,
        unit_id="ID",
        seed=30000
        ,

        # Unit Test Parameters
        sas_log_name="estimato_01a05.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=5,
        msg_list_sas=[
            "ERROR: Estimator: Estimator 0: EXCLUDEIMPUTED cannot be specified for an EF estimator that does not calculate an average or does not have a random error to calculate.",
            "ERROR: Estimator: Estimator 0: EXCLUDEOUTLIERS cannot be specified for an EF estimator that does not calculate an average or does not have a random error to calculate.",
            "ERROR: Estimator: Estimator 0: COUNTCRITERIA cannot be specified for an EF estimator that does not calculate an average.",
            "ERROR: Estimator: Estimator 0: PERCENTCRITERIA cannot be specified for an EF estimator that does not calculate an average.",
            "ERROR: Estimator: Estimator 0: WEIGHTVARIABLE cannot be specified for an EF estimator that does not calculate an average."
        ]
    )