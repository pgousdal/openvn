from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml

from .errors import ProjectError


@dataclass(frozen=True)
class Project:
    root: Path
    name: str
    entry: str
    format_version: str

    @property
    def entry_path(self) -> Path:
        return self.root / self.entry

    @property
    def output_path(self) -> Path:
        return self.root / "story.openvn.json"


def _require_string(data: dict[str, Any], key: str) -> str:
    value = data.get(key)
    if not isinstance(value, str) or not value.strip():
        raise ProjectError(f"project.yaml field '{key}' must be a non-empty string")
    return value.strip()


def load_project(project_dir: str | Path) -> Project:
    root = Path(project_dir).resolve()
    manifest = root / "project.yaml"

    if not root.is_dir():
        raise ProjectError(f"project directory does not exist: {root}")
    if not manifest.is_file():
        raise ProjectError(f"missing project manifest: {manifest}")

    try:
        raw = yaml.safe_load(manifest.read_text(encoding="utf-8"))
    except yaml.YAMLError as exc:
        raise ProjectError(f"invalid YAML in {manifest}: {exc}") from exc

    if not isinstance(raw, dict):
        raise ProjectError("project.yaml must contain a mapping")

    project = Project(
        root=root,
        name=_require_string(raw, "name"),
        entry=_require_string(raw, "entry"),
        format_version=str(raw.get("format_version", "0.1")),
    )

    entry = Path(project.entry)
    if entry.is_absolute():
        raise ProjectError("entry must be relative to the project directory")

    resolved_entry = (root / entry).resolve()
    try:
        resolved_entry.relative_to(root)
    except ValueError as exc:
        raise ProjectError("entry must stay inside the project directory") from exc

    return project
