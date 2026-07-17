import json
import struct
from pathlib import Path

from PIL import Image, ImageDraw

from openvn.backends.amiga import export_amiga_package, load_amiga_profile
from openvn.backends.amiga.conversion import (
    build_shared_palette,
    convert_character_png_to_ilbm,
)
from openvn.compiler import compile_project

REPOSITORY = Path(__file__).parents[2]
DEMO = REPOSITORY / "examples" / "demo"


def _chunk(data: bytes, chunk_id: bytes) -> bytes:
    offset = 12
    while offset < len(data):
        current = data[offset : offset + 4]
        size = struct.unpack(">I", data[offset + 4 : offset + 8])[0]
        payload = data[offset + 8 : offset + 8 + size]
        if current == chunk_id:
            return payload
        offset += 8 + size + (size % 2)
    raise AssertionError(f"missing chunk: {chunk_id!r}")


def test_character_is_cropped_scaled_and_uses_transparent_index_zero(tmp_path: Path) -> None:
    source = tmp_path / "alice.png"
    image = Image.new("RGBA", (100, 140), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.ellipse((20, 10, 79, 129), fill=(220, 80, 60, 255))
    image.save(source)

    palette = build_shared_palette([source], colors=32)
    destination, metadata = convert_character_png_to_ilbm(
        source,
        tmp_path / "alice.iff",
        colors=32,
        palette=palette,
        max_width=40,
        max_height=80,
    )

    data = destination.read_bytes()
    bmhd = _chunk(data, b"BMHD")
    body = _chunk(data, b"BODY")
    width, height = struct.unpack(">HH", bmhd[:4])

    assert width <= 40
    assert height <= 80
    assert bmhd[8] == 5
    assert bmhd[9] == 2
    assert len(_chunk(data, b"CMAP")) == 32 * 3
    assert 0 in body
    assert any(index != 0 for index in body)
    assert metadata["mask"] is True
    assert metadata["transparent_index"] == 0
    assert metadata["source_crop_x"] == 20
    assert metadata["source_crop_y"] == 10


def test_demo_package_uses_shared_palette_and_cropped_character(tmp_path: Path) -> None:
    story = compile_project(DEMO, strict=True)
    profile = load_amiga_profile(REPOSITORY / "profiles" / "amiga-ocs.yaml")

    output = export_amiga_package(
        project_root=DEMO,
        story_path=story,
        output_dir=tmp_path / "amiga",
        profile=profile,
        clean=True,
    )

    manifest = json.loads((output / "manifest.json").read_text(encoding="utf-8"))
    assets = {item["source"]: item for item in manifest["assets"]}
    background = assets["backgrounds/room.png"]
    character = assets["characters/alice_neutral.png"]

    assert manifest["palette"] == {"mode": "shared", "transparent_index": 0}
    assert background["conversion"]["shared_palette"] is True
    assert character["conversion"]["kind"] == "character"
    assert character["conversion"]["mask"] is True
    assert character["conversion"]["width"] <= 192
    assert character["conversion"]["height"] <= 220

    background_cmap = _chunk(
        (output / "assets" / background["packaged"]).read_bytes(),
        b"CMAP",
    )
    character_cmap = _chunk(
        (output / "assets" / character["packaged"]).read_bytes(),
        b"CMAP",
    )
    assert background_cmap == character_cmap
