import json
from pathlib import Path

import pytest

from openvn.backends.amiga import export_amiga_package, load_amiga_profile
from openvn.compiler import compile_project
from openvn.errors import OpenVNError

REPOSITORY = Path(__file__).parents[2]
PROJECT = REPOSITORY / "examples" / "the-last-lighthouse"


def test_classic_package_rejects_mixed_music(tmp_path: Path) -> None:
    story = compile_project(PROJECT, strict=True)
    profile = load_amiga_profile(REPOSITORY / "profiles" / "amiga-ocs.yaml")

    with pytest.raises(OpenVNError, match="must be supplied as MOD"):
        export_amiga_package(
            project_root=PROJECT,
            story_path=story,
            output_dir=tmp_path / "amiga",
            profile=profile,
            clean=True,
        )


def test_export_rtg_keeps_native_formats(tmp_path: Path) -> None:
    story = compile_project(PROJECT, strict=True)
    profile = load_amiga_profile(REPOSITORY / "profiles" / "amiga-rtg.yaml")

    output = export_amiga_package(
        project_root=PROJECT,
        story_path=story,
        output_dir=tmp_path / "amiga",
        profile=profile,
    )

    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    assert manifest["profile"] == "amiga-rtg"
    assert manifest["runtime"]["arexx_first"] is True
    assert manifest["runtime"]["ace_dependency"] is False
    assert (output / "story/main.rexx").is_file()
    assert (output / "story/story.openvn.json").is_file()

    png_assets = [asset for asset in manifest["assets"] if asset["conversion"]["source"] == "png"]
    wav_assets = [asset for asset in manifest["assets"] if asset["conversion"]["source"] == "wav"]
    assert png_assets
    assert wav_assets
    assert all(asset["conversion"]["target"] == "png" for asset in png_assets)
    assert all(asset["conversion"]["target"] == "wav" for asset in wav_assets)
