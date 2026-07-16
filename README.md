# OpenVN

OpenVN is an open, runtime-independent visual novel toolchain.

## Current pipeline

```text
Ink
 ↓
OpenVN compiler
 ↓
OpenVN Story Format 0.3
 ↓
Ren'Py backend
```

## Export to Ren'Py

```bash
uv run --project compiler openvn export examples/minimal   --backend renpy   --output build/renpy-minimal
```

Open `build/renpy-minimal` in the Ren'Py launcher.

## Amiga direction

The Amiga runtime is planned as ARexx-first with native C only where needed.
It does not depend on ACE.

Open Narrative Format remains deferred.
