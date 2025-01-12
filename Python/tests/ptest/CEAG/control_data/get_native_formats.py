import os
import banff as b

def get_native_formats(files = [], sort_by=None):
    for f in files:
        fn = os.path.splitext(f)[0] # get extensionless filename
        foo = b.io_util.type_converters.DF_from_sas_file(f)
        if sort_by is not None:
            try:
                foo = foo.sort_values(by=sort_by)
            except:
                pass
        b.io_util.type_converters.DF_to_feather_file(foo, f"{fn}.feather")
        b.io_util.type_converters.DF_to_parq_file(foo, f"{fn}.parquet")

get_native_formats(
    sort_by=['P41_10_3_CURR', 'P51_24_3_CURR'],
    files=[
        "estimato_ceag_a_indata.sas7bdat",
        "estimato_ceag_a_indata_hist.sas7bdat",
        "estimato_ceag_a_inestimator.sas7bdat",
        "estimato_ceag_a_instatus.sas7bdat",
        "estimato_ceag_a_outdata.sas7bdat",
        "estimato_ceag_a_outest_parm.sas7bdat",
        "estimato_ceag_a_outstatus.sas7bdat",
    ], 
)

get_native_formats(
    sort_by=['P54_08_3', 'P51_21_3'],
    files=[
        "donorimp_ceag_a_donormap.sas7bdat",
        "donorimp_ceag_a_indata.sas7bdat",
        "donorimp_ceag_a_instatus.sas7bdat",
        "donorimp_ceag_a_outdata.sas7bdat",
        "donorimp_ceag_a_outstatus.sas7bdat",
    ], 
)

get_native_formats(
    sort_by=['P41_10_3', 'P41_10_3_CURR', 'P51_24_3', 'P51_24_3_CURR'],
    files=[
        "errorloc_ceag_a_indata.sas7bdat",
        "errorloc_ceag_a_outreject.sas7bdat",
        "errorloc_ceag_a_outstatus.sas7bdat",
    ], 
)

get_native_formats(
    sort_by=['I43600'],
    files=[
        "prorate_ceag_a_indata.sas7bdat",
        "prorate_ceag_a_instatus.sas7bdat",
        "prorate_ceag_a_outdata.sas7bdat",
        "prorate_ceag_a_outreject.sas7bdat",
        "prorate_ceag_a_outstatus.sas7bdat",
    ], 
)
