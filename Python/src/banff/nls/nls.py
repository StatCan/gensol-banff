"""Native Language Support (NLS).

Implement bilingual message support via `gettext`.
"""
from banff._common.src.nls import (
    get_language,
)
from banff._common.src.nls.nls import (
    _init_nls,
)

translation_instances = {}
_init_nls(translation_instances, domain="banff", anchor="banff.nls")

def custom_gettext(a: str) -> str: # aka _(a):
    """Get gettext translation.

    Implements custom `gettext()` function.

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
