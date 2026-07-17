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
