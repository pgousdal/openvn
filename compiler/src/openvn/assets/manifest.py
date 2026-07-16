from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml

from ..errors import OpenVNError


@dataclass(frozen=True)
class AssetManifest:
    root: Path
    backgrounds: dict[str, str]
    characters: dict[str, dict[str, str]]
    music: dict[str, str]
    sfx: dict[str, str]

    def all_files(self) -> list[Path]:
        files: list[Path] = []
        files.extend(self.root / path for path in self.backgrounds.values())
        for poses in self.characters.values():
            files.extend(self.root / path for path in poses.values())
        files.extend(self.root / path for path in self.music.values())
        files.extend(self.root / path for path in self.sfx.values())
        return files


def _mapping(data: dict[str, Any], key: str) -> dict[str, Any]:
    value = data.get(key, {})
    if not isinstance(value, dict):
        raise OpenVNError(f"asset manifest field '{key}' must be a mapping")
    return value


def load_asset_manifest(project_root: str | Path) -> AssetManifest:
    root = Path(project_root).resolve()
    manifest_path = root / "assets" / "manifest.yaml"
    if not manifest_path.is_file():
        return AssetManifest(
            root=root / "assets",
            backgrounds={},
            characters={},
            music={},
            sfx={},
        )

    raw = yaml.safe_load(manifest_path.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise OpenVNError("assets/manifest.yaml must contain a mapping")

    backgrounds = {name: str(value["file"]) for name, value in _mapping(raw, "backgrounds").items()}
    characters = {
        character: {pose: str(path) for pose, path in poses.items()}
        for character, poses in _mapping(raw, "characters").items()
    }
    music = {name: str(value["file"]) for name, value in _mapping(raw, "music").items()}
    sfx = {name: str(value["file"]) for name, value in _mapping(raw, "sfx").items()}

    manifest = AssetManifest(
        root=root / "assets",
        backgrounds=backgrounds,
        characters=characters,
        music=music,
        sfx=sfx,
    )

    missing = [path for path in manifest.all_files() if not path.is_file()]
    if missing:
        raise OpenVNError("missing asset files: " + ", ".join(str(path) for path in missing))

    return manifest
