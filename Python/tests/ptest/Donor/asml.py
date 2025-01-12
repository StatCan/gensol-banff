# import necessary Python packages
import banff.testing as banfftest
# create Banff call 

def test_donorimp_userguide_01_a(capfd):
    """Donor Imputation User Guide example 1"""
    banff_call= banfftest.pytest_donorimp(
    indata=r"indata.sas7bdat",
    instatus=r"statusall_filtered.sas7bdat",
    seed=1,
    #presort=True,

    edits="""
    F43008 >= 0;
    F45801 >= 0;
    F45701 >= 0;
    F47101 >= 0;
    F47201 >= 0;
    F51101 >= 0;
    F51201 >= 0;
    F51301 >= 0;
    F61306 >= 0;
    F61501 >= 0;
    F61502 >= 0;
    F61601 >= 0;
    F61801 >= 0;
    F61802 >= 0;
    F61901 >= 0;
    F62001 >= 0;
    F62101 >= 0;
    F62201 >= 0;
    F62301 >= 0;
    F62401 >= 0;
    F62503 >= 0;
    F62504 >= 0;
    F62505 >= 0;
    F62511 >= 0;
    F62601 >= 0;
    F69101 >= 0;
    F61514 >= 0;
    F62506 >= 0;
    F62507 >= 0;
    F62508 >= 0;
    F62509 >= 0;""",
   min_donors=5,
   percent_donors=1,
   n=99,
   eligdon='ORIGINAL',
   random=True,
   accept_negative=False,
   outmatching_fields=True,
   
   unit_id='I43600',
   must_match='F60000 F40000',
   rand_num_var='RandomNbr_rand',
   data_excl_var='_EXCLUDE',
   by='P54_12_3 P54_09_3 P54_06_3',
   trace=True,
   #capture=None,
   presort=True,

                pytest_capture=capfd,
                expected_warning_count=None,
                expected_outdata=r"outdata.sas7bdat",
                expected_outstatus=r"outstatus.sas7bdat",
                expected_outdonormap=r"donormap.sas7bdat",
            )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()