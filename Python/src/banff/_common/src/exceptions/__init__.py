### Procedure Exceptions ###
class ProcedureError(Exception):
    pass

class ProcedureNotLoadedError(ProcedureError):
    """For attempts to call procedure C code build when it is not loaded."""


class ProcedureIOError(ProcedureError):
    pass

class ProcedureInputError(ProcedureIOError):
    pass

class ProcedureOutputError(ProcedureIOError):
    pass

class ProcedureValidationError(ProcedureError):
    pass

class ProcedureCError(ProcedureError):
    """For errors during C code execution.

    Specify the C function's return code with `return_code` when constructing this exception.
    """

    def __init__(self, *args, return_code=None):
        super().__init__(*args)
        self.return_code = return_code


### IO Util Exceptions ###
class IOUtilError(Exception):
    pass

class TypeConverterError(IOUtilError):
    pass

class ProcessingError(IOUtilError):
    pass

class NormalizationError(ProcessingError):
    pass
