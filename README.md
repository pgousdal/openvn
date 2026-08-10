# OpenVN

OpenVN is an open-source visual novel compiler and runtime project. Stories are
authored in a deliberately small Ink-compatible syntax, compiled to the OpenVN
Story Format, and exported to target backends. The primary hardware target is
the Commodore Amiga; a Ren'Py backend is also maintained.

OpenVN is under active development. The current milestone is
[M6 — Native Amiga Runtime Completion](docs/M6.md). The compiler, static Amiga
story/asset generation, host-testable runtime core, OCS graphics path,
interactive choices, deterministic variables and conditions, and four-channel
MOD playback exist. Native execution still requires validation with an Amiga
cross-toolchain and on an emulator or machine; Save/Load is not implemented.

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
- FS-UAE and a legally obtained Kickstart ROM to run the packaged demo

Install development dependencies:

```sh
uv sync --project compiler --all-extras
```

## Build and test

```sh
uv run --project compiler ruff format --check .
uv run --project compiler ruff check .
uv run --project compiler pytest -q
cmake -S runtimes/amiga-native -B build/host -DOPENVN_BUILD_TESTS=ON
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
./scripts/release-check.sh
```

`release-check.sh` also performs repository hygiene checks and the m68k Amiga
build. Override `OPENVN_TOOLCHAIN_FILE` and `OPENVN_TOOLCHAIN_PATH` when the
toolchain is not at `~/Projects/AmigaCMakeCrossToolchains/` and `/opt/amiga`.

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

M6 is focused only on completing and validating a small real native Amiga
visual novel with graphics, interaction, branching and MOD playback. See
[`ROADMAP.md`](ROADMAP.md) for milestone status and [`docs/M6.md`](docs/M6.md)
for the exact completion gate. M7 is reserved for Save/Load after M6 is green.

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
