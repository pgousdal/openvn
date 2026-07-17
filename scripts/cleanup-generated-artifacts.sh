#!/usr/bin/env bash
set -euo pipefail

find . -type d -name '__pycache__' \
  -not -path './.git/*' \
  -not -path './compiler/.venv/*' \
  -prune -exec rm -rf {} +

find . -type f \( -name '*.pyc' -o -name '*.pyo' \) \
  -not -path './.git/*' \
  -not -path './compiler/.venv/*' \
  -delete

rm -rf runtimes/amiga-native/build-host
