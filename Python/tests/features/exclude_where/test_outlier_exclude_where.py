"""Basic test validates outlier's `exclude_where_indata` option

Tests derived from user guide example 4.
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa

@pytest.mark.m_auto_pass
def test_outlier_excl_where_a(capfd, outlierdata_f, outlierhist_f, expected_stats_1, expected_stats_2):
    """Run user guide example 4 as-is, validate expected statistics"""
    # create Banff call 
    banff_call= banfftest.pytest_outlier(
        indata=outlierdata_f,
        indata_hist=outlierhist_f,
        method="sigmagap",
        beta_i=0.5,
        beta_e=0.3,
        unit_id="ident",
        var="X01 X02",
        with_var="X01 X01",
        by="Prov",

        pytest_capture=capfd,
        expected_warning_count=8,
        msg_list_contains=[
            expected_stats_1,
            expected_stats_2,
        ]
    )

@pytest.mark.m_auto_pass
def test_outlier_excl_where_b(capfd, outlierdata_f, outlierhist_f, different_stats_1, different_stats_2):
    """Run user guide example 4 dropping negative values, validate difference in statistics"""
    # create Banff call 
    banff_call= banfftest.pytest_outlier(
        indata=outlierdata_f,
        indata_hist=outlierhist_f,
        method="sigmagap",
        beta_i=0.5,
        beta_e=0.3,
        unit_id="ident",
        var="X01 X02",
        with_var="X01 X01",
        by="Prov",
        exclude_where_indata="X01<0 or X02 < 0",

        pytest_capture=capfd,
        expected_warning_count=4,
        msg_list_contains=[
            different_stats_1,
            different_stats_2,
        ]
    )

@pytest.mark.m_auto_pass
def test_outlier_excl_where_c(capfd, outlierdata_f, outlierhist_f, expected_stats_1, expected_stats_2):
    """Run user guide example 4 as-is, validate expected statistics
    
    Passing empty string for `exclude_where...` should be ignored"""
    # create Banff call 
    banff_call= banfftest.pytest_outlier(
        indata=outlierdata_f,
        indata_hist=outlierhist_f,
        method="sigmagap",
        beta_i=0.5,
        beta_e=0.3,
        unit_id="ident",
        var="X01 X02",
        with_var="X01 X01",
        by="Prov",
        exclude_where_indata=" ",

        trace=True,  # verbose logging
        capture=True,  # so pytest can capture python logging statements

        pytest_capture=capfd,
        expected_warning_count=8,
        msg_list_contains=[
            "Ignoring option `exclude_where_indata=",
            expected_stats_1,
            expected_stats_2,
        ]
    )

@pytest.mark.m_auto_pass
def test_outlier_excl_where_d(capfd, outlierdata_f, outlierhist_f, different_stats_1, different_stats_2):
    """Run user guide example 4 passing a `exclude_where_indata` with disallowed ';' character"""
    # create Banff call 
    try:
        banff_call= banfftest.pytest_outlier(
            indata=outlierdata_f,
            indata_hist=outlierhist_f,
            method="sigmagap",
            beta_i=0.5,
            beta_e=0.3,
            unit_id="ident",
            var="X01 X02",
            with_var="X01 X01",
            by="Prov",
            exclude_where_indata="X01<0 or X02 < 0;",

            pytest_capture=capfd,
            expected_warning_count=4,
            msg_list_contains=[
                different_stats_1,
                different_stats_2,
            ]
        )
    except ValueError as e:
        assert e.args[0] == "Parameter `exclude_where_indata` cannot contain the substring ';'"
    else:
        raise AssertionError("Procedure should have raised a ValueError")

@pytest.mark.m_auto_pass
def test_outlier_excl_where_e(capfd, outlierdata_f, outlierhist_f, different_stats_1, different_stats_2):
    """Run user guide example 4 passing invalid `exclude_where_indata_hist`, which is only supported by estimator.  
    
    An exception should be raised"""
    # create Banff call 
    try:
        banff_call= banfftest.pytest_outlier(
            indata=outlierdata_f,
            indata_hist=outlierhist_f,
            method="sigmagap",
            beta_i=0.5,
            beta_e=0.3,
            unit_id="ident",
            var="X01 X02",
            with_var="X01 X01",
            by="Prov",
            exclude_where_indata_hist="X01<0 or X02 < 0",

            pytest_capture=capfd,
            expected_warning_count=4,
            msg_list_contains=[
                different_stats_1,
                different_stats_2,
            ]
        )
    except TypeError as e:
        assert "unexpected keyword argument 'exclude_where_indata_hist'" in e.args[0]
    else:
        raise AssertionError("Procedure should have raised a ValueError")

##### DATA #####

@pytest.fixture
def expected_stats_1():
    return """
NOTE: The following messages are for the by group:
      Prov=10

NOTE: Number of observations in indata data set .....................:      15
NOTE: Number of valid observations in indata data set ...............:      15

NOTE: Number of observations in indata_hist data set .....................:      15
NOTE: Number of valid observations in indata_hist data set ...............:      15"""

@pytest.fixture
def expected_stats_2():
    return """
NOTE: The following messages are for the by group:
      Prov=11

NOTE: Number of observations in indata data set .....................:      15
NOTE: Number of valid observations in indata data set ...............:      15

NOTE: Number of observations in indata_hist data set .....................:      15
NOTE: Number of valid observations in indata_hist data set ...............:      15"""

@pytest.fixture
def different_stats_1():
    return """
NOTE: The following messages are for the by group:
      Prov=10

NOTE: Number of observations in indata data set .....................:      12
NOTE: Number of valid observations in indata data set ...............:      12

NOTE: Number of observations in indata_hist data set .....................:      15
NOTE:  dropped because unit_id only in indata_hist ........................:       3
NOTE: Number of valid observations in indata_hist data set ...............:      12"""

@pytest.fixture
def different_stats_2():
    return """
NOTE: The following messages are for the by group:
      Prov=11

NOTE: Number of observations in indata data set .....................:      12
NOTE: Number of valid observations in indata data set ...............:      12

NOTE: Number of observations in indata_hist data set .....................:      15
NOTE:  dropped because unit_id only in indata_hist ........................:       3
NOTE: Number of valid observations in indata_hist data set ...............:      12"""

@pytest.fixture
def outlierdata_f():
    outlierdata = pd.DataFrame(columns= ["IDENT", "X01", "X02", "Prov"])

    import random
    random.seed(8)
    for n in range(1, 31):
        current_label = len(outlierdata)
        outlierdata.loc[current_label] = {
            'IDENT': 'R{:07d}'.format(n),
            'X01': int(random.uniform(1,20)),
            'X02': int(random.uniform(1,20)),
            'Prov': 10 if (n)%2==0 else 11}
        if n%5==0:
            outlierdata.loc[current_label, 'X01'] *= -1
            outlierdata.loc[current_label, 'X02'] *= -1

    outlierdata.sort_values(by=["Prov"], inplace=True)
    return outlierdata


@pytest.fixture
def outlierhist_f():
    outlierhist = pd.DataFrame(columns= ["IDENT", "X01", "X02", "Prov"])

    import random
    random.seed(8)
    for n in range(1, 31):
        current_label = len(outlierhist)
        outlierhist.loc[current_label] = {
            'IDENT': 'R{:07d}'.format(n),
            'X01': int(random.uniform(1,30)),
            'X02': int(random.uniform(1,30)),
            'Prov': 10 if (n)%2==0 else 11}
        if n%7==0:
            outlierhist.loc[current_label, 'X01'] *= -1
            outlierhist.loc[current_label, 'X02'] *= -1

    outlierhist.sort_values(by=["Prov"], inplace=True)
    return outlierhist

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()