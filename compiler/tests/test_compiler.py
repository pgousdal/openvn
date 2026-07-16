import json
from pathlib import Path

from openvn.compiler import compile_project


def test_compile_writes_story_json(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Minimal\nentry: main.ink\nformat_version: '0.1'\n"
    )
    (tmp_path / "main.ink").write_text("Hello\n-> END\n")

    output = compile_project(tmp_path)
    data = json.loads(output.read_text())

    assert output.name == "story.openvn.json"
    assert data["format"] == "openvn-story"
    assert data["project_name"] == "Minimal"
    assert data["source"]["entry"] == "main.ink"
    assert data["source_text"] == "Hello\n-> END\n"
