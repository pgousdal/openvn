from pathlib import Path

from openvn.validator import validate_project


def test_valid_project(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("Hello\n-> END\n")
    result = validate_project(tmp_path)
    assert result.ok
    assert result.errors == ()


def test_missing_ink_file(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    result = validate_project(tmp_path)
    assert not result.ok
    assert "missing Ink entry file" in result.errors[0]


def test_requires_end_terminator(tmp_path: Path) -> None:
    (tmp_path / "project.yaml").write_text("name: Demo\nentry: main.ink\n")
    (tmp_path / "main.ink").write_text("Hello\n")
    result = validate_project(tmp_path)
    assert not result.ok
    assert "-> END" in result.errors[0]
