from __future__ import annotations

import json
from pathlib import Path

from .errors import OpenVNError
from .ink_parser import parse_ink_file
from .model.validation import unreachable_diagnostics, validate_story
from .project import load_project
from .schema_validation import validate_document


def _schema_path(project_root: Path) -> Path:
    return project_root.parents[1] / "schemas" / "openvn-story-0.3.schema.json"


def compile_project(project_dir: str | Path, *, strict: bool = False) -> Path:
    project = load_project(project_dir)
    story = parse_ink_file(project.entry_path)

    diagnostics = validate_story(story)
    if strict:
        diagnostics.extend(unreachable_diagnostics(story))

    document = story.to_dict()
    schema_path = _schema_path(project.root)
    if schema_path.is_file():
        diagnostics.extend(validate_document(document, schema_path=schema_path))

    blocking = [
        diagnostic
        for diagnostic in diagnostics
        if diagnostic.severity == "error" or (strict and diagnostic.severity == "warning")
    ]
    if blocking:
        first = blocking[0]
        raise OpenVNError(first.message, diagnostic=first)

    project.output_path.write_text(
        json.dumps(document, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    return project.output_path
