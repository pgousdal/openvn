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
    assert "direct fullscreen copy" not in bitmap_source
    assert "openvn_planar_from_chunky" in graphics_source
    assert "openvn_amiga_bitmap_blit" in graphics_source
    assert "ACE" not in bitmap_source
    assert "ACE" not in graphics_source


def test_native_text_uses_visible_demo_palette_pens() -> None:
    graphics_source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")
    font_source = (RUNTIME / "src/bitmap_font_amiga.c").read_text(encoding="utf-8")

    assert "#define OPENVN_TEXT_PEN 2U" in graphics_source
    assert "#define OPENVN_SELECTED_TEXT_PEN 1U" in graphics_source
    assert "WritePixel(amiga_context->rastport, x, y);" in font_source


def test_double_buffer_waits_until_the_draw_buffer_is_safe() -> None:
    source = (RUNTIME / "src/display_amiga.c").read_text(encoding="utf-8")
    draw_function = source.split("struct RastPort *openvn_amiga_display_draw_rastport", 1)[1].split(
        "int openvn_amiga_display_present", 1
    )[0]

    assert "if (display->swap_pending)" in draw_function
    assert "WaitPort(display->safe_port);" in draw_function
    assert draw_function.index("WaitPort(display->safe_port);") < draw_function.index(
        "display->draw_rastport.BitMap"
    )
    assert "return &display->draw_rastport;" in draw_function


def test_choice_mouse_input_hit_tests_the_rendered_rows() -> None:
    source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")

    assert "mouse_y = message->MouseY;" in source
    assert "clicked_index = (size_t)(mouse_y - layout.text_y)" in source
    assert "context->choice_selected = clicked_index;" in source
