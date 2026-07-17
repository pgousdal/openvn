# Amiga Planar ILBM Rendering

M5 PR5.2 completes the classic graphics path from loaded ILBM pixels to native
Amiga bitplanes.

## Portable conversion

The runtime converts indexed chunky pixels into one plane per color bit.

The converter:

- pads rows to Amiga word boundaries
- supports one to eight bitplanes
- optionally creates a one-bit transparency mask
- provides host-side pixel reconstruction tests

## Native AmigaOS bitmap allocation

The Amiga adapter uses:

- `AllocBitMap`
- `FreeBitMap`
- `CopyMem`
- `BltBitMapRastPort`
- `BltMaskBitMapRastPort`
- `WaitBlit`

Backgrounds are copied without a mask. Character images use palette index zero
as transparent and are composited with a mask.

## Current presentation policy

- background at `(0, 0)`
- character horizontally centered
- character aligned to the bottom of the window
- background redrawn before character composition

This is intentionally simple and deterministic. Position metadata, multiple
characters, transitions and double buffering can be layered onto the graphics
service without changing story execution.

The runtime remains ARexx-first, native, system-friendly and independent of ACE.
