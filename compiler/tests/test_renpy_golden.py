from pathlib import Path

from openvn.backends.renpy.generator import render_script

GOLDEN = Path(__file__).parent / "golden" / "renpy-branching"


def test_branching_script_matches_golden() -> None:
    import json

    document = json.loads((GOLDEN / "story.openvn.json").read_text(encoding="utf-8"))
    expected = (GOLDEN / "script.rpy").read_text(encoding="utf-8")
    assert render_script(document) == expected
