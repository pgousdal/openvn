# Amiga Story Execution Core

M5 PR4 adds a portable C loader and executor for OpenVN Story Format 0.4.

Supported nodes:

- text
- choice
- jump
- end
- scene
- show
- hide
- music
- sound

Protocol additions:

- `STEP`
- `CHOOSE <index>`
- `STATUS`

The portable host runner prints semantic events. The AmigaOS build will route
the same events to ARexx and native, system-friendly graphics and audio
services.

The JSON reader is intentionally small and purpose-built for compiler-generated
OpenVN documents. It is not intended to be a general JSON library.
