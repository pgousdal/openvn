from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator

from .diagnostics import Diagnostic


def load_schema(schema_path: str | Path) -> dict[str, Any]:
    path = Path(schema_path)
    return json.loads(path.read_text(encoding="utf-8"))


def validate_document(
    document: dict[str, Any],
    *,
    schema_path: str | Path,
) -> list[Diagnostic]:
    validator = Draft202012Validator(load_schema(schema_path))
    diagnostics: list[Diagnostic] = []

    for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path)):
        location = ".".join(str(part) for part in error.path)
        message = error.message
        if location:
            message = f"{location}: {message}"
        diagnostics.append(
            Diagnostic(
                severity="error",
                code="OVN005",
                message=message,
                file=str(Path(schema_path)),
            )
        )

    return diagnostics
