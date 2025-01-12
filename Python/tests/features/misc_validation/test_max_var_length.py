"""Tests of maximum variable length allowed by C code
C code accepts parameters in which users specify the names of variables which appear in one or more
input datasets.  The length of the names of these variables must be equal to or less than the maximum
allowed length, otherwise an error should occur.  

To implement these tests, user guide examples were used to create a function for each procedure which
allows the caller to define the length of various variables used in the procedure.  The function then 
generates input and expected output datasets and parameters accordingly and runs the procedure via `banff.testing`.  
"""
import banff.testing as banfftest
import pandas as pd
from io import StringIO
import pytest
import pyarrow as pa
import random

# maximum allowed variable (column) name length by C code
max_allowed=64

def repeat_to_length(string_to_expand, length=None):
    """repeat a string until it reaches a certain length"""
    if length is None:
        length=len(string_to_expand)

    return (string_to_expand * (int(length/len(string_to_expand))+1))[:length]

def run_max_var_len_determin(
        pytest_capture,
        id_len=None,
        by_len=None,
        total_len=None,
        q1_len=None,
        **kwargs,
):
    """Function for testing variable lengths in deterministic procedure"""
    id_name = repeat_to_length("ident", id_len)
    by_name = repeat_to_length("prov", by_len)
    _total_name = repeat_to_length("TOTAL", total_len)
    _q1_name = repeat_to_length("Q1", q1_len)

    indata = banfftest.PAT_from_string(f"""
        s n
        {id_name} {_total_name} {_q1_name} Q2 Q3 Q4 staff {by_name}
        REC01 500 100 125 125 150 2000 24
        REC02 750 200 170 130 250 2500 24
        REC03 400 80 90 100 130 1500 24
        REC04 1000 150 250 350 250 3500 24
        REC05 3000 500 500 1000 1000 5000 24
        REC06 50 10 15 500 20 100 24
        REC07 600 110 140 230 45 2400 30
        REC08 900 175 999 999 300 3000 30
        REC09 2500 400 555 600 5000 89 30
        REC10 800 11 12 13 14 2800 30
        REC11 -25 -10 -5 -5 -10 3000 30
    """)

    instatus = banfftest.PAT_from_string(f"""
        s
        fieldid status {id_name}
        Q3 FTI REC06
        Q4 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        staff FTI REC09
        {_q1_name} FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        Q4 FTI REC11
    """)

    outdata = banfftest.PAT_from_string(f"""
        s n
        {id_name}	{_q1_name}	Q2	Q3	Q4	{_total_name}
        REC06	10	15	5	20	50
        REC07	110	140	230	120	600
        REC09	400	555	600	945	2500
        REC11	-10	-5	-5	-5	-25
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        REC06	Q3	IDE	5
        REC07	Q4	IDE	120
        REC09	Q4	IDE	945
        REC11	Q4	IDE	-5
    """)

    banfftest.pytest_determin(
        indata=indata,
        instatus=instatus,
        edits=f"{_q1_name} + Q2 + Q3 + Q4 - {_total_name} = 0;",
        accept_negative=True,
        unit_id=id_name,
        by=by_name,

        pytest_capture=pytest_capture,
        expected_outdata=outdata,
        expected_outstatus=outstatus,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_determin(capfd, )

@pytest.mark.m_auto_pass
def test_var_len_determin__edits_max(capfd):
    """edits: max valid variable lengths"""
    run_max_var_len_determin(
        capfd,
        total_len=max_allowed,
        q1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin__edits_p1(capfd):
    """edits: max +1 variable lengths"""
    run_max_var_len_determin(
        capfd,
        total_len=max_allowed+1,
        q1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=2,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin__edits_x2(capfd):
    """edits: max *2 variable lengths"""
    run_max_var_len_determin(
        capfd,
        total_len=max_allowed*2,
        q1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=2,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_determin(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_determin(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_determin__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_determin(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

def run_max_var_len_donorimp(
        pytest_capture,
        id_len=None,
        must_match_len=None,
        data_excl_len=None,
        by_len=None,
        total_len=None,
        q1_len=None,
        **kwargs,
):
    """Function for testing variable lengths in donorimp procedure"""
    id_name = repeat_to_length("IDENT", length=id_len)
    must_match_name = repeat_to_length("STAFF", length=must_match_len)
    data_excl_name = repeat_to_length("TOEXCL", length=data_excl_len)
    by_name = repeat_to_length("prov", length=by_len)
    _total_name = repeat_to_length("TOTAL", total_len)
    _q1_name = repeat_to_length("Q1", q1_len)

    def add_exclude(row):
        if row[_total_name] > 1000:
            return 'E'
        else:
            return ''

    indata = pd.read_csv(StringIO(f"""
        {id_name} {_total_name} {_q1_name} Q2 Q3 Q4 {must_match_name} {by_name}
        REC01 500 120 150 80 150 50 24
        REC02 750 200 170 130 250 75 24
        REC03 400 80 90 100 130 40 24
        REC04 1000 150 250 350 250 100 24
        REC05 1050 200 225 325 300 100 24
        REC06 500 100 125 5000 130 45 24
        REC07 400 80 90 100 15 40 30
        REC08 950 150 999 999 200 90 30
        REC09 1025 200 225 300 10 10 30
        REC10 800 11 12 13 14 80 30
        REC11 -25 -10 -5 -5 -10 3000 30
        REC12 1000 150 250 350 250 100 30
        REC13 999 200 225 325 300 100 30
        REC14 -25 -10 -5 -10 -5 3000 30
    """), sep=r'\s+')

    # we added a new column, "TOEXCL" to the indata dataframe 
    indata[data_excl_name] = indata.apply(add_exclude, axis=1)

    instatus = pd.read_csv(StringIO(f"""
        FIELDID STATUS {id_name}
        Q3 IPR REC01
        Q4 FTE REC04
        Q3 FTI REC06
        Q2 FTI REC07
        Q2 FTI REC08
        Q3 FTI REC08
        Q4 FTI REC09
        STAFF FTI REC09
        {_q1_name} FTI REC10
        Q2 FTI REC10
        Q3 FTI REC10
        Q4 FTI REC10
        {_q1_name} FTI REC11
        Q2 FTI REC11
        Q3 FTI REC11
        Q4 FTI REC11
        Q2 FTI REC13
        Q3 FTI REC13
    """), sep=r'\s+')

    outdata = banfftest.PAT_from_string(f"""
        s n
        {id_name}	{_q1_name}	Q2	Q3	Q4	{_total_name}
        REC06	100	125	100	130	500
        REC08	150	250	350	200	950
        REC09	200	225	300	250	1025
        REC10	-10	-5	-10	-5	800
        REC11	-10	-5	-10	-5	-25
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        REC06	Q3	IDN	100
        REC08	Q2	IDN	250
        REC08	Q3	IDN	350
        REC09	Q4	IDN	250
        REC10	{_q1_name}	IDN	-10
        REC10	Q2	IDN	-5
        REC10	Q3	IDN	-10
        REC10	Q4	IDN	-5
        REC11	{_q1_name}	IDN	-10
        REC11	Q2	IDN	-5
        REC11	Q3	IDN	-10
        REC11	Q4	IDN	-5
    """)

    outdonormap = banfftest.PAT_from_string(f"""
        s s n n
        RECIPIENT	DONOR	NUMBER_OF_ATTEMPTS	DONORLIMIT
        REC06	REC03	1	1
        REC08	REC12	1	2
        REC09	REC12	1	2
        REC10	REC14	1	2
        REC11	REC14	1	2
    """)

    banff_call= banfftest.pytest_donorimp(
        indata=indata,
        instatus=instatus,
        unit_id=id_name, 
        edits=f"{_q1_name} + Q2 + Q3 + Q4 - {_total_name} = 0;",
        post_edits=f"{_q1_name} + Q2 + Q3 + Q4 - {_total_name} <= 0;",
        min_donors=1,
        percent_donors=1,
        n=1,
        n_limit=1,
        mrl=0.5,
        random=True,
        eligdon="original",
        outmatching_fields=True,
        accept_negative=True,
        must_match=must_match_name,
        data_excl_var=data_excl_name,
        by=by_name,

        pytest_capture=pytest_capture,
        expected_outdata=outdata,
        expected_outstatus=outstatus,
        expected_outdonormap=outdonormap,
        expected_warning_count=None,  # ignore warning count
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_donorimp(capfd)

@pytest.mark.m_auto_pass
def test_var_len_donorimp__edits_max(capfd):
    """edits: max valid variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        total_len=max_allowed,
        q1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp__edits_p1(capfd):
    """edits: max +1 variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        total_len=max_allowed+1,
        q1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=2,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp__edits_x2(capfd):
    """edits: max *2 variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        total_len=max_allowed*2,
        q1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=2,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
        must_match_len=max_allowed,
        data_excl_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        must_match_len=max_allowed+1,
        data_excl_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_donorimp__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_donorimp(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        must_match_len=max_allowed*2,
        data_excl_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def run_max_var_len_editstat(
        pytest_capture,
        by_len=None,
        x1_len=None,
        **kwargs,
):
    """Function for testing variable lengths in editstats procedure"""
    by_name = repeat_to_length("Group", by_len)
    _x1_name = repeat_to_length("X1", x1_len)

    indata = banfftest.PAT_from_string(f"""
        n n n s
        {_x1_name} x2 x3 {by_name}
        4 3 2 A
        -4 3 2 A
        6 3 2 B
        4 3 NaN A
        6 3 NaN B
    """)

    outedit_applic = banfftest.PAT_from_string(f"""
        s n n n n n s
        FIELDID	EDIT_APPLIC_PASSED	EDIT_APPLIC_MISSED	EDIT_APPLIC_FAILED	EDIT_APPLIC_NOTINVOLVED	EDITS_INVOLVED	{by_name}
        {_x1_name}	7	1	1	3	3	A
        X2	6	2	1	3	3	A
        X3	4	2	0	6	2	A
        {_x1_name}	2	1	3	2	3	B
        X2	3	2	1	2	3	B
        X3	1	2	1	4	2	B
    """)

    outedit_status = banfftest.PAT_from_string(f"""
        n n n n s 
        EDITID	OBS_PASSED	OBS_MISSED	OBS_FAILED	{by_name}
        0	2	0	1	A
        1	3	0	0	A
        2	2	1	0	A
        3	2	1	0	A
        0	2	0	0	B
        1	0	0	2	B
        2	1	1	0	B
        3	0	1	1	B
    """)

    outkedit_status = banfftest.PAT_from_string(f"""
        n n n n s 
        K_EDITS	OBS_PASSED	OBS_MISSED	OBS_FAILED	{by_name}
        0	0	2	2	A
        1	0	0	1	A
        2	1	1	0	A
        3	1	0	0	A
        4	1	0	0	A
        0	0	1	0	B
        1	1	0	1	B
        2	1	1	1	B
        3	0	0	0	B
        4	0	0	0	B
    """)

    outglobal_status = banfftest.PAT_from_string(f"""
        n n n n s
        OBS_PASSED	OBS_MISSED	OBS_FAILED	OBS_TOTAL	{by_name}
        1	1	1	3	A
        0	0	2	2	B
    """)

    outedits_reduced = pa.Table.from_arrays(
        [
            [0,1,2,3],
            [f"- {_x1_name} + X2 <= 1 ",f"{_x1_name} <= 5 ","- X2 + X3 <= 0 ",f"{_x1_name} + X2 + X3 <= 9 "],
        ],
        schema=pa.schema([
            pa.field('EDITID', pa.float64()),
            pa.field('EDIT_EQUATION', pa.string())
        ])
    )

    outvars_role = banfftest.PAT_from_string(f"""
        s n n n n s
        FIELDID	OBS_PASSED	OBS_MISSED	OBS_FAILED	OBS_NOT_APPLICABLE	{by_name}
        {_x1_name}	1	1	1	0	A
        X2	1	1	1	0	A
        X3	1	1	0	1	A
        {_x1_name}	0	0	2	0	B
        X2	0	0	1	1	B
        X3	0	0	1	1	B
    """)

    banfftest.pytest_editstat(
        indata=indata,
        edits=f"""
            {_x1_name}+1>=x2;
            {_x1_name}<=5;
            x2>=x3;
            {_x1_name}+x2+x3<=9;
        """,
        by=by_name,
        accept_negative=True,

        pytest_capture=pytest_capture,
        expected_outedit_applic=outedit_applic,
        expected_outedit_status=outedit_status,
        expected_outglobal_status=outglobal_status,
        expected_outk_edits_status=outkedit_status,
        expected_outedits_reduced=outedits_reduced,
        expected_outvars_role=outvars_role,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_editstat(capfd)

@pytest.mark.m_auto_pass
def test_var_len_editstat__edits_max(capfd):
    """edits: max valid variable lengths"""
    run_max_var_len_editstat(
        capfd,
        x1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat__edits_p1(capfd):
    """edits: max +1 variable lengths"""
    run_max_var_len_editstat(
        capfd,
        x1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=4,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat__edits_x2(capfd):
    """edits: max *2 variable lengths"""
    run_max_var_len_editstat(
        capfd,
        x1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=4,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_editstat(
        capfd,
        by_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_editstat(
        capfd,
        by_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'by'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_editstat__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_editstat(
        capfd,
        by_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'by'",
        expected_error_count=1,
    )

def run_max_var_len_errorloc(
        pytest_capture,
        id_len=None,
        by_len=None,
        x1_len=None,
        x2_len=None,
        **kwargs,
):
    """Function for testing variable lengths in errorloc procedure"""
    id_name = repeat_to_length("IDENT", length=id_len)
    by_name = repeat_to_length("ZONE", length=by_len)
    _x1_name = repeat_to_length("X1", x1_len)
    _x2_name = repeat_to_length("X2", x2_len)

    indata = banfftest.PAT_from_string(f"""
        s n n s
        {id_name} {_x1_name} {_x2_name} {by_name}
        R03 10 40 B
        R02 -4 49 A
        R04 4 49 A
        R01 16 49 A
        R05 15 51 B
        R07 -4 29 B
        R06 30 70 B
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        R01	{_x1_name}	FTI	16
        R02	{_x1_name}	FTI	-4
        R04	{_x2_name}	FTI	49
        R05	{_x2_name}	FTI	51
    """)

    outreject = banfftest.PAT_from_string(f"""
        s s
{id_name},NAME_ERROR
R06,CARDINALITY EXCEEDED
R07,CARDINALITY EXCEEDED
    """,
    sep=',')

    banff_call= banfftest.pytest_errorloc(
        indata=indata,
        edits=f"{_x1_name}>=-5; {_x1_name}<=15; {_x2_name}>=30; {_x1_name}+{_x2_name}<=50;",
        weights=f"{_x1_name}=1.5;",
        cardinality=2,
        time_per_obs=0.1,
        unit_id=id_name,
        by=by_name,

        pytest_capture=pytest_capture,
        expected_outstatus=outstatus,
        expected_outreject=outreject,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_errorloc(capfd)

@pytest.mark.m_auto_pass
def test_var_len_errorloc__edits_max(capfd):
    """edits: max valid variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__edits_p1(capfd):
    """edits: max +1 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=4,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__edits_x2(capfd):
    """edits: max *2 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=4,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__weights_max(capfd):
    """weights: max valid variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x2_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__weights_p1(capfd):
    """weights: max +1 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x2_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=3,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__weights_x2(capfd):
    """weights: max *2 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        x2_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Invalid edits.",
            "ERROR: Edits parser: Variable name too large!",
        ],
        expected_error_count=3,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_errorloc__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_errorloc(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

def run_max_var_len_estimato(
        pytest_capture,
        id_len=None,
        by_len=None,
        data_excl_len=None,
        hist_excl_len=None,
        **kwargs,
):
    """Function for testing variable lengths in estimator procedure"""
    id_name = repeat_to_length("ident", length=id_len)
    by_name = repeat_to_length("prov", length=by_len)
    data_excl_name = repeat_to_length("CURRDATAEXCL", length=data_excl_len)
    hist_excl_name = repeat_to_length("HISTDATAEXCL", length=hist_excl_len)

    # create indata
    currentdata = pd.read_csv(StringIO(f"""
        {id_name} x {by_name}
        REC01 -500 24
        REC02 750 24
        REC03 -400 24
        REC04 1000 24
        REC05 1050 24
        REC06 500 24
        REC07 400 30
        REC08 950 30
        REC09 1025 30
        REC10 -800 30
        REC12 10000 30
        REC13 500 24
        REC14 750 24
        REC15 400 24
        REC16 -1000 24
        REC17 1050 24
        REC18 500 24
        REC19 -400 30
        REC20 950 30
        REC21 -1025 30
        REC22 800 30
        REC23 10000 30
    """), sep=r'\s+')

    # create indata_hist
    histdata = pd.read_csv(StringIO(f"""
        {id_name} x {by_name}
        REC01 500 24
        REC02 -750 24
        REC03 400 24
        REC05 870 24
        REC06 500 24
        REC07 400 30
        REC08 950 30
        REC09 950 30
        REC10 -800 30
        REC11 800 30
        REC12 500 24
        REC13 750 24
        REC14 -400 24
        REC15 870 24
        REC16 500 24
        REC17 -400 30
        REC18 950 30
        REC19 950 30
        REC20 800 30
        REC21 800 30
    """), sep=r'\s+')

    # create instatus
    curstatus = pd.read_csv(StringIO(f"""
        {id_name} fieldid status
        REC01 x FTI
        REC03 x FTI
        REC10 x FTI
        REC16 x FTI
        REC19 x FTI
        REC21 x FTI
    """), sep=r'\s+')

    # add exclusion column to current data
    def add_exclude(row):
        if row['x'] > 1000:
            return 'E'
        else:
            return ''

    currentdata[data_excl_name] = currentdata.apply(add_exclude, axis=1)

    # add exclusion column to historical data
    # TIP: alternatively we can use a "lambda function" to add the exclude column without creating a separate function
    histdata[hist_excl_name] = histdata.apply(lambda row: 'E' if row['x'] > 2000 else '' , axis=1)

    # sort datasets
    currentdata = currentdata.sort_values(by=[by_name, id_name])
    histdata = histdata.sort_values(by=[by_name, id_name])

    # create inalgorithm
    # TIP: For this single row dataframe, we can create it very simply 
    # create empty dataframe without any columns defined
    algorithm = pd.DataFrame()
    # add values; this magically adds the column, places the value in the first row, only really useful for a single row dataset
    algorithm['algorithmname'] = ['prevalue']
    algorithm['type'] = ['ef']
    algorithm['status'] = ['vp']
    algorithm['formula'] = ['fieldid(h,v)']
    algorithm['description'] = ['Previous value is imputed.']

    # create inestimator
    # TIP: for creating more complicated datasets including missing data, we can use a different approach
    # create dataless dataframe with column names pre-populated
    estimator = pd.DataFrame(columns= ["est_setid", "algorithmname", "excludeimputed", 'excludeoutliers', "randomerror", "countcriteria", "percentcriteria", "weightvariable", "variancevariable", "varianceperiod", "varianceexponent", "fieldid", "auxvariables"])

    # add data; `len(estimator) refers to the number of rows in the estimator DataFrame.  Given the 0-based index, this allows us to append rows`
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'algorithmname':'prevalue', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set1', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1, 'algorithmname':'curmean', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'n', 'excludeoutliers':'n', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'diftrend', 'fieldid':'x'}
    estimator.loc[len(estimator)] = {'est_setid':'set2', 'excludeimputed':'y', 'excludeoutliers':'y', 'randomerror':'y', 'countcriteria':1, 'percentcriteria':1,  'algorithmname':'histreg', 'fieldid':'x'}

    # create a list of columns to convert
    character_column_list = ['est_setid', 'algorithmname', 'excludeimputed', 'excludeoutliers', 'randomerror', 'weightvariable', 'variancevariable', 'varianceperiod', 'fieldid', 'auxvariables']
    # convert column datatype
    estimator[character_column_list] = estimator[character_column_list].astype("string")

    outdata = banfftest.PAT_from_string(f"""
        s n
        {id_name}	x
        REC03	1900
        REC01	1650
        REC16	1650
        REC10	-800
        REC19	950
        REC21	800
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        REC03	x	IVP	1900
        REC01	x	IVP	1650
        REC16	x	IVP	1650
        REC10	x	IVP	-800
        REC19	x	IVP	950
        REC21	x	IVP	800
    """)

    banff_call= banfftest.pytest_estimato(
        indata=currentdata,
        indata_hist=histdata,
        instatus=curstatus,
        inalgorithm=algorithm,
        inestimator=estimator.query('est_setid=="set1"'),
        accept_negative=True,
        data_excl_var=data_excl_name,
        hist_excl_var=hist_excl_name,
        by=by_name,
        unit_id=id_name,
        seed=1732552490,

        pytest_capture=pytest_capture,
        expected_warning_count=None,  # ignore warning count
        expected_outdata=outdata,
        expected_outstatus=outstatus,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_estimato__typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_estimato(capfd)

# estimator has no edits, hence no `_b`, `_c`, `_d` test cases

@pytest.mark.m_auto_pass
def test_var_len_estimato__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_estimato(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
        data_excl_len=max_allowed,
        hist_excl_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_estimato__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_estimato(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        data_excl_len=max_allowed+1,
        hist_excl_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_estimato__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_estimato(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        data_excl_len=max_allowed*2,
        hist_excl_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

def run_max_var_len_massimpu(
        pytest_capture,
        id_len=None,
        by_len=None,
        total_len=None,
        q1_len=None,
        **kwargs,
):
    """Function for testing variable lengths in massimputation procedure"""
    id_name = repeat_to_length("IDENT", length=id_len)
    by_name = repeat_to_length("STAFF", length=by_len)
    _total_name = repeat_to_length("TOTAL", total_len)
    _q1_name = repeat_to_length("Q1", q1_len)

    # create indata
    indata = pd.read_csv(StringIO(f"""
        {id_name} {_total_name} {_q1_name} Q2 Q3 Q4 {by_name}
        REC01 500 100 125 125 150 1000
        REC02 750 200 170 130 250 2000
        REC03 400 80 90 100 130 1000
        REC04 1000 150 250 350 250 2000
        REC05 3000 500 500 1000 1000 1000
        REC06 800 200 225 200 175 2000
        REC07 600 125 150 175 150 1000
        REC08 900 175 200 225 300 2000
        REC09 2500 500 650 600 750 1000
        REC10 800 150 175 225 250 2000
        REC21 3000 -45 -50 75 -234 2000
        REC11 575 NaN NaN NaN NaN 1000
        REC12 850 NaN NaN NaN NaN 2000
        REC13 375 NaN NaN NaN NaN 1000
        REC14 1100 NaN NaN NaN NaN 2000
        REC15 3100 NaN NaN NaN NaN 1000
        REC16 750 NaN NaN NaN NaN 2000
        REC17 675 NaN NaN NaN NaN 1000
        REC18 875 NaN NaN NaN NaN 2000
        REC19 4000 NaN NaN NaN NaN 1000
        REC20 NaN NaN NaN NaN NaN 2000
    """), sep=r'\s+')

    # sort dataset
    indata = indata.sort_values(by=[by_name, id_name])

    outdata = banfftest.PAT_from_string(f"""
        s n
        {id_name}	Q4	Q3	Q2	{_q1_name}
        REC11	150	175	150	125
        REC13	130	100	90	80
        REC15	1000	1000	500	500
        REC17	150	175	150	125
        REC19	1000	1000	500	500
        REC12	250	225	175	150
        REC14	-234	75	-50	-45
        REC16	250	130	170	200
        REC18	300	225	200	175
        REC20	250	350	250	150
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        REC11	Q4	IMAS	150
        REC11	Q3	IMAS	175
        REC11	Q2	IMAS	150
        REC11	{_q1_name}	IMAS	125
        REC13	Q4	IMAS	130
        REC13	Q3	IMAS	100
        REC13	Q2	IMAS	90
        REC13	{_q1_name}	IMAS	80
        REC15	Q4	IMAS	1000
        REC15	Q3	IMAS	1000
        REC15	Q2	IMAS	500
        REC15	{_q1_name}	IMAS	500
        REC17	Q4	IMAS	150
        REC17	Q3	IMAS	175
        REC17	Q2	IMAS	150
        REC17	{_q1_name}	IMAS	125
        REC19	Q4	IMAS	1000
        REC19	Q3	IMAS	1000
        REC19	Q2	IMAS	500
        REC19	{_q1_name}	IMAS	500
        REC12	Q4	IMAS	250
        REC12	Q3	IMAS	225
        REC12	Q2	IMAS	175
        REC12	{_q1_name}	IMAS	150
        REC14	Q4	IMAS	-234
        REC14	Q3	IMAS	75
        REC14	Q2	IMAS	-50
        REC14	{_q1_name}	IMAS	-45
        REC16	Q4	IMAS	250
        REC16	Q3	IMAS	130
        REC16	Q2	IMAS	170
        REC16	{_q1_name}	IMAS	200
        REC18	Q4	IMAS	300
        REC18	Q3	IMAS	225
        REC18	Q2	IMAS	200
        REC18	{_q1_name}	IMAS	175
        REC20	Q4	IMAS	250
        REC20	Q3	IMAS	350
        REC20	Q2	IMAS	250
        REC20	{_q1_name}	IMAS	150
    """)

    outdonormap = banfftest.PAT_from_string(f"""
        s s n
RECIPIENT,DONOR,NUMBER_OF_ATTEMPTS,DONORLIMIT
REC11,REC07,1,
REC13,REC03,1,
REC15,REC05,1,
REC17,REC07,1,
REC19,REC05,1,
REC12,REC10,1,
REC14,REC21,1,
REC16,REC02,1,
REC18,REC08,1,
REC20,REC04,0,
    """,
    sep=',')

    banff_call= banfftest.pytest_massimpu(
        indata=indata,
        min_donors=1,
        percent_donors=1,
        random=True,
        accept_negative=True,
        unit_id=id_name,
        must_impute=f"{_q1_name} Q2 Q3 Q4",
        must_match=_total_name,
        by=by_name,
        seed=1732554102,

        pytest_capture=pytest_capture,
        expected_outdata=outdata,
        expected_outstatus=outstatus,
        expected_outdonormap=outdonormap,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_massimpu(capfd)

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_match_max(capfd):
    """must_match: max valid variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        total_len=max_allowed,
        q1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_match_p1(capfd):
    """must_match: max +1 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        total_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'must_match'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_match_x2(capfd):
    """must_match: max *2 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        total_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'must_match'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_impute_max(capfd):
    """must_impute: max valid variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        total_len=max_allowed,
        q1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_impute_p1(capfd):
    """must_impute: max +1 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        q1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'must_impute'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__must_impute_x2(capfd):
    """must_impute: max *2 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        q1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'must_impute'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_massimpu__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_massimpu(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

def run_max_var_len_outlier(
        pytest_capture,
        id_len=None,
        by_len=None,
        x01_len=None,
        **kwargs,
):
    """Function for testing variable lengths in outlier procedure"""
    id_name = repeat_to_length("ident", length=id_len)
    by_name = repeat_to_length("prov", length=by_len)
    _x01_name = repeat_to_length("X01", x01_len)

    outlierdata = pd.DataFrame(columns= [id_name, _x01_name, "X02", by_name])

    random.seed(8)
    for n in range(1, 31):
        current_label = len(outlierdata)
        outlierdata.loc[current_label] = {
            id_name: 'R{:07d}'.format(n),
            _x01_name: int(random.uniform(0,20)),
            'X02': int(random.uniform(0,20)),
            by_name: 10 if (n)%2==0 else 11}
        if n%5==0:
            outlierdata.loc[current_label, _x01_name] *= -1
            outlierdata.loc[current_label, 'X02'] *= -1

    # sort dataset
    outlierdata = outlierdata.sort_values(by=[by_name])

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        R0000010	{_x01_name}	FTI	-8
        R0000020	{_x01_name}	FTE	-3
        R0000004	{_x01_name}	FTE	19
        R0000030	X02	FTI	-18
        R0000010	X02	FTI	-18
        R0000020	X02	FTI	-18
        R0000005	{_x01_name}	FTE	-12
        R0000015	X02	FTE	-13
        R0000025	X02	FTE	-13
    """)

    outstatus_detailed = banfftest.PAT_from_string(f"""
        s s s s n
        {id_name}	FIELDID	OUTLIER_STATUS	METHOD	CURRENT_VALUE	IMP_BND_L	EXCL_BND_L	EXCL_BND_R	IMP_BND_R
        R0000010	{_x01_name}	ODIL	HB	-8	-5	-2	18	27
        R0000020	{_x01_name}	ODEL	HB	-3	-5	-2	18	27
        R0000004	{_x01_name}	ODER	HB	19	-5	-2	18	27
        R0000030	X02	ODIL	HB	-18	-11	-5	27.5	41
        R0000010	X02	ODIL	HB	-18	-11	-5	27.5	41
        R0000020	X02	ODIL	HB	-18	-11	-5	27.5	41
        R0000005	{_x01_name}	ODEL	HB	-12	-12	-6	21.5	32
        R0000015	X02	ODEL	HB	-13	-22	-10	20	26
        R0000025	X02	ODEL	HB	-13	-22	-10	20	26
    """)

    banff_call= banfftest.pytest_outlier(
        indata=outlierdata,
        method="current",
        mii=4,
        mei=2.5,
        mdm=0.05,
        outlier_stats=True,
        accept_negative=True,
        unit_id=id_name,
        var=f"{_x01_name} X02",
        by=by_name,

        pytest_capture=pytest_capture,
        expected_outstatus=outstatus,
        expected_outstatus_detailed=outstatus_detailed,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_outlier(capfd)

@pytest.mark.m_auto_pass
def test_var_len_outlier__var_max(capfd):
    """var: max valid variable lengths"""
    run_max_var_len_outlier(
        capfd,
        x01_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier__var_p1(capfd):
    """var: max +1 variable lengths"""
    run_max_var_len_outlier(
        capfd,
        x01_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'var'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier__var_x2(capfd):
    """var: max *2 variable lengths"""
    run_max_var_len_outlier(
        capfd,
        x01_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'var'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_outlier(
        capfd,
        id_len=max_allowed,
        by_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_outlier(
        capfd,
        id_len=max_allowed+1,
        by_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_outlier__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_outlier(
        capfd,
        id_len=max_allowed*2,
        by_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

def run_max_var_len_prorate(
        pytest_capture,
        id_len=None,
        q1_len=None,
        **kwargs,
):
    """Function for testing variable lengths in prorate procedure"""
    id_name = repeat_to_length("ident", length=id_len)
    _q1_name = repeat_to_length("Q1", q1_len)

    indata = banfftest.PAT_from_string(f"""
        s n
        {id_name} {_q1_name} Q2 Q3 Q4 YEAR
        R01 -25 42 27 25 40
        R02 -25 44 20 20 40
        R03 -18 44 15 5 40
        R04 -18 42 15 5 40
        R05 12 30 15 5 40
        R06 12 20 10 5 40
    """)

    outdata = banfftest.PAT_from_string(f"""
        s n
        {id_name}	{_q1_name}	Q2	Q3	Q4	YEAR
        R01	5.2	29.3	10.7	-5.2	40
        R02	4.7	30.9	8.2	-3.8	40
        R03	1.6	32	6.9	-0.5	40
        R04	-3.6	33.6	9	1	40
        R05	3.8	24.8	9.8	1.6	40
        R06	8.9	18.7	8.7	3.7	40
    """)

    outreject = banfftest.PAT_from_string(f"""
        s s s s n
        {id_name}	NAME_ERROR	TOTAL_NAME	FIELDID	RATIO_ERROR
    """)

    outstatus = banfftest.PAT_from_string(f"""
        s s s n
        {id_name}	FIELDID	STATUS	VALUE
        R01	{_q1_name}	IPR	5.2
        R01	Q2	IPR	29.3
        R01	Q3	IPR	10.7
        R01	Q4	IPR	-5.2
        R02	{_q1_name}	IPR	4.7
        R02	Q2	IPR	30.9
        R02	Q3	IPR	8.2
        R02	Q4	IPR	-3.8
        R03	{_q1_name}	IPR	1.6
        R03	Q2	IPR	32
        R03	Q3	IPR	6.9
        R03	Q4	IPR	-0.5
        R04	{_q1_name}	IPR	-3.6
        R04	Q2	IPR	33.6
        R04	Q3	IPR	9
        R04	Q4	IPR	1
        R05	{_q1_name}	IPR	3.8
        R05	Q2	IPR	24.8
        R05	Q3	IPR	9.8
        R05	Q4	IPR	1.6
        R06	{_q1_name}	IPR	8.9
        R06	Q2	IPR	18.7
        R06	Q3	IPR	8.7
        R06	Q4	IPR	3.7
    """)

    banff_call= banfftest.pytest_prorate(
        indata=indata,
        edits=f"{_q1_name}+4Q2+2Q3+Q4=YEAR;",
        method="BASIC",
        decimal=1,
        lower_bound=-100,
        upper_bound=100,
        modifier="always",
        accept_negative=True,
        unit_id=id_name,

        pytest_capture=pytest_capture,
        expected_outdata=outdata,
        expected_outreject=outreject,
        expected_outstatus=outstatus,
        **kwargs,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate_typical(capfd):
    """typical valid variable lengths"""
    run_max_var_len_prorate(capfd)

@pytest.mark.m_auto_pass
def test_var_len_prorate__edits_max(capfd):
    """edits: max valid variable lengths"""
    run_max_var_len_prorate(
        capfd,
        q1_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate__edits_p1(capfd):
    """edits: max +1 variable lengths"""
    run_max_var_len_prorate(
        capfd,
        q1_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Prorating edits parser: Variable name too large!",
        ],
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate__edits_x2(capfd):
    """edits: max *2 variable lengths"""
    run_max_var_len_prorate(
        capfd,
        q1_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=[
            "ERROR: Prorating edits parser: Variable name too large!",
        ],
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate__varlist_max(capfd):
    """varlists: max valid variable lengths"""
    run_max_var_len_prorate(
        capfd,
        id_len=max_allowed,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate__varlist_p1(capfd):
    """varlists: max +1 variable lengths"""
    run_max_var_len_prorate(
        capfd,
        id_len=max_allowed+1,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

@pytest.mark.m_auto_pass
def test_var_len_prorate__varlist_x2(capfd):
    """varlists: max *2 variable lengths"""
    run_max_var_len_prorate(
        capfd,
        id_len=max_allowed*2,
        rc_should_be_zero=False,
        msg_list_contains_exact=f"ERROR: variable name exceeds max length ({max_allowed}) in varlist 'unit_id'",
        expected_error_count=1,
    )

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()