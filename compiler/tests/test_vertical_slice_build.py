import json
from pathlib import Path

from openvn.cli import main

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "demo"


def test_vertical_slice_builds_both_targets(tmp_path: Path) -> None:
    output = tmp_path / "demo-build"

    assert main(["build", str(PROJECT), "--output", str(output), "--clean"]) == 0

    summary = json.loads((output / "build.json").read_text(encoding="utf-8"))
    assert summary["project"] == "OpenVN Asset Demo"
    assert summary["targets"] == {"renpy": "renpy", "amiga-ocs": "amiga-ocs"}

    assert (output / "renpy/game/script.rpy").is_file()
    assert (output / "renpy/game/backgrounds/room.png").is_file()
    assert (output / "renpy/game/music/intro.mod").is_file()

    assert (output / "amiga-ocs/manifest.json").is_file()
    assert (output / "amiga-ocs/assets/backgrounds/room.iff").is_file()
    assert (output / "amiga-ocs/assets/characters/alice_neutral.iff").is_file()
    assert (output / "amiga-ocs/assets/music/intro.mod").is_file()
    assert (output / "amiga-ocs/assets/sfx/click.8svx").is_file()

    script = (output / "renpy/game/script.rpy").read_text(encoding="utf-8")
    assert "scene bg room" in script
    assert "show alice neutral" in script

    images = (output / "renpy/game/images.rpy").read_text(encoding="utf-8")
    assert 'image alice neutral = "characters/alice_neutral.png"' in images
    assert "play music audio.intro" in script
    assert "ENDING: A New Story" in script
    assert "ENDING: The Last Note" in script
