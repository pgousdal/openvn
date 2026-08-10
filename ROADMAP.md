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

The native graphics/story vertical slice and automated release gates are green,
and real MOD/Paula output is demonstrated. Final explicit human audio-quality
and post-choice acceptance is the sole remaining completion blocker; see
[`docs/M6.md`](docs/M6.md).

## Next milestone

### M7 — Save/Load

Status: planned after M6 is completely green.

Define deterministic persistence and restoration of story position, typed
variables and the presentation/audio state needed to resume consistently. M7
concerns persistence, not new presentation features.

## Deferred

- Open Narrative Format: after OpenVN 1.0, potentially as a separate project.
- Broader presentation, animation and audio-mixing features: schedule only after
  the core runtime milestones are stable.
