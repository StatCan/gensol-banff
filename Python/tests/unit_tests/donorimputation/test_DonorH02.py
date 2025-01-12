# If BY variable(s) have been specified, then the BY variable(s) should appear 
# in the output status data set in addition to the key variable, FIELDID, and STATUS.
import pytest
from banff.testing import assert_dataset_equal
import banff

import pandas as pd
from io import StringIO

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorH02(capfd, indataCharKey, expected_outstatus_01b, expected_outmatching_fields_01b):

    instat = pd.read_csv(StringIO("""
    ident area fieldid status
    REC01 A1 Q3 IPR
    REC04 A1 Q2 FTE
    REC04 A1 Q3 FTI
    REC04 A1 Q4 FTI
    """), sep=r'\s+', dtype={'ident': str, 'area': str, 'fieldid': str, 'status': str})
    
    indataCharKey['toexcl'] = indataCharKey['toexcl'].astype(str)
    for i, row in indataCharKey.iterrows():
        val = "."
        if indataCharKey.at[i,'total'] > 1000:
            val = "E"
        indataCharKey.at[i,'toexcl'] = val

    #### act
    try:
        #Create Banff call
        donorimputation_h02= banff.donorimp(
            indata=indataCharKey,
            instatus=instat,
            outdata=None,
            outstatus=None,#sasuser.outstat two-level name not possible here?
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
            by="area"
        )

        rc = donorimputation_h02.rc
    except Exception as e:
        print(e)

    #### assert
    out_act, err = capfd.readouterr()
    
    assert_dataset_equal(donorimputation_h02.outstatus, expected_outstatus_01b, "OUTSTATUS")
    assert_dataset_equal(donorimputation_h02.outmatching_fields, expected_outmatching_fields_01b, "OUTMATCHING_FIELDS")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)