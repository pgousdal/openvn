# OpenVN

OpenVN is an open, runtime-independent visual novel toolchain.

## Status

- M2 Compiler Foundation: complete
- M3 Story Graph and Diagnostics: release candidate
- M4 Ren'Py backend: next
- M5 Native Amiga runtime: planned

## Architecture

```text
Ink
 ↓
OpenVN compiler
 ↓
OpenVN Story Format
 ├── Ren'Py backend
 └── Amiga runtime: ARexx-first + native C
```

The Amiga runtime does not depend on ACE.

Open Narrative Format is deferred until OpenVN has multiple working backends.

## Development

```bash
uv sync --project compiler --extra dev
uv run --project compiler ruff check compiler/src compiler/tests
uv run --project compiler pytest
uv run --project compiler openvn validate examples/minimal
uv run --project compiler openvn compile examples/minimal --strict
uv run --project compiler openvn doctor .
```
