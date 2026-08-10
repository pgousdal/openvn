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
    assert "BeginIO" in source
    assert "ADIOF_PERVOL" in source
    assert "MEMF_CHIP | MEMF_PUBLIC" in source
    assert "SendIO((struct IORequest *)request)" not in source
    assert "SendIO((struct IORequest *)loop_request)" not in source
    assert "AbortIO" in source
    assert "CloseDevice" in source
    assert "ACE" not in source

    sound_body = source.split("static int amiga_sound", maxsplit=1)[1].split(
        "static int amiga_stop_music", maxsplit=1
    )[0]
    assert "openvn_mod_player_stop" not in sound_body
    assert "stop_timer" not in sound_body
    assert "stop_all_channels" not in sound_body
