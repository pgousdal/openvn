from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_double_buffering_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_palette.h",
        RUNTIME / "include/openvn_display_amiga.h",
        RUNTIME / "src/palette.c",
        RUNTIME / "src/display_amiga.c",
        RUNTIME / "tests/test_palette.c",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_display_uses_native_screen_buffer_api() -> None:
    source = (RUNTIME / "src/display_amiga.c").read_text(encoding="utf-8")

    assert "AllocScreenBuffer" in source
    assert "ChangeScreenBuffer" in source
    assert "FreeScreenBuffer" in source
    assert "CreateMsgPort" in source
    assert "LoadRGB32" in source
    assert "WaitPort" in source
    assert "ACE" not in source
