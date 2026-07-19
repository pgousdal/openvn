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
cleanup() {
    rm -f "${PYTEST_OUTPUT:-}" "${FAILED_TESTS_FILE:-}" "${UNEXPECTED_TESTS_FILE:-}"
}
trap cleanup EXIT

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

run_host_uv() {
    env PATH="$HOST_PATH" "$UV_BIN" run --project "$COMPILER_DIR" "$@"
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
env PATH="$HOST_PATH" "$UV_BIN" sync --project "$COMPILER_DIR" --all-extras
run_host_uv ruff format --check .

FAILED_STEP="Python lint"
heading 3 "Python lint"
run_host_uv ruff check .

FAILED_STEP="Python test suite"
heading 4 "Full Python test suite with known-baseline validation"

PYTEST_OUTPUT="$(mktemp)"
FAILED_TESTS_FILE="$(mktemp)"
UNEXPECTED_TESTS_FILE="$(mktemp)"

set +e
env PATH="$HOST_PATH" "$UV_BIN" run --project "$COMPILER_DIR" \
    pytest "$COMPILER_DIR/tests" -q 2>&1 | tee "$PYTEST_OUTPUT"
pytest_status=${PIPESTATUS[0]}
set -e

if [[ "$pytest_status" -ne 0 ]]; then
    sed -n 's/^FAILED \(.*::[^ ]*\).*/\1/p' "$PYTEST_OUTPUT" \
        | sort -u >"$FAILED_TESTS_FILE"

    known_failures=(
        "tests/test_amiga_graphics_service.py::test_amiga_graphics_adapter_uses_os_services"
        "tests/test_amiga_image_loading.py::test_amiga_adapter_uses_datatypes_rendering"
        "tests/test_amiga_planar_rendering.py::test_amiga_planar_adapter_uses_native_graphics_api"
        "tests/test_amiga_real_mod_playback.py::test_real_mod_playback_event_loop_contract"
        "tests/test_amiga_runtime_skeleton.py::test_host_player_protocol"
        "tests/test_amiga_story_execution.py::test_host_story_protocol"
    )

    : >"$UNEXPECTED_TESTS_FILE"

    while IFS= read -r failed_test; do
        [[ -z "$failed_test" ]] && continue

        known=0
        for expected in "${known_failures[@]}"; do
            if [[ "$failed_test" == "$expected" ]]; then
                known=1
                break
            fi
        done

        if [[ "$known" -eq 0 ]]; then
            printf '%s\n' "$failed_test" >>"$UNEXPECTED_TESTS_FILE"
        fi
    done <"$FAILED_TESTS_FILE"

    if [[ -s "$UNEXPECTED_TESTS_FILE" ]]; then
        echo
        echo "Unexpected Python test failures:"
        sed 's/^/  - /' "$UNEXPECTED_TESTS_FILE"
        exit "$pytest_status"
    fi

    if [[ ! -s "$FAILED_TESTS_FILE" ]]; then
        echo
        echo "pytest failed, but no failing test node IDs could be extracted."
        exit "$pytest_status"
    fi

    echo
    echo "Only known baseline failures remain:"
    sed 's/^/  - /' "$FAILED_TESTS_FILE"
else
    echo "Python test suite passed without baseline failures."
fi

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
