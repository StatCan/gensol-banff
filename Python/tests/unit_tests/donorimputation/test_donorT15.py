#############################################################################################################
## Description: Conversion of SAS test case to Python: donorN01
## Command to run in CMD: python -m pytest test_donorT15.py
## Notes: Error occurred during execution of Banff's Errorlocalization module
## Last Updated: 2023-apr-24
#############################################################################################################
from banff.testing import assert_log_consistent
import os

# import Banff package
import banff
#import pandas as pd
import pandas as pd
import numpy as np
import pytest
@pytest.mark.m_incomplete # need to review
#@pytest.mark.m_performance_test
def test_donorT15_a():#capfd, data1_func, data2_func, JA_instat31):
    assert 1==0, "This test is incomplete"
    ''' TEST CASE DISABLED: will implement later
    #### arrange
    indata1 = data1_func

    def listToString(s):
        myStr = ','.join(map(str, s))
  
        return myStr
    
    indata2 = data2_func

    #synthetic input dataset - instatus
    instat = JA_instat31

    #### act
    try:
 
        errorloc_T15 = banff.errorloc(
            indata = listToString(indata1),
            outstatus = None,
            outreject = None,
            edits = """Q00>0;Q01>0;Q02>0;Q03>0;Q04>0;Q05>0;Q06>0;Q07>0;Q08>0;Q09>0;
                    Q10>0;Q11>0;Q12>0;Q13>0;Q14>0;Q15>0;Q16>0;Q17>0;Q18>0;Q19>0;
                    Q20>0;Q21>0;Q22>0;Q23>0;Q24>0;Q25>0;Q26>0;Q27>0;Q28>0;Q29>0;
                    Q30>0;Q31>0;Q32>0;Q33>0;Q34>0;Q35>0;Q36>0;Q37>0;Q38>0;Q39>0;
                    Q40>0;Q41>0;Q42>0;Q43>0;Q44>0;Q45>0;Q46>0;Q47>0;Q48>0;Q49>0;
                    Q50>0;Q51>0;Q52>0;Q53>0;Q54>0;Q55>0;Q56>0;Q57>0;Q58>0;Q59>0;
                    Q60>0;Q61>0;Q62>0;Q63>0;Q64>0;Q65>0;Q66>0;Q67>0;Q68>0;Q69>0;
                    Q70>0;Q71>0;Q72>0;Q73>0;Q74>0;Q75>0;Q76>0;Q77>0;Q78>0;Q79>0;""",
            unit_id="ident"
        )
        
        
    except Exception as e:
        print("The T15 (errorloc) call had an unhandled exception")
        print(e)    
    #### assert
    # no test case
    '''

def test_donorT15_b():#capfd, data1_func, data2_func, ):
    assert 1==0, "This test is incomplete"
    ''' TEST CASE DISABLED: will implement later
    #### arrange
    indata1 = data1_func
    indata = pd.DataFrame(indata1)
    indata2 = data2_func
    #synthetic input dataset - instatus
    instat = JA_instat31

    indata = indata.merge(pd.DataFrame(indata2))



    #### act
    try:
        #create Banff call
        donorimp_T15 = banff.donorimp(
                indata=indata,
                instatus = instat,
                outdata=None,
                outstatus=None,
                outdonormap=None, 
                edits="""Q00>0;Q01>0;Q02>0;Q03>0;Q04>0;Q05>0;Q06>0;Q07>0;Q08>0;Q09>0;
                        Q10>0;Q11>0;Q12>0;Q13>0;Q14>0;Q15>0;Q16>0;Q17>0;Q18>0;Q19>0;
                        Q20>0;Q21>0;Q22>0;Q23>0;Q24>0;Q25>0;Q26>0;Q27>0;Q28>0;Q29>0;
                        Q30>0;Q31>0;Q32>0;Q33>0;Q34>0;Q35>0;Q36>0;Q37>0;Q38>0;Q39>0;
                        Q40>0;Q41>0;Q42>0;Q43>0;Q44>0;Q45>0;Q46>0;Q47>0;Q48>0;Q49>0;
                        Q50>0;Q51>0;Q52>0;Q53>0;Q54>0;Q55>0;Q56>0;Q57>0;Q58>0;Q59>0;
                        Q60>0;Q61>0;Q62>0;Q63>0;Q64>0;Q65>0;Q66>0;Q67>0;Q68>0;Q69>0;
                        Q70>0;Q71>0;Q72>0;Q73>0;Q74>0;Q75>0;Q76>0;Q77>0;Q78>0;Q79>0;""",
                post_edits="""Q00>0;Q01>0;Q02>0;Q03>0;Q04>0;Q05>0;Q06>0;Q07>0;Q08>0;Q09>0;
                            Q10>0;Q11>0;Q12>0;Q13>0;Q14>0;Q15>0;Q16>0;Q17>0;Q18>0;Q19>0;
                            Q20>0;Q21>0;Q22>0;Q23>0;Q24>0;Q25>0;Q26>0;Q27>0;Q28>0;Q29>0;
                            Q30>0;Q31>0;Q32>0;Q33>0;Q34>0;Q35>0;Q36>0;Q37>0;Q38>0;Q39>0;
                            Q40>0;Q41>0;Q42>0;Q43>0;Q44>0;Q45>0;Q46>0;Q47>0;Q48>0;Q49>0;
                            Q50>0;Q51>0;Q52>0;Q53>0;Q54>0;Q55>0;Q56>0;Q57>0;Q58>0;Q59>0;
                            Q60>0;Q61>0;Q62>0;Q63>0;Q64>0;Q65>0;Q66>0;Q67>0;Q68>0;Q69>0;
                            Q70>0;Q71>0;Q72>0;Q73>0;Q74>0;Q75>0;Q76>0;Q77>0;Q78>0;Q79>0;""",
                min_donors=1,
                percent_donors=1,
                n=1,
                outmatching_fields=True, 
                eligdon="any",
                accept_negative=False,
                seed=1,
                unit_id="IDENT",
                must_match=("Q80","Q81", "Q82", "Q83", "Q84","Q85", "Q86", "Q87", "Q88","Q89", "Q90", "Q91", "Q92"),
                #data_excl_var= "TOEXCL"
                )                
    
    except Exception as e:
        print("The T15 (donorimputation) call had an unhandled exception")
        print(e)
        
    #### assert
    #no test case
    '''

''' taken from `conftest.py`

@pytest.fixture
def data1_func():
        n = 1
        while (n<=100000):
            header="IDENT='R'||put(1,Z1.)||put(n,Z7.)"
            Q00=round(pyranuni.PyRanuni.ranuni(3)*1000)-75
            Q01=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.025
            Q02=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q03=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-250
            Q04=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-5000
            Q05=Q01+Q02+Q03+Q04
            Q06=round(Q05/(pyranuni.PyRanuni.ranuni(2)*100),1)-250
            Q07=Q01*100+round(pyranuni.PyRanuni.ranuni(3)*100)
            Q08=Q02*100+round(pyranuni.PyRanuni.ranuni(3)*100)
            Q09=Q03*100+round(pyranuni.PyRanuni.ranuni(3)*100)
            Q10=round(pyranuni.PyRanuni.ranuni(3)*1000)-175
            Q11=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.25
            Q12=round(pyranuni.PyRanuni.ranuni(1)*100)-95
            Q13=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-750
            Q14=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q15=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q16=round((pyranuni.PyRanuni.ranuni(2)*100),1)
            Q17=round(pyranuni.PyRanuni.ranuni(3)*10000)-9000
            Q18=round(pyranuni.PyRanuni.ranuni(3)*1000)-50
            Q19=round(pyranuni.PyRanuni.ranuni(3)*100)-60
            Q20=round(pyranuni.PyRanuni.ranuni(3)*1000)-75
            Q21=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.25
            Q22=round(pyranuni.PyRanuni.ranuni(1)*100)-5
            Q23=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-950
            Q24=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q25=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q26=round((pyranuni.PyRanuni.ranuni(2)*100),1)-50
            Q27=round(pyranuni.PyRanuni.ranuni(3)*10000)-1000
            Q28=round(pyranuni.PyRanuni.ranuni(3)*1000)-500
            Q29=round(pyranuni.PyRanuni.ranuni(3)*100)-16
            Q30=round(pyranuni.PyRanuni.ranuni(3)*1000)-5
            Q31=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.025
            Q32=round(pyranuni.PyRanuni.ranuni(1)*100)-29
            Q33=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-750
            Q34=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q35=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q36=round((pyranuni.PyRanuni.ranuni(2)*100),1)
            Q37=round(pyranuni.PyRanuni.ranuni(3)*10000)-100
            Q38=round(pyranuni.PyRanuni.ranuni(3)*1000)-950
            Q39=round(pyranuni.PyRanuni.ranuni(3)*100)-10
            Q40=round(pyranuni.PyRanuni.ranuni(3)*1000)-700
            Q41=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.25
            Q42=round(pyranuni.PyRanuni.ranuni(1)*100)-25
            Q43=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-590
            Q44=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q45=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q46=round((pyranuni.PyRanuni.ranuni(2)*100),1)-50
            Q47=round(pyranuni.PyRanuni.ranuni(3)*10000)-8100
            Q48=round(pyranuni.PyRanuni.ranuni(3)*1000)-50
            Q49=round(pyranuni.PyRanuni.ranuni(3)*100)-1
            Q50=round(pyranuni.PyRanuni.ranuni(3)*1000)-750
            Q51=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.25
            Q52=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q53=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-15
            Q54=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q55=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q56=round((pyranuni.PyRanuni.ranuni(2)*100),1)-29
            Q57=round(pyranuni.PyRanuni.ranuni(3)*10000)-100
            Q58=round(pyranuni.PyRanuni.ranuni(3)*1000)-50
            Q59=round(pyranuni.PyRanuni.ranuni(3)*100)-89
            Q60=round(pyranuni.PyRanuni.ranuni(3)*1000)-750
            Q61=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.925
            Q62=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q63=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-50
            Q64=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-5000
            Q65=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q66=round((pyranuni.PyRanuni.ranuni(2)*100),1)-59
            Q67=round(pyranuni.PyRanuni.ranuni(3)*10000)-1000
            Q68=round(pyranuni.PyRanuni.ranuni(3)*1000)-500
            Q69=round(pyranuni.PyRanuni.ranuni(3)*100)-77
            Q70=round(pyranuni.PyRanuni.ranuni(3)*1000)-555
            Q71=round(pyranuni.PyRanuni.ranuni(2)*0.001)-0.25
            Q72=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q73=round(pyranuni.PyRanuni.ranuni(2)*1000,1)-509
            Q74=round(pyranuni.PyRanuni.ranuni(1)*10000,1)-500
            Q75=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)-15000
            Q76=round((pyranuni.PyRanuni.ranuni(2)*100),1)-95
            Q77=round(pyranuni.PyRanuni.ranuni(3)*10000)-9100
            Q78=round(pyranuni.PyRanuni.ranuni(3)*1000)-500
            Q79=round(pyranuni.PyRanuni.ranuni(3)*100)-99
            Q80=n
            Q81=n
            Q82=n
            Q83=n
            Q84=n
            Q85=n
            Q86=n
            Q87=n
            Q88=n
            Q89=n
            Q90=n
            Q91=n
            Q92=n
            Q93=round(pyranuni.PyRanuni.ranuni(2)*1000,1)
            Q94=round(pyranuni.PyRanuni.ranuni(1)*10000,1)
            Q95=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)
            Q96=round((pyranuni.PyRanuni.ranuni(2)*100),1)
            Q97=round(pyranuni.PyRanuni.ranuni(3)*10000)
            Q98=round(pyranuni.PyRanuni.ranuni(3)*1000)
            Q99=round(pyranuni.PyRanuni.ranuni(3)*100)
            Q100=round(pyranuni.PyRanuni.ranuni(3)*1000)
            Q101=round(pyranuni.PyRanuni.ranuni(2)*0.001)
            Q102=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q103=round(pyranuni.PyRanuni.ranuni(2)*1000,1)
            Q104=round(pyranuni.PyRanuni.ranuni(1)*10000,1)
            Q105=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)
            Q106=round((pyranuni.PyRanuni.ranuni(2)*100),1)
            Q107=round(pyranuni.PyRanuni.ranuni(3)*10000)
            Q108=round(pyranuni.PyRanuni.ranuni(3)*1000)
            Q109=round(pyranuni.PyRanuni.ranuni(3)*100)
            Q110=round(pyranuni.PyRanuni.ranuni(3)*1000)
            Q111=round(pyranuni.PyRanuni.ranuni(2)*0.001)
            Q112=round(pyranuni.PyRanuni.ranuni(1)*100)
            Q113=round(pyranuni.PyRanuni.ranuni(2)*1000,1)
            Q114=round(pyranuni.PyRanuni.ranuni(1)*10000,1)
            Q115=round(pyranuni.PyRanuni.ranuni(1)*1000000,1)
            Q116=round((pyranuni.PyRanuni.ranuni(2)*100),1)
            Q117=round(pyranuni.PyRanuni.ranuni(3)*10000)
            Q118=round(pyranuni.PyRanuni.ranuni(3)*1000)
            Q119=round(pyranuni.PyRanuni.ranuni(3)*100)
            n = n + 1
            data1 = np.array([
                header,
                Q00,
                Q01,
                Q02,
                Q03,
                Q04,
                Q05,
                Q06,
                Q07,
                Q08,
                Q09,
                Q10,
                Q11,
                Q12,
                Q13,
                Q14,
                Q15,
                Q16,
                Q17,
                Q18,
                Q19,
                Q20,
                Q21,
                Q22,
                Q23,
                Q24,
                Q25,
                Q26,
                Q27,
                Q28,
                Q29,
                Q30,
                Q31,
                Q32,
                Q33,
                Q34,
                Q35,
                Q36,
                Q37,
                Q38,
                Q39,
                Q40,
                Q41,
                Q42,
                Q43,
                Q44,
                Q45,
                Q46,
                Q47,
                Q48,
                Q49,
                Q50,
                Q51,
                Q52,
                Q53,
                Q54,
                Q55,
                Q56,
                Q57,
                Q58,
                Q59,
                Q60,
                Q61,
                Q62,
                Q63,
                Q64,
                Q65,
                Q66,
                Q67,
                Q68,
                Q69,
                Q70,
                Q71,
                Q72,
                Q73,
                Q74,
                Q75,
                Q76,
                Q77,
                Q78,
                Q79,
                Q80,
                Q81,
                Q82,
                Q83,
                Q84,
                Q85,
                Q86,
                Q87,
                Q88,
                Q89,
                Q90,
                Q91,
                Q92,
                Q93,
                Q94,
                Q95,
                Q96,
                Q97,
                Q98,
                Q99,
                Q100,
                Q101,
                Q102,
                Q103,
                Q104,
                Q105,
                Q106,
                Q107,
                Q108,
                Q109,
                Q110,
                Q111,
                Q112,
                Q113,
                Q114,
                Q115,
                Q116,
                Q117,
                Q118,
                Q119
            ], dtype=object)
            
            
        return data1


@pytest.fixture
def data2_func():
        n = 100001
        while (n<=200000):
            header = "IDENT='R'||put(1,Z1.)||put(n,Z7.)" 
            Q00=round(pyranuni.PyRanuni.ranuni(6)*1000) 
            Q01=round(pyranuni.PyRanuni.ranuni(3)*0.001) 
            Q02=round(pyranuni.PyRanuni.ranuni(5)*100) 
            Q03=round(pyranuni.PyRanuni.ranuni(27)*1000,1) 
            Q04=round(pyranuni.PyRanuni.ranuni(3)*10000,1) 
            Q05=Q01+Q02+Q03+Q04 
            Q06=round(Q05/(pyranuni.PyRanuni.ranuni(1)*100),1) 
            Q07=Q01*100+round(pyranuni.PyRanuni.ranuni(3)*100) 
            Q08=Q02*100+round(pyranuni.PyRanuni.ranuni(3)*100) 
            Q09=Q03*100+round(pyranuni.PyRanuni.ranuni(3)*100) 
            Q10=round(pyranuni.PyRanuni.ranuni(3)*1000) 
            Q11=round(pyranuni.PyRanuni.ranuni(3)*0.001) 
            Q12=round(pyranuni.PyRanuni.ranuni(1)*100) 
            Q13=round(pyranuni.PyRanuni.ranuni(2)*1000,1) 
            Q14=round(pyranuni.PyRanuni.ranuni(33)*10000,1) 
            Q15=round(pyranuni.PyRanuni.ranuni(18)*1000000,1) 
            Q16=round((pyranuni.PyRanuni.ranuni(96)*100),1) 
            Q17=round(pyranuni.PyRanuni.ranuni(33)*10000) 
            Q18=round(pyranuni.PyRanuni.ranuni(34)*1000) 
            Q19=round(pyranuni.PyRanuni.ranuni(7)*100) 
            Q20=round(pyranuni.PyRanuni.ranuni(389)*1000) 
            Q21=round(pyranuni.PyRanuni.ranuni(26)*0.001) 
            Q22=round(pyranuni.PyRanuni.ranuni(11)*100) 
            Q23=round(pyranuni.PyRanuni.ranuni(21)*1000,1) 
            Q24=round(pyranuni.PyRanuni.ranuni(18)*10000,1) 
            Q25=round(pyranuni.PyRanuni.ranuni(15)*1000000,1) 
            Q26=round((pyranuni.PyRanuni.ranuni(276)*100),1) 
            Q27=round(pyranuni.PyRanuni.ranuni(39)*10000) 
            Q28=round(pyranuni.PyRanuni.ranuni(32)*1000) 
            Q29=round(pyranuni.PyRanuni.ranuni(31)*100) 
            Q30=round(pyranuni.PyRanuni.ranuni(37)*1000) 
            Q31=round(pyranuni.PyRanuni.ranuni(28)*0.001) 
            Q32=round(pyranuni.PyRanuni.ranuni(18)*100) 
            Q33=round(pyranuni.PyRanuni.ranuni(25)*1000,1) 
            Q34=round(pyranuni.PyRanuni.ranuni(13)*10000,1) 
            Q35=round(pyranuni.PyRanuni.ranuni(17)*1000000,1) 
            Q36=round((pyranuni.PyRanuni.ranuni(32)*100),1) 
            Q37=round(pyranuni.PyRanuni.ranuni(33)*10000) 
            Q38=round(pyranuni.PyRanuni.ranuni(33)*1000) 
            Q39=round(pyranuni.PyRanuni.ranuni(33)*100) 
            Q40=round(pyranuni.PyRanuni.ranuni(35)*1000) 
            Q41=round(pyranuni.PyRanuni.ranuni(26)*0.001) 
            Q42=round(pyranuni.PyRanuni.ranuni(13)*100) 
            Q43=round(pyranuni.PyRanuni.ranuni(25)*1000,1) 
            Q44=round(pyranuni.PyRanuni.ranuni(16)*10000,1) 
            Q45=round(pyranuni.PyRanuni.ranuni(13)*1000000,1) 
            Q46=round((pyranuni.PyRanuni.ranuni(22)*100),1) 
            Q47=round(pyranuni.PyRanuni.ranuni(35)*10000) 
            Q48=round(pyranuni.PyRanuni.ranuni(35)*1000) 
            Q49=round(pyranuni.PyRanuni.ranuni(32)*100) 
            Q50=round(pyranuni.PyRanuni.ranuni(37)*1000) 
            Q51=round(pyranuni.PyRanuni.ranuni(23)*0.001) 
            Q52=round(pyranuni.PyRanuni.ranuni(14)*100) 
            Q53=round(pyranuni.PyRanuni.ranuni(28)*1000,1) 
            Q54=round(pyranuni.PyRanuni.ranuni(1)*10000,1) 
            Q55=round(pyranuni.PyRanuni.ranuni(1)*1000000,1) 
            Q56=round((pyranuni.PyRanuni.ranuni(2)*100),1) 
            Q57=round(pyranuni.PyRanuni.ranuni(8)*10000) 
            Q58=round(pyranuni.PyRanuni.ranuni(43)*1000) 
            Q59=round(pyranuni.PyRanuni.ranuni(323)*100) 
            Q60=round(pyranuni.PyRanuni.ranuni(33)*1000) 
            Q61=round(pyranuni.PyRanuni.ranuni(25)*0.001) 
            Q62=round(pyranuni.PyRanuni.ranuni(17)*100) 
            Q63=round(pyranuni.PyRanuni.ranuni(24)*1000,1) 
            Q64=round(pyranuni.PyRanuni.ranuni(12)*10000,1) 
            Q65=round(pyranuni.PyRanuni.ranuni(178)*1000000,1) 
            Q66=round((pyranuni.PyRanuni.ranuni(23)*100),1) 
            Q67=round(pyranuni.PyRanuni.ranuni(35)*10000) 
            Q68=round(pyranuni.PyRanuni.ranuni(35)*1000) 
            Q69=round(pyranuni.PyRanuni.ranuni(34)*100) 
            Q70=round(pyranuni.PyRanuni.ranuni(32)*1000) 
            Q71=round(pyranuni.PyRanuni.ranuni(27)*0.001) 
            Q72=round(pyranuni.PyRanuni.ranuni(13)*100) 
            Q73=round(pyranuni.PyRanuni.ranuni(29)*1000,1) 
            Q74=round(pyranuni.PyRanuni.ranuni(14)*10000,1) 
            Q75=round(pyranuni.PyRanuni.ranuni(13)*1000000,1) 
            Q76=round((pyranuni.PyRanuni.ranuni(12)*100),1) 
            Q77=round(pyranuni.PyRanuni.ranuni(39)*10000) 
            Q78=round(pyranuni.PyRanuni.ranuni(33)*1000) 
            Q79=round(pyranuni.PyRanuni.ranuni(39)*100) 
            Q80=n-10000 
            Q81=n-10000 
            Q82=n-10000 
            Q83=n-10000 
            Q84=n-10000 
            Q85=n-10000 
            Q86=n-10000 
            Q87=n-10000 
            Q88=n-10000 
            Q89=n-10000 
            Q90=n-10000 
            Q91=n-10000 
            Q92=n-10000 
            Q93=round(pyranuni.PyRanuni.ranuni(26)*1000,1) 
            Q94=round(pyranuni.PyRanuni.ranuni(13)*10000,1) 
            Q95=round(pyranuni.PyRanuni.ranuni(18)*1000000,1) 
            Q96=round((pyranuni.PyRanuni.ranuni(42)*100),1) 
            Q97=round(pyranuni.PyRanuni.ranuni(36)*10000) 
            Q98=round(pyranuni.PyRanuni.ranuni(32)*1000) 
            Q99=round(pyranuni.PyRanuni.ranuni(37)*100) 
            Q100=round(pyranuni.PyRanuni.ranuni(33)*1000) 
            Q101=round(pyranuni.PyRanuni.ranuni(72)*0.001) 
            Q102=round(pyranuni.PyRanuni.ranuni(219)*100) 
            Q103=round(pyranuni.PyRanuni.ranuni(22)*1000,1) 
            Q104=round(pyranuni.PyRanuni.ranuni(17)*10000,1) 
            Q105=round(pyranuni.PyRanuni.ranuni(11)*1000000,1) 
            Q106=round((pyranuni.PyRanuni.ranuni(4)*100),1) 
            Q107=round(pyranuni.PyRanuni.ranuni(356)*10000) 
            Q108=round(pyranuni.PyRanuni.ranuni(34)*1000) 
            Q109=round(pyranuni.PyRanuni.ranuni(33)*100) 
            Q110=round(pyranuni.PyRanuni.ranuni(38)*1000) 
            Q111=round(pyranuni.PyRanuni.ranuni(20)*0.001) 
            Q112=round(pyranuni.PyRanuni.ranuni(12)*100) 
            Q113=round(pyranuni.PyRanuni.ranuni(27)*1000,1) 
            Q114=round(pyranuni.PyRanuni.ranuni(12)*10000,1) 
            Q115=round(pyranuni.PyRanuni.ranuni(15)*1000000,1) 
            Q116=round((pyranuni.PyRanuni.ranuni(338)*100),1) 
            Q117=round(pyranuni.PyRanuni.ranuni(4)*10000) 
            Q118=round(pyranuni.PyRanuni.ranuni(8)*1000) 
            Q119=round(pyranuni.PyRanuni.ranuni(2)*100) 
            n = n + 1
        
            data2 = np.array([
               header,
               Q00,
               Q01,
               Q02,
               Q03,
               Q04,
               Q05,
               Q06,
               Q07,
               Q08,
               Q09,
               Q10,
               Q11,
               Q12,
               Q13,
               Q14,
               Q15,
               Q16,
               Q17,
               Q18,
               Q19,
               Q20,
               Q21,
               Q22,
               Q23,
               Q24,
               Q25,
               Q26,
               Q27,
               Q28,
               Q29,
               Q30,
               Q31,
               Q32,
               Q33,
               Q34,
               Q35,
               Q36,
               Q37,
               Q38,
               Q39,
               Q40,
               Q41,
               Q42,
               Q43,
               Q44,
               Q45,
               Q46,
               Q47,
               Q48,
               Q49,
               Q50,
               Q51,
               Q52,
               Q53,
               Q54,
               Q55,
               Q56,
               Q57,
               Q58,
               Q59,
               Q60,
               Q61,
               Q62,
               Q63,
               Q64,
               Q65,
               Q66,
               Q67,
               Q68,
               Q69,
               Q70,
               Q71,
               Q72,
               Q73,
               Q74,
               Q75,
               Q76,
               Q77,
               Q78,
               Q79,
               Q80,
               Q81,
               Q82,
               Q83,
               Q84,
               Q85,
               Q86,
               Q87,
               Q88,
               Q89,
               Q90,
               Q91,
               Q92,
               Q93,
               Q94,
               Q95,
               Q96,
               Q97,
               Q98,
               Q99,
               Q100,
               Q101,
               Q102,
               Q103,
               Q104,
               Q105,
               Q106,
               Q107,
               Q108,
               Q109,
               Q110,
               Q111,
               Q112,
               Q113,
               Q114,
               Q115,
               Q116,
               Q117,
               Q118,
               Q119,
            ], dtype=object)
        return data2

        
@pytest.fixture
def JA_instat31():
    return pd.read_csv(StringIO("""
        IDENT FIELDID STATUS
        REC01 Q3 IPR
        REC04 Q2 FTE
        REC04 Q3 FTI
        REC04 Q4 FTI
        """), sep=r'\s+')
'''