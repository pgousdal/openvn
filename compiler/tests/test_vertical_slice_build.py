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


def test_demo_is_the_native_m6_integration_fixture() -> None:
    story = json.loads((PROJECT / "story.openvn.json").read_text(encoding="utf-8"))
    nodes = story["nodes"]
    node_types = {node["type"] for node in nodes}

    assert {
        "scene",
        "show",
        "text",
        "choice",
        "set_bool",
        "set_int",
        "set_string",
        "condition",
        "jump",
        "music",
    } <= node_types
    assert sum(node["type"] == "text" for node in nodes) >= 2
    assert sum(node["type"] == "end" for node in nodes) >= 2

    manifest = (PROJECT / "assets/manifest.yaml").read_text(encoding="utf-8")
    assert "music/intro.mod" in manifest


def test_demo_mod_contains_audible_repeating_melody() -> None:
    module = (PROJECT / "assets" / "music" / "intro.mod").read_bytes()

    assert module[1080:1084] == b"M.K."
    assert module[950] == 1
    sample_length = int.from_bytes(module[42:44], "big") * 2
    loop_length = int.from_bytes(module[48:50], "big") * 2
    assert sample_length >= 64
    assert loop_length == sample_length

    pattern = module[1084 : 1084 + 1024]
    note_events = [pattern[offset : offset + 4] for offset in range(0, 1024, 4)]
    audible_notes = [event for event in note_events if event[0] & 0x0F or event[1]]
    assert len(audible_notes) >= 8
