import json
from pathlib import Path

import pytest

from openvn.ink_parser import parse_ink_file

GOLDEN_ROOT = Path(__file__).parent / "golden"


@pytest.mark.parametrize("case", ["minimal", "branching"])
def test_golden_story(case: str) -> None:
    case_dir = GOLDEN_ROOT / case
    actual = parse_ink_file(case_dir / "main.ink").to_dict()
    expected = json.loads((case_dir / "expected.json").read_text(encoding="utf-8"))
    assert actual == expected
