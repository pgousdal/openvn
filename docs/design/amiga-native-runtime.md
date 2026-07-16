# Native Amiga Runtime Direction

The OpenVN Amiga runtime will not depend on ACE.

## ARexx-first

ARexx owns all practical high-level behavior:

- story flow
- choices
- variables and conditions when introduced
- scene orchestration
- save/load orchestration

## Native C

Plain native Amiga C is used only where ARexx is unsuitable:

- graphics
- audio
- input
- file access
- memory management
- performance-critical decoding or rendering

The C layer should expose a small ARexx command surface rather than becoming a
general-purpose game engine.
