import pytest
import banff.testing as banfftest
from banff.testing import assert_dataset_value

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
def test_donorimp_i02_a(capfd, donorimp_indata_02, donorimp_instatus_04):
    """If a donor was found randomly, then the number of attempts variable for 
    the corresponding recipient should be 0 in the donor map set.
    """

    test_call = banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_02,
        instatus=donorimp_instatus_04,
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        random=True,
        unit_id="ident"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True
    )

    assert_dataset_value(dataset=test_call.banff_call.outdonormap, dataset_name=test_call.banff_call._outdonormap.name, 
                         row_num=0, col_name='RECIPIENT',           expected_values=['REC04'])

    assert_dataset_value(dataset=test_call.banff_call.outdonormap, dataset_name=test_call.banff_call._outdonormap.name, 
                         row_num=0, col_name='DONOR',               expected_values=['REC01', 'REC02', 'REC05'])

    assert_dataset_value(dataset=test_call.banff_call.outdonormap, dataset_name=test_call.banff_call._outdonormap.name, 
                         row_num=0, col_name='NUMBER_OF_ATTEMPTS',  expected_values=[0])


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()