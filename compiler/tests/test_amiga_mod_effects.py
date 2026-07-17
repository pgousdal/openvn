from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PLAYER_HEADER = ROOT / "runtimes" / "amiga-native" / "include" / "openvn_mod_player.h"
PLAYER_SOURCE = ROOT / "runtimes" / "amiga-native" / "src" / "audio_mod_player.c"
CMAKE = ROOT / "runtimes" / "amiga-native" / "CMakeLists.txt"


def test_protracker_effect_engine_contract() -> None:
    header = PLAYER_HEADER.read_text(encoding="utf-8")
    source = PLAYER_SOURCE.read_text(encoding="utf-8")
    cmake = CMAKE.read_text(encoding="utf-8")

    assert "OpenVNMODChannelState" in header
    assert "openvn_mod_player_channel_state" in header
    effects = (
        "case 0x00U",
        "case 0x01U",
        "case 0x02U",
        "case 0x0AU",
        "case 0x0BU",
        "case 0x0CU",
        "case 0x0DU",
        "case 0x0FU",
    )
    for effect in effects:
        assert effect in source
    assert "test-openvn-mod-effects" in cmake
