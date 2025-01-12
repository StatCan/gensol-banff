"""Native Language Support (bilingual messages)."""

from banff._common.src.nls import (
    SupportedLanguage,
    get_language,
    set_language,
)
from banff.nls.nls import (
    custom_gettext as _,  # create commonly used `_()` alias for gettext
)


def get_test_message():
    return _("TEST MESSAGE for Native Language Support: Forest, naive, French, Creme Brulee")

__all__ = [
    "SupportedLanguage",
    "_",
    "get_language",
    "set_language",
]
