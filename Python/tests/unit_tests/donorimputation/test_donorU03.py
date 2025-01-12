from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_incomplete # just putting this one off for now, SAS errors...
def test_donorU03_a(capfd, JA_indata32, JA_instat33):
    assert 1 == 0, "TEST CASE INCOMPLETE"
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata32
    
    #synthetic input dataset - instatus
    instat = JA_instat33
    """
    #### act
    try:
        #create Banff call
       donorimp_U03_1 = banff.donorimp(
                indata=indata,
                instatus=instat,
                outdata=None,
                outstatus=None,
                outdonormap=None,
                edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
                min_donors=1,
                percent_donors=1,
                n=25,
                n_limit=.5,
                mrl=-.5,
                seed=100,
                unit_id="IDENT",
                must_match="VAR2",
                by="AREA"
                )     
                
    except Exception as e:
        print("The U03_1 call had an unhandled exception")
        print(e)
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorU03_sas.txt"

    assert out.count("ERROR") == 0"""
    #assert out.count("WARNING") == 0

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorU03_b(capfd, JA_indata32, JA_instat33):
    pytest_donorimp(
        # Banff Parameters
        indata=JA_indata32,
        instatus=JA_instat33,
        edits="VAR1>=0; VAR2>=0; VAR1<=VAR2;",
        min_donors=1,
        percent_donors=1,
        n=25,
        n_limit=1,
        mrl=-.5,
        seed=100,
        unit_id="IDENT",
        must_match="VAR2",
        by="AREA"
        ,

        # Unit Test Parameters
        sas_log_name="donorU03_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=False,
        expected_error_count=1,
        msg_list_sas="ERROR: Invalid mrl. mrl must be greater than 0."
    )

