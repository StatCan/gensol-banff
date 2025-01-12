# import necessary Python packages
import banff.testing as banfftest

def ceag_errorloc(capfd, extension='sas7bdat'):
    import banff
    banff.proc.BanffProcedure._reload_all_procs(debug=True)
    banff_call= banfftest.pytest_errorloc(
        indata=f"errorloc_ceag_a_indata.{extension}",
        seed=1,
        edits="""
            F47031_P3 >= -999999999999;
            F61054_SR2 >= -999999999999;
            F72000 >= -999999999999;
            F61054_SR10 >= -999999999999;
            F61054_SR92 >= -999999999999;
            F61054_SR1 >= -999999999999;
            F40005_SR2 >= -999999999999;
            F40005_SR10 >= -999999999999;
            F47031_P6 >= -999999999999;
            F47031_P2 >= -999999999999;
            F47031_P1 >= -999999999999;
            F47029_P141 >= -999999999999;
            F60001_SR1 >= -999999999999;
            F61054_SR18 >= -999999999999;
            F61054_SR17 >= -999999999999;
            F61054_SR16 >= -999999999999;
            F61054_SR15 > -999999999999;
            F61054_SR14 >= -999999999999;
            F61052_SR30 >= -999999999999;
            F61052_SR27 >= -999999999999;
            F61052_SR51 >= -999999999999;
            F61052_SR32 >= -999999999999;
            F61054_SR12 >= -999999999999;
            F61054_SR11 >= -999999999999;
            F61054_SR8 >= -999999999999;
            F61054_SR6 >= -999999999999;
            F61052_SR7 >= -999999999999;
            F61054_SR4 >= -999999999999;
            F61054_SR23 >= -999999999999;
            F61052_SR4 >= -999999999999;
            F61052_SR3 >= -999999999999;
            F61052_SR2 >= -999999999999;
            F60010 >= -999999999999;
            F51313_SR2 >= -999999999999;
            F40010 >= 0;
            F40005_SR4 >= -999999999999;
            F51317_SR1 >= -999999999999;
            F47031_P146 >= -999999999999;
            F47031_P7 >= -999999999999;
            F40005_SR3 >= -999999999999;
            F47031_P13 >= -999999999999;
            F40005_SR11 >= -999999999999;
            F47031_P12 >= -999999999999;
            F47031_P10 >= -999999999999;
            F47029_P26 > -999999999999;
            F47029_P25 >= -999999999999;
            F47029_P18 >= -999999999999;
            F47029_P16 >= -999999999999;
            F47029_P14 >= -999999999999;
            F47029_P15 >= -999999999999;
            F47031_P5 >= -999999999999;
            F47029_P4 >= -999999999999;
            F47029_P5 >= -999999999999;
            F47029_P8 >= -999999999999;
            F47029_P6 >= -999999999999;
            F47031_P4 >= -999999999999;
            F47029_P2 >= -999999999999;""",
        time_per_obs=5,
        accept_negative=True,
        unit_id='I43600',
        rand_num_var='RandomNbr_rand',
        by='P41_10_3 P41_10_3_CURR P51_24_3 P51_24_3_CURR',

        no_by_stats=True,
        trace=True,

        expected_outstatus=f"errorloc_ceag_a_outstatus.{extension}",
        expected_outreject=f"errorloc_ceag_a_outreject.{extension}",
        pytest_capture=capfd,
        expected_warning_count=None,
    )

def test_errorloc_ceag_sas(capfd):
    ceag_errorloc(capfd, 'sas7bdat')

def test_errorloc_ceag_feather(capfd):
    ceag_errorloc(capfd, 'feather')

def test_errorloc_ceag_parquet(capfd):
    ceag_errorloc(capfd, 'parquet')

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()