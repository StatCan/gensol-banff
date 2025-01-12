# import necessary Python packages
import banff.testing as banfftest
# create Banff call 

def ceag_donorimp(capfd, extension='sas7bdat'):
    import banff
    banff.proc.BanffProcedure._reload_all_procs(debug=True)
    banff_call= banfftest.pytest_donorimp(
        indata=f"donorimp_ceag_a_indata.{extension}",
        instatus=f"donorimp_ceag_a_instatus.{extension}",
        seed=1,
        edits="""
        B17026_Y10_SR33 >= 0;
            B17026_Y10_SR34 >= 0;
            B17026_Y10_SR32 >= 0;
            B17026_Y10_SR31 >= 0;
            B17026_Y10_SR85 >= 0;
            B17026_Y101_SR34 >= 0;
            B17026_Y101_SR33 >= 0;
            B17026_Y101_SR32 >= 0;
            B17026_Y101_SR31 >= 0;
            B17026_Y101_SR85 >= 0;
            B17026_Y91T130 >= 0;
            B17026_Y9_SR34T130 >= 0;
            B17026_Y9_SR33T130 >= 0;
            B17026_Y9_SR32T130 >= 0;
            B17026_Y9_SR31T130 >= 0;
            B17026_Y8T130 >= 0;
            B17026_Y101T130 >= 0;
            B17026_Y10T130 >= 0;""",
        post_edits="""
            B17026_Y10_SR33 >= 0;
            B17026_Y10_SR34 >= 0;
            B17026_Y10_SR32 >= 0;
            B17026_Y10_SR31 >= 0;
            B17026_Y10_SR85 >= 0;
            B17026_Y101_SR34 >= 0;
            B17026_Y101_SR33 >= 0;
            B17026_Y101_SR32 >= 0;
            B17026_Y101_SR31 >= 0;
            B17026_Y101_SR85 >= 0;
            B17026_Y91T130 >= 0;
            B17026_Y9_SR34T130 >= 0;
            B17026_Y9_SR33T130 >= 0;
            B17026_Y9_SR32T130 >= 0;
            B17026_Y9_SR31T130 >= 0;
            B17026_Y8T130 >= 0;
            B17026_Y10T130 >= 0;
            B17026_Y101T130 >= 0.1;
            B17026_Y91T130 >= 0.1;
            B17026_Y8T130 + B17026_Y9_SR31T130 + B17026_Y9_SR32T130 + B17026_Y9_SR33T130 + B17026_Y9_SR34T130 >= 0.1;""",
        min_donors=3,
        percent_donors=1,
        n=7000,
        eligdon="ANY",
        accept_negative=False, 
        outmatching_fields=True, 
        unit_id="I43600",
        must_match='B17026_y101T130',
        rand_num_var='RandomNbr_rand',
        data_excl_var='_EXCLUDE',
        by='P54_08_3 P51_21_3',

        no_by_stats=True,
        trace=True,

        pytest_capture=capfd,
        expected_warning_count=None,
        expected_outdata=f"donorimp_ceag_a_outdata.{extension}",
        expected_outstatus=f"donorimp_ceag_a_outstatus.{extension}",
        expected_outdonormap=f"donorimp_ceag_a_donormap.{extension}",
    )

def test_donorimp_ceag_sas(capfd):
    ceag_donorimp(capfd, 'sas7bdat')

def test_donorimp_ceag_feather(capfd):
    ceag_donorimp(capfd, 'feather')

def test_donorimp_ceag_parquet(capfd):
    ceag_donorimp(capfd, 'parquet')

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()