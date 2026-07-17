from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def test_complete_protracker_effect_contract() -> None:
    header = (ROOT / "runtimes/amiga-native/include/openvn_mod_player.h").read_text()
    source = (ROOT / "runtimes/amiga-native/src/audio_mod_player.c").read_text()
    for field in (
        "target_period",
        "vibrato_position",
        "tremolo_position",
        "sample_offset",
        "pattern_loop_count",
        "note_delay_tick",
    ):
        assert field in header
    for effect in (
        "case 0x03U",
        "case 0x04U",
        "case 0x05U",
        "case 0x06U",
        "case 0x07U",
        "case 0x09U",
    ):
        assert effect in source
    for extended in ("case 0x6U", "sub==0x9U", "case 0xCU", "case 0xDU", "case 0xEU"):
        assert extended in source
