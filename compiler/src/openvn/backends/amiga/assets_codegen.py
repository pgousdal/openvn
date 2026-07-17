from __future__ import annotations

from pathlib import Path

from ...assets import AssetManifest


def _c_string(value: str) -> str:
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def render_assets_header() -> str:
    return """\
#ifndef OPENVN_ASSETS_GENERATED_H
#define OPENVN_ASSETS_GENERATED_H

#include "openvn_assets_runtime.h"

extern const OpenVNAssetTable OPENVN_GENERATED_ASSETS;

#endif
"""


def _rows(entries: list[tuple[str, str]]) -> str:
    return ",\n".join(
        f"    {{{_c_string(asset_id)}, {_c_string(path)}}}" for asset_id, path in entries
    )


def render_assets_source(
    manifest: AssetManifest,
    packaged_paths: dict[Path, Path],
) -> str:
    backgrounds: list[tuple[str, str]] = []
    characters: list[tuple[str, str]] = []
    music: list[tuple[str, str]] = []
    sounds: list[tuple[str, str]] = []

    for asset_id, relative in manifest.backgrounds.items():
        source = manifest.root / relative
        backgrounds.append((asset_id, str(packaged_paths[source])))

    for character, poses in manifest.characters.items():
        for pose, relative in poses.items():
            source = manifest.root / relative
            characters.append((f"{character}.{pose}", str(packaged_paths[source])))

    for asset_id, relative in manifest.music.items():
        source = manifest.root / relative
        music.append((asset_id, str(packaged_paths[source])))

    for asset_id, relative in manifest.sfx.items():
        source = manifest.root / relative
        sounds.append((asset_id, str(packaged_paths[source])))

    return f"""\
#include "assets.generated.h"

static const OpenVNAssetEntry backgrounds[] = {{
{_rows(backgrounds)}
}};

static const OpenVNAssetEntry characters[] = {{
{_rows(characters)}
}};

static const OpenVNAssetEntry music[] = {{
{_rows(music)}
}};

static const OpenVNAssetEntry sounds[] = {{
{_rows(sounds)}
}};

const OpenVNAssetTable OPENVN_GENERATED_ASSETS = {{
    backgrounds,
    {len(backgrounds)}U,
    characters,
    {len(characters)}U,
    music,
    {len(music)}U,
    sounds,
    {len(sounds)}U
}};
"""


def write_generated_assets(
    manifest: AssetManifest,
    packaged_paths: dict[Path, Path],
    output_dir: str | Path,
) -> tuple[Path, Path]:
    output = Path(output_dir)
    output.mkdir(parents=True, exist_ok=True)

    header = output / "assets.generated.h"
    implementation = output / "assets.generated.c"

    header.write_text(render_assets_header(), encoding="utf-8")
    implementation.write_text(
        render_assets_source(manifest, packaged_paths),
        encoding="utf-8",
    )
    return header, implementation
