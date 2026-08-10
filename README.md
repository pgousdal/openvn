# OpenVN

OpenVN is an open-source visual novel compiler and runtime project. Stories are
authored in a deliberately small Ink-compatible syntax, compiled to the OpenVN
Story Format, and exported to target backends. The primary hardware target is
the Commodore Amiga; a Ren'Py backend is also maintained.

OpenVN is under active development. The current milestone is
[M7 — Save/Load](ROADMAP.md). The M6 native Amiga vertical slice is complete:
the canonical demo has been cross-built and validated under FS-UAE with OCS
graphics, interaction, deterministic branching, both endings and four-channel
MOD playback. Save/Load is not implemented.

## Architecture

```text
Ink-like source + project.yaml + assets
                  |
             Python compiler
                  |
        OpenVN Story Format 0.3/0.4
             /             \
       Ren'Py export    Amiga package
                       static C tables
                             |
                  native C Amiga runtime
             Intuition/Graphics/DataTypes/Paula
```

The native runtime does not parse JSON on the Amiga. The compiler generates C
story and asset tables, converts classic assets to ILBM/8SVX, and preserves MOD
music. Portable C services provide story execution, variables, conditions,
graphics and audio contracts; host adapters make those contracts testable.

## Prerequisites

- Python 3.11 or newer
- [uv](https://docs.astral.sh/uv/)
- a C compiler, CMake 3.16+ and CTest for host runtime tests
- for native builds, an `m68k-amigaos-gcc` toolchain, Amiga NDK headers and an
  Amiga CMake toolchain file (the build script defaults are documented below)
- FS-UAE, a legally obtained AmigaOS 3.x Kickstart ROM, and a legal OS3 system
  directory to run the packaged demo

Exact Amiga GCC/NDK layouts and configuration variables are documented in
[`docs/amiga-development.md`](docs/amiga-development.md).

Install development dependencies:

```sh
uv sync --project compiler --all-extras
```

## Build and test

```sh
uv run --project compiler ruff format --check .
uv run --project compiler ruff check .
uv run --project compiler pytest -q
cmake -S runtimes/amiga-native -B build/host
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
./scripts/release-check.sh
```

`release-check.sh` is the full host-and-Amiga release gate. It accepts explicit
`OPENVN_CMAKE`, `OPENVN_CTEST`, `OPENVN_AMIGA_GCC`, `OPENVN_AMIGA_SDK` and
`OPENVN_AMIGA_TOOLCHAIN_FILE` settings; it does not depend on a particular user
home directory and does not download target tools.

Use `./scripts/release-check.sh --host-only` for the explicitly labeled host
gate. It is not a substitute for the default full release check.

## Quick start

The canonical integration story is [`examples/demo`](examples/demo). It uses a
background, character, dialogue, choices, variables, conditions, labels/jumps,
MOD music, sound and two endings.

```sh
uv run --project compiler openvn validate examples/demo
uv run --project compiler openvn compile examples/demo --strict
uv run --project compiler openvn build examples/demo --clean
./scripts/build-m68k-demo-player.sh
```

The final command cross-builds the player and creates an FS-UAE package under
`examples/demo/dist/fs-uae`. Compiler output under an example is generated and
should not be committed.

Launch it with explicit, external legal OS media:

```sh
OPENVN_FS_UAE_KICKSTART=/path/to/amiga-os-3.x.rom \
OPENVN_FS_UAE_SYSTEM_DIR=/path/to/amiga-os-3.x-system \
./scripts/run-amiga-demo.sh
```

## Repository layout

- `compiler/`: Python compiler, CLI, exporters and pytest suite
- `runtimes/amiga-native/`: portable runtime core, Amiga adapters and C tests
- `backends/`: backend-specific documentation
- `profiles/`: OCS, AGA and RTG target profiles
- `schemas/`: OpenVN Story Format JSON schemas
- `examples/`: small projects and integration assets
- `docs/`: specifications, design notes, tutorials and milestone plans
- `scripts/`: build, release and repository-maintenance commands

## Status and roadmap

M6 is complete, including native validation of the canonical Amiga demo with
graphics, interaction, branching and MOD playback. M7 — Save/Load is the active
milestone. See [`ROADMAP.md`](ROADMAP.md) for milestone status and
[`docs/M6.md`](docs/M6.md) for the completed M6 validation record.

## Current limitations

- The authoring parser supports an intentional subset of Ink, not full Ink.
- Classic Amiga output is currently centered on the 320x256, 32-colour OCS
  profile; AGA and RTG profiles are not equally validated end to end.
- The native player uses generated story/asset tables and fixed runtime display
  defaults; package metadata is not dynamically interpreted.
- Character placement, transitions, multiple simultaneous characters and an
  audio mixer are outside the current vertical slice.
- Save/Load is not implemented.
- Real-hardware/emulator behavior cannot be proven by host contract tests alone.

OpenVN is licensed under the repository license. Example asset licensing is
documented in [`LICENSE-assets.md`](LICENSE-assets.md) and per-example files.
