# Native Amiga Audio Service

M5 PR6 introduces the audio service boundary used by the story player.

## Interface

The runtime supports:

- play music
- play sound effect
- stop music
- stop all audio
- open and close audio services

Story `music` and `sound` nodes are routed through this interface.

## 8SVX effects

The portable loader reads uncompressed IFF 8SVX:

- `FORM 8SVX`
- `VHDR`
- `BODY`

The native adapter converts sample rate to Paula period and submits the
sample through `audio.device`.

## Native AmigaOS ownership

The adapter uses:

- `CreateMsgPort`
- `CreateIORequest`
- `OpenDevice(AUDIONAME, ...)`
- `CMD_WRITE`
- `SendIO`
- `AbortIO`
- `WaitIO`
- `CloseDevice`

## MOD music

Music assets are resolved through generated static asset tables. Classic
profiles require MOD input. Actual MOD sequencing and Paula channel mixing
follow in M5 PR6.1.

RTG profiles may later use DataTypes or another system-friendly streaming
backend for OGG/WAV music.

The runtime remains ARexx-first, native, system-friendly and independent of
ACE.
