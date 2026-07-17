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


def test_export_rtg_generates_story_and_asset_tables(tmp_path: Path) -> None:
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
    assert manifest["story"]["runtime_parses_json"] is False
    assert manifest["runtime"]["static_story_tables"] is True
    assert manifest["runtime"]["static_asset_tables"] is True
    assert (output / "story/story.generated.h").is_file()
    assert (output / "story/story.generated.c").is_file()
    assert (output / "story/assets.generated.h").is_file()
    assert (output / "story/assets.generated.c").is_file()
    assert (output / "story/main.rexx").is_file()

    asset_source = (output / "story/assets.generated.c").read_text(encoding="utf-8")
    assert "assets/backgrounds/lighthouse_storm.png" in asset_source
    assert "assets/characters/erik_neutral.png" in asset_source
