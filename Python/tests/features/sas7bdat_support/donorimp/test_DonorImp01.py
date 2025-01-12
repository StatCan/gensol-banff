# import necessary Python packages
import banff.testing as banfftest
# create Banff call 
import pytest

@pytest.mark.m_auto_pass
@pytest.mark.m_sas_dataset
@pytest.mark.m_needs_sas_data
def test_donorimp_userguide_01_a(capfd):
    """Donor Imputation User Guide example 1"""
    banff_call= banfftest.pytest_donorimp(
                indata="donorimp_ug01_a_indata.sas7bdat",
                instatus="donorimp_ug01_a_instatus.sas7bdat",
                unit_id="IDENT", 
                edits="Q1 + Q2 + Q3 + Q4 - TOTAL = 0;",
                post_edits="Q1 + Q2 + Q3 + Q4 - TOTAL <= 0;",
                min_donors=1,
                percent_donors=1,
                n=1,
                n_limit=1,
                mrl=0.5,
                random=True,
                eligdon="original",
                outmatching_fields=True,
                accept_negative=True,
                must_match="STAFF",
                data_excl_var="TOEXCL",
                by="prov",

                pytest_capture=capfd,
                expected_warning_count=None,
                expected_outdata="donorimp_ug01_a_outdata.sas7bdat",
                # outstatus changed in version 3.1.1b12: `expected_outstatus="donorimp_ug01_a_outstatus.sas7bdat",`
                expected_outdonormap="donorimp_ug01_a_donormap.sas7bdat",
            )


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()