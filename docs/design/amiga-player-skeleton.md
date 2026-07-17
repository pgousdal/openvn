# Native Amiga Player Skeleton

M5 PR3 establishes the command boundary between ARexx and native AmigaOS C.

## ARexx port

The native process publishes:

```text
OPENVNPLAYER
```

The generated `main.rexx` bootstrap addresses this port and starts the story.

## Native layer

The native C layer is system-friendly and deliberately small. It owns only
services that require native access or performance.

Planned adapters:

- Exec messages and memory
- DOS files
- Intuition windows and input
- Graphics and Layers rendering
- DataTypes loading for RTG assets
- IFF ILBM and 8SVX loading for classic profiles
- audio.device and timer.device

ACE is not used.

## Test strategy

The command parser and dispatcher are portable C. A host executable exposes
the same protocol through standard input, allowing CI tests before an Amiga
cross-toolchain or emulator is introduced.
