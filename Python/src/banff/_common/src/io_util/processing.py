"""A place for functions which perform pre or post processing in relation to the input or output of datasets to or from Procedures."""
import duckdb
import pandas as pd
import pyarrow as pa

from ..io_util.normalization import match_case
from ..nls import _


def add_status_by_vars(ds_stat, ds_data, unit_id=None, by=None):
    """Add BY variables to input status dataset.

        Filters `ds_stat` down to `unit_id`, 'FIELDID', and 'STATUS', then
        add `by` variables from `ds_data`, matching on `unit_id` in a safe
        case insensitive manner.

        ds_stat     - instatus or instatus_hist dataset, `pandas.DataFrame`
        ds_data     - indata or indata_hist dataset, `pandas.DataFrame`
        unit_id     - name of unit_id, `str`
        by          - list of 0+ variable names, `str`

    pandas support via `pandas.DataFrame.join()`
        Research indicates this as possibly more efficient than `pandas.merge()`.
        Join always happens on the "index", so `.set_index()` is used to specify which
        column to join the dataframes on.
            - calls `.reset_index()` on merged dataset, to restore that column

    pyarrow support via `pyarrow.Table.join()`
        Nothing to really note here.

    Case Insensitivity (safely)
        Merging/joining pandas dataframes is a case sensitive operation.
        Procedures are case insensitive with respect to variable (column) names provided in:
            - datasets
            - edit strings
            - parameters

        In this function, case insensitivity is implemented without modifying the
        case of either dataset, instead generating dataset-case-specific metadata.
        - see `match_case()` for details
    """
    # replace defaults
    by      = "" if by is None  else by
    unit_id = "" if unit_id is None else unit_id

    # validate input types
    if not (
        isinstance(ds_stat, (pd.DataFrame, pa.Table))
        and isinstance (ds_data, (pd.DataFrame, pa.Table))
    ):
        mesg = _("Invalid type: `{}` only supports types: {}").format("add_status_by_vars()", "`pandas.DataFrame`, `pyarrow.Table`")
        raise TypeError(mesg)
    if type(ds_stat) is not type(ds_data):
        mesg = _("Invalid type: `{}` requires identically typed datasets").format("add_status_by_vars()")
        raise TypeError(mesg)

    ## calculate unit_id for each dataset
    unit_id_data = match_case(unit_id, ds_data)[0]
    unit_id_stat = match_case(unit_id, ds_stat)[0]

    ## generate select list for status dataset
    select_stat = match_case(
        list_source=f"{unit_id} FiElDiD sTAtuS",
        target=ds_stat,
    )

    ## generate select list for data dataset
    select_data = match_case(
        list_source=f"{by} {unit_id}",
        target=ds_data,
    )


    if isinstance(ds_stat, pd.DataFrame):
        # `pandas.DataFrame.join()`
        # Research indicates this as possibly more efficient than `pandas.merge()`.
        #   - call `.set_index()` on both datasets to specify the column to merge on
        #   - use `select_data` to filter data dataset down to `unit_id` and `by` vars
        #   - use `select_stat` to filter stat dataset down to minimum set of mandatory columns
        ds_stat = ds_stat[select_stat].set_index(unit_id_stat)
        ds_data = ds_data[select_data].set_index(unit_id_data)
        merged = ds_stat.join(ds_data, how="inner")

        # call `.reset_index()` on merged dataset to restore stat's `unit_id`
        return merged.reset_index(names=unit_id_stat)

    if isinstance(ds_stat, pa.Table):
        ds_stat = ds_stat.select(select_stat)
        ds_data = ds_data.select(select_data)

        def join_pyarrow():
            """Use inner function to avoid repeating in `try` and `except`."""
            return ds_stat.join(
                right_table=ds_data,
                keys=unit_id_stat,
                right_keys=unit_id_data,
                join_type="inner",
            )

        try:
            merged = join_pyarrow()
        except pa.ArrowInvalid:
            # likely caused by inconsistent string data types, try again
            ds_stat = handle_arrow_string_data(ds_stat)
            ds_data = handle_arrow_string_data(ds_data)
            merged = join_pyarrow()

        return merged

    mesg = _("Invalid type: `{}` only supports types: {}").format("add_status_by_vars()", "`pandas.DataFrame`, `pyarrow.Table`")
    raise TypeError(mesg)

def flag_rows_where(ds_in, where_stmt, new_col_name="_exclude", flag_value="E"):
    """Add a new 'flag' string column with a value flagging certain records.

    Add a new string column, `new_col_name`, to a intermediate dataset.
    For records matching `where_stmt`, set the value to `flag_value`.
    Unflagged records will have a null (missing) value.

    Returns the modified dataset and new column name (may differ from `new_col_name`).

    If `new_col_name` exists
        A case insensitive search for `new_col_name` is performed
        If any matches are found, a random numeric suffix will be added
        to the new column name.  The new random name isn't validated.

    Use Case: add 'exclude' flag to indata or indata_hist datasets
    """
    temp_table_name="ds_temp"

    # import into duckdb table
    duckdb.sql(f"CREATE TABLE {temp_table_name} AS SELECT * from ds_in")  # noqa: S608 # no user arguments here, no concerns

    try:
        # if column exists (case insensitive)
        if duckdb.sql(f"""
            SELECT * FROM information_schema.columns
            WHERE table_name='{temp_table_name}' AND LOWER(column_name)=LOWER('{new_col_name}')
        """).arrow().num_rows > 0:  # noqa: S608 # private temporary database, no concerns
            # add a random suffix
            # Code Quality: ruff rule S311 ignored as this code is not related to security
            import random
            new_col_name=f"{new_col_name}_{str(random.randint(1000,9999))}"  # noqa: S311

        # modify
        duckdb.sql(f"ALTER TABLE {temp_table_name} ADD COLUMN {new_col_name} VARCHAR")  # noqa: S608 # private temporary database, no concerns
        duckdb.sql(f"UPDATE {temp_table_name} SET {new_col_name} = '{flag_value}' WHERE {where_stmt}")  # noqa: S608 # private temporary database, no concerns

        # export from duckdb
        if isinstance(ds_in, pd.DataFrame):
            ds_out = duckdb.sql(f"SELECT * from {temp_table_name}").df()  # noqa: S608 # private temporary database, no concerns
            ds_out = handle_pandas_string_data(ds_out)
        elif isinstance(ds_in, pa.Table):
            ds_out = duckdb.sql(f"SELECT * from {temp_table_name}").arrow()  # noqa: S608 # private temporary database, no concerns
    finally:
        # free duckdb table
        duckdb.sql(f"DROP TABLE IF EXISTS {temp_table_name}")  # noqa: S608 # private temporary database, no concerns

    return ds_out, new_col_name

def handle_arrow_string_data(ds, dest_type=None):
    """Return new Table with all string data converted to one datatype.

    Casts pyarrow.Table `ds` string columns (string and large_string) to `dest_type`.
    By default `dest_type` is `pa.large_string()`.
    """
    # default dest_type requires function call
    if dest_type is None:
        dest_type = pa.large_string()

    new_fields=[]
    for col in ds.schema:
        if (
            pa.types.is_large_string(col.type)
            or pa.types.is_string(col.type)
        ):
            new_fields.append(pa.field(col.name, dest_type))
        else:
            new_fields.append(col)
    new_schema = pa.schema(new_fields)

    # only cast dataset if changes were made to schema
    if not new_schema.equals(ds.schema):
        ds = ds.cast(new_schema)

    return ds

def handle_pandas_string_data(df):
    """Force string columns to use nullable string datatype.

    Pandas 2.x often has issues loading "missing" *string* values.  In some cases
    the column's type will become lost and assumed to be numeric.  To avoid this,
    code below forceably converts the column to a string type such that it will
    be identified as an arrow string when received by C code.

    Optimize: The memory efficiency of this method has not yet been assessed
    Furthermore, SAS itself provided a single space character (' ') to C when a character
    value was missing.  If this causes issues, we could convert all missing
    character values to a single space, or have C emulate that behaviour for missing char values.
    """
    # get character columns
    char_columns = df.select_dtypes(object).columns

    # convert to string
        ##### Implementation Choices #####
        # To convert character data to properly nullable string types,
        # ensuring Pandas 2.x default settings don't lose track of column type when all rows are "missing"
        # the following advice was followed, taken from: https://pandas.pydata.org/docs/user_guide/pyarrow.html
        ## > The string alias "string[pyarrow]" maps to pd.StringDtype("pyarrow")
        ## > which is not equivalent to specifying dtype=pd.ArrowDtype(pa.string()).
        ## > Generally, operations on the data will behave similarly
        ## > except pd.StringDtype("pyarrow") can return NumPy-backed nullable types
        ## > while pd.ArrowDtype(pa.string()) will return ArrowDtype.
    df[char_columns] = df[char_columns].astype(pd.StringDtype(storage="pyarrow"))
    return df

def remove_rows_where(ds_in, where_stmt):
    """Remove (delete/drop) certain records.

    Remove rows that match `where_stmt`
    """
    temp_table_name="ds_temp"

    # import into duckdb table
    duckdb.sql(f"CREATE TABLE {temp_table_name} AS SELECT * from ds_in")  # noqa: S608 # private temporary database, no concerns

    try:
        # modify
        duckdb.sql(f"DELETE FROM {temp_table_name} WHERE {where_stmt}")  # noqa: S608 # private temporary database, no concerns

        # export from duckdb
        if isinstance(ds_in, pd.DataFrame):
            ds_out = duckdb.sql(f"SELECT * from {temp_table_name}").df()  # noqa: S608 # private temporary database, no concerns
            ds_out = handle_pandas_string_data(ds_out)
        elif isinstance(ds_in, pa.Table):
            ds_out = duckdb.sql(f"SELECT * from {temp_table_name}").arrow()  # noqa: S608 # private temporary database, no concerns
    finally:
        # free duckdb table
        duckdb.sql(f"DROP TABLE IF EXISTS {temp_table_name}")  # noqa: S608 # private temporary database, no concerns

    return ds_out

def sort_dataset(dataset, by=None, case_sensitive=False, inplace=False):
    """Sort dataset in ascending order in order of `by` variable list.

    When `case_sensitive=False` (default), the dataset's column names
    are normalized prior to sorting and restored after.

    If no variables are passed, the original dataset is returned.
    If dataset format is not supported, a `TypeError` is raised.
    """
    if by is None or dataset is None:
        return dataset

    # validate input types
    if not isinstance(dataset, (pd.DataFrame, pa.Table)):
        mesg = _("Invalid type: `{}` only supports types: {}").format("sort_dataset()", "`pandas.DataFrame`, `pyarrow.Table`")
        raise TypeError(mesg)

    # convert by `str` to `list`
    if isinstance(by, str):
        by = by.split()

    if len(by) == 0:
        return dataset

    if not case_sensitive:
        by = match_case(list_source=by, target=dataset)

    # sort dataset
    if isinstance(dataset, pd.DataFrame):
        sorted_ds = dataset.sort_values(
            by=by,
            inplace=inplace,
            ignore_index=True,
        )
    elif isinstance(dataset, pa.Table):
        pa_by = []
        for b in by:
            # pyarrow needs tuples which contain column and sort order
            pa_by += [(b, "ascending")]

        sorted_ds = dataset.sort_by(pa_by)

    return sorted_ds
