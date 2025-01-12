#############################################################################################################
## Description: Conversion of SAS test case to Python: donorV03
## Command to run in CMD: python -m pytest test_donorV03.py
## Notes:
## Last Updated: 2023-apr-28
#############################################################################################################
from banff.testing import assert_log_consistent
import os

# import Banff package
import banff

def test_donorV03_a(capfd, JA_indata43, JA_instat44):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata43 
    
    #synthetic input dataset - instatus
    instat = JA_instat44

    #print(instat['REC'])
    #### act
    try:
        #create Banff call
        donorimp_V03_1 = banff.donorimp(
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
            #seed=1,
            display_level=1,
            unit_id="REC",
            #rand_num_var="randkey"
            )  
                
    except Exception as e:
        print("The V03_1 call had an unhandled exception")
        print(e)
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV03_sas.txt"


    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0

def test_donorV03_b(capfd, JA_indata43, JA_instat44):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata43 
    
    #synthetic input dataset - instatus
    instat = JA_instat44

    #print(instat['REC'])
    #### act       
    try:
        #create Banff call
        donorimp_V03_2 = banff.donorimp(
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
            seed=1,
            display_level=1,
            unit_id="REC",
            #rand_num_var="randkey"
            )     
                
    except Exception as e:
        print("The V03_2 call had an unhandled exception")
        print(e)    
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV03_sas.txt"


    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0        

def test_donorV03_c(capfd, JA_indata43, JA_instat44):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata43 
    
    #synthetic input dataset - instatus
    instat = JA_instat44

    #print(instat['REC'])
    #### act
    try:
        #create Banff call
        donorimp_V03_3 = banff.donorimp(
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
            #seed=1,
            display_level=1,
            unit_id="REC",
            rand_num_var="randkey"
            )      
                
    except Exception as e:
        print("The V03_3 call had an unhandled exception")
        print(e)
    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV03_sas.txt"


    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0

def test_donorV03_d(capfd, JA_indata43, JA_instat44):
    #### arrange
    #synthetic input dataset - indata
    indata = JA_indata43 
    
    #synthetic input dataset - instatus
    instat = JA_instat44

    #print(instat['REC'])
    #### act 

    try:
        #create Banff call
        donorimp_V03_4 = banff.donorimp(
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
            seed=1,
            display_level=1,
            unit_id="REC",
            rand_num_var="randkey"
            )      
                
    except Exception as e:
        print("The V03_4 call had an unhandled exception")
        print(e)

    #### assert
    out, err = capfd.readouterr()
    sas_log_path = os.path.dirname(__file__) + "/output-sas/donorV03_sas.txt"


    assert out.count("ERROR") == 0
    assert out.count("WARNING") == 0

    