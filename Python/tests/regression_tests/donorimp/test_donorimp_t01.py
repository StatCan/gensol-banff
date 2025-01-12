import pytest
import banff.testing as banfftest

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_t01_a(capfd, donorimp_indata_11, donorimp_instatus_11, donorimp_outdonormap_05):
    """To verify that variables with IDE status in the input data set are considered original/not imputed.
    """

    test_call = banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_11,
        instatus=donorimp_instatus_11,
        outdonormap='pandas',
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=7,
        outmatching_fields=True,
        eligdon="o",
        unit_id="ident",
        must_match="staff",
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

    # outdonormap's RECIPIENT, NUMBER_OF_ATTEMPTS, and DONORLIMIT columns should be fixed, so subset the output dataset and compare it
    banfftest.assert_dataset_equal(test_call.banff_call.outdonormap[["RECIPIENT", "NUMBER_OF_ATTEMPTS", "DONORLIMIT"]], donorimp_outdonormap_05, test_call.banff_call._outdonormap.name, True)

    # in outdonormap's DONOR column, all donors should end with 2
    for row in range(test_call.banff_call.outdonormap.shape[0]):
        banfftest.assert_dataset_value(test_call.banff_call.outdonormap, test_call.banff_call._outdonormap.name, 
                                    row, "DONOR", ["REC02", "REC12", "REC22", "REC32", "REC42", "REC52"])


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()