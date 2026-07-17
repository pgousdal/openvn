# Native Amiga Graphics Service

M5 PR5 introduces the graphics service boundary used by the story player.

## Interface

The portable service supports:

- open
- close
- scene
- show
- hide
- present

The story execution core calls this interface when it reaches `scene`, `show`
or `hide` nodes.

## Host adapter

The host adapter logs operations and stores current scene/character state. It
supports deterministic CTest and Python integration testing on Debian.

## AmigaOS adapter

The native adapter skeleton uses:

- `intuition.library` for screen and window lifecycle
- `graphics.library` for rendering
- `datatypes.library` for RTG picture loading
- native ILBM/BitMap paths for OCS and AGA

The adapter does not use ACE.

Actual DataTypes rendering, ILBM decoding and sprite composition follow in
M5 PR5.1. This PR establishes the stable contract and system lifecycle.
