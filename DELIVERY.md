# M2 Delivery

This archive is intended to be merged into the repository root.

It adds:

- `compiler/`
- `.github/workflows/ci.yml`
- compiler architecture documentation
- ARexx-first runtime principle documentation
- updated minimal example
- story schema
- README and changelog

Run:

```bash
uv sync --project compiler --extra dev
uv run --project compiler ruff check compiler/src compiler/tests
uv run --project compiler ruff format --check compiler/src compiler/tests
uv run --project compiler pytest
uv run --project compiler openvn validate examples/minimal
uv run --project compiler openvn compile examples/minimal
```
