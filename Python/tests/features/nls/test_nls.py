# this file MUST be use UTF-8 encoding
"""Tests of Native Language Support (NLS)
"""
import banff.nls
import banff.testing as banfftest
import banff
import pytest
from basic_runners import *

expected_header_messages = [
    "NOTE: --- Système Banff ",
    "développé à Statistique Canada ---",
    "NOTE: PROCÉDURE ",
    "NOTE: Créée le",
    "NOTE: Courriel:",
]

@pytest.mark.m_auto_pass
def test_python_nls_en():
    original_lang = banff.nls.get_language()
    try:
        banff.set_language(banff.SupportedLanguage.en)
        test_msg = banff.nls.get_test_message()
        assert test_msg == 'TEST MESSAGE for Native Language Support: Forest, naive, French, Creme Brulee'
    finally:
        banff.set_language(original_lang)

@pytest.mark.m_auto_pass
def test_python_nls_fr():
    original_lang = banff.nls.get_language()
    try:
        banff.set_language(banff.SupportedLanguage.fr)
        test_msg = banff.nls.get_test_message()
        assert test_msg == 'MESSAGE DE TEST pour la prise en charge de la langue maternelle: forêt, naïf, français, crème brûlée'
    finally:
        banff.set_language(original_lang)

@pytest.mark.m_auto_pass
def test_nls_determin_a(capfd):
    # set C language to French
    banff.determin._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_determin_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_determin_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.determin._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_donorimp_a(capfd):
    # set C language to French
    banff.donorimp._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_donorimp_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_donorimp_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.donorimp._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_editstats_a(capfd):
    # set C language to French
    banff.editstats._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_editstats_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_editstats_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.editstats._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_errorloc_a(capfd):
    # set C language to French
    banff.errorloc._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_errorloc_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_errorloc_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.errorloc._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_estimator_a(capfd):
    # set C language to French
    banff.estimator._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_estimator_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_estimator_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.estimator._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_massimp_a(capfd):
    # set C language to French
    banff.massimp._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_massimp_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_massimp_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.massimp._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_outlier_a(capfd):
    # set C language to French
    banff.outlier._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_outlier_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_outlier_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.outlier._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_prorate_a(capfd):
    # set C language to French
    banff.prorate._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_prorate_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_prorate_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.prorate._reload_c(lang=original_lang)

@pytest.mark.m_auto_pass
def test_nls_verifyedits_a(capfd):
    # set C language to French
    banff.verifyedits._reload_c(lang=banff.SupportedLanguage.fr)

    original_lang = banff.nls.get_language()
    try:
        # run example 1, search for French header
        run_verifyedits_ex01(
            capfd=capfd,
            capture=False,
            msg_list_contains_exact=expected_header_messages,
        )

        run_verifyedits_ex01(
            capfd=capfd,
            capture=True,
            msg_list_contains_exact=expected_header_messages,
        )
    finally:
        banff.verifyedits._reload_c(lang=original_lang)

# invoke pytest if this file is executed directly
if __name__ == "__main__":
    banfftest.run_pytest()