# import necessary Python packages
import banff.testing as banfftest

def ceag_prorate(capfd, extension='sas7bdat'):
    import banff
    banff.proc.BanffProcedure._reload_all_procs(debug=True)
    banff_call= banfftest.pytest_prorate(
        indata=f"prorate_ceag_a_indata.{extension}",
        instatus=f"prorate_ceag_a_instatus.{extension}",
        edits="""
         B17026_Y8T130 + B17026_Y9_SR31T130 + B17026_Y9_SR32T130 + B17026_Y9_SR33T130 + B17026_Y9_SR34T130 = B17026_Y91T130;
         B17026_Y101_SR85 + B17026_Y10_SR85 = B17026_Y8T130;
         B17026_Y101_SR31 + B17026_Y10_SR31 = B17026_Y9_SR31T130;
         B17026_Y101_SR32 + B17026_Y10_SR32 = B17026_Y9_SR32T130;
         B17026_Y101_SR33 + B17026_Y10_SR33 = B17026_Y9_SR33T130;
         B17026_Y101_SR34 + B17026_Y10_SR34 = B17026_Y9_SR34T130;""",
        decimal=9,
        lower_bound=0,
        method='BASIC',
        modifier='ALWAYS',
        accept_negative=False,
        unit_id='I43600',

        no_by_stats=True,
        trace=True,
        expected_outdata=f"prorate_ceag_a_outdata.{extension}",
        expected_outstatus=f"prorate_ceag_a_outstatus.{extension}",
        expected_outreject=f"prorate_ceag_a_outreject.{extension}",

        pytest_capture=capfd,
        expected_warning_count=None,
    )

def test_prorate_ceag_sas(capfd):
    ceag_prorate(capfd, 'sas7bdat')

def test_prorate_ceag_feather(capfd):
    ceag_prorate(capfd, 'feather')

def test_prorate_ceag_parquet(capfd):
    ceag_prorate(capfd, 'parquet')

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()