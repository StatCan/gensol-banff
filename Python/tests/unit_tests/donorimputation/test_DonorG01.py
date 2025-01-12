# To create a permanent output data set, a two-level name must be specified.
import pytest
from banff.testing import assert_log_consistent, assert_dataset_equal
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_missing_feature # see https://jira.statcan.ca/browse/ITSLMBANFF-47
def test_donorG01(capfd, indataCharKey, instatCharKey, expected_outdata_02, expected_outdonormap_02):
    #### arrange
    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        donorimputation_g01= banff.donorimp(
            indata=indataCharKey,
            instatus=instatCharKey,
            outdata=None,#sasuser.outdata two-level name not possible here?
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

        rc = donorimputation_g01.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorG01_sas.txt"
    
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, msg="NOTE: OUT =")
    assert_dataset_equal(donorimputation_g01.outdata, expected_outdata_02, "OUTDATA")
    assert_dataset_equal(donorimputation_g01.outdonormap, expected_outdonormap_02, "outdonormap")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)