from pathlib import Path

from openvn.assets.manifest import AssetManifest
from openvn.backends.renpy.assets import copy_assets, render_audio, render_images


def manifest(tmp_path: Path) -> AssetManifest:
    root = tmp_path / "assets"
    (root / "backgrounds").mkdir(parents=True)
    (root / "backgrounds/room.png").write_bytes(b"png")
    return AssetManifest(
        root=root,
        backgrounds={"room": "backgrounds/room.png"},
        characters={"alice": {"happy": "characters/alice_happy.png"}},
        music={"intro": "music/intro.ogg"},
        sfx={"click": "sfx/click.ogg"},
    )


def test_render_images(tmp_path: Path) -> None:
    text = render_images(manifest(tmp_path))
    assert 'image bg room = "backgrounds/room.png"' in text
    assert 'image alice happy = "characters/alice_happy.png"' in text


def test_render_audio(tmp_path: Path) -> None:
    text = render_audio(manifest(tmp_path))
    assert 'define audio.intro = "music/intro.ogg"' in text
    assert 'define audio.click = "sfx/click.ogg"' in text


def test_copy_assets(tmp_path: Path) -> None:
    data = manifest(tmp_path)
    game = tmp_path / "game"
    copy_assets(
        AssetManifest(
            root=data.root,
            backgrounds=data.backgrounds,
            characters={},
            music={},
            sfx={},
        ),
        game,
    )
    assert (game / "backgrounds/room.png").is_file()
