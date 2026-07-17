from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_graphics_service_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_graphics.h",
        RUNTIME / "include/openvn_graphics_host.h",
        RUNTIME / "include/openvn_graphics_amiga.h",
        RUNTIME / "include/openvn_player.h",
        RUNTIME / "src/graphics.c",
        RUNTIME / "src/graphics_host.c",
        RUNTIME / "src/graphics_amiga.c",
        RUNTIME / "src/player.c",
        RUNTIME / "tests/test_graphics.c",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_graphics_adapter_uses_os_services() -> None:
    graphics_source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")
    display_source = (RUNTIME / "src/display_amiga.c").read_text(encoding="utf-8")

    assert "DisposeDTObject" in graphics_source
    assert "OpenScreenTags" in display_source
    assert "OpenWindowTags" in display_source
    assert "CloseWindow" in display_source
    assert "CloseScreen" in display_source
    assert "ACE" not in graphics_source
    assert "ACE" not in display_source
