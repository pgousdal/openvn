
# OpenVN

OpenVN is an open, runtime-independent visual novel toolchain.

## Status

**Current milestone:** M2 – Compiler Foundation ✅

### Principles

- Ink authoring
- OpenVN intermediate format
- ARexx-first Amiga runtime
- Native C only where required
- No mandatory ACE dependency

## Development

```bash
uv sync --project compiler --extra dev
uv run --project compiler ruff check compiler/src compiler/tests
uv run --project compiler pytest
uv run --project compiler openvn validate examples/minimal
uv run --project compiler openvn compile examples/minimal
```

## Next milestone

M3 – Semantic Story Model
