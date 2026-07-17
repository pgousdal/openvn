import json
from pathlib import Path

from openvn.cli import main

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "demo"


def _package(tmp_path: Path) -> Path:
    build = tmp_path / "build"
    player = tmp_path / "openvn-player"
    player.write_bytes(b"Amiga executable fixture\n")

    assert main(["build", str(PROJECT), "--output", str(build), "--clean"]) == 0
    assert (
        main(
            [
                "package-fsuae",
                str(PROJECT),
                "--output",
                str(tmp_path / "fs-uae"),
                "--player",
                str(player),
                "--clean",
            ]
        )
        == 0
    )
    return tmp_path / "fs-uae"


def test_packages_vertical_slice_for_fsuae(tmp_path: Path) -> None:
    output = _package(tmp_path)
    summary = json.loads((output / "package.json").read_text(encoding="utf-8"))

    assert summary["format"] == "openvn-fsuae-package"
    assert summary["version"] == "0.2"
    assert summary["startup_mode"] == "direct-player"
    assert (output / "OpenVNDemo.fs-uae").is_file()
    assert (output / "harddrive/S/Startup-Sequence").is_file()
    assert (output / "harddrive/OpenVN/runtime/openvn-player").read_bytes() == (
        b"Amiga executable fixture\n"
    )
    assert (output / "harddrive/OpenVN/assets/music/intro.mod").is_file()
    assert (output / "harddrive/OpenVN/story/main.rexx").is_file()


def test_startup_sequence_requires_no_external_c_commands(tmp_path: Path) -> None:
    output = _package(tmp_path)
    startup = (output / "harddrive/S/Startup-Sequence").read_text(encoding="ascii")

    assert "CD DH0:OpenVN" in startup
    assert "runtime/openvn-player" in startup
    assert "DH0:OpenVN/runtime/openvn-player" not in startup
    assert "C:Assign" not in startup
    assert "C:Run" not in startup
    assert "C:Wait" not in startup
    assert "C:RX" not in startup


def test_clean_packaging_removes_stale_generated_files(tmp_path: Path) -> None:
    output = _package(tmp_path)
    stale = output / "harddrive" / "stale-file"
    stale.write_text("stale\n", encoding="utf-8")

    player = tmp_path / "openvn-player"
    assert (
        main(
            [
                "package-fsuae",
                str(PROJECT),
                "--output",
                str(output),
                "--player",
                str(player),
                "--clean",
            ]
        )
        == 0
    )

    assert not stale.exists()
