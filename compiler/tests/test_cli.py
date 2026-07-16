from pathlib import Path

from openvn.cli import main


def create_valid_project(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("Hello\n-> END\n")


def test_validate_cli(tmp_path: Path, capsys) -> None:
    create_valid_project(tmp_path)
    assert main(["validate", str(tmp_path)]) == 0
    assert "valid" in capsys.readouterr().out


def test_compile_cli(tmp_path: Path) -> None:
    create_valid_project(tmp_path)
    assert main(["compile", str(tmp_path)]) == 0
    assert (tmp_path / "story.openvn.json").is_file()


def test_invalid_cli_returns_nonzero(tmp_path: Path) -> None:
    assert main(["validate", str(tmp_path)]) == 1
