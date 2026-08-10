# Amiga development and validation

OpenVN's validated native target is classic m68k AmigaOS using a Bebbo-style
`m68k-amigaos-gcc` toolchain. Repository diagnostics record GCC 6.5.0b as the
last compiler known to have produced the player. Newer compatible releases may
work, but must pass the compiler/header/link probe and the complete target gate.

## Required tools

- `uv` and the locked compiler environment
- CMake 3.16 or newer
- `m68k-amigaos-gcc`, with its matching assembler and linker
- AmigaOS headers for Exec, DOS, Intuition, Graphics, DataTypes and audio/timer
- `amiga.lib` or an equivalent `libamiga.a`
- `file`, `od`, `grep` and other standard POSIX command-line tools
- FS-UAE 3.x and a legally obtained Kickstart ROM for emulator execution

The canonical GCC distribution normally includes compatible headers, startup
objects and libraries. An external NDK is also supported when it uses one of
these layouts:

```text
$OPENVN_AMIGA_SDK/Include_H/proto/exec.h
$OPENVN_AMIGA_SDK/lib/amiga.lib
```

```text
$OPENVN_AMIGA_SDK/include/proto/exec.h
$OPENVN_AMIGA_SDK/lib/libamiga.a
```

The build does not download a compiler, SDK or ROM.

## Configuration

The normal configuration variables are:

- `OPENVN_AMIGA_GCC`: compiler executable or absolute path
- `OPENVN_AMIGA_SDK`: optional external SDK/NDK root
- `OPENVN_AMIGA_TOOLCHAIN_FILE`: optional CMake toolchain file; defaults to the
  repository's `runtimes/amiga-native/cmake/m68k-amigaos-gcc.cmake`
- `OPENVN_CMAKE`: CMake executable or absolute path
- `OPENVN_UV`: uv executable or absolute path
- `OPENVN_AMIGA_BUILD_DIR`: CMake target build directory
- `OPENVN_PLAYER_OUT`: copied m68k player output
- `OPENVN_DEMO`: project to build; defaults to `examples/demo`

`OPENVN_TOOLCHAIN_FILE`, `OPENVN_TOOLCHAIN_PATH` and `M68K_CC` remain accepted
for compatibility with the earlier developer-local workflow. New automation
should use the `OPENVN_AMIGA_*` names.

Before compiling OpenVN, the build script verifies CMake and uv, resolves the
GCC-provided assembler and linker, compiles the required NDK headers, and links
a probe with `-lamiga`. A missing dependency therefore fails before generated
project output or the target build is changed.

## Build and verify the canonical demo

With a compiler-integrated SDK:

```sh
OPENVN_AMIGA_GCC=/path/to/bin/m68k-amigaos-gcc \
OPENVN_CMAKE=/path/to/cmake \
./scripts/build-m68k-demo-player.sh
```

With an external NDK:

```sh
OPENVN_AMIGA_GCC=/path/to/bin/m68k-amigaos-gcc \
OPENVN_AMIGA_SDK=/path/to/NDK_3.2R4 \
OPENVN_CMAKE=/path/to/cmake \
./scripts/build-m68k-demo-player.sh
```

The command performs the complete source-to-package path and writes:

- `dist/openvn-player-m68k-amigaos`
- `build/amiga-demo-player/openvn-player`
- `examples/demo/dist/amiga-ocs/`
- `examples/demo/dist/fs-uae/OpenVNDemo.fs-uae`

`scripts/verify-amiga-artifacts.sh` checks the Amiga Hunk magic and `file(1)`
classification, then verifies the generated story, asset tables, ILBM
background/character, MOD and 8SVX files. A filename alone is not accepted as
proof of a target executable.

## Release gates

`scripts/release-check.sh` is the full release gate. It requires both host and
Amiga tools and must not be reported as passing when the target toolchain is
missing. CMake and CTest may be supplied explicitly when they live in an
isolated project environment:

```sh
OPENVN_CMAKE=/path/to/cmake \
OPENVN_CTEST=/path/to/ctest \
OPENVN_AMIGA_GCC=/path/to/m68k-amigaos-gcc \
./scripts/release-check.sh
```

Ordinary CI currently runs the portable compiler/host checks. A release
requires the full script on a configured Amiga build worker in addition to CI.
The same host gate can be invoked explicitly without implying target success:

```sh
./scripts/release-check.sh --host-only
```

That mode prints that Amiga validation was not requested. The target-only
source-to-package gate is `scripts/build-m68k-demo-player.sh`; the default
`scripts/release-check.sh` runs both and remains the release authority.

## Run with FS-UAE

FS-UAE must be installed separately and configured with a legally obtained
Kickstart ROM. OpenVN does not distribute or select a proprietary ROM.

After the target build succeeds:

```sh
fs-uae examples/demo/dist/fs-uae/OpenVNDemo.fs-uae
```

Repeat this checklist for both choices:

1. The player starts without an AmigaDOS error.
2. The room background and Alice character render with the expected palette
   and transparent character mask.
3. Dialogue advances and the choice selector responds to input.
4. Each choice reaches its corresponding ending.
5. MOD music remains audible while the choice waits for input.
6. The player exits cleanly after the ending.

These observations are manual unless a separately documented emulator harness
captures them. Package generation and Hunk inspection do not prove rendering,
input or audio behavior.
