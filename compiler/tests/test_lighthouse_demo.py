from pathlib import Path

from openvn.assets import load_asset_manifest
from openvn.backends.renpy import export_renpy_project
from openvn.compiler import compile_project
from openvn.ink_parser import parse_ink_file
from openvn.model.validation import unreachable_node_ids, validate_story

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "the-last-lighthouse"


def test_lighthouse_story_has_five_endings() -> None:
    story = parse_ink_file(PROJECT / "main.ink")
    ending_symbols = {name for name in story.symbols if name.startswith("ending_")}
    assert ending_symbols == {
        "ending_all",
        "ending_jonas",
        "ending_mina",
        "ending_light",
        "ending_sacrifice",
    }


def test_lighthouse_story_is_valid_and_reachable() -> None:
    story = parse_ink_file(PROJECT / "main.ink")
    assert validate_story(story) == []
    assert unreachable_node_ids(story) == set()


def test_lighthouse_assets_are_real_files() -> None:
    manifest = load_asset_manifest(PROJECT)
    files = manifest.all_files()
    assert len(files) == 18
    assert all(path.stat().st_size > 32 for path in files)

    png_files = [path for path in files if path.suffix == ".png"]
    wav_files = [path for path in files if path.suffix == ".wav"]
    assert all(path.read_bytes().startswith(b"\x89PNG\r\n\x1a\n") for path in png_files)
    assert all(path.read_bytes().startswith(b"RIFF") for path in wav_files)


def test_lighthouse_end_to_end_export(tmp_path: Path) -> None:
    story_path = compile_project(PROJECT, strict=True)
    output = export_renpy_project(
        story_path,
        tmp_path / "lighthouse-renpy",
        clean=True,
    )

    expected = [
        output / "game/script.rpy",
        output / "game/images.rpy",
        output / "game/audio.rpy",
        output / "game/options.rpy",
        output / "game/backgrounds/lighthouse_storm.png",
        output / "game/characters/erik_neutral.png",
        output / "game/music/storm.wav",
        output / "game/sfx/radio.wav",
    ]
    assert all(path.is_file() for path in expected)

    script = (output / "game/script.rpy").read_text(encoding="utf-8")
    assert "scene bg lighthouse_storm" in script
    assert "play music audio.storm" in script
    assert "ENDING: The Last Signal" in script
    assert "ENDING: Safe Harbour" in script
    assert "ENDING: The Hidden Stair" in script
    assert "ENDING: The Light Endures" in script
    assert "ENDING: Keeper of the Storm" in script
