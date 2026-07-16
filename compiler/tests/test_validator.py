from pathlib import Path

from openvn.validator import validate_project


def test_valid_project(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("Hello\n-> END\n")
    result = validate_project(tmp_path)
    assert result.ok
    assert result.errors == ()
    assert result.diagnostics == ()


def test_missing_ink_file(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    result = validate_project(tmp_path)
    assert not result.ok
    assert result.diagnostics[0].code == "OVN001"
    assert "missing Ink source file" in result.errors[0]


def test_unknown_target_diagnostic(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("-> missing\n")
    result = validate_project(tmp_path)
    assert not result.ok
    assert result.diagnostics[0].code == "OVN002"


def test_unreachable_node_is_warning(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("=== start ===\n-> END\n=== unused ===\n-> END\n")
    result = validate_project(tmp_path)
    assert result.ok
    warnings = [item for item in result.diagnostics if item.severity == "warning"]
    assert warnings[0].code == "OVN004"
