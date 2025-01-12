# import necessary Python packages
import banff

def ceag_estimato(extension="sas7bdat"):
    # enable C debug stats
    banff.proc.BanffProcedure._reload_all_procs(debug=True)

    banff_call= banff.estimato(
        indata=f"control_data/estimato_ceag_a_indata.{extension}",
        indata_hist=f"control_data/estimato_ceag_a_indata_hist.{extension}",
        instatus=f"control_data/estimato_ceag_a_instatus.{extension}",
        inestimator=f"control_data/estimato_ceag_a_inestimator.{extension}",
        seed=1,
        accept_negative=True,
        unit_id='I43600',
        by='P41_10_3_CURR P51_24_3_CURR',

        no_by_stats=True,
        trace=True,
    )