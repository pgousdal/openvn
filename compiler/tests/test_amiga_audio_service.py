from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_audio_service_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_audio.h",
        RUNTIME / "include/openvn_audio_host.h",
        RUNTIME / "include/openvn_audio_amiga.h",
        RUNTIME / "include/openvn_8svx.h",
        RUNTIME / "src/audio.c",
        RUNTIME / "src/audio_host.c",
        RUNTIME / "src/audio_amiga.c",
        RUNTIME / "src/audio_8svx.c",
        RUNTIME / "tests/test_audio.c",
        RUNTIME / "tests/test_8svx.c",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_audio_adapter_uses_audio_device() -> None:
    source = (RUNTIME / "src/audio_amiga.c").read_text(encoding="utf-8")

    assert "AUDIONAME" in source
    assert "OpenDevice" in source
    assert "CreateIORequest" in source
    assert "CMD_WRITE" in source
    assert "SendIO" in source
    assert "AbortIO" in source
    assert "CloseDevice" in source
    assert "ACE" not in source
