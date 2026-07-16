
# Contributing

Before opening a PR:

- `uv sync --project compiler --extra dev`
- `uv run --project compiler ruff check compiler/src compiler/tests`
- `uv run --project compiler pytest`
- `uv run --project compiler openvn validate examples/minimal`
- `uv run --project compiler openvn compile examples/minimal`

Do not merge unless all checks pass.
