from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .diagnostics import Diagnostic
from .errors import OpenVNError
from .ink_parser import parse_ink_file
from .model.validation import unreachable_diagnostics, validate_story
from .project import Project, load_project


@dataclass(frozen=True)
class ValidationResult:
    ok: bool
    diagnostics: tuple[Diagnostic, ...]
    project: Project | None = None

    @property
    def errors(self) -> tuple[str, ...]:
        return tuple(
            diagnostic.message for diagnostic in self.diagnostics if diagnostic.severity == "error"
        )


def validate_project(project_dir: str | Path) -> ValidationResult:
    try:
        project = load_project(project_dir)
        story = parse_ink_file(project.entry_path)
        diagnostics = tuple(validate_story(story) + unreachable_diagnostics(story))
        ok = not any(diagnostic.severity == "error" for diagnostic in diagnostics)
        return ValidationResult(ok=ok, diagnostics=diagnostics, project=project)
    except OpenVNError as exc:
        diagnostic = exc.diagnostic or Diagnostic("error", "OVN005", str(exc))
        return ValidationResult(ok=False, diagnostics=(diagnostic,))
