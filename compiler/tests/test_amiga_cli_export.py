from pathlib import Path

from openvn.cli import main

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "the-last-lighthouse"


def test_amiga_rtg_export_cli(tmp_path: Path) -> None:
    output = tmp_path / "amiga-rtg"
    result = main(
        [
            "export",
            str(PROJECT),
            "--backend",
            "amiga",
            "--profile",
            "amiga-rtg",
            "--output",
            str(output),
            "--clean",
        ]
    )
    assert result == 0
    assert (output / "manifest.json").is_file()
    assert (output / "story/main.rexx").is_file()
    assert (output / "assets/backgrounds/lighthouse_storm.png").is_file()


def test_amiga_export_requires_profile(tmp_path: Path, capsys) -> None:
    result = main(
        [
            "export",
            str(PROJECT),
            "--backend",
            "amiga",
            "--output",
            str(tmp_path / "out"),
        ]
    )
    assert result == 1
    assert "--profile is required" in capsys.readouterr().err
