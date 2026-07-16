import json
from pathlib import Path

from openvn.compiler import compile_project

GOLDEN_ROOT = Path(__file__).parent / "golden" / "compile-minimal"


def test_compile_matches_golden(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        (GOLDEN_ROOT / "project.yaml").read_text(encoding="utf-8"),
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text(
        (GOLDEN_ROOT / "main.ink").read_text(encoding="utf-8"),
        encoding="utf-8",
    )

    output = compile_project(tmp_path)
    actual = json.loads(output.read_text(encoding="utf-8"))
    expected = json.loads((GOLDEN_ROOT / "expected.json").read_text(encoding="utf-8"))

    assert actual == expected
