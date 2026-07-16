import json
from pathlib import Path

from openvn.cli import main


def create_valid_project(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("Hello\n-> END\n")


def test_validate_cli(tmp_path: Path, capsys) -> None:
    create_valid_project(tmp_path)
    assert main(["validate", str(tmp_path)]) == 0
    assert "valid" in capsys.readouterr().out


def test_validate_json_cli(tmp_path: Path, capsys) -> None:
    create_valid_project(tmp_path)
    assert main(["validate", str(tmp_path), "--json"]) == 0
    payload = json.loads(capsys.readouterr().out)
    assert payload == {"ok": True, "diagnostics": []}


def test_validate_json_cli_reports_errors(tmp_path: Path, capsys) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("* [Broken]\n")
    assert main(["validate", str(tmp_path), "--json"]) == 1
    payload = json.loads(capsys.readouterr().out)
    assert payload["ok"] is False
    assert payload["diagnostics"][0]["code"] == "OVN001"


def test_compile_cli(tmp_path: Path) -> None:
    create_valid_project(tmp_path)
    assert main(["compile", str(tmp_path)]) == 0
    assert (tmp_path / "story.openvn.json").is_file()


def test_compile_strict_rejects_warning(tmp_path: Path, capsys) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("=== start ===\n-> END\n=== unused ===\n-> END\n")
    assert main(["compile", str(tmp_path), "--strict"]) == 1
    assert "OVN004" in capsys.readouterr().err


def test_invalid_cli_returns_structured_error(tmp_path: Path, capsys) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("* [Broken]\n")
    assert main(["validate", str(tmp_path)]) == 1
    error = capsys.readouterr().err
    assert "error OVN001" in error
    assert "main.ink:1:1" in error


def test_validate_cli_prints_warning(tmp_path: Path, capsys) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("=== start ===\n-> END\n=== unused ===\n-> END\n")
    assert main(["validate", str(tmp_path)]) == 0
    output = capsys.readouterr().out
    assert "warning OVN004" in output
