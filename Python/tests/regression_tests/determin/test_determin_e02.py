import pytest
import banff.testing as banfftest
from banff.exceptions import ProcedureValidationError

@pytest.mark.m_auto_pass
def test_determin_e02_a(capfd, determin_indata_01, determin_instatus_01):
    """The names specified for the two output files must be different. 
    If not, Deterministic should not proceed.  
    Error messages should be printed in the log file.
    """
    try:
        banfftest.pytest_determin(
            # Procedure Parameters
            indata=determin_indata_01,
            instatus=determin_instatus_01,
            outdata="out.parquet",
            outstatus="out.parquet",
            edits="""VA >= 0.001; VC >= 0.0001; VE >= 0.0001; 
                        VA + VB + VC + VD = TOT1; VE + VF + VG = TOT2; TOT1 + TOT2 = GT;""",
            accept_negative=True,
            unit_id="REC",

            # Test Parameters
            pytest_capture=capfd,
        )
    except ProcedureValidationError as e:
        print(e)
        return
    else:
        assert False, "exception should occur"

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()