import pytest
import banff
import banff.testing

@pytest.mark.m_auto_pass
def test_valid_kwargs(capfd, ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """Call a Banff procedure and provide valid keyword argument `_BP_c_log_handlers`.  

    The procedure should execute without any exceptions.  
    """
    try:
        banff_call = banff.testing.pytest_determin(
            indata=ug_determin_ex1_indata,
            instatus=ug_determin_ex1_instatus,
            edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
            accept_negative=True,
            unit_id="ident",
            by="prov",

            _BP_c_log_handlers=[],

            pytest_capture=capfd,
        )
    except Exception as e:
        raise AssertionError(f"No exception should occur, but the following occurred: {e}")

@pytest.mark.m_auto_pass
def test_invalid_kwargs(capfd, ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """Call a Banff procedure and provide invalid keyword arguments.  

    The procedure should raise a TypeError exception.   
    """
    try:
        banff_call = banff.testing.pytest_determin(
            indata=ug_determin_ex1_indata,
            instatus=ug_determin_ex1_instatus,
            edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
            accept_negative=True,
            unit_id="ident",
            by="prov",

            invalid_kwarg=[],

            pytest_capture=capfd,
        )
    except TypeError as e:
        print(f"TypeError occurred, as expected: {e}")
    else:
        raise AssertionError("TypeError should have occurred but did not")



# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()