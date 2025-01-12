import pytest
import banff.testing as banfftest
import banff
import time

@pytest.mark.m_pass_as_is
@pytest.mark.m_auto_pass
@pytest.mark.m_randomness # tests randomness, be careful: failure doesn't necessarily indicate an issue
def test_donorimp_p02_a(capfd, donorimp_indata_09, donorimp_instatus_09, donorimp_outdonormap_empty):
    """When the RANDOM option is specified, recipients without matching fields should 
    be matched with donors at random. The program must be run at least twice in 
    succession in order to obtain the necessary results.
    """

    call_1 = banfftest.pytest_donorimp(
        # Procedure Parameters
        indata=donorimp_indata_09,
        instatus=donorimp_instatus_09,
        outdonormap='pandas',
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        random=True,
        unit_id="ident",
        # must_match="staff", # intentionally commented out
        data_excl_var="toexcl"
        ,

        # Test Parameters
        pytest_capture=capfd,
        rc_should_be_zero=True,
    )

    banfftest.assert_dataset_value(call_1.banff_call.outdonormap, call_1.banff_call._outdonormap.name, 
                                   0, 'DONOR', ['REC01', 'REC02', 'REC05'])
    banfftest.assert_dataset_value(call_1.banff_call.outdonormap, call_1.banff_call._outdonormap.name, 
                                   0, 'RECIPIENT', ['REC04'])
    banfftest.assert_dataset_value(call_1.banff_call.outdonormap, call_1.banff_call._outdonormap.name, 
                                   0, 'NUMBER_OF_ATTEMPTS', [0])

    # Now we validate that the DONOR is truly a random selection
    # continue calling the procedure until a different DONOR is used

    # pytest: don'r capture log during this series of calls, it breaks things and is pointless
    print("\nRe-running repeatedly until different random donor used, procedure output suppressed")
    call_2 = banff.donorimp(
        # Procedure Parameters
        indata=donorimp_indata_09,
        instatus=donorimp_instatus_09,
        outdonormap='pandas',
        edits="Q1 + Q2 + Q3 + Q4 = total;",
        post_edits="Q1 + Q2 + Q3 + Q4 - total <= 0;",
        min_donors=1,
        percent_donors=1,
        n=3,
        outmatching_fields=True,
        eligdon="any",
        random=True,
        unit_id="ident",
        # must_match="staff", # intentionally commented out
        data_excl_var="toexcl",
        capture=None
    )

    banfftest.assert_dataset_value(call_2.outdonormap, call_2._outdonormap.name, 
                                0, 'DONOR', ['REC01', 'REC02', 'REC05'])
    banfftest.assert_dataset_value(call_2.outdonormap, call_1.banff_call._outdonormap.name, 
                                0, 'RECIPIENT', ['REC04'])
    banfftest.assert_dataset_value(call_2.outdonormap, call_1.banff_call._outdonormap.name, 
                                0, 'NUMBER_OF_ATTEMPTS', [0])

    tries_total = 15
    tries_remaining = tries_total
    while tries_remaining > 0 and call_1.banff_call.outdonormap.at[0,'DONOR'] == call_2.outdonormap.at[0,'DONOR']:
        print("Attempt {}: Both calls produced the same DONOR, {}, trying again".format(tries_total - tries_remaining + 1, call_2.outdonormap.at[0,'DONOR']))
        tries_remaining = tries_remaining - 1
        # wait 0.25 seconds: otherwise chances of getting the same result are higher
        time.sleep(.25)
        call_2.rerun()
    if tries_remaining <= 0:
        print("  FAILED")
    assert tries_remaining > 0, "failed to randomly produce a different result after {} calls to procedure".format(tries_total - tries_remaining + 1)
    print("Different DONOR selected on subsequent call number {}".format(tries_total - tries_remaining + 1))
    print("The first {} call{} used {}, last call used {}".format(tries_total - tries_remaining, "s" if tries_total - tries_remaining > 1 else "", call_1.banff_call.outdonormap.at[0,'DONOR'], call_2.outdonormap.at[0,'DONOR']))


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()