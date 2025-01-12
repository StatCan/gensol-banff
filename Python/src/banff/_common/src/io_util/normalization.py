"""A place for functions which perform normalization of variable names.

For the purpose of implementing safe case-insensitive operations.
"""
import pandas as pd
import pyarrow as pa

from ..exceptions import NormalizationError
from ..nls import _


def flip_dict(the_dict):
    """Swap keys and values in a dictionary.

    Values in resulting dictionary values will be a list of keys
    original dict's keys, allowing duplicate values (in original dict) to be retained
    """
    flipped = {}
    for k,v in the_dict.items():
        if v not in flipped:
            flipped[v] = [k]
        else:
            flipped[v].append(k)

    return flipped

def get_norm_dict(varlist, raise_collision_exception=True):
    """Get mapping of names to normalized names.

    Given `varlist`, a list or whitespace-separated string of variable names,
    returns a dictionary which maps "normalized" variable names to their
    original value(s).  Varlist can also be a Pandas Dataframe or PyArrow Table.

        ['foo', 'Bar', 'BAZ'] ->    {
                                        'FOO': ['foo'],
                                        'BAR': ['Bar'],
                                        'BAZ': ['BAZ']
                                    }

    NOTE: collisions
        when two or more members of `varlist` have the same normalized names, a collision
        has occurred.  In this case, each of the non-normalized values will appear under
        the same normalized key in the resulting dictionary

        ['foo', 'Foo'] -> {'FOO': ['foo', 'Foo']}
    `raise_collision_exception`
        When `True`, value lists in returned dictionary will contain at most 1 member.
        A collision occurs when two or more names in `varlist` have identical normalized names.
        For example
            ['foo', 'Bar', 'bAR'] ->    {
                                            'FOO': ['foo'],
                                            'BAR': ['Bar', 'bAR']
            }

        If a name has a collision, it cannot deterministically be converted back to its original
        state following normalization.

        Specify `raise_collision_exception=False` to receive the dictionary including collisions.
    """
    # if string, upgrade to list
    # "Var1 Var2 vaR2" -> ['Var1', 'Var2', 'vaR2']
    vl = varlist.split() if isinstance(varlist, str) else varlist

    # if dataframe, get column list (wow, Python is so cool)
    if isinstance(vl, pd.DataFrame):
        vl = vl.columns.to_list()

    if isinstance(vl, pa.Table):
        vl = vl.column_names

    # use dictionary comprehension to get mapping of values to normalized values
    # -> {'Var1': 'VAR1', 'Var2': 'VAR2', 'vaR2': 'VAR2'}
    vl_norm = {v: v.upper() for v in vl}

    # flip dict so we have normalized values mapping to list of original values
    # -> {'VAR1': ['Var1'], 'VAR2': ['Var2', 'vaR2']}
    vl_norm_flip = flip_dict(vl_norm)

    if (raise_collision_exception
        and len({k:v for k,v in vl_norm_flip.items() if len(v) > 1}) > 0
    ):
        raise NormalizationError(_("Unable to normalize variable list: `{}`").format(varlist))

    return vl_norm_flip

def match_case(list_source, target):
    """Get list using case used in target.

    Given a `list_source` of variable names and a `target`, returns a list containing
    each name in `list_source` in the alphanumeric case used in `target`.
    `list_source` and `target` can be
        - `list` of strings
        - `str` of whitespace separated variable names
        - `pandas.DataFrame`: its `.columns.to_list()` will be used

    Matching is performed by normalizing all variable names in source and target
    and matching on the normalized names.

    Exceptions:
        During normalization, it is possible that 2 or more names share the same normalized
        name.  In such a case, matching cannot be reliably performed and an exception is raised
    """
    # `get_norm_dict` gets dictionary mapping normalized to original names, and checks for collisions
    ls_norm_dict = get_norm_dict(list_source)
    target_norm_dict = get_norm_dict(target)

    # get list of normalized source names
    ls_norm = list(ls_norm_dict.keys())

    # get source names in target case
    out = [target_norm_dict[v][0] for v in ls_norm]
    return out

def get_column_mapping(ds_source, ds_target, include_unmatched=False):
    """Get mapping (dictionary) of column names, source to target.

    Given a source and target dataset (or list of their column names),
    produces a dictionary mapping source column names to target column names.
    Matches are made on "normalized" names (see `get_norm_dict()`).

    Only includes columns where a match was found (by default).
    When `include_unmatched=True`, unmatched source columns map to themselves.

    Use case: `ds_source.rename(columns=get_column_mapping()
    """
    # get mapping of "normalized" to "actual" column names
    nd_source = get_norm_dict(ds_source)
    nd_target = get_norm_dict(ds_target)

    # get list of "normalized" source column names
    normalized_source_columns = list(nd_source.keys())

    # make dict of source column name -> target column name
    out = {}
    for v in normalized_source_columns:
        if v in nd_target:
            out[nd_source[v][0]] = nd_target[v][0]
        elif include_unmatched:
            out[nd_source[v][0]] = nd_source[v][0]

    return out
