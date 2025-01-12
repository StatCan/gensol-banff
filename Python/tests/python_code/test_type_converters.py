import pytest
import banff
import banff.testing
import tempfile
import banff._common.src.io_util.type_converters as tc

def run_determin_ex01(indata, instatus, outdata=None):
    """helper function for calling determinsitic user guide example 1"""
    banff_call = banff.deterministic(
        indata=indata,
        instatus=instatus,
        outdata=outdata,
        edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
        accept_negative=True,
        unit_id="ident",
        by="prov",
        capture=None
    )

    return banff_call

@pytest.mark.m_auto_pass
def test_parquet_io(ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """validate that the funcion `PAT_from_parquet_file` works as expected"""
    # run proc, output to pyarrow object
    banff_call_pyarrow = run_determin_ex01(
        indata=ug_determin_ex1_indata,
        instatus=ug_determin_ex1_instatus,
        outdata="pyarrow",
    )

    print(banff_call_pyarrow.outdata)

    # save output to temporary file
    temp_dir = tempfile.TemporaryDirectory()
    temp_ds_path = f"{temp_dir.name}\\outdata_pyarrow.parquet"
    tc.PAT_to_parquet_file(
        dataset=banff_call_pyarrow.outdata,
        destination=temp_ds_path,
    )

    # default options
    outdata_from_file = tc.PAT_from_parquet_file(
        temp_ds_path,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using default options"

    # memory map enabled
    outdata_from_file = tc.PAT_from_parquet_file(
        temp_ds_path,
        memory_map=True,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using `memory_map=True`"

    # memory map disabled
    outdata_from_file = tc.PAT_from_parquet_file(
        temp_ds_path,
        memory_map=False,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using `memory_map=False`"

@pytest.mark.m_auto_pass
def test_feather_io(ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """validate that the funcion `PAT_from_feather_file` works as expected"""
    # run proc, output to pyarrow object
    banff_call_pyarrow = run_determin_ex01(
        indata=ug_determin_ex1_indata,
        instatus=ug_determin_ex1_instatus,
        outdata="pyarrow",
    )

    print(banff_call_pyarrow.outdata)

    # save output to temporary file
    temp_dir = tempfile.TemporaryDirectory()
    temp_ds_path = f"{temp_dir.name}\\outdata.feather"
    tc.PAT_to_feather_file(
        dataset=banff_call_pyarrow.outdata,
        destination=temp_ds_path,
    )

    # default options
    outdata_from_file = tc.PAT_from_feather_file(
        temp_ds_path, 
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using default options"

    # using IPC memory map
    outdata_from_file = tc.PAT_from_feather_file(
        temp_ds_path, 
        memory_map=tc.feather_mmap_mode.pa_ipc_mmap,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using `memory_map=tc.feather_mmap_mode.pa_ipc_mmap`"

    # using feather memory map
    outdata_from_file = tc.PAT_from_feather_file(
        temp_ds_path, 
        memory_map=tc.feather_mmap_mode.pa_feather_mmap,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using `memory_map=tc.feather_mmap_mode.pa_feather_mmap`"

    # no memory map
    outdata_from_file = tc.PAT_from_feather_file(
        temp_ds_path, 
        memory_map=tc.feather_mmap_mode.pa_feather_nommap,
    )
    assert banff_call_pyarrow.outdata.equals(outdata_from_file), "PAT_from_parquet_file failed using `memory_map=tc.feather_mmap_mode.pa_feather_nommap`"

@pytest.mark.m_auto_pass
def test_csv_io_pandas(ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """validate that CSV file functions (implemented with pandas) work as expected"""
    # run proc, output to pandas object
    banff_call = run_determin_ex01(
        indata=ug_determin_ex1_indata,
        instatus=ug_determin_ex1_instatus,
        outdata="pandas",
    )
    print(banff_call.outdata)

    temp_dir = tempfile.TemporaryDirectory()

    # TEST: round trip outdata and compare
    temp_ds_path = f"{temp_dir.name}\\outdata.csv"
    tc.DF_to_csv_file(
        dataset=banff_call.outdata,
        destination=temp_ds_path,
    )
    outdata_from_file = tc.DF_from_csv_file(
        temp_ds_path, 
    )
    print(outdata_from_file)
    assert banff_call.outdata.equals(outdata_from_file), "DataFrame round tripped to CSV not equal to original dataframe"

    # TEST: round trip indata, compare outdata to previous outdata
    temp_indata_path = f"{temp_dir.name}\\indata.csv"
    tc.DF_to_csv_file(
        dataset=ug_determin_ex1_indata,
        destination=temp_indata_path,
    )
    banff_call_2 = run_determin_ex01(
        indata=temp_indata_path,
        instatus=ug_determin_ex1_instatus,
        outdata="pandas",
    )
    assert banff_call_2.outdata.equals(banff_call.outdata), "round tripped indata creates different outdata"

@pytest.mark.m_auto_pass
def test_csv_io_pyarrow(ug_determin_ex1_indata, ug_determin_ex1_instatus):
    """validate that CSV file functions (implemented with pyarrow) work as expected"""
    # run proc, output to pyarrow object
    banff_call = run_determin_ex01(
        indata=ug_determin_ex1_indata,
        instatus=ug_determin_ex1_instatus,
        outdata="pyarrow",
    )
    print(banff_call.outdata)

    temp_dir = tempfile.TemporaryDirectory()

    # TEST: round trip outdata and compare
    temp_ds_path = f"{temp_dir.name}\\outdata.csv"
    tc.PAT_to_csv_file(
        dataset=banff_call.outdata,
        destination=temp_ds_path,
    )
    outdata_from_file = tc.PAT_from_csv_file(
        temp_ds_path, 
    )
    print(outdata_from_file)
    # NOTE: datatypes likely to differ, so we cast each with the other's schema and compare
    assert banff_call.outdata.equals(outdata_from_file.cast(banff_call.outdata.schema)), "pyarrow Table round tripped to CSV not equal to original table (original schema)"
    assert banff_call.outdata.cast(outdata_from_file.schema).equals(outdata_from_file), "pyarrow Table round tripped to CSV not equal to original table (round tripped schema)"

    # TEST: round trip indata, compare outdata to previous outdata
    temp_indata_path = f"{temp_dir.name}\\indata.csv"
    tc.PAT_to_csv_file(
        dataset=tc.DF_to_PAT(ug_determin_ex1_indata),
        destination=temp_indata_path,
    )
    banff_call_2 = run_determin_ex01(
        indata=temp_indata_path,
        instatus=ug_determin_ex1_instatus,
        outdata="pyarrow",
    )
    assert banff_call_2.outdata.equals(banff_call.outdata), "round tripped indata creates different outdata"


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()