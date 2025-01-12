import pytest
import banff.testing as banfftest

@pytest.mark.m_pass
@pytest.mark.m_sas_dataset
def test_determin_h01_a_SAS(capfd):
    """To ensure that the FTI status is recognized and the imputed value is correct. 
    VB should be imputed with the value 113.95.
    """
    banfftest.pytest_determin(
        # Procedure Parameters
        indata="determin_h01_a_indata.sas7bdat",
        instatus="determin_h01_a_instatus.sas7bdat",
        edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                    VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
        unit_id="REC",

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        expected_outdata="determin_h01_a_outdata.sas7bdat",
        # disable comparison of `outstatus` for version `3.1.1b13`, which has new `VALUE` column
        #expected_outstatus="determin_h01_a_outstatus.sas7bdat"
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()