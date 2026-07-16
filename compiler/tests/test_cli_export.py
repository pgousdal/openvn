from pathlib import Path

from openvn.cli import main


def make_project(root: Path) -> Path:
    project = root / "project"
    project.mkdir()
    (project / "project.yaml").write_text(
        "name: Demo\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (project / "main.ink").write_text(
        "=== start ===\nHello\n-> END\n",
        encoding="utf-8",
    )
    return project


def test_export_renpy_cli_with_clean(tmp_path: Path) -> None:
    project = make_project(tmp_path)
    output = tmp_path / "renpy-output"
    output.mkdir()
    (output / "stale.txt").write_text("stale", encoding="utf-8")

    assert (
        main(
            [
                "export",
                str(project),
                "--backend",
                "renpy",
                "--output",
                str(output),
                "--clean",
            ]
        )
        == 0
    )
    assert not (output / "stale.txt").exists()
    assert (output / "game/script.rpy").is_file()
