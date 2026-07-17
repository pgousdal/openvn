from __future__ import annotations

import json
import shutil
from pathlib import Path

from ...assets import load_asset_manifest
from ...errors import OpenVNError
from .conversion import convert_asset
from .profiles import AmigaProfile


def render_arexx_bootstrap() -> str:
    return (
        "/* OpenVN Amiga bootstrap */\n"
        "ADDRESS 'OPENVNPLAYER'\n"
        "'LOAD story/story.openvn.json'\n"
        "'RUN'\n"
        "EXIT\n"
    )


def _music_paths(manifest) -> set[Path]:
    return {manifest.root / path for path in manifest.music.values()}


def export_amiga_package(
    *,
    project_root: str | Path,
    story_path: str | Path,
    output_dir: str | Path,
    profile: AmigaProfile,
    clean: bool = False,
) -> Path:
    project = Path(project_root).resolve()
    story = Path(story_path).resolve()
    output = Path(output_dir).resolve()

    if not story.is_file():
        raise OpenVNError(f"missing OpenVN story file: {story}")

    if clean and output.exists():
        shutil.rmtree(output)

    manifest = load_asset_manifest(project)
    (output / "story").mkdir(parents=True, exist_ok=True)
    (output / "assets").mkdir(parents=True, exist_ok=True)
    (output / "runtime").mkdir(parents=True, exist_ok=True)

    shutil.copy2(story, output / "story" / "story.openvn.json")
    (output / "story" / "main.rexx").write_text(
        render_arexx_bootstrap(),
        encoding="utf-8",
    )

    music_files = _music_paths(manifest)
    assets = []
    for source in manifest.all_files():
        relative = source.relative_to(manifest.root)
        destination, conversion = convert_asset(
            source,
            output / "assets",
            relative=relative,
            profile=profile,
            is_music=source in music_files,
        )
        assets.append(
            {
                "source": str(relative),
                "packaged": str(destination.relative_to(output / "assets")),
                "conversion": conversion,
            }
        )

    package_manifest = {
        "format": "openvn-amiga-package",
        "version": "0.2",
        "profile": profile.id,
        "story": "story/story.openvn.json",
        "bootstrap": "story/main.rexx",
        "runtime": {
            "player": "runtime/openvn-player",
            "arexx_first": True,
            "native_c": True,
            "ace_dependency": False,
            "system_friendly": True,
        },
        "assets": sorted(assets, key=lambda item: item["source"]),
    }

    (output / "manifest.json").write_text(
        json.dumps(package_manifest, indent=2) + "\n",
        encoding="utf-8",
    )
    (output / "runtime" / "README.txt").write_text(
        "OpenVN native AmigaOS player will be added in M5 PR3.\n"
        "The player must use ARexx and direct AmigaOS APIs without ACE.\n",
        encoding="utf-8",
    )

    return output
