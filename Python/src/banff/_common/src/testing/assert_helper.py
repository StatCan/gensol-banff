import io  # for printing: creating StringIO buffer
import re  # preprocessing console log
import sys
from pathlib import Path

import numpy as np  # referring to types, like `np.int64`
import pandas as pd
import pyarrow as pa
import pyarrow.compute as pc

from .._log import get_misc_logger
from ..io_util import (
    load_input_dataset,
)
from ..io_util.io_util import (
    interm_to_DF,
    interm_to_PAT,
)
from ..io_util.processing import (
    handle_arrow_string_data,
)

# dictionary of keyword arguments passed in each procedure call
_default_proc_kwargs = {}

def assert_datasets_equal(ds_list, round_data=None, drop_columns=None):
    """Assert that list of dataset pairs are equal.

    takes a list of dataset pairs (`[test_dataset, control_dataset]`) and comparison options,
    calls `assert_dataset_equal()` on each dataset pair in the list.
        `ds_list = [[<test_ds_1, control_ds_1], [<test_ds2, control_ds2], ...]`
         - *test_dataset* must be correspond to the procedure call's `_<dataset-name>` member (of type `..io_util.GensysDataset`).
         - *control_dataset* must be a type supported by `assert_dataset_equal(control_dataset=)`
    # Override options
        - To override option parameters for a specific dataset pair, specify a dictionary as a third member in that pair
            - for example, to override the `drop_columns` use `[test_dataset, control_dataset, {'drop_columns':None}]`
    """
    # create options dictionary using function parameters
    options_parm = {
        "round_data"    : round_data,
        "drop_columns"  : drop_columns,
    }

    # process each dataset pair
    for ds_item in ds_list:
        # extract datasets
        ds_test = ds_item[0]
        ds_control = ds_item[1]

        if ds_control is None:
            continue  # not specified by user, go to next pair

        # validate dataset types
        allowed_types = (pd.DataFrame, pa.Table)
        if (not isinstance(ds_control, allowed_types)
            or not isinstance(ds_test.user_output, allowed_types)
        ):
            print(f"control or test dataset not of a valid type, skipping {ds_test.name}")
            print(f"  allowed types: {allowed_types}")
            print(f"  control dataset type: {type(ds_control)}")
            print(f"  test dataset type: {type(ds_test.user_output)}")
            mesg = "assert_datasets_equal(): received invalid dataset format"
            raise TypeError(mesg)

        # build options dictionary
        options_run = options_parm.copy()
        if len(ds_item) >= 3: # optional third member is dict of parameter overrides
            options_run.update(ds_item[2])

        assert_dataset_equal(
            round_data      = options_run["round_data"],
            drop_columns    = options_run["drop_columns"],
            dataset_name    = ds_test.name,
            test_dataset    = ds_test.user_output,
            control_dataset = ds_control,
        )

def assert_dataset_equal(
    test_dataset,
    control_dataset,
    dataset_name,
    sort_columns=True,
    sort_values=True,
    round_data=None,
    convert_columns=True,
    drop_columns=None,
    compare_with=None,
):
    """Check that test and control datasets are sufficiently equal.

    Handles common issues such as
        - empty datasets
        - some type mismatches
        - different column sort order
        - float precision issues

    `dataset_name` : str
        used in print statements

    `sort_columns` : bool
        sort both dataset's columns before comparison

    `sort_values` : bool
        sort values of all columns in both dataset's before comparison

    `round_data` : int | None
        If integer, round floating point values to `round_data` decimal places

    `convert_columns` : bool
        convert integer columns to floating point columns

    `drop_columns` : str | list of str
        drop these columns, if found, on control datasets prior to comparison

    `compare_with` : None | `pandas.DataFrame` | `pyarrow.Table`
        Convert test and control datasets to the specified format for comparison.
        When unspecified (or `None`) use the the type that `test_dataset` uses.
    """
    print(f"assert_dataset_equal(dataset_name={dataset_name})")

    # determine format/library to use for comparison
    if compare_with is None:
        if isinstance(test_dataset, pd.DataFrame):
            compare_with = pd.DataFrame
        elif isinstance(test_dataset, pa.Table):
            compare_with = pa.Table

    print(f"  comparing as {compare_with}")
    try:
        if compare_with is pa.Table:
            # convert test and control datasets to proper format
            test_dataset = interm_to_PAT(test_dataset)
            control_dataset = interm_to_PAT(control_dataset)

            # set both dataset's string data to same type
            test_dataset=handle_arrow_string_data(test_dataset)
            control_dataset=handle_arrow_string_data(control_dataset)

            # handle parameter: drop_columns
            if isinstance(drop_columns, (str, list)):
                # promote string to list of strings
                if isinstance(drop_columns, str):
                    drop_columns=drop_columns.split()

                print(f"   drop columns: {drop_columns}")

                # remove columns
                for drop_col in drop_columns:
                    drop_col = drop_col.upper()
                    for in_col in control_dataset.column_names:
                        if in_col.upper() == drop_col:
                            print(f"     dropped column '{in_col}' from control dataset")
                            control_dataset = control_dataset.drop_columns(columns=in_col)
                    for in_col in test_dataset.column_names:
                        if in_col.upper() == drop_col:
                            print(f"     dropped column '{in_col}' from test dataset")
                            test_dataset = test_dataset.drop_columns(columns=in_col)

            # handle parameter: round data
            if isinstance(round_data, int):
                def round_pat(pat):
                    new_cols = []
                    for col in pat.columns:
                        if pa.types.is_floating(col.type):
                            new_cols.append(pc.round(col, ndigits=round_data))
                        else:
                            new_cols.append(col)
                    pat = pa.table(data=new_cols, schema=pat.schema)
                    return pat
                test_dataset = round_pat(test_dataset)
                control_dataset = round_pat(control_dataset)

            # handle parameter: sort_columns
            if sort_columns:
                print("   sort columns: True")
                # sort columns
                test_dataset = test_dataset.select(sorted(test_dataset.column_names))
                control_dataset = control_dataset.select(sorted(control_dataset.column_names))

            # handle parameter: sort_values
            if sort_values:
                print("   sort values: True")
                # sort values
                sort_keys_test = [(name, "ascending") for name in test_dataset.column_names]
                sort_indices_test = pc.sort_indices(test_dataset, sort_keys_test)
                test_dataset = pc.take(test_dataset, sort_indices_test)

                sort_keys_control = [(name, "ascending") for name in control_dataset.column_names]
                sort_indices_control = pc.sort_indices(control_dataset, sort_keys_control)
                control_dataset = pc.take(control_dataset, sort_indices_control)

            # test for equality
            assert test_dataset.equals(control_dataset), f"assert_dataset_equal: {dataset_name} dataset does not match expected data"
        elif compare_with is pd.DataFrame:
            # convert test and control datasets to proper format
            test_dataset = interm_to_DF(test_dataset)
            control_dataset = interm_to_DF(control_dataset)
            # handle parameter: drop_columns
            if isinstance(drop_columns, (str, list)):
                # promote string to list of strings
                if isinstance(drop_columns, str):
                    drop_columns=drop_columns.split()

                print(f"   drop columns: {drop_columns}")

                # remove columns
                for drop_col in drop_columns:
                    drop_col = drop_col.upper()
                    for in_col in control_dataset.columns.to_list():
                        if in_col.upper() == drop_col:
                            print(f"     dropped column '{in_col}' from control dataset")
                            control_dataset.drop(columns=in_col, inplace=True)
                    for in_col in test_dataset.columns.to_list():
                        if in_col.upper() == drop_col:
                            print(f"     dropped column '{in_col}' from test dataset")
                            test_dataset.drop(columns=in_col, inplace=True)

            # handle parameter: sort_columns
            if sort_columns:
                print("   sort columns: True")
                test_dataset = test_dataset.reindex(sorted(test_dataset.columns), axis=1)
                control_dataset = control_dataset.reindex(sorted(control_dataset.columns), axis=1)

            # handle parameter: sort_values
            if sort_values:
                print("   sort values: True")
                try:
                    test_dataset_s = test_dataset.transform(np.sort)
                    control_dataset_s = control_dataset.transform(np.sort)
                    # only overwrite original if both sorted without exception
                    test_dataset = test_dataset_s
                    control_dataset = control_dataset_s
                except Exception: # noqa: BLE001
                    print("     unable to sort values, continuing with original sort order")

            if test_dataset.empty and control_dataset.empty:
                print("   both datasets are empty")
                assert test_dataset.columns.equals(control_dataset.columns), f"assert_dataset_equal: {dataset_name} datasets both empty, but columns differ"
            else:
                # set both dataset's numeric data to same type
                if convert_columns:
                    for ds in [test_dataset, control_dataset]:
                        # find integer columns
                        int_columns = ds.select_dtypes(np.int64).columns
                        # convert them to float columns
                        ds[int_columns] = ds[int_columns].astype(np.float64)

                # handle parameter: round data
                if isinstance(round_data, int):
                    print("   round data:    {} decimal places".format(round_data))
                    test_dataset = test_dataset.round(decimals=round_data)
                    control_dataset = control_dataset.round(decimals=round_data)

                assert test_dataset.equals(control_dataset), f"assert_dataset_equal: {dataset_name} dataset does not match expected data"

            print("   datasets equal: True")
    except Exception:
        print("   datasets equal: False")
        print("\n   ************** Inspecting Dataset Differences: START *********************")
        inspect_dataset_difference(test_dataset=test_dataset, control_dataset=control_dataset)
        print("   *************** Inspecting Dataset Differences: END **********************\n")
        raise

def assert_dataset_value(dataset=None, dataset_name="", row_num=None, col_name=None, expected_values=None):
    """Validate specific dataset value against set of expected values.

    Specity the row number (int, 0-index) and column name (str)
    a list ['of', 'expected', 'values']
    """
    print("assert_dataset_value(...)")
    print(f"   dataset:   '{dataset_name}' (row {row_num}, column '{col_name}')")
    print(f"   expecting: {expected_values}")

    # ensure dataset in `pandas.DataFrame` format
    dataset = interm_to_DF(dataset)

    actual_value = dataset.at[row_num, col_name]
    print(f"   found: {actual_value}")
    assert actual_value in expected_values, f"assert_dataset_value: unexpected value found: {actual_value}"

def assert_log_contains(msg, test_log, clean_whitespace=False):
    """Check for `msg` in `test log`.

    Assert that it is found
    See `assert_log_consistent` for more details
    """
    print(f"assert_log_contains(clean_whitespace={clean_whitespace}, ...)")
    print(f"   searching for:     {msg}")

    # preprocess inputs
    if clean_whitespace:
        msg_P = preprocess_input(msg)
        test_log_P = preprocess_input(test_log)
    else:
        msg_P = msg
        test_log_P = test_log
    # replace '\r\n' with '\n'
    msg_P = msg_P.replace("\r\n", "\n")
    test_log_P = test_log_P.replace("\r\n", "\n")

    # perform the check
    in_test = msg_P in test_log_P
    print(f"   found in test log: {in_test}")

    assert in_test, f"assert_log_contains: could not find '{msg}'"

def assert_log_consistent(msg, test_log, sas_log_path, must_exist=False, clean_whitespace=False):
    """Check for `msg` in `test_log` and in log file found at `sas_log_path`.

    assert that it is either present in both, or not present in both
    i.e. assert fails if there's an inconsistency w.r.t. presence

    Tighten the check using `must_exist=True` - assert also fails if `msg` not found in the SAS log
    """
    Warning(f"`assert_log_consistent` deprecated, using assert_log_contains, ignoring must_exist ({must_exist}) and sas_log_path ({sas_log_path})")
    return assert_log_contains(msg=msg, test_log=test_log, clean_whitespace=clean_whitespace)

def assert_substr_count_consistent(substr_to_count, test_log, sas_log_path):
    """Check count of `substr_to_count` in `test_log` and.

    in the log file found at `sas_log_path`.
    asserts that the counts are equal
    """
    print(f"searching for: {substr_to_count}")

    sas_log = read_log_file(sas_log_path)
    sas_log_count = sas_log.count(substr_to_count)
    test_log_count = test_log.count(substr_to_count)
    print(f"   sas_log_count: {sas_log_count}")
    print(f"   test_log_count: {test_log_count}")

    mesg = f"assert_substr_count_consistent: inconsistent count of '{substr_to_count}', TEST has {test_log_count}, SAS has {sas_log_count}"
    assert sas_log_count == test_log_count, mesg

def assert_substr_count(substr_to_count="ERROR:", test_log=None, expected_count = 0):
    """Calculate count of `substr_to_count` in `test_log`.

    Asserts that it is equal to `expected_count`
    """
    print("assert_substr_count(...)")
    print(f"   searching for:   {substr_to_count}")
    print(f"   expected_count:  {expected_count}")

    if test_log is None:
        mesg = "test_log must be non-empty `str`"
        raise ValueError(mesg)

    test_count = test_log.count(substr_to_count)
    print(f"   test_log count:  {test_count}")

    # the following two lines collectively assert that `test_count == expected_count`, while allowing specific error messaging
    assert test_count >= expected_count, f"assert_substr_count: found {expected_count - test_count} fewer than expected of '{substr_to_count}'"
    assert test_count <= expected_count, f"assert_substr_count: found {test_count - expected_count} more than expected of '{substr_to_count}'"

def get_console_log(pytest_capture):
    return pytest_capture.readouterr()[0]

def get_control_dataset_path(dataset_name=None, depth=2):
    """Prepend path to `control_data` directory to `dataset_name` if appropriate.

    In the case where a filename (without path) is provided, and that file exists
    in the `control_data` folder, the path to that file is returned.
    Otherwise, the original value is returned.

    `depth` - varies depending on the stack depth from the original test file.
    """
    if not isinstance(dataset_name, str):
        return dataset_name

    control_path = Path(sys._getframe(depth).f_code.co_filename).parent / "control_data" / dataset_name  # noqa: SLF001 # don't mind this sketchy code

    if control_path.exists():
        print(f"{__package__}: dataset '{dataset_name}' found in control_data folder, will load file at path '{control_path}'")
        return str(control_path)

    return dataset_name

def get_log_path(sas_log_name, stack_level=2):
    """Given a filename, determine the full path.

    Looks `stack_level` levels up in the call stack, determines function's file path,
    generates path to sas log files
    """
    return str(Path(sys._getframe(stack_level).f_code.co_filename).parent / "output-sas" / sas_log_name)  # noqa: SLF001 # don't mind this sketchy code

def inspect_dataset_difference(test_dataset=None, control_dataset=None):
    """Inspect two datasets and print summary of various differences, including.

    - overall equality
    - number of rows
    - number of columns
    - different sets of columns (names)
    - different values
    - different datatypes
    """
    # print info and contents of both datasets, then try and compare them
    indent="     "
    if type(test_dataset) is not type(control_dataset):
        print(f"{indent}unable to inspect difference, test and control datasets are not the same type")
        print(f"{indent}  test dataset type: {type(test_dataset)}")
        print(f"{indent}  control dataset type: {type(control_dataset)}")
        return

    if isinstance(test_dataset, pa.Table):
        # INSPECT: number of rows
        test_row_count = test_dataset.num_rows
        cntl_row_count = control_dataset.num_rows
        row_count_equal = test_row_count == cntl_row_count

        if row_count_equal:
            print(f"{indent}number of rows equal: True")
        else:
            print(f"{indent}number of rows equal: False")
            print(f"{indent}  number of control (expected) rows: {cntl_row_count}")
            print(f"{indent}  number of test (actual) rows:      {test_row_count}\n")
        print() # blank line


        # INSPECT: number of columns
        test_col_count = test_dataset.num_columns
        cntl_col_count = control_dataset.num_columns
        col_count_equal = test_col_count == cntl_col_count

        if col_count_equal:
            print(f"{indent}number of columns equal: True")
        else:
            print(f"{indent}number of columns equal: False")
            print(f"{indent}  number of control (expected) columns: {cntl_col_count}")
            print(f"{indent}  number of test (actual) columns:      {test_col_count}")
        print() # blank line


        # INSPECT: names of columns
        test_col_set = set(test_dataset.column_names)
        cntl_col_set = set(control_dataset.column_names)
        symmetric_difference = test_col_set ^ cntl_col_set
        col_names_identical = len(symmetric_difference) == 0
        if col_names_identical:
            print(f"{indent}column names equal: True")
            print(f"{indent}{indent}{test_col_set}")
        else:
            print(f"{indent}column names equal: False")

            # control only
            if len(cntl_col_set - test_col_set) > 0:
                print(f"{indent}  columns only on control (expected) dataset:\n{indent}{indent}{cntl_col_set - test_col_set}")
            else:
                print(f"{indent}  columns only on control (expected) dataset:\n{indent}{indent}{{}}")

            # test only
            if len(test_col_set - cntl_col_set) > 0:
                print(f"{indent}  columns only on test (actual) dataset:\n{indent}{indent}{test_col_set - cntl_col_set}")
            else:
                print(f"{indent}  columns only on test (actual) dataset:\n{indent}{indent}{{}}")

            # common columns
            print(f"{indent}  columns common to both datasets:\n{indent}{indent}{test_col_set.intersection(cntl_col_set)}")

            return # no point in continuing to inspect if the names don't match
        print() # blank line

        # only continue comparison if all equal so far
        if not(row_count_equal and col_count_equal and col_names_identical):
            return

        # INSPECT: `pandas.DataFrame.compare()`
        try:
            print(f"{indent}Temporarily converting to pandas dataframe for value comparison")
            test_pandas = interm_to_DF(test_dataset)
            control_pandas = interm_to_DF(control_dataset)
            with pd.option_context(
                "display.max_rows",     5000,
                "display.max_columns",  5000,
                "display.width",        500,
                "display.precision",    20,
            ):
                diff = test_pandas.compare(control_pandas, result_names=("test", "control"))
                print(f"{indent}The following values differ (printing output of pandas `.compare()`):")
                print(indent + diff.to_string().replace("\n", "\n     "))
        except Exception as e:  # noqa: BLE001 # in the .testing subpackage, we don't care
            print(f"{indent}unable to compare datasets using pandas `.compare()`:\n{indent}  {e}")
            # don't raise
        print() # blank line


        # INSPECT datatypes
        if test_dataset.schema.equals(control_dataset.schema):
            print(f"{indent}datatypes equal: True")
        else:
            print(f"{indent}datatypes equal: False")
            try:
                test_type_set = set(test_dataset.schema)
                control_type_set = set(control_dataset.schema)

                # control only
                if len(control_type_set - test_type_set) > 0:
                    print(f"{indent}  column types only on control (expected) dataset:\n{indent}{indent}{control_type_set - test_type_set}")
                else:
                    print(f"{indent}  column types only on control (expected) dataset:\n{indent}{indent}{{}}")

                # test only
                if len(test_type_set - control_type_set) > 0:
                    print(f"{indent}  column types only on test (actual) dataset:\n{indent}{indent}{test_type_set - control_type_set}")
                else:
                    print(f"{indent}  column types only on test (actual) dataset:\n{indent}{indent}{{}}")
            except Exception as e:  # noqa: BLE001 # in the .testing subpackage, we don't care
                print(f"{indent}unable to compare datatypes:\n{indent}  {e}")
                # INSPECT: `pandas.DataFrame.info()`
                for ds, ds_name in [(test_dataset, "test"), (control_dataset, "control")]:
                    print(f"   printing {ds_name} dataset:")
                    print(ds)
                    print()
            else:
                return
    elif isinstance(test_dataset, pd.DataFrame):
        # INSPECT: number of rows
        test_row_count = test_dataset.shape[0]
        cntl_row_count = control_dataset.shape[0]
        row_count_equal = test_row_count == cntl_row_count

        if row_count_equal:
            print(f"{indent}number of rows equal: True")
        else:
            print(f"{indent}number of rows equal: False")
            print(f"{indent}  number of control (expected) rows: {cntl_row_count}")
            print(f"{indent}  number of test (actual) rows:      {test_row_count}\n")
        print() # blank line


        # INSPECT: number of columns
        test_col_count = test_dataset.shape[1]
        cntl_col_count = control_dataset.shape[1]
        col_count_equal = test_col_count == cntl_col_count

        if col_count_equal:
            print(f"{indent}number of columns equal: True")
        else:
            print(f"{indent}number of columns equal: False")
            print(f"{indent}  number of control (expected) columns: {cntl_col_count}")
            print(f"{indent}  number of test (actual) columns:      {test_col_count}")
        print() # blank line


        # INSPECT: names of columns
        test_col_set = set(test_dataset.columns.to_list())
        cntl_col_set = set(control_dataset.columns.to_list())
        symmetric_difference = test_col_set ^ cntl_col_set
        col_names_identical = len(symmetric_difference) == 0
        if col_names_identical:
            print(f"{indent}column names equal: True")
            print(f"{indent}{indent}{test_col_set}")
        else:
            print(f"{indent}column names equal: False")

            # control only
            if len(cntl_col_set - test_col_set) > 0:
                print(f"{indent}  columns only on control (expected) dataset:\n{indent}{indent}{cntl_col_set - test_col_set}")
            else:
                print(f"{indent}  columns only on control (expected) dataset:\n{indent}{indent}{{}}")

            # test only
            if len(test_col_set - cntl_col_set) > 0:
                print(f"{indent}  columns only on test (actual) dataset:\n{indent}{indent}{test_col_set - cntl_col_set}")
            else:
                print(f"{indent}  columns only on test (actual) dataset:\n{indent}{indent}{{}}")

            # common columns
            print(f"{indent}  columns common to both datasets:\n{indent}{indent}{test_col_set.intersection(cntl_col_set)}")

            return # no point in continuing to inspect if the names don't match
        print() # blank line

        # only continue comparison if all equal so far
        if not(row_count_equal and col_count_equal and col_names_identical):
            return

        # INSPECT: `pandas.DataFrame.compare()`
        try:
            with pd.option_context(
                "display.max_rows",     5000,
                "display.max_columns",  5000,
                "display.width",        500,
                "display.precision",    20,
            ):
                diff = test_dataset.compare(control_dataset, result_names=("test", "control"))
                print(f"{indent}The following values differ (printing output of pandas `.compare()`):")
                print(indent + diff.to_string().replace("\n", "\n     "))
        except Exception as e:  # noqa: BLE001 # in the .testing subpackage, we don't care
            print(f"{indent}unable to compare datasets using pandas `.compare()`:\n{indent}  {e}")
            # don't raise
        print() # blank line


        # INSPECT datatypes
        if test_dataset.dtypes.equals(control_dataset.dtypes):
            print(f"{indent}datatypes equal: True")
        else:
            print(f"{indent}datatypes equal: False")
            try:
                diff = test_dataset.dtypes.compare(control_dataset.dtypes, result_names=("test", "control"))
                print(f"{indent}  comparing datatypes")
                print(indent + diff.to_string().replace("\n", f"\n{indent}"))
            except Exception as e:  # noqa: BLE001 # in the .testing subpackage, we don't care
                print(f"{indent}unable to compare datatypes automatically using pandas `.dtypes.compare()`:\n{indent}  {e}")
                # INSPECT: `pandas.DataFrame.info()`
                for ds, ds_name in [(test_dataset, "test"), (control_dataset, "control")]:
                    print(f"   printing pandas `.info()` for {ds_name} dataset:")
                    print_dataset_info(dataset=ds, indent=f"{indent}  ", print_output=True)
                    print()
            else:
                return

    print() # blank line

def load_control_dataset(ds_ref, depth=4):
    """Load `ds_ref` using the normal load function.  First however, process `ds_ref`.

    to search *control_data* folder
    """
    if ds_ref is None:
        return ds_ref

    return load_input_dataset(get_control_dataset_path(ds_ref, depth=depth), get_misc_logger())

def preprocess_input(str_in):
    str_out = str_in
    # remote whitespace
    str_out = re.sub(r"\s+", " ", str_out)
    # replace  "text..........:" => "text.:"
    str_out = re.sub(r"(\.)(\.)+:", ".:", str_out)
    return str_out

def print_dataset_contents_verbose(dataset=None, indent="", print_output=True):
    """Print a pandas dataframe with many rows, columns, and much precision.

    Returns a string with the printed dataset.
    Prints string by default, specify `print_dataset=False` to suppress printing.
    Specify an `indent="   "` to prepend spaces (or any text) to each line of output.
    """
    with pd.option_context(
        "display.max_rows",     5000,
        "display.max_columns",  5000,
        "display.width",        500,
        "display.precision",    20,
    ):
        # print dataset with indent
        temp_str = indent + dataset.to_string().replace("\n", f"\n{indent}")
        if print_output:
            print(temp_str)

        return temp_str

def print_dataset_info(dataset=None, indent="", print_output=True):
    temp_buff=io.StringIO()
    dataset.info(buf=temp_buff, memory_usage=False, show_counts=False)
    out_str = indent + temp_buff.getvalue().replace("\n", f"\n{indent}")
    if print_output:
        print(out_str)
    return out_str

def read_log_file(log_path):
    if Path(log_path).exists():
        mesg = f"log file does not exist: {log_path}"
        raise FileNotFoundError(mesg)

    with open(log_path, "r") as log_file:  # noqa: PTH123  # No thanks, I'll use `open()`
        log_content= log_file.read()
        return log_content

def run_standard_assertions(expect_zero_rc=True, rc=None, python_log=None,
                          sas_log_path=None, msg_list_sas=None, msg_list_sas_exact=None,
                          msg_list_contains=None, msg_list_contains_exact=None,
                          expect_error_count=None, expect_warning_count=None,
                          ds_compare_list=None, round_data=None, drop_columns=None,
                          pytest_capture=None):
    if msg_list_sas is None:
        msg_list_sas=[]
    if msg_list_sas_exact is None:
        msg_list_sas_exact=[]
    if msg_list_contains is None:
        msg_list_contains=[]
    if msg_list_contains_exact is None:
        msg_list_contains_exact=[]
    if ds_compare_list is None:
        ds_compare_list=[]

    print("####################################### RUNNING ASSERTIONS #######################################")
    try:
        # build SAS log path
        if sas_log_path is not None:
            sas_log_path = get_log_path(sas_log_name=sas_log_path, stack_level=4)

        # assert that "cleaned" message exists in both SAS and Python logs
        if isinstance(msg_list_sas,str):
            msg_list_sas = [msg_list_sas]
        for message in msg_list_sas:
            assert sas_log_path is not None, "UNIT TEST SETUP ERROR: `sas_log_path` erroneously blank"
            assert_log_consistent(test_log=python_log, sas_log_path=sas_log_path, must_exist=True, clean_whitespace=True, msg=message)
            print()

        # assert exact message exists in both SAS and Python logs
        if isinstance(msg_list_sas_exact,str):
            msg_list_sas_exact = [msg_list_sas_exact]
        for message in msg_list_sas_exact:
            assert sas_log_path is not None, "UNIT TEST SETUP ERROR: `sas_log_path` erroneously blank"
            assert_log_consistent(test_log=python_log, sas_log_path=sas_log_path, must_exist=True,clean_whitespace=False, msg=message)
            print()

        # assert that "cleaned" message exists in Python log
        if isinstance(msg_list_contains,str):
            msg_list_contains = [msg_list_contains]
        for message in msg_list_contains:
            assert_log_contains(test_log=python_log, clean_whitespace=True, msg=message)
            print()

        # assert that exact message exists in Python log
        if isinstance(msg_list_contains_exact,str):
            msg_list_contains_exact = [msg_list_contains_exact]
        for message in msg_list_contains_exact:
            assert_log_contains(test_log=python_log, clean_whitespace=False, msg=message)
            print()

        # assert return value is correct
        if isinstance(expect_zero_rc, bool):
            if expect_zero_rc:
                assert rc == 0, f"Procedure returned non-zero value when zero was expected: {rc}"
                print("asserted RC == 0\n")
            else:
                assert rc != 0, f"Procedure return code should be non-zero, but is: {rc}"
                print("asserted RC != 0\n")
            print()

        # assert ERROR: count is correct
        if isinstance(expect_error_count,int):
            assert_substr_count(substr_to_count="ERROR:", test_log=python_log, expected_count=expect_error_count)
            print()

        # assert WARNING: count is correct
        if isinstance(expect_warning_count,int):
            assert_substr_count(substr_to_count="WARNING:", test_log=python_log, expected_count=expect_warning_count)
            print()

        # assert test and control datasets match
        assert_datasets_equal(ds_compare_list, round_data=round_data, drop_columns=drop_columns)
        print()

        print("################################ ASSERTIONS COMPLETE WITHOUT ERROR ###############################")
    except Exception:
        print("####################################### ASSERTIONS FAILED ########################################")
        raise
    finally:
        out_assert = get_console_log(pytest_capture) # get console log for arrange and act

        print(python_log)
        print(out_assert)

class PytestProcedure:
    """Base class for Procedure Pytest helper functions.

    SUBCLASS MUST
        - populate the `self.ds_compare_list` with list containing a tuple for each output dataset
          - for associating expected and actual output datasets
          - populate tuple with `(<output-dataset>, expected_<output-dataset>)`

    """

    def __init__(
            self,
            #### Unit test parameters
            pytest_capture          = None, # for the `capfd` which captures the console log
            sas_log_name            = None,
            msg_list_sas            = None,
            msg_list_sas_exact      = None,
            msg_list_contains       = None,
            msg_list_contains_exact = None,
            expected_error_count    = 0,
            expected_warning_count  = 0,
            rc_should_be_zero       = True,
            round_data              = None,
            drop_columns            = True,
            by                      = None,
    ):
        if msg_list_sas is None:
            msg_list_sas=[]
        if msg_list_sas_exact is None:
            msg_list_sas_exact=[]
        if msg_list_contains is None:
            msg_list_contains=[]
        if msg_list_contains_exact is None:
            msg_list_contains_exact=[]

        print("############################# PROCEDURE LOG ENDED on previous line ##############################")

        #### assert
        self.out_act = get_console_log(pytest_capture) # get console log for arrange and act

        # load expected datasets
        for i in range(len(self.ds_compare_list)):
            self.ds_compare_list[i][1] = load_control_dataset(self.ds_compare_list[i][1])

        if drop_columns is True:
            drop_columns = by

        run_standard_assertions(
            expect_zero_rc      = rc_should_be_zero,    rc                      = self.c_return_code,
            python_log          = self.out_act,         sas_log_path            = sas_log_name,
            msg_list_sas        = msg_list_sas,         msg_list_sas_exact      = msg_list_sas_exact,
            msg_list_contains   = msg_list_contains,    msg_list_contains_exact = msg_list_contains_exact,
            expect_error_count  = expected_error_count, expect_warning_count    = expected_warning_count,
            ds_compare_list     = self.ds_compare_list,      round_data              = round_data,
            drop_columns        = drop_columns,
            pytest_capture      = pytest_capture,
        )
