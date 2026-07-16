from pathlib import Path

from openvn.cli import main


def test_export_renpy_cli(tmp_path: Path) -> None:
    project = tmp_path / "project"
    project.mkdir()
    (project / "project.yaml").write_text(
        "name: Demo\nentry: main.ink\nformat_version: '0.3'\n",
        encoding="utf-8",
    )
    (project / "main.ink").write_text("Hello\n-> END\n", encoding="utf-8")

    output = tmp_path / "renpy-output"
    assert (
        main(
            [
                "export",
                str(project),
                "--backend",
                "renpy",
                "--output",
                str(output),
            ]
        )
        == 0
    )
    assert (output / "game/script.rpy").is_file()
