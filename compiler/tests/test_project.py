from pathlib import Path

import pytest

from openvn.errors import ProjectError
from openvn.project import load_project


def make_project(tmp_path: Path, manifest: str = "name: Demo\nentry: main.ink\n") -> Path:
    (tmp_path / "project.yaml").write_text(manifest, encoding="utf-8")
    (tmp_path / "main.ink").write_text("Hello\n-> END\n", encoding="utf-8")
    return tmp_path


def test_load_project(tmp_path: Path) -> None:
    project = load_project(make_project(tmp_path))
    assert project.name == "Demo"
    assert project.entry == "main.ink"
    assert project.format_version == "0.1"


def test_missing_manifest(tmp_path: Path) -> None:
    with pytest.raises(ProjectError, match="missing project manifest"):
        load_project(tmp_path)


def test_manifest_requires_name(tmp_path: Path) -> None:
    make_project(tmp_path, "entry: main.ink\n")
    with pytest.raises(ProjectError, match="'name'"):
        load_project(tmp_path)


def test_entry_cannot_escape_project(tmp_path: Path) -> None:
    make_project(tmp_path, "name: Demo\nentry: ../outside.ink\n")
    with pytest.raises(ProjectError, match="inside"):
        load_project(tmp_path)
