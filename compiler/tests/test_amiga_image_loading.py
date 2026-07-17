from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_image_loading_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_image.h",
        RUNTIME / "include/openvn_assets_runtime.h",
        RUNTIME / "src/image_ilbm.c",
        RUNTIME / "src/assets_runtime.c",
        RUNTIME / "tests/test_image.c",
        RUNTIME / "tests/test_assets_runtime.c",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_adapter_uses_datatypes_rendering() -> None:
    source = (RUNTIME / "src/graphics_amiga.c").read_text(encoding="utf-8")

    assert "NewDTObject" in source
    assert "DrawDTObject" in source
    assert "GID_PICTURE" in source
    assert "openvn_ilbm_load_file" in source
    assert "ACE" not in source
