# To verify that if the NOTSORTED option in the BY statement is used and the 
# observations are not sorted by BY variable in the input data set, no errors will result.
import pytest
from banff.testing import assert_log_consistent
import os
import banff

@pytest.mark.m_validated
@pytest.mark.m_missing_feature # see https://jira.statcan.ca/browse/ITSLMBANFF-36
def test_donorD04(capfd, indataCharKey, instatCharKey, expected_outdonormap_03):
    #### arrange
    indataCharKey.at[1,'area'] = "A2"
    indataCharKey.at[3,'area'] = "A1"

    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        donorimputation_d04= banff.donorimp(
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
            data_excl_var="toexcl",
            by="area notsorted"
        )

        rc = donorimputation_d04.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorD04_sas.txt"
    
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="NOTE: There were 1 observations dropped from indata data set because they are potential donors but toexcl value is 'E' for these observations.")
    assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="""Number of valid observations ......................:       1     100.00%
Number of donors ..................................:       0       0.00%
Number of recipients ..............................:       1     100.00%
    not imputed (insufficient number of donors) ...:       1     100.00%

NOTE: The above message was for the following by group:
      area=A1""")
    #assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="Number of recipients ..................:     1      33.33%")
    #assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="    with matching fields ..............:     0       0.00%")
    #assert_log_consistent(test_log=out_act, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg="    without matching fields ...........:     1      33.33%")
    #assert_dataset_equal(donorimputation_d04.outdonormap, expected_outdonormap_03, "outdonormap")
    assert rc==0, f"Banff returned non-zero value: {rc}"
    
    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)