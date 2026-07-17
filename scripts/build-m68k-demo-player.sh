#!/usr/bin/env bash
set -euo pipefail

REPO="${OPENVN_REPO:-$PWD}"
TOOLCHAIN_FILE="${OPENVN_TOOLCHAIN_FILE:-$HOME/Projects/AmigaCMakeCrossToolchains/m68k-amigaos-gcc10.cmake}"
TOOLCHAIN_PATH="${OPENVN_TOOLCHAIN_PATH:-/opt/amiga}"
DEMO="${OPENVN_DEMO:-$REPO/examples/demo}"
BUILD_DIR="${OPENVN_AMIGA_BUILD_DIR:-$REPO/build/amiga-demo-player}"
PLAYER_OUT="${OPENVN_PLAYER_OUT:-$REPO/dist/openvn-player-m68k-amigaos}"

fail() { printf 'error: %s\n' "$*" >&2; exit 1; }

[[ -f "$REPO/compiler/pyproject.toml" ]] || fail "run from the OpenVN repository root, or set OPENVN_REPO"
[[ -f "$TOOLCHAIN_FILE" ]] || fail "missing toolchain file: $TOOLCHAIN_FILE"
[[ -x "$TOOLCHAIN_PATH/bin/m68k-amigaos-gcc" ]] || fail "missing compiler: $TOOLCHAIN_PATH/bin/m68k-amigaos-gcc"

TOOLCHAIN_FILE="$(realpath "$TOOLCHAIN_FILE")"
TOOLCHAIN_PATH="$(realpath "$TOOLCHAIN_PATH")"
COMPILER="$TOOLCHAIN_PATH/bin/m68k-amigaos-gcc"
WRAPPER_TOOLCHAIN="$BUILD_DIR/openvn-m68k-toolchain.cmake"

printf '[1/4] Building OpenVN demo packages\n'
cd "$REPO"
uv run --project compiler openvn build "$DEMO" --clean

AMIGA_PACKAGE="$DEMO/dist/amiga-ocs"
STORY="$AMIGA_PACKAGE/story/story.generated.c"
ASSETS="$AMIGA_PACKAGE/story/assets.generated.c"
[[ -f "$STORY" ]] || fail "missing generated story: $STORY"
[[ -f "$ASSETS" ]] || fail "missing generated assets: $ASSETS"

printf '[2/4] Cross-compiling openvn-player for m68k AmigaOS\n'
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Some AmigaCMakeCrossToolchains releases default to /opt/m68k-amigaos and do
# not honor M68K_TOOLCHAIN_PATH from the CMake command line. Load the original
# toolchain through this wrapper and force the compiler location afterwards.
cat > "$WRAPPER_TOOLCHAIN" <<CMAKE
set(M68K_TOOLCHAIN_PATH "$TOOLCHAIN_PATH" CACHE PATH "OpenVN m68k toolchain root" FORCE)
set(M68K_TOOLCHAIN_PREFIX "$TOOLCHAIN_PATH" CACHE PATH "OpenVN m68k toolchain prefix" FORCE)
set(CMAKE_C_COMPILER "$COMPILER" CACHE FILEPATH "m68k AmigaOS C compiler" FORCE)
include("$TOOLCHAIN_FILE")
set(M68K_TOOLCHAIN_PATH "$TOOLCHAIN_PATH" CACHE PATH "OpenVN m68k toolchain root" FORCE)
set(M68K_TOOLCHAIN_PREFIX "$TOOLCHAIN_PATH" CACHE PATH "OpenVN m68k toolchain prefix" FORCE)
set(CMAKE_C_COMPILER "$COMPILER" CACHE FILEPATH "m68k AmigaOS C compiler" FORCE)
CMAKE

export PATH="$TOOLCHAIN_PATH/bin:$PATH"
export CC="$COMPILER"

cmake \
  -S "$REPO/runtimes/amiga-native" \
  -B "$BUILD_DIR" \
  -DCMAKE_TOOLCHAIN_FILE="$WRAPPER_TOOLCHAIN" \
  -DCMAKE_C_COMPILER="$COMPILER" \
  -DM68K_TOOLCHAIN_PATH="$TOOLCHAIN_PATH" \
  -DM68K_TOOLCHAIN_PREFIX="$TOOLCHAIN_PATH" \
  -DOPENVN_GENERATED_STORY_SOURCE="$STORY" \
  -DOPENVN_GENERATED_ASSETS_SOURCE="$ASSETS" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --target openvn-player --parallel "$(nproc)"

PLAYER="$BUILD_DIR/openvn-player"
[[ -f "$PLAYER" ]] || fail "build did not produce: $PLAYER"
mkdir -p "$(dirname "$PLAYER_OUT")"
cp "$PLAYER" "$PLAYER_OUT"
chmod +x "$PLAYER_OUT"

printf '[3/4] Verifying executable format\n'
if [[ -x "$TOOLCHAIN_PATH/bin/m68k-amigaos-readelf" ]]; then
  openvn_hunk_magic="$(od -An -tx1 -N4 "$PLAYER_OUT" | tr -d ' \n')"
  if [[ "$openvn_hunk_magic" != "000003f3" ]]; then
    echo "error: expected Amiga Hunk header 000003f3, got $openvn_hunk_magic" >&2
    exit 1
  fi
  echo "Verified Amiga Hunk executable: $PLAYER_OUT"
elif command -v file >/dev/null 2>&1; then
  file "$PLAYER_OUT"
fi

printf '[4/4] Packaging runnable FS-UAE demo\n'
uv run --project compiler openvn package-fsuae "$DEMO" \
  --player "$PLAYER_OUT" \
  --clean

printf '\nDone.\n'
printf 'Player: %s\n' "$PLAYER_OUT"
printf 'FS-UAE config: %s\n' "$DEMO/dist/fs-uae/OpenVNDemo.fs-uae"
