import contextlib
import math

import pyarrow as pa


def PAT_from_string(foo, sep=None):
    """Generate a pyarrow table from a text string.

    This function aids in the creation of control datasets for test cases.
    It generates a pyarrow table from a specially crafted multi-line text string which
    provides the table's column names, types, and values.

    For example, a table having 1 string and 5 numeric columns with 2 rows of data,
    including an empty string in the ident column and missing value in Q4:
        '''
        s     n      n   n   n   n
        ident total  Q1  Q2  Q3  Q4
        REC04 1000  150 250 130 250
        ''    1001  151 251 131 NaN
        '''

    Empty lines are ignored
    1st line describes the column types
        s- string
        n- numeric
        NOTE: types are always separated by whitespace, regardless of `sep=`
    2nd line names the columns
    subsequent lines (if any) provide values for each row

    Whitespace is used to delimit values in each row by default.
        specify `sep` to override: example `sep=','` to use commas

    When fewer types (1st line) are provided that columns, the final type is applied
    to all remaining columns
        i.e. the type line above with "s n" would produce the same result

    Empty Strings:
        When using the default `sep=None`, empty string values should be specified as
        empty single quotes (''), which are converted to an empty string.
        Alternatively (and if the value SHOULD be two single quotes), specify `sep=` to use a
        non-whitespace character (like a comma).
    """
    col_values=[]
    # for each row in string, discarding empty lines
    for i, row in enumerate(foo.strip().splitlines()):
        if i == 0:  # extract type specifiers ('n'- numeric, 's' - string)
            type_list = row.split()
        elif i == 1:  # extract column names
            col_names = row.split(sep=sep)

            # add placeholders to values, build schema type list
            sch_type_list = []
            cur_type="<not specified>"
            for j, name in enumerate(col_names):
                # add placeholder for column's values
                col_values.append([])

                # determine column's type
                with contextlib.suppress(IndexError):  # use last extracted type if `IndexError` occurs
                    cur_type = type_list[j].lower()

                # add column to schema type list
                match cur_type:
                    case "n":
                        sch_type_list.append((name, pa.float64()))
                    case "s":
                        sch_type_list.append((name, pa.large_string()))
                    case _:
                        mesg = f"invalid value in type list: {cur_type}"
                        raise ValueError(mesg)
        else:  # extract values for each column
            for j, val in enumerate(row.split(sep=sep)):
                # determine column type
                with contextlib.suppress(IndexError):  # use last extracted type if `IndexError` occurs
                    cur_type = type_list[j].lower()

                if cur_type == "n":
                    if len(val) == 0 or math.isnan(float(val)):
                        col_values[j].append(None)  # use `None` instead of `NaN` so pyarrow marks it as missing
                    else:
                        col_values[j].append(float(val))
                else:
                    if sep is None and val == "''":
                        val=""
                    col_values[j].append(val)

    # create schema
    sch = pa.schema(sch_type_list)
    # create table
    pat = pa.table(data=col_values, schema=sch)

    return pat
