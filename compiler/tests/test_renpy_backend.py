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
                "text": "Hello, verden 🌍.",
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


def test_render_script_is_deterministic() -> None:
    document = story_document()
    assert render_script(document) == render_script(document)


def test_render_script_supports_unicode_and_all_nodes() -> None:
    document = story_document()
    document["nodes"].insert(
        2,
        {"id": "jump-node", "type": "jump", "target": "end-0001"},
    )
    script = render_script(document)
    assert "Hello, verden 🌍." in script
    assert "label ovn_jump_node:" in script
    assert "jump ovn_end_0001" in script


def test_empty_text_node_is_rendered() -> None:
    document = story_document()
    document["nodes"][0]["text"] = ""
    assert '    ""' in render_script(document)


def test_export_clean_removes_stale_files(tmp_path: Path) -> None:
    story = tmp_path / "story.openvn.json"
    story.write_text(json.dumps(story_document()), encoding="utf-8")
    output = tmp_path / "renpy"
    output.mkdir()
    (output / "stale.txt").write_text("old", encoding="utf-8")

    export_renpy_project(story, output, clean=True)

    assert not (output / "stale.txt").exists()
    assert (output / "game/script.rpy").is_file()


def test_rejects_unknown_target() -> None:
    document = story_document()
    document["nodes"][0]["next"] = "missing"
    with pytest.raises(OpenVNError, match="unknown text target"):
        render_script(document)


def test_rejects_label_collision() -> None:
    document = story_document()
    document["nodes"] = [
        {"id": "a-b", "type": "end"},
        {"id": "a_b", "type": "end"},
    ]
    document["entry"] = "a-b"
    with pytest.raises(OpenVNError, match="collide"):
        render_script(document)


def test_rejects_unknown_version() -> None:
    document = story_document()
    document["version"] = "9.9"
    with pytest.raises(OpenVNError, match="unsupported OpenVN format version"):
        render_script(document)
