"""Native Language Support (NLS).

Implement bilingual message support via `gettext`.
"""
import gettext
import importlib.resources
import locale
from enum import Enum
from pathlib import Path
from warnings import warn


class SupportedLanguage(Enum):
    """Languages supported by Native Language Support module.

    Enums have the form <name> = <[list, of, values]>
    Users should refer to Enums by member-name, like `SupportedLanguage.en`.

    Internally, `.value` is used to associate a list of language tags with a supported language.
        <name> - only use valid RFC1766 language tags
        <value> - should be a list of associated language tags
            these tags should include both RFC1766 tags
            as well non RFC1766 tags observed in the wild (...on Windows)
    """

    en = ["en", "english"]
    fr = ["fr", "french"]
    UNKNOWN = []

def _init_nls(ti, domain="gscommon", anchor=None):
    """Initialize Native Language Support.

    Initializes the translation for each supported language,
    and sets the unknown language instance to English
    """
    ti[SupportedLanguage.en] = _init_translations_instance(
        language=SupportedLanguage.en,
        domain=domain,
        anchor=anchor,
    )
    ti[SupportedLanguage.fr] = _init_translations_instance(
        language=SupportedLanguage.fr,
        domain=domain,
        anchor=anchor,
    )
    ti[SupportedLanguage.UNKNOWN] = ti[SupportedLanguage.en]

def _init_translations_instance(
    language : SupportedLanguage,
    domain : str = "gscommon",
    locale_dir_relative : Path = "messages/",
    anchor : str | None = None,
    fallback : bool = True,
) -> gettext.NullTranslations:
    """Initialize translation instance for a given language.

    `language`: the `SupportedLanguage` member associated with the requested language
    `domain`: name (without extension) of the translation binary (i.e. "gscommon" for file "gscommon.mo")
    `locale_dir_relative`: relative path to locale directory
        - if `anchor` specified it is relative to the anchor
        - otherwise it is relative to this file's directory
    `anchor`: string representing where in the package the `locale_dir_relative` is relative to
    """
    if anchor is not None:
        locale_dir = importlib.resources.files(anchor) / locale_dir_relative
    else:
        locale_dir = Path(__file__).parent / locale_dir_relative

    # gettext expects a list of language codes
    locale_lang = [language.value[0]]

    return gettext.translation(
        languages=locale_lang,
        domain=domain,
        localedir=locale_dir,
        fallback=fallback,
    )

# store language settings in module level variables, access via functions (defined below)
default_language = SupportedLanguage.en
active_language = default_language
translation_instances = {}
_init_nls(translation_instances)

def custom_gettext(a: str) -> str: # aka _(a):
    """Call translation instances gettext function.

    Custom implementation of `gettext()` function.
    To be aliased as `_()` when used by other modules.

    Uses the gettext instance corresponding to the active language.

    Why not use `gettext.install()`?
        - it goes against a tsunami of recommendations against modifying builtins
        - it plays poorly with intellisense
        - it makes the origin of the `_()` function untraceable to new developers
        - it may affect (or be affected by) 3rd party Python Code
          - installed `_()` can become broken (redefined, undefined) when running interactively.
    """
    return translation_instances[get_language()].gettext(a)

def determine_language() -> SupportedLanguage:
    """Try to determine user's language and return corresponding `SupportedLanguage` member.

    Calls `locale.getlocale()`, performs custom search on language portion of return value
    for French and English language codes as observed in the wild (i.e., Windows AVD, Linux VM).
        Users can affect the outcome using `locale.setlocale()` prior to importing the package.
        For example, to use French run `locale.setlocale(locale.LC_CTYPE, "fr_CA")`

    Returns associated `SupportedLanguage` member.
        `.UNKNOWN` if language not supported

    about `locale.getlocale()`
        [non RFC1766 language codes](https://github.com/python/cpython/issues/82986)
        - Around Python 3.8 on Windows, this function started returning codes
          like 'English_United States', instead of 'en_US'
    about `locale.getdefaultlocale()`
        [getdefaultlocale() deprecated](https://github.com/python/cpython/pull/31206)
        - tempting to use since it returns rfc1755 codes seemingly in all versions
        - not used because it's deprecated in Python 3.13
    """
    try:
        # get current language
        current_locale = locale.getlocale()[0] # like `'en_US'` or `'eNgLIsH_United States'`
        current_language = current_locale.lower().split("_")[0] # like `'en'` or `'english'`

        # return associated supported language
        # search for supported language's with matching language value
        for sl in SupportedLanguage:
            if current_language in sl.value:
                return sl

        return SupportedLanguage.UNKNOWN  # noqa: TRY300
    except IndexError:
        return SupportedLanguage.UNKNOWN

def get_language() -> SupportedLanguage:
    """Return the active language."""
    return active_language

def set_language(lang : SupportedLanguage | None= None) -> None:
    """Set the active language to `lang`, or determine language based on locale.

    Specify a member of `SupportedLanguage` for `lang`.

    The value set by this function will affect log message output.  It may be referred to
    (via `get_language()`) by other modules in order to implement NLS.
    """
    _ = custom_gettext
    # get or validate `lang`
    if lang is None:
        lang = determine_language()
    elif not isinstance(lang, SupportedLanguage):
        mesg = _("Invalid `lang`, expecting `SupportedLanguage` member")
        raise TypeError(mesg)

    # Set language (module level variable)
    global active_language # use module-level `active_language` variable
    if lang == SupportedLanguage.UNKNOWN:
        active_language = default_language
        mesg = _("Language not supported, using default language: {}").format(default_language.name)
        warn(mesg, stacklevel=2)
    else:
        active_language = lang
