from pathlib import Path

import pytest

from openvn.assets import load_asset_manifest
from openvn.errors import OpenVNError


def test_load_manifest(tmp_path: Path) -> None:
    assets = tmp_path / "assets"
    (assets / "backgrounds").mkdir(parents=True)
    (assets / "backgrounds/room.png").write_bytes(b"png")
    (assets / "manifest.yaml").write_text(
        """
backgrounds:
  room:
    file: backgrounds/room.png
""",
        encoding="utf-8",
    )

    manifest = load_asset_manifest(tmp_path)
    assert manifest.backgrounds == {"room": "backgrounds/room.png"}


def test_missing_asset_fails(tmp_path: Path) -> None:
    assets = tmp_path / "assets"
    assets.mkdir()
    (assets / "manifest.yaml").write_text(
        """
music:
  intro:
    file: music/intro.ogg
""",
        encoding="utf-8",
    )

    with pytest.raises(OpenVNError, match="missing asset files"):
        load_asset_manifest(tmp_path)
