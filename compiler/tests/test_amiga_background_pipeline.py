import struct
from pathlib import Path

from PIL import Image

from openvn.backends.amiga import load_amiga_profile
from openvn.backends.amiga.conversion import convert_asset, convert_png_to_ilbm

REPOSITORY = Path(__file__).parents[2]


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


def test_ocs_background_is_compiled_to_exact_screen_geometry(tmp_path: Path) -> None:
    source = tmp_path / "room.png"
    Image.new("RGB", (640, 360), (20, 40, 60)).save(source)
    profile = load_amiga_profile(REPOSITORY / "profiles/amiga-ocs.yaml")

    destination, metadata = convert_asset(
        source,
        tmp_path / "output",
        relative=Path("backgrounds/room.png"),
        profile=profile,
        is_music=False,
    )

    data = destination.read_bytes()
    bmhd = _chunk(data, b"BMHD")
    width, height = struct.unpack(">HH", bmhd[:4])

    assert (width, height) == (320, 256)
    assert bmhd[8] == 5
    assert len(_chunk(data, b"CMAP")) == 32 * 3
    assert len(_chunk(data, b"BODY")) == 320 * 256
    assert metadata == {
        "source": "png",
        "target": "ilbm",
        "width": 320,
        "height": 256,
        "colors": 32,
        "fit": "cover",
    }


def test_cover_transform_crops_instead_of_distorting(tmp_path: Path) -> None:
    source = tmp_path / "wide.png"
    image = Image.new("RGB", (4, 2), (255, 0, 0))
    image.paste((0, 255, 0), (0, 0, 1, 2))
    image.paste((0, 0, 255), (3, 0, 4, 2))
    image.save(source)

    destination = convert_png_to_ilbm(
        source,
        tmp_path / "wide.iff",
        colors=4,
        width=2,
        height=2,
        fit="cover",
    )

    data = destination.read_bytes()
    bmhd = _chunk(data, b"BMHD")
    assert struct.unpack(">HH", bmhd[:4]) == (2, 2)
    assert len(_chunk(data, b"BODY")) == 4
