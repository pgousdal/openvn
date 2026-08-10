#!/usr/bin/env bash
set -Eeuo pipefail

REPO="${OPENVN_REPO:-$PWD}"
DEMO="${OPENVN_DEMO:-$REPO/examples/demo}"
BUILD_DIR="${OPENVN_AMIGA_BUILD_DIR:-$REPO/build/amiga-demo-player}"
PLAYER_OUT="${OPENVN_PLAYER_OUT:-$REPO/dist/openvn-player-m68k-amigaos}"

fail() { printf 'error: %s\n' "$*" >&2; exit 1; }
find_command() {
    local configured="$1"
    local default_name="$2"
    local description="$3"
    local resolved

    if [[ -n "$configured" ]]; then
        if [[ "$configured" == */* ]]; then
            [[ -x "$configured" ]] || fail "$description is not executable: $configured"
            realpath "$configured"
            return
        fi
        resolved="$(command -v "$configured" || true)"
        [[ -n "$resolved" ]] || fail "$description was not found in PATH: $configured"
        realpath "$resolved"
        return
    fi

    resolved="$(command -v "$default_name" || true)"
    [[ -n "$resolved" ]] || fail "missing $description: set the corresponding OPENVN_* variable or add $default_name to PATH"
    realpath "$resolved"
}

[[ -f "$REPO/compiler/pyproject.toml" ]] || fail "set OPENVN_REPO to the OpenVN repository root"
[[ -d "$DEMO" ]] || fail "canonical demo directory is missing: $DEMO"

CMAKE_BIN="$(find_command "${OPENVN_CMAKE:-}" cmake CMake)"
UV_BIN="$(find_command "${OPENVN_UV:-}" uv uv)"

LEGACY_GCC=""
if [[ -n "${OPENVN_TOOLCHAIN_PATH:-}" ]]; then
    LEGACY_GCC="$OPENVN_TOOLCHAIN_PATH/bin/m68k-amigaos-gcc"
fi
AMIGA_GCC="$(find_command "${OPENVN_AMIGA_GCC:-${M68K_CC:-$LEGACY_GCC}}" m68k-amigaos-gcc "m68k Amiga GCC")"

TOOLCHAIN_FILE="${OPENVN_AMIGA_TOOLCHAIN_FILE:-${OPENVN_TOOLCHAIN_FILE:-$REPO/runtimes/amiga-native/cmake/m68k-amigaos-gcc.cmake}}"
[[ -f "$TOOLCHAIN_FILE" ]] || fail "missing CMake toolchain file: $TOOLCHAIN_FILE (set OPENVN_AMIGA_TOOLCHAIN_FILE)"
TOOLCHAIN_FILE="$(realpath "$TOOLCHAIN_FILE")"

AMIGA_SDK="${OPENVN_AMIGA_SDK:-}"
SDK_INCLUDE=""
SDK_LIBRARY=""
if [[ -n "$AMIGA_SDK" ]]; then
    [[ -d "$AMIGA_SDK" ]] || fail "Amiga SDK/NDK directory does not exist: $AMIGA_SDK"
    AMIGA_SDK="$(realpath "$AMIGA_SDK")"
    if [[ -f "$AMIGA_SDK/Include_H/proto/exec.h" ]]; then
        SDK_INCLUDE="$AMIGA_SDK/Include_H"
    elif [[ -f "$AMIGA_SDK/include/proto/exec.h" ]]; then
        SDK_INCLUDE="$AMIGA_SDK/include"
    elif [[ -f "$AMIGA_SDK/m68k-amigaos/ndk-include/proto/exec.h" ]]; then
        SDK_INCLUDE="$AMIGA_SDK/m68k-amigaos/ndk-include"
    else
        fail "missing Amiga NDK headers below OPENVN_AMIGA_SDK=$AMIGA_SDK"
    fi

    if [[ -f "$AMIGA_SDK/lib/amiga.lib" || -f "$AMIGA_SDK/lib/libamiga.a" ]]; then
        SDK_LIBRARY="$AMIGA_SDK/lib"
    elif [[ -f "$AMIGA_SDK/m68k-amigaos/lib/libamiga.a" ]]; then
        SDK_LIBRARY="$AMIGA_SDK/m68k-amigaos/lib"
    else
        fail "missing amiga.lib or libamiga.a below OPENVN_AMIGA_SDK=$AMIGA_SDK"
    fi
fi

AMIGA_AS="$($AMIGA_GCC -print-prog-name=as)"
AMIGA_LD="$($AMIGA_GCC -print-prog-name=ld)"
[[ "$AMIGA_AS" == */* && -x "$AMIGA_AS" ]] || command -v "$AMIGA_AS" >/dev/null 2>&1 || fail "m68k assembler required by $AMIGA_GCC was not found: $AMIGA_AS"
[[ "$AMIGA_LD" == */* && -x "$AMIGA_LD" ]] || command -v "$AMIGA_LD" >/dev/null 2>&1 || fail "m68k linker required by $AMIGA_GCC was not found: $AMIGA_LD"

CHECK_DIR="$(mktemp -d /tmp/openvn-amiga-toolchain.XXXXXX)"
trap 'rm -rf "$CHECK_DIR"' EXIT
printf '%s\n' \
    '#include <exec/types.h>' \
    '#include <proto/exec.h>' \
    '#include <proto/graphics.h>' \
    '#include <proto/intuition.h>' \
    '#include <proto/datatypes.h>' \
    '#include <clib/alib_protos.h>' \
    'int main(void) { return 0; }' >"$CHECK_DIR/probe.c"

PROBE_ARGS=()
[[ -n "$SDK_INCLUDE" ]] && PROBE_ARGS+=("-I$SDK_INCLUDE")
[[ -n "$SDK_LIBRARY" ]] && PROBE_ARGS+=("-L$SDK_LIBRARY")
if ! "$AMIGA_GCC" "${PROBE_ARGS[@]}" -D__AMIGA__ -D__USE_INLINE__ \
    -fsyntax-only "$CHECK_DIR/probe.c" >"$CHECK_DIR/headers.log" 2>&1; then
    printf 'error: Amiga SDK/header probe failed:\n' >&2
    sed 's/^/  /' "$CHECK_DIR/headers.log" >&2
    fail "supply compatible NDK headers through the GCC sysroot or OPENVN_AMIGA_SDK"
fi
if ! "$AMIGA_GCC" "${PROBE_ARGS[@]}" -D__AMIGA__ -D__USE_INLINE__ \
    "$CHECK_DIR/probe.c" -lamiga -o "$CHECK_DIR/probe" >"$CHECK_DIR/link.log" 2>&1; then
    printf 'error: Amiga linker/library probe failed:\n' >&2
    sed 's/^/  /' "$CHECK_DIR/link.log" >&2
    fail "supply compatible startup files and amiga.lib/libamiga.a through the GCC sysroot or OPENVN_AMIGA_SDK"
fi

printf 'Toolchain configuration:\n'
printf '  CMake: %s\n' "$CMAKE_BIN"
printf '  GCC: %s\n' "$AMIGA_GCC"
printf '  assembler: %s\n' "$AMIGA_AS"
printf '  linker: %s\n' "$AMIGA_LD"
printf '  toolchain file: %s\n' "$TOOLCHAIN_FILE"
printf '  SDK/NDK: %s\n' "${AMIGA_SDK:-compiler-integrated}"

printf '[1/4] Building OpenVN demo packages\n'
"$UV_BIN" run --project "$REPO/compiler" openvn build "$DEMO" --clean

AMIGA_PACKAGE="$DEMO/dist/amiga-ocs"
STORY="$AMIGA_PACKAGE/story/story.generated.c"
ASSETS="$AMIGA_PACKAGE/story/assets.generated.c"
[[ -f "$STORY" ]] || fail "missing generated story: $STORY"
[[ -f "$ASSETS" ]] || fail "missing generated assets: $ASSETS"

printf '[2/4] Cross-compiling openvn-player for m68k AmigaOS\n'
BUILD_DIR="$(realpath -m "$BUILD_DIR")"
if [[ "$BUILD_DIR" == "/" || "$BUILD_DIR" == "$REPO" ]]; then
    fail "refusing unsafe OPENVN_AMIGA_BUILD_DIR: $BUILD_DIR"
fi
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

export OPENVN_AMIGA_GCC="$AMIGA_GCC"
[[ -n "$AMIGA_SDK" ]] && export OPENVN_AMIGA_SDK="$AMIGA_SDK"

"$CMAKE_BIN" \
    -S "$REPO/runtimes/amiga-native" \
    -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_C_COMPILER="$AMIGA_GCC" \
    -DOPENVN_AMIGA_SDK="$AMIGA_SDK" \
    -DOPENVN_GENERATED_STORY_SOURCE="$STORY" \
    -DOPENVN_GENERATED_ASSETS_SOURCE="$ASSETS" \
    -DCMAKE_BUILD_TYPE=Release
"$CMAKE_BIN" --build "$BUILD_DIR" --target openvn-player --parallel

PLAYER="$BUILD_DIR/openvn-player"
[[ -f "$PLAYER" ]] || fail "build did not produce: $PLAYER"
mkdir -p "$(dirname "$PLAYER_OUT")"
cp "$PLAYER" "$PLAYER_OUT"
chmod +x "$PLAYER_OUT"

printf '[3/4] Verifying executable and package artifacts\n'
OPENVN_PLAYER="$PLAYER_OUT" OPENVN_AMIGA_PACKAGE="$AMIGA_PACKAGE" \
    "$REPO/scripts/verify-amiga-artifacts.sh"

printf '[4/4] Packaging runnable FS-UAE demo\n'
"$UV_BIN" run --project "$REPO/compiler" openvn package-fsuae "$DEMO" \
    --player "$PLAYER_OUT" \
    --clean

OPENVN_PLAYER="$DEMO/dist/fs-uae/harddrive/OpenVN/runtime/openvn-player" \
OPENVN_AMIGA_PACKAGE="$DEMO/dist/fs-uae/harddrive/OpenVN" \
    "$REPO/scripts/verify-amiga-artifacts.sh"

printf '\nDone.\n'
printf 'Player: %s\n' "$PLAYER_OUT"
printf 'FS-UAE config: %s\n' "$DEMO/dist/fs-uae/OpenVNDemo.fs-uae"
