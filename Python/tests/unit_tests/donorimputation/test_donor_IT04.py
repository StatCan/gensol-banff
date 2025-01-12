"""Test the optional BY variables on status file.  
Results should be identical regardless of BY variable presence on status input file.  
However, performance and console-logged WARNING: and NOTE: messages will differ"""
import pytest
from banff.testing import assert_dataset_equal, assert_substr_count
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donor_IT04(capfd, indataCharKey, instatCharKey, expected_outdata_02):
    #### arrange
    indataCharKey.at[1,'staff'] = 100

    indataCharKey['toexcl'] = indataCharKey['toexcl'].astype(str)
    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        donorimputation_d03= banff.donorimp(
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
            data_excl_var="toexcl",
            by="area staff",
            prefill_by_vars=False,
        )

        rc = donorimputation_d03.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    
    assert_substr_count(test_log=out_act, expected_count=1, substr_to_count="NOTE: This procedure can perform faster if the instatus data set contains the by variables.")
    assert_substr_count(test_log=out_act, expected_count=0, substr_to_count="ERROR:")
    assert_substr_count(test_log=out_act, expected_count=4, substr_to_count="WARNING:")

    assert_dataset_equal(donorimputation_d03.outdata, expected_outdata_02, "OUTDATA")
    assert rc==0, f"Banff returned non-zero value: {rc}"
    

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donor_IT04_by_var(capfd, indataCharKey, instatCharKey_by_var, expected_outdata_02):
    #### arrange
    indataCharKey.at[1,'staff'] = 100

    indataCharKey['toexcl'] = indataCharKey['toexcl'].astype(str)
    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        banff_call= banff.donorimp(
            indata=indataCharKey,
            instatus=instatCharKey_by_var,
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
            data_excl_var="toexcl",
            by="area staff"
        )

        rc = banff_call.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    
    assert_substr_count(test_log=out_act, expected_count=0, substr_to_count="NOTE: This procedure can perform faster if the instatus data set contains the by variables.")
    assert_substr_count(test_log=out_act, expected_count=0, substr_to_count="ERROR:")
    assert_substr_count(test_log=out_act, expected_count=2, substr_to_count="WARNING:")

    assert_dataset_equal(banff_call.outdata, expected_outdata_02, "OUTDATA")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)