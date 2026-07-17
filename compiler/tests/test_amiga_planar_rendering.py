from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_planar_rendering_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_planar.h",
        RUNTIME / "include/openvn_bitmap_amiga.h",
        RUNTIME / "src/planar.c",
        RUNTIME / "src/bitmap_amiga.c",
        RUNTIME / "tests/test_planar.c",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_planar_adapter_uses_native_graphics_api() -> None:
    bitmap_source = (RUNTIME / "src/bitmap_amiga.c").read_text(encoding="utf-8")
    graphics_source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")

    assert "AllocBitMap" in bitmap_source
    assert "FreeBitMap" in bitmap_source
    assert "BltBitMapRastPort" in bitmap_source
    assert "BltMaskBitMapRastPort" in bitmap_source
    assert "WaitBlit" in bitmap_source
    assert "openvn_planar_from_chunky" in graphics_source
    assert "openvn_amiga_bitmap_blit" in graphics_source
    assert "ACE" not in bitmap_source
    assert "ACE" not in graphics_source
