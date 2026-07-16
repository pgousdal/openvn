import json
from pathlib import Path

from openvn.cli import main
from openvn.doctor import run_doctor


def create_repository(root: Path) -> None:
    (root / "compiler/src/openvn").mkdir(parents=True)
    (root / "schemas").mkdir()
    (root / "schemas/openvn-story-0.3.schema.json").write_text("{}")
    (root / "examples/minimal").mkdir(parents=True)


def test_doctor_core_checks_pass(tmp_path: Path) -> None:
    create_repository(tmp_path)
    checks = run_doctor(tmp_path)
    assert all(check.ok for check in checks[:4])
    assert not checks[4].ok
    assert not checks[5].ok


def test_doctor_cli(tmp_path: Path, capsys) -> None:
    create_repository(tmp_path)
    assert main(["doctor", str(tmp_path)]) == 0
    output = capsys.readouterr().out
    assert "Compiler package" in output
    assert "Ren'Py backend" in output
    assert "MISSING" in output


def test_doctor_json_cli(tmp_path: Path, capsys) -> None:
    create_repository(tmp_path)
    assert main(["doctor", str(tmp_path), "--json"]) == 0
    payload = json.loads(capsys.readouterr().out)
    assert payload["ok"] is True
    assert payload["checks"][0]["name"] == "OpenVN version"


def test_doctor_fails_when_core_is_missing(tmp_path: Path) -> None:
    assert main(["doctor", str(tmp_path)]) == 1
