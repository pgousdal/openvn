# Amiga development and validation

OpenVN's validated build target is classic 68000 AmigaOS using a Bebbo-style
`m68k-amigaos-gcc` toolchain. The tested configuration is GCC 6.5.0b build
20260731 from `stefanreinauer/amiga-gcc`, OCI digest
`sha256:fa6e66d487fbfdbc78073943a817cd04398165b70a84fcf7b3be4c66269b3e8a`.
It produced and linked the canonical player on 2026-08-10. Newer compatible
releases may work, but must pass the compiler/header/link probe and complete
target gate; 6.5.0b is the tested configuration, not a proven minimum.
The image identifies upstream branch `amiga6`, GCC version `6.5.0b`, compiler
build date `20260731`, and image creation time 2026-08-08. It does not embed an
upstream Git commit, so the immutable OCI digest—not a guessed commit—is the
exact tested distribution revision.

The image packages the upstream AmigaPorts/Bebbo GCC sources and an integrated
SDK. GCC/binutils are free software under their upstream licenses; Amiga SDK
components can have separate redistribution terms. Keep the extracted prefix
external and do not redistribute it with OpenVN.

## Required tools

- `uv` and the locked compiler environment
- CMake 3.16 or newer
- `m68k-amigaos-gcc`, with its matching assembler and linker
- AmigaOS headers for Exec, DOS, Intuition, Graphics, DataTypes and audio/timer
- `amiga.lib` or an equivalent `libamiga.a`
- `file`, `od`, `grep` and other standard POSIX command-line tools
- FS-UAE 3.x, a legally obtained AmigaOS 3.x Kickstart ROM, and a legal
  AmigaOS 3.x system directory for emulator execution

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

## Tested toolchain installation

The tested binary distribution can be extracted to any user-writable external
prefix without installing host packages. Pin the immutable digest shown above,
create a temporary container, and copy `/opt/amiga-6.5.0b` to a versioned path
such as `$HOME/.local/openvn/toolchains/amiga-gcc-6.5.0b-20260731`. The
distribution is relocatable. Docker is only an acquisition mechanism; OpenVN
invokes the extracted compiler directly.

```sh
docker pull \
  stefanreinauer/amiga-gcc@sha256:fa6e66d487fbfdbc78073943a817cd04398165b70a84fcf7b3be4c66269b3e8a
docker create --name openvn-amiga-gcc-extract \
  stefanreinauer/amiga-gcc@sha256:fa6e66d487fbfdbc78073943a817cd04398165b70a84fcf7b3be4c66269b3e8a
docker cp openvn-amiga-gcc-extract:/opt/amiga-6.5.0b \
  /your/external/toolchains/amiga-gcc-6.5.0b-20260731
docker rm openvn-amiga-gcc-extract
```

Developers who prefer a source build can use the upstream
[`AmigaPorts/m68k-amigaos-gcc`](https://github.com/AmigaPorts/m68k-amigaos-gcc)
Makefile with an exact checked-out revision and a user-writable `PREFIX`.
OpenVN does not require `/opt/amiga`.

## Configuration

The normal configuration variables are:

- `OPENVN_AMIGA_GCC`: compiler executable or absolute path
- `OPENVN_AMIGA_SDK`: optional external SDK/NDK root
- `OPENVN_AMIGA_TOOLCHAIN_FILE`: optional CMake toolchain file; defaults to the
  repository's `runtimes/amiga-native/cmake/m68k-amigaos-gcc.cmake`
- `OPENVN_AMIGA_TARGET_FLAGS`: compiler/linker ABI flags; defaults to
  `-m68000 -msoft-float -noixemul`
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
a probe with the target ABI and `-lamiga`. The libnix `-noixemul` CRT avoids the
default newlib executable's runtime dependency on `mathieeedoubbas.library`;
the explicit CPU/float flags preserve the A500/OCS baseline. A missing
dependency therefore fails before generated project output or the target build
is changed.

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

FS-UAE must be installed separately. OpenVN uses AmigaOS 3.x APIs, including
DataTypes, so both a legally obtained OS3 ROM and a legal OS3 system directory
are required. OpenVN does not distribute or auto-select either one.

After the target build succeeds:

```sh
OPENVN_FS_UAE_KICKSTART=/path/to/amiga-os-3.x.rom \
OPENVN_FS_UAE_SYSTEM_DIR=/path/to/amiga-os-3.x-system \
./scripts/run-amiga-demo.sh
```

The launcher copies the supplied system directory to a temporary writable
directory, boots it as DH0, mounts the generated OpenVN package as DH1, and
removes the temporary copy after FS-UAE exits. Proprietary files never enter
the repository or generated package.

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
