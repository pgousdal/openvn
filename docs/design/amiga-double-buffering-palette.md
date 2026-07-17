# Amiga Double Buffering and Palette Presentation

M5 PR5.3 adds the native screen-presentation layer for classic and RTG
rendering.

## Screen buffers

The AmigaOS display adapter uses:

- `AllocScreenBuffer`
- `ChangeScreenBuffer`
- `FreeScreenBuffer`
- safe and display message ports
- explicit safe-buffer waiting before reuse

Rendering targets the hidden buffer. Presentation swaps it with the visible
screen buffer after the blitter has finished.

## Palette loading

ILBM `CMAP` values are converted to the `LoadRGB32` table format.

The background image owns the active scene palette. Character graphics are
expected to be exported against the same target palette for classic OCS and
AGA profiles.

## Presentation flow

```text
acquire hidden RastPort
    ↓
draw background
    ↓
composite character with mask
    ↓
wait for blitter
    ↓
ChangeScreenBuffer
    ↓
wait until old buffer is safe
```

The host runtime remains deterministic and unchanged. Amiga-specific screen
buffer lifecycle is isolated behind the display adapter.

The runtime remains ARexx-first, native, system-friendly and independent of
ACE.
