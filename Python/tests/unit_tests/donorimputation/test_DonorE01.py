# The input data set need not be sorted in ascending order by the key variable. 
# To verify that unsorted (by key variable) data sets are accepted.
import pytest
from banff.testing import assert_dataset_equal
import banff

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorE01(capfd, indataCharKey, instatCharKey, expected_outdata_03, expected_outdonormap_05):
    #### arrange
    indataCharKey['toexcl'] = indataCharKey['toexcl'].astype(str)
    indataCharKey.at[1,'ident'] = "REC04"
    indataCharKey.at[2,'ident'] = "REC02"
    indataCharKey.at[0,'area'] = "A2"

    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        donorimputation_e01= banff.donorimp(
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

        rc = donorimputation_e01.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    
    assert_dataset_equal(donorimputation_e01.outdata, expected_outdata_03, "OUTDATA")
    assert_dataset_equal(donorimputation_e01.outdonormap, expected_outdonormap_05, "outdonormap")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)