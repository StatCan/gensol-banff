"""Tests related to the Outlier procedure's `outsummary` dataset
"""
import banff.testing as banfftest
import pytest
import pandas as pd

@pytest.mark.m_auto_pass
def test_outlier_outsummary_a(capfd, indata_ug02):
    """When using "sigmagap" method, specifying only one of `beta_e` and `beta_i` should not
    result in an error.  

    Tests specifying both `beta_e` and `beta_i`.  
    """

    banfftest.pytest_outlier(
        indata=indata_ug02,
        method="sg",
        beta_e=1,
        beta_i=1.75,
        sigma="MAD",
        start_centile=60,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        rc_should_be_zero=True
    )

@pytest.mark.m_auto_pass
def test_outlier_outsummary_b(capfd, indata_ug02):
    """When using "sigmagap" method, specifying only one of `beta_e` and `beta_i` should not
    result in an error.  

    Tests specifying `beta_e` but not `beta_i`.  
    """

    banfftest.pytest_outlier(
        indata=indata_ug02,
        method="sg",
        beta_e=1,
        #beta_i=1.75,
        sigma="MAD",
        start_centile=60,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        rc_should_be_zero=True
    )

@pytest.mark.m_auto_pass
def test_outlier_outsummary_c(capfd, indata_ug02):
    """When using "sigmagap" method, specifying only one of `beta_e` and `beta_i` should not
    result in an error.  

    Tests specifying `beta_i` but not `beta_e`.  
    """

    banfftest.pytest_outlier(
        indata=indata_ug02,
        method="sg",
        #beta_e=1,
        beta_i=1.75,
        sigma="MAD",
        start_centile=60,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        rc_should_be_zero=True
    )

@pytest.mark.m_auto_pass
def test_outlier_outsummary_d(capfd, indata_ug02):
    """When using "sigmagap" method, one or more of of `beta_e` and `beta_i` must be specified,
    otherwise an error occurs.  

    Tests specifying neither of `beta_e` or `beta_i`.  
    """

    banfftest.pytest_outlier(
        indata=indata_ug02,
        method="sg",
        #beta_e=1,
        #beta_i=1.75,
        sigma="MAD",
        start_centile=60,
        outlier_stats=True,
        accept_negative=True,
        unit_id="ident",
        var="X01 X02",
        by="Prov",

        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_contains_exact="ERROR: Either beta_e, beta_i, or both must be specified.",
    )

##### DATA #####

@pytest.fixture
def indata_ug02():
    """indata dataset from Outlier User Guide example 2"""
    # create indata, define columnms but add no data
    outlierdata = pd.DataFrame(columns= ["IDENT", "X01", "X02", "Prov"])

    # populate dataframe using for loop
    import random
    random.seed(8)
    for n in range(1, 31):
        current_label = len(outlierdata)
        outlierdata.loc[current_label] = {
            'IDENT': 'R{:07d}'.format(n),
            'X01': int(random.uniform(0,20)),
            'X02': int(random.uniform(0,20)),
            'Prov': 10 if (n)%2==0 else 11}
        if n%5==0:
            outlierdata.loc[current_label, 'X01'] *= -1
            outlierdata.loc[current_label, 'X02'] *= -1

    # sort dataset
    outlierdata = outlierdata.sort_values(by=["Prov"])

    return outlierdata

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()