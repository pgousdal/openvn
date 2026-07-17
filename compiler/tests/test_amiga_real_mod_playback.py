from pathlib import Path

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def test_real_mod_playback_event_loop_contract() -> None:
    audio_header = (RUNTIME / "include/openvn_audio.h").read_text(encoding="utf-8")
    player_header = (RUNTIME / "include/openvn_player.h").read_text(encoding="utf-8")
    amiga_main = (RUNTIME / "src/main_amiga.c").read_text(encoding="utf-8")
    player_state = (RUNTIME / "src/player_state.c").read_text(encoding="utf-8")

    assert "signal_mask" in audio_header
    assert "openvn_player_update" in player_header
    assert "openvn_player_signal_mask" in player_header
    assert "Wait(wait_mask)" in amiga_main
    assert "openvn_state_update" in amiga_main
    assert "openvn_state_signal_mask" in player_state
