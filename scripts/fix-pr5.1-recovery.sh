#!/usr/bin/env bash
set -euo pipefail

cd "${1:-$(pwd)}"

find . \
  -path './.git' -prune -o \
  -path './compiler/.venv' -prune -o \
  -type d -name '__pycache__' -exec rm -rf {} +

find . \
  -path './.git' -prune -o \
  -path './compiler/.venv' -prune -o \
  -type f -name '*.pyc' -delete

rm -rf runtimes/amiga-native/build-host
echo "PR5.1 recovery cleanup complete."
