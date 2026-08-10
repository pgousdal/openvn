#!/usr/bin/env bash
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
COMPILER_DIR="$ROOT_DIR/compiler"
HOST_BUILD_DIR="${OPENVN_HOST_BUILD_DIR:-$ROOT_DIR/build/release-host}"
AMIGA_BUILD_DIR="${OPENVN_AMIGA_BUILD_DIR:-$ROOT_DIR/build/amiga-demo-player}"

ORIGINAL_PATH="$PATH"
HOST_PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"
UV_BIN="$(command -v uv || true)"

if [[ -z "$UV_BIN" ]]; then
    echo "ERROR: uv was not found in PATH."
    exit 1
fi

FAILED_STEP=""

on_error() {
    local exit_code=$?
    local line_number=$1
    echo
    echo "FAILED${FAILED_STEP:+ during $FAILED_STEP} at line $line_number (exit $exit_code)."
    exit "$exit_code"
}
trap 'on_error "$LINENO"' ERR

heading() {
    printf '\n[%s/9] %s\n' "$1" "$2"
}

run_compiler_uv() {
    (
        cd "$COMPILER_DIR"
        env PATH="$HOST_PATH" "$UV_BIN" run "$@"
    )
}

echo
echo "=========================================="
echo " OpenVN Release Verification"
echo "=========================================="

cd "$ROOT_DIR"

FAILED_STEP="repository hygiene"
heading 1 "Repository hygiene"
./scripts/check-repository-hygiene.sh

FAILED_STEP="Python formatting"
heading 2 "Python formatting"
(
    cd "$COMPILER_DIR"
    env PATH="$HOST_PATH" "$UV_BIN" sync --all-extras
)
run_compiler_uv ruff format --check .

FAILED_STEP="Python lint"
heading 3 "Python lint"
run_compiler_uv ruff check .

FAILED_STEP="Python test suite"
heading 4 "Full Python test suite"
run_compiler_uv pytest -q

FAILED_STEP="host CMake configuration"
heading 5 "Host runtime configuration"
rm -rf "$HOST_BUILD_DIR"
env PATH="$HOST_PATH" cmake \
    -S "$ROOT_DIR/runtimes/amiga-native" \
    -B "$HOST_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENVN_BUILD_TESTS=ON

FAILED_STEP="host runtime build"
heading 6 "Host runtime build"
env PATH="$HOST_PATH" cmake --build "$HOST_BUILD_DIR" --parallel

FAILED_STEP="native runtime tests"
heading 7 "Native runtime tests"
env PATH="$HOST_PATH" ctest \
    --test-dir "$HOST_BUILD_DIR" \
    --output-on-failure

FAILED_STEP="m68k Amiga build"
heading 8 "m68k Amiga demo-player build"
if [[ ! -x "$ROOT_DIR/scripts/build-m68k-demo-player.sh" ]]; then
    echo "ERROR: scripts/build-m68k-demo-player.sh is missing or not executable."
    exit 1
fi
env PATH="$ORIGINAL_PATH" "$ROOT_DIR/scripts/build-m68k-demo-player.sh"

FAILED_STEP="Amiga executable verification"
heading 9 "Amiga executable verification"

if [[ ! -d "$AMIGA_BUILD_DIR" ]]; then
    echo "ERROR: Expected Amiga build directory not found:"
    echo "  $AMIGA_BUILD_DIR"
    exit 1
fi

AMIGA_EXECUTABLE="$(
    find "$AMIGA_BUILD_DIR" -maxdepth 4 -type f \
        \( -name 'openvn-player' -o -name 'openvn-player.exe' -o -name 'OpenVN' \) \
        -print -quit
)"

if [[ -z "$AMIGA_EXECUTABLE" ]]; then
    AMIGA_EXECUTABLE="$(
        find "$AMIGA_BUILD_DIR" -maxdepth 4 -type f -perm -u+x -print -quit
    )"
fi

if [[ -z "$AMIGA_EXECUTABLE" ]]; then
    echo "ERROR: No Amiga executable was found under:"
    echo "  $AMIGA_BUILD_DIR"
    exit 1
fi

echo "Executable: $AMIGA_EXECUTABLE"
file "$AMIGA_EXECUTABLE"

if ! file "$AMIGA_EXECUTABLE" | grep -Eiq 'amiga|m68k|loadseg|hunk'; then
    echo "ERROR: The generated file was not recognized as an Amiga/m68k executable."
    exit 1
fi

echo
echo "=========================================="
echo " OpenVN release verification passed"
echo "=========================================="
