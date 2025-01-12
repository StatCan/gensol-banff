# import necessary Python packages
import banff.testing as banfftest

def ceag_estimato(capfd, extension='sas7bdat'):
    import banff
    banff.proc.BanffProcedure._reload_all_procs(debug=True)
    banff_call= banfftest.pytest_estimato(
        indata=f"estimato_ceag_a_indata.{extension}",
        indata_hist=f"estimato_ceag_a_indata_hist.{extension}",
        instatus=f"estimato_ceag_a_instatus.{extension}",
        inestimator=f"estimato_ceag_a_inestimator.{extension}",
        seed=1,
        accept_negative=True,
        unit_id='I43600',
        by='P41_10_3_CURR P51_24_3_CURR',

        no_by_stats=True,
        trace=True,

        pytest_capture=capfd,
        expected_warning_count=None,
        expected_outdata=f"estimato_ceag_a_outdata.{extension}",
        expected_outstatus=f"estimato_ceag_a_outstatus.{extension}",
        expected_outest_parm=f"estimato_ceag_a_outest_parm.{extension}",
    )

def test_estimato_ceag_sas(capfd):
    ceag_estimato(capfd, 'sas7bdat')

def test_estimato_ceag_feather(capfd):
    ceag_estimato(capfd, 'feather')

def test_estimato_ceag_parquet(capfd):
    ceag_estimato(capfd, 'parquet')

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()