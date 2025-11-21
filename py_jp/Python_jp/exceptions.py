"""
Custom exceptions for PyJaponica
"""

class PyJaponicaError(Exception):
    """Base exception for PyJaponica"""
    pass


class TranslationError(PyJaponicaError):
    """Exception raised when translation fails"""
    pass


class PreprocessorError(PyJaponicaError):
    """Exception raised when preprocessing fails"""
    pass


class RunnerError(PyJaponicaError):
    """Exception raised when running code fails"""
    pass