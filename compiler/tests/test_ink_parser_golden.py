import json
from pathlib import Path

from openvn.ink_parser import parse_ink_file

GOLDEN_ROOT = Path(__file__).parent / "golden" / "story-graph"


def test_story_graph_golden() -> None:
    actual = parse_ink_file(GOLDEN_ROOT / "main.ink").to_dict()
    expected = json.loads((GOLDEN_ROOT / "expected.json").read_text(encoding="utf-8"))
    assert actual == expected
