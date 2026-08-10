# OpenVN Roadmap

## Completed foundations

- M2 — Compiler Foundation
- M3 — Story Graph and Diagnostics
- M4 — Ren'Py Backend
- M5 — Native Amiga Runtime Foundation

Historical release tags and design documents remain the source of detail for
completed work; this roadmap does not redefine them.

## Current milestone

### M6 — Native Amiga Runtime Completion

Status: in progress.

Complete the real native vertical slice: background and character graphics,
dialogue, interactive choices, deterministic variables, conditions,
jumps/labels and MOD music, with no tolerated release failures. The authoritative
completion gate and validation procedure are in [`docs/M6.md`](docs/M6.md).

## Next milestone

### M7 — Save/Load

Status: planned after M6 is completely green.

Define deterministic serialization and restoration of story position,
variables and presentation/audio state. Save/Load is explicitly outside M6 and
must not begin until the native vertical slice and release suite are complete.

## Deferred

- Open Narrative Format: after OpenVN 1.0, potentially as a separate project.
- Broader presentation, animation and audio-mixing features: schedule only after
  the core runtime milestones are stable.
