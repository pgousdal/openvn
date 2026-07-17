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
    source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")

    assert "OpenScreenTags" in source
    assert "OpenWindowTags" in source
    assert "DisposeDTObject" in source
    assert "CloseWindow" in source
    assert "CloseScreen" in source
    assert "ACE" not in source
