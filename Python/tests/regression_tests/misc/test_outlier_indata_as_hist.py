"""Outlier is unique by allowing `indata` to also serve as `indata_hist` for certain methods.  

C code which allows `indata` to also be used as `indata_hist` was found to fail in version `3.1.1b12`
when `weight` was specified.  The following test cases validate that the issue is resolved.  

"""

# Import packages
import pytest
import numpy as np
import pandas as pd
import banff.testing

@pytest.mark.m_pass
@pytest.mark.m_auto_pass
def test_outlier_indata_as_hist_a(capfd):
    """Use `indata` also as `indata_hist` with 'R' method and `weight`.  

    This test case is known to cause a fatal access violation in banff `3.1.1b12`.  
    It should run to completion without warnings or errors.    
    """
    # Set up parameters
    max_rand=40
    seed=5
    mei=2.5
    size=100
    # Dataframe structure
    data = pd.DataFrame({'ident': range(1,size+1),
                        'var1': None,
                        'var2': None,
                        'weight': None})
    # Change type of ident from int to string
    data['ident'] = data['ident'].astype(str)
    # Generate simulated data
    np.random.seed(seed)
    for i in range(len(data)):
        data.loc[i,'var1'] = np.random.randint(1, max_rand+1)
        data.loc[i, 'var2'] = np.random.randint(1, data.loc[i,'var1']+1)
        data.loc[i,'weight'] = np.random.randint(1, 21) 
    # Call outlier procedure
    outliers_ratio = banff.testing.pytest_outlier(
        indata=   data,
        method= 'R',
        mei= mei,
        outlier_stats= True,
        unit_id= 'ident',
        var= 'var2',
        with_var= 'var1',
        weight= 'weight',

        pytest_capture=capfd,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()