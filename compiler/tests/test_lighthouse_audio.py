from pathlib import Path

from openvn.assets import load_asset_manifest
from openvn.backends.renpy import export_renpy_project
from openvn.backends.renpy.assets import render_audio
from openvn.compiler import compile_project

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "the-last-lighthouse"


def test_lighthouse_audio_definitions() -> None:
    manifest = load_asset_manifest(PROJECT)
    audio = render_audio(manifest)

    assert 'define audio.storm = "music/storm.wav"' in audio
    assert 'define audio.tension = "music/tension.wav"' in audio
    assert 'define audio.dawn = "music/dawn.wav"' in audio
    assert 'define audio.radio = "sfx/radio.wav"' in audio
    assert 'define audio.thunder = "sfx/thunder.wav"' in audio
    assert 'define audio.generator = "sfx/generator.wav"' in audio
    assert 'define audio.beacon = "sfx/beacon.wav"' in audio


def test_generated_options_enable_conflict_checks(tmp_path: Path) -> None:
    story_path = compile_project(PROJECT, strict=True)
    output = export_renpy_project(story_path, tmp_path / "renpy", clean=True)
    options = (output / "game/options.rpy").read_text(encoding="utf-8")

    assert "define config.check_conflicting_properties = True" in options
