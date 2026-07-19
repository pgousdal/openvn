#!/usr/bin/env bash
set -euo pipefail

echo
echo "=========================================="
echo " OpenVN Release Verification"
echo "=========================================="

echo
echo "[1/8] Ruff"
uv run ruff format .
uv run ruff check .

echo
echo "[2/8] Python tests"
uv run pytest

echo
echo "[3/8] Host build"
rm -rf build/host

cmake \
    -S runtimes/amiga-native \
    -B build/host

cmake \
    --build build/host \
    -j$(nproc)

echo
echo "[4/8] Native C tests"

ctest \
    --test-dir build/host \
    --output-on-failure

echo
echo "[5/8] Build Amiga demo"

rm -rf \
    build/amiga-demo-player \
    examples/demo/dist

./scripts/build-m68k-demo-player.sh

echo
echo "[6/8] Verify executable"

file dist/openvn-player-m68k-amigaos

echo
echo "[7/8] Run demo"

fs-uae \
    examples/demo/dist/fs-uae/OpenVNDemo.fs-uae

echo
echo "[8/8] Render log"

cat \
examples/demo/dist/fs-uae/harddrive/OpenVN/openvn-render.log

echo
echo
echo "=========================================="
echo "SUCCESS"
echo "=========================================="
