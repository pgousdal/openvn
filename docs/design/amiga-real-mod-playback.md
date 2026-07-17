# Amiga real MOD playback

M5 PR6.5 connects packaged MOD assets to the native player event loop.

The audio service exposes a platform signal mask. On AmigaOS, the player waits for both ARexx commands and timer.device completion signals. Every completed tracker timer request advances the MOD player by one tick and schedules the next request using the current BPM.

The host runtime uses the same player update API with a zero signal mask, keeping the integration testable without Amiga hardware. Story `music` nodes and the `MUSIC` dispatch command resolve asset IDs through the generated asset table before loading the packaged `.mod` file.

This milestone does not add mixing of independent sound effects over MOD music; that remains a later audio-mixer milestone.
