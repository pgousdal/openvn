from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_mod_replayer_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_mod.h",
        RUNTIME / "include/openvn_mod_player.h",
        RUNTIME / "src/audio_mod.c",
        RUNTIME / "src/audio_mod_player.c",
        RUNTIME / "tests/test_mod.c",
        RUNTIME / "tests/test_mod_player.c",
        RUNTIME / "include/openvn_paula.h",
        RUNTIME / "src/audio_paula.c",
        RUNTIME / "tests/test_paula.c",
        RUNTIME / "tests/fixtures/storm.mod",
    ]
    assert all(path.is_file() for path in expected)


def test_amiga_mod_replayer_uses_timer_device_contract() -> None:
    source = (RUNTIME / "src/audio_amiga.c").read_text(encoding="utf-8")

    assert "TIMERNAME" in source
    assert "UNIT_MICROHZ" in source
    assert "TR_ADDREQUEST" in source
    assert "openvn_mod_player_tick" in source
    assert "openvn_mod_player_channel" in source
    assert "ACE" not in source


def test_amiga_mod_replayer_has_four_channel_paula_contract() -> None:
    source = (RUNTIME / "src/audio_amiga.c").read_text(encoding="utf-8")
    header = (RUNTIME / "include/openvn_audio_amiga.h").read_text(encoding="utf-8")

    assert "channel_requests[OPENVN_MOD_CHANNELS]" in header
    assert "loop_requests[OPENVN_MOD_CHANNELS]" in header
    assert "openvn_paula_trigger_note" in source
    assert "ioa_Period" in source
    assert "ioa_Volume" in source
    assert "ioa_Cycles = 0U" in source
    assert "SendIO" in source
