import json
from pathlib import Path

import pytest

from openvn.compiler import compile_project
from openvn.errors import OpenVNError


def test_compile_writes_semantic_story_json(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Minimal\nentry: main.ink\nformat_version: '0.2'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text("Hello\n-> END\n", encoding="utf-8")

    output = compile_project(tmp_path)
    data = json.loads(output.read_text(encoding="utf-8"))

    assert output.name == "story.openvn.json"
    assert data["format"] == "openvn-story"
    assert data["version"] == "0.2"
    assert data["nodes"] == [
        {"id": "start-0001", "type": "text", "text": "Hello"},
        {"id": "start-0002", "type": "end"},
    ]
    assert "source_text" not in data


def test_compile_rejects_invalid_targets(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Broken\nentry: main.ink\nformat_version: '0.2'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text("-> missing\n", encoding="utf-8")

    with pytest.raises(OpenVNError, match="unknown jump target"):
        compile_project(tmp_path)
