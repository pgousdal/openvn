from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .errors import OpenVNError
from .ink import load_ink, validate_minimal_ink
from .project import Project, load_project


@dataclass(frozen=True)
class ValidationResult:
    ok: bool
    errors: tuple[str, ...]
    project: Project | None = None


def validate_project(project_dir: str | Path) -> ValidationResult:
    try:
        project = load_project(project_dir)
        source = load_ink(project.entry_path)
        errors = tuple(validate_minimal_ink(source))
        return ValidationResult(ok=not errors, errors=errors, project=project)
    except OpenVNError as exc:
        return ValidationResult(ok=False, errors=(str(exc),))
