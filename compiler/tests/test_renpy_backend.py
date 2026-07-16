import json
from pathlib import Path

import pytest

from openvn.backends.renpy.generator import export_renpy_project, render_script
from openvn.errors import OpenVNError


def story_document() -> dict[str, object]:
    return {
        "format": "openvn-story",
        "version": "0.3",
        "entry": "start-0001",
        "symbols": {"start": "start-0001"},
        "nodes": [
            {
                "id": "start-0001",
                "type": "text",
                "text": "Hello.",
                "next": "start-0002",
            },
            {
                "id": "start-0002",
                "type": "choice",
                "options": [
                    {"text": "Continue", "target": "end-0001"},
                ],
            },
            {"id": "end-0001", "type": "end"},
        ],
    }


def test_render_script() -> None:
    script = render_script(story_document())
    assert "label start:" in script
    assert '    "Hello."' in script
    assert "menu:" in script
    assert '        "Continue":' in script
    assert "jump ovn_end_0001" in script


def test_export_project(tmp_path: Path) -> None:
    story = tmp_path / "story.openvn.json"
    story.write_text(json.dumps(story_document()), encoding="utf-8")
    output = export_renpy_project(story, tmp_path / "renpy")

    assert (output / "game/script.rpy").is_file()
    assert (output / "game/options.rpy").is_file()
    assert (output / "README.md").is_file()


def test_rejects_unknown_version() -> None:
    document = story_document()
    document["version"] = "9.9"
    with pytest.raises(OpenVNError, match="unsupported OpenVN format version"):
        render_script(document)


def test_rejects_unknown_node_type() -> None:
    document = story_document()
    document["nodes"] = [{"id": "x", "type": "unknown"}]
    with pytest.raises(OpenVNError, match="unsupported Ren'Py node type"):
        render_script(document)
