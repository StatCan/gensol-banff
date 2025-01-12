from banff.testing import pytest_donorimp
import pytest

@pytest.mark.m_validated
@pytest.mark.m_auto_pass
def test_donorT14(capfd, indata_DonorT14, instatus_DonorT14, expected_outdonormap_06):
    pytest_donorimp(
        # Banff Parameters
        indata=indata_DonorT14,
        instatus=instatus_DonorT14,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        accept_negative=True,
        seed=1,
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Unit Test Parameters
        expected_outdonormap=expected_outdonormap_06,
        sas_log_name="donorT14_sas.txt",
        pytest_capture=capfd,
        rc_should_be_zero=True,
        expected_warning_count=1,
        msg_list_sas=[
            """WARNING: There were 3 observations dropped from indata data set because one/more edits 
                variables or one/more must_match variables outside the edits group are missing in 
                indata data set and there is no corresponding FTI in instatus data set.""",
            """Number of observations ............................:      10
                Number of observations dropped ....................:       3
                    missing key ...................................:       0
                    missing data ..................................:       3
                    negative data .................................:       0""",
            "Number of valid observations ......................:       7     100.00%",
            "Number of donors ..................................:       4      57.14%",
            "Number of recipients ..............................:       3      42.86%"
        ]
    )