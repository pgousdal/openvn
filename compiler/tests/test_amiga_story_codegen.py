import json
import subprocess
from pathlib import Path

from openvn.backends.amiga.story_codegen import (
    render_story_source,
    write_generated_story,
)


def document() -> dict[str, object]:
    return {
        "format": "openvn-story",
        "version": "0.4",
        "entry": "start",
        "symbols": {"start": "start"},
        "nodes": [
            {
                "id": "start",
                "type": "text",
                "text": "Hello.",
                "next": "choice",
            },
            {
                "id": "choice",
                "type": "choice",
                "options": [
                    {"text": "Continue", "target": "end"},
                ],
            },
            {"id": "end", "type": "end"},
        ],
    }


def test_render_generated_story_source() -> None:
    source = render_story_source(document())
    assert "const OpenVNGeneratedStory OPENVN_GENERATED_STORY" in source
    assert "OPENVN_NODE_TEXT" in source
    assert '{"Continue", "end"}' in source
    assert '"start"' in source


def test_generated_story_compiles(tmp_path: Path) -> None:
    story = tmp_path / "story.openvn.json"
    story.write_text(json.dumps(document()), encoding="utf-8")
    generated = tmp_path / "generated"
    _, implementation = write_generated_story(story, generated)

    repository = Path(__file__).parents[2]
    include = repository / "runtimes/amiga-native/include"

    subprocess.run(
        [
            "/usr/bin/gcc",
            "-std=c90",
            "-I",
            str(include),
            "-I",
            str(generated),
            "-c",
            str(implementation),
            "-o",
            str(tmp_path / "story.o"),
        ],
        check=True,
    )


def test_render_runtime_variable_commands() -> None:
    variable_document = document()
    variable_document["nodes"] = [
        {
            "id": "bool",
            "type": "set_bool",
            "name": "has_key",
            "value": True,
            "next": "int",
        },
        {
            "id": "int",
            "type": "set_int",
            "name": "score",
            "value": 10,
            "next": "string",
        },
        {
            "id": "string",
            "type": "set_string",
            "name": "player_name",
            "value": "Alice",
            "next": "end",
        },
        {"id": "end", "type": "end"},
    ]
    variable_document["entry"] = "bool"

    source = render_story_source(variable_document)
    assert "OPENVN_NODE_SET_BOOL" in source
    assert '"has_key", "true"' in source
    assert "OPENVN_NODE_SET_INT" in source
    assert '"score", "10"' in source
    assert "OPENVN_NODE_SET_STRING" in source
    assert '"player_name", "Alice"' in source


def test_render_condition_node() -> None:
    condition_document = document()
    condition_document["nodes"] = [
        {
            "id": "condition",
            "type": "condition",
            "condition": {
                "variable_name": "score",
                "value_type": "int",
                "operator": ">=",
                "bool_value": False,
                "int_value": 10,
                "string_value": "",
            },
            "true_target": "true",
            "false_target": "false",
        },
        {"id": "true", "type": "end"},
        {"id": "false", "type": "end"},
    ]
    condition_document["entry"] = "condition"

    source = render_story_source(condition_document)
    assert "OPENVN_NODE_CONDITION" in source
    assert '"score", OPENVN_VARIABLE_INT' in source
    assert "OPENVN_CONDITION_GREATER_EQUAL" in source
    assert '10, ""}, "true", "false"' in source
