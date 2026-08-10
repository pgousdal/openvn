#!/usr/bin/env bash
set -Eeuo pipefail

MODE="all"
if [[ "${1:-}" == "--host-only" ]]; then
    MODE="host"
    shift
fi
if [[ "$#" -ne 0 ]]; then
    echo "usage: $0 [--host-only]" >&2
    exit 2
fi

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
COMPILER_DIR="$ROOT_DIR/compiler"
HOST_BUILD_DIR="${OPENVN_HOST_BUILD_DIR:-$ROOT_DIR/build/release-host}"
ORIGINAL_PATH="$PATH"
UV_BIN="$(command -v uv || true)"

resolve_tool() {
    local configured="$1"
    local default_name="$2"

    if [[ -z "$configured" ]]; then
        command -v "$default_name" || true
    elif [[ "$configured" == */* ]]; then
        printf '%s\n' "$configured"
    else
        command -v "$configured" || true
    fi
}

CMAKE_BIN="$(resolve_tool "${OPENVN_CMAKE:-}" cmake)"
CTEST_BIN="$(resolve_tool "${OPENVN_CTEST:-}" ctest)"

if [[ -z "$UV_BIN" ]]; then
    echo "ERROR: uv was not found in PATH."
    exit 1
fi
if [[ -z "$CMAKE_BIN" || ! -x "$CMAKE_BIN" ]]; then
    echo "ERROR: CMake was not found. Install it or set OPENVN_CMAKE to its executable."
    exit 1
fi
if [[ -z "$CTEST_BIN" || ! -x "$CTEST_BIN" ]]; then
    echo "ERROR: CTest was not found. Install it or set OPENVN_CTEST to its executable."
    exit 1
fi

CMAKE_BIN="$(realpath "$CMAKE_BIN")"
CTEST_BIN="$(realpath "$CTEST_BIN")"
HOST_PATH="$(dirname "$CMAKE_BIN"):$(dirname "$CTEST_BIN"):/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin"

FAILED_STEP=""
TOTAL_STEPS=10
[[ "$MODE" == "host" ]] && TOTAL_STEPS=9

on_error() {
    local exit_code=$?
    local line_number=$1
    echo
    echo "FAILED${FAILED_STEP:+ during $FAILED_STEP} at line $line_number (exit $exit_code)."
    exit "$exit_code"
}
trap 'on_error "$LINENO"' ERR

heading() {
    printf '\n[%s/%s] %s\n' "$1" "$TOTAL_STEPS" "$2"
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
heading 5 "Canonical demo validation"
run_compiler_uv openvn validate "$ROOT_DIR/examples/demo"

FAILED_STEP="canonical demo strict compilation"
heading 6 "Canonical demo strict compilation"
run_compiler_uv openvn compile "$ROOT_DIR/examples/demo" --strict

FAILED_STEP="host CMake configuration"
heading 7 "Host runtime configuration"
rm -rf "$HOST_BUILD_DIR"
env PATH="$HOST_PATH" "$CMAKE_BIN" \
    -S "$ROOT_DIR/runtimes/amiga-native" \
    -B "$HOST_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release

FAILED_STEP="host runtime build"
heading 8 "Host runtime build"
env PATH="$HOST_PATH" "$CMAKE_BIN" --build "$HOST_BUILD_DIR" --parallel

FAILED_STEP="native runtime tests"
heading 9 "Native runtime tests"
env PATH="$HOST_PATH" "$CTEST_BIN" \
    --test-dir "$HOST_BUILD_DIR" \
    --output-on-failure

if [[ "$MODE" == "host" ]]; then
    echo
    echo "=========================================="
    echo " OpenVN host verification passed"
    echo " Amiga target verification was not requested"
    echo "=========================================="
    exit 0
fi

FAILED_STEP="m68k Amiga build"
heading 10 "m68k Amiga build, artifact verification and FS-UAE packaging"
if [[ ! -x "$ROOT_DIR/scripts/build-m68k-demo-player.sh" ]]; then
    echo "ERROR: scripts/build-m68k-demo-player.sh is missing or not executable."
    exit 1
fi
env PATH="$ORIGINAL_PATH" "$ROOT_DIR/scripts/build-m68k-demo-player.sh"

echo
echo "=========================================="
echo " OpenVN release verification passed"
echo "=========================================="
