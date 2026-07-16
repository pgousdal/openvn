# OpenVN

OpenVN is an open, runtime-independent visual novel toolchain.

The current milestone is **M2: Compiler Foundation**.

## Design principles

- Ink is the authoring format.
- OpenVN JSON is the intermediate format.
- Runtime backends are separate from the compiler.
- The Amiga runtime will be ARexx-first.
- Native C will only be used where necessary.
- ACE is not a required dependency.

## Development

Requires Python 3.11+ and `uv`.

```bash
uv sync --project compiler
uv run --project compiler pytest
uv run --project compiler ruff check src tests
uv run --project compiler openvn validate examples/minimal
uv run --project compiler openvn compile examples/minimal
```

The example paths above are relative to the repository root.

## Commands

```bash
openvn validate <project-directory>
openvn compile <project-directory>
openvn dump <project-directory>
```

`compile` validates `project.yaml` and `main.ink`, then writes
`story.openvn.json`.
