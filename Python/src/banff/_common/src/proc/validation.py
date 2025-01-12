from ..nls import _


def validate_arg_type(arg_val, parm_name, allowed_types, skip_none=False, log=None):
    """Validate argument's type, log and raise `TypeError` if type invalid.

    Logs error message if `log` specified.
    To facilitate skipping validation of unspecified parameters,
    skips validation if `arg_val` is None and `skip_none` is True.
    """
    if skip_none and arg_val is None:
        return

    if not isinstance(arg_val, allowed_types):
        mesg = _("option '{}' must be type `{}`, but it is `{}`").format(
            parm_name,
            allowed_types,
            type(arg_val),
        )
        if log is not None:
            log.error(mesg)
        raise(TypeError(mesg))

def string_parm_is_empty(arg_val):
    """Whether string value is considered "empty".

    Returns `True` if empty, `False` otherwise
    """
    return len(arg_val.strip()) == 0
