from __future__ import annotations

from .diagnostics import Diagnostic


class OpenVNError(Exception):
    """Base exception for user-facing OpenVN errors."""

    def __init__(self, message: str, diagnostic: Diagnostic | None = None) -> None:
        super().__init__(message)
        self.diagnostic = diagnostic

    def user_message(self) -> str:
        if self.diagnostic is not None:
            return self.diagnostic.format_text()
        return str(self)


class ProjectError(OpenVNError):
    """Raised when an OpenVN project is invalid."""


class SourceError(OpenVNError):
    """Raised when source material is invalid."""
