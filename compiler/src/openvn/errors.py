class OpenVNError(Exception):
    """Base exception for user-facing OpenVN errors."""


class ProjectError(OpenVNError):
    """Raised when an OpenVN project is invalid."""


class SourceError(OpenVNError):
    """Raised when source material is invalid."""
