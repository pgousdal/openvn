import json
from pathlib import Path

import pytest

from openvn.compiler import compile_project
from openvn.errors import OpenVNError


def test_compile_writes_story_graph(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Minimal\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text("Hello\n-> END\n", encoding="utf-8")

    output = compile_project(tmp_path)
    data = json.loads(output.read_text(encoding="utf-8"))

    assert data["version"] == "0.3"
    assert data["entry"] == "start-0001"
    assert data["symbols"] == {"start": "start-0001"}
    assert data["nodes"][0]["next"] == "start-0002"


def test_compile_resolves_knot_targets(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Branching\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text(
        "=== start ===\n-> ending\n=== ending ===\n-> END\n",
        encoding="utf-8",
    )

    output = compile_project(tmp_path)
    data = json.loads(output.read_text(encoding="utf-8"))
    assert data["nodes"][0]["target"] == "ending-0001"


def test_compile_rejects_invalid_targets(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Broken\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text("-> missing\n", encoding="utf-8")

    with pytest.raises(OpenVNError, match="unknown jump target"):
        compile_project(tmp_path)


def test_strict_compile_rejects_unreachable_nodes(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text(
        "name: Strict\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (tmp_path / "main.ink").write_text(
        "=== start ===\n-> END\n=== unused ===\n-> END\n",
        encoding="utf-8",
    )

    with pytest.raises(OpenVNError) as captured:
        compile_project(tmp_path, strict=True)

    assert captured.value.diagnostic is not None
    assert captured.value.diagnostic.code == "OVN004"
