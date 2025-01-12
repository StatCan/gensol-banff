#############################################################################################################
## Description: Conversion of SAS test case to Python: donorV02
## Command to run in CMD: python -m pytest test_donorV02.py
## Notes:
## Last Updated: 2023-apr-28
#############################################################################################################
from banff.testing import assert_log_consistent
import os

# import Banff package
import banff
 
def test_donorV02_a(capfd,JA_indata41, JA_instat42):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata41
    
    #synthetic input dataset - instatus
    instat = JA_instat42

    #print(instat['REC'])
    
    #### act
    try:
        #create Banff call
        donorimp_V02_1 = banff.donorimp(
            indata=indata,
            instatus=instat,
            outdata=None and test,
            outstatus=None and test,
            outdonormap=None and test,
            edits="x=y;",
            post_edits="x<=y;",
            min_donors=1,
            percent_donors=1,
            n=1,
            unit_id="REC",
            rand_num_var="randkey"
            )  
                
    except Exception as e:
        print("The V02_1 call had an unhandled exception")
        print(e)
        
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV02_sas.txt"

    #does not work
    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0

    
def test_donorV02_b(capfd,JA_indata41, JA_instat42):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata41
    
    #synthetic input dataset - instatus
    instat = JA_instat42

    #print(instat['REC'])
    
    #### act
    try:
        #create Banff call
        donorimp_V02_2 = banff.donorimp(
                indata=indata,
                instatus=instat,
                outdata=None and test,
                outstatus=None and test,
                outdonormap=None and test ,
                edits="x=y;",
                post_edits="x<=y;",
                min_donors=1,
                percent_donors=1,
                n=1,
                display_level=0,
                unit_id="REC",
                rand_num_var="randkey"
                )     
                
    except Exception as e:
        print("The V02_2 call had an unhandled exception")
        print(e)    

    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV02_sas.txt"

    #does not work
    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0

def test_donorV02_c(capfd,JA_indata41, JA_instat42):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata41
    
    #synthetic input dataset - instatus
    instat = JA_instat42

    #print(instat['REC'])
    
    #### act
    try:
        #create Banff call
        donorimp_V02_3 = banff.donorimp(
                indata=indata,
                instatus=instat,
                outdata=None and test,
                outstatus=None and test,
                outdonormap=None and test,
                edits="x=y;",
                post_edits="x<=y;",
                min_donors=1,
                percent_donors=1,
                n=1,
                display_level=1,
                unit_id="REC",
                rand_num_var="randkey"
                )     
                
    except Exception as e:
        print("The V02_3 call had an unhandled exception")
        print(e)
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV02_sas.txt"

    #does not work
    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0    