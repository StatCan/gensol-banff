## Purpose: Test case should terminate early due to mixed datatype in column
##              - 'toexcl' column is character, but contains numeric value 5
## Comment: In SAS mixing datatypes is impossible, so this test is unique to Python
##          - based on DonorC03, which incorrectly mixed datatypes
## Comment: This test fails for Banff <= 3.01.001a2.dev3
## Date written: 2023-April-27
## Written by: Stephen Arsenault
import pytest
from banff.testing import assert_log_contains
import banff

@pytest.mark.m_validated
@pytest.mark.m_for_review # Mixed datatype columns now causes an exception during packing: PyArrow doesn't know what to do
@pytest.mark.m_jira_69
def test_donor_IT02(capfd, indataCharKey, instatCharKey):
    #### arrange
    indataCharKey.at[2,'total'] = 1200
    indataCharKey.at[0,'toexcl'] = "."
    indataCharKey.at[1,'toexcl'] = "%"
    indataCharKey.at[2,'toexcl'] = "."
    indataCharKey.at[3,'toexcl'] = 5
    
    #### act
    try:
        #Create Banff call
        donor_call= banff.donorimp(
            indata=indataCharKey,
            instatus=instatCharKey,
            outdata=None,
            outstatus=None,
            outdonormap=None,
            edits="Q1 + Q2 + Q3 + Q4 = total;",
            post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
            min_donors=1,
            percent_donors=1,
            n=3,
            outmatching_fields=True,
            eligdon="any",
            unit_id="ident",
            must_match="STAFF",
            data_excl_var="toexcl"
        )

        rc = donor_call.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()

    assert_log_contains(test_log=out_act, msg="ERROR: invalid data detected while reading character variable 'toexcl' on row 3")
    assert rc != 0, f"Banff return code should be non-zero, but is: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()