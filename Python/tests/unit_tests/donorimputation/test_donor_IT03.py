# If the specified DATA set does not exist, DonorImp should terminate.
# Error messages should be printed to the log file.
import pytest
from banff.testing import assert_log_contains, assert_dataset_equal
import banff
import tempfile

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_jira_63
def test_donor_IT03_a(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test reading from a CSV file.  
        Expect: run without errors, produce expected data """
    #### arrange
    temp_dir = tempfile.TemporaryDirectory()
    ds_file = temp_dir.name + "/indata.csv"
    indataCharKey.to_csv(ds_file, index=False)

    #### act
    #Create Banff call
    donor_call= banff.donorimp(
        trace=True,
        indata=ds_file,
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
        unit_id="ident"
        #must_match="STAFF",
        #data_excl_var="toexcl"
    )

    rc = donor_call.rc

    #### assert
    out_act, err = capfd.readouterr()
    
    #assert_log_contains(test_log=out_act, must_exist=True, msg="ERROR: File WORK.INDAT.DATA does not exist")
    assert_dataset_equal(donor_call.outdata, expected_outdata_04, "OUTDATA")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_jira_63
def test_donor_IT03_b(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test non-existent file path
        - folder exists, but file does not
        Expect: FileNotFoundError exception"""
    #### arrange
    temp_dir = tempfile.TemporaryDirectory()
    ds_file = temp_dir.name + "/indata.csv"
    # don't actually create the file though #indataCharKey.to_csv(ds_file, index=False)

    #### act
    try:
        #Create Banff call
        donor_call= banff.donorimp(
            trace=True,
            indata=ds_file,
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
            capture=True
            #must_match="STAFF",
            #data_excl_var="toexcl"
        )

        rc = donor_call.rc
    except Exception as e:
        #### assert
        assert  isinstance(e.__cause__, FileNotFoundError)
        print(e)
    out_act, err = capfd.readouterr()
    
    assert_log_contains(test_log=out_act, msg="Input dataset file path does not exist: ")

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_jira_63
def test_donor_IT03_c(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test non-existent file path
        - syntatically valid path where all parts of the path are non-existant
        Expect: FileNotFoundError exception"""
    #### arrange
    ds_file = "Z:/neither/directory/nor/file/actually/exist/indata.csv"

    #### act
    try:
        #Create Banff call
        donor_call= banff.donorimp(
            trace=True,
            indata=ds_file,
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
            capture=True
            #must_match="STAFF",
            #data_excl_var="toexcl"
        )

        rc = donor_call.rc
    except Exception as e:
        #### assert
        assert  isinstance(e.__cause__, FileNotFoundError)
        print(e)

    out_act, err = capfd.readouterr()
    
    assert_log_contains(test_log=out_act, msg="Input dataset specification unrecognized, assuming non-existent file path:")

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
@pytest.mark.m_jira_63
def test_donor_IT03_d(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test non-existent file path
        - empty string
        Expect: ValueError exception"""
    #### arrange
    ds_file = ""

    #### act
    try:
        #Create Banff call
        donor_call= banff.donorimp(
            trace=True,
            indata=ds_file,
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
            capture=True
            #must_match="STAFF",
            #data_excl_var="toexcl"
        )

        rc = donor_call.rc
    except Exception as e:
        #### assert
        assert  isinstance(e.__cause__, ValueError)
        print(e)

    out_act, err = capfd.readouterr()
    
    assert_log_contains(test_log=out_act, msg="Input dataset invalid: empty string")

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_for_review # providing raw JSON for input datasets no longer supported: PyArrow compatible type required
@pytest.mark.m_jira_63
def test_donor_IT03_e(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test non-existent file path
        - empty string
        Expect: ValueError exception"""
    #### arrange

    ds_json = indataCharKey.to_json(orient='split')

    #### act
    #Create Banff call
    donor_call= banff.donorimp(
        trace=True,
        indata=ds_json,
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
        unit_id="ident"
        #must_match="STAFF",
        #data_excl_var="toexcl"
    )

    rc = donor_call.rc


    out_act, err = capfd.readouterr()
    
    #assert_log_contains(test_log=out_act, must_exist=True, msg="ERROR: File WORK.INDAT.DATA does not exist")
    assert_dataset_equal(donor_call.outdata, expected_outdata_04, "OUTDATA")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)

@pytest.mark.m_validated
@pytest.mark.m_missing_validation # procedure itself should validate that JSON object conforms to requirements
# could validate using JSON schema, in Python.... 
# should look at JSON schema validation from C code
def test_donor_IT03_f(capfd, indataCharKey, instatCharKey, expected_outdata_04):
    """Test non-existent file path
        - empty string
        Expect: ValueError exception"""
    #### arrange

    ds_json = '{fake: "JSON"{}'

    #### act
    #Create Banff call
    donor_call= banff.donorimp(
        trace=True,
        indata=ds_json,
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
        unit_id="ident"
        #must_match="STAFF",
        #data_excl_var="toexcl"
    )

    rc = donor_call.rc


    out_act, err = capfd.readouterr()
    
    #assert_log_contains(test_log=out_act, must_exist=True, msg="ERROR: File WORK.INDAT.DATA does not exist")
    assert_dataset_equal(donor_call.outdata, expected_outdata_04, "OUTDATA")
    assert rc==0, f"Banff returned non-zero value: {rc}"

    out_assert, err2 = capfd.readouterr()
    print(out_act+out_assert)