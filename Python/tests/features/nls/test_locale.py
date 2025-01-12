import pytest
import banff.testing
import locale

@pytest.mark.m_auto_pass
def test_numeric_locale(capfd, ug_errorloc_ex02_indata):
    """Some non-English locales cause C code to misinterpret numeric strings.  
    For example C function `strtod()` converts '1.2' to 1.2 when the locale is English
    but expects results in 1 if French because that locale uses commas instead of periods as
    a decimal separator.  

    C code should temporarily set the locale LC_NUMERIC locale category to English during
    execution, and should restore the original locale after execution.  
    """
    pre_test_locale = locale.setlocale(locale.LC_NUMERIC)
    print(f"original LC_NUMERIC locale: {pre_test_locale}")

    try:
        # different systems have different available locales, so try until success
        locales_to_try = [
            "fr",
            "fr_FR",
            "fr_FR.UTF-8",
            "fr_CA",
            "fr_CA.UTF-8",
            "fr-FR",
            "fr-FR.UTF-8",
            "fr-CA",
            "fr-CA.UTF-8",
        ]
        locale_success = False
        for loc in locales_to_try:
            try:
                locale.setlocale(locale.LC_NUMERIC, loc)
            except:
                print(f"failed to set locale to '{loc}'")
                pass
            else:
                print(f"successfuly to set locale to '{loc}'")
                locale_success = True
                break
        if locale_success is False:
            raise AssertionError("Unable to set locale")

        pre_proc_locale = locale.setlocale(locale.LC_NUMERIC)
        print(f"LC_NUMERIC locale set to: {pre_proc_locale}")

        banff.testing.pytest_errorloc(
            indata=ug_errorloc_ex02_indata,
            edits="X+Y>=6.0; X<=4; Y<=5;",  # of key importance is the '0.6'
            unit_id="REC",
            rand_num_var="randkey",

            pytest_capture=capfd,
            expected_error_count=0,
            rc_should_be_zero=True,
        )

        post_proc_locale = locale.setlocale(locale.LC_NUMERIC)
        print(f"LC_NUMERIC locale after execution: {post_proc_locale}")

        assert pre_proc_locale == post_proc_locale, "C code modified but did not restore LC_NUMERIC locale"
    finally:
        locale.setlocale(locale.LC_NUMERIC, pre_test_locale)

    print(f"restored original locale: {locale.setlocale(locale.LC_NUMERIC)}")


# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banff.testing.run_pytest()