#!/usr/bin/env bash
set -euo pipefail

fail=0

unexpected_files=(
  DELIVERY.md
  DELIVERY-M2-FIX.md
  README-patch.md
  CHANGELOG-patch.md
  VERIFICATION.md
  README-replacement.md
  ci-snippet.yml
  REMOVE-FILES.txt
  MERGE.md
)

for file in "${unexpected_files[@]}"; do
  if [[ -e "$file" ]]; then
    echo "Unexpected file: $file"
    fail=1
  fi
done

if find . \
  -path './.git' -prune -o \
  -path './compiler/.venv' -prune -o \
  -type d -name '__pycache__' -print -quit |
  grep -q .
then
  echo "__pycache__ found outside ignored directories"
  fail=1
fi

if find . \
  -path './.git' -prune -o \
  -path './compiler/.venv' -prune -o \
  -type f -name '*.pyc' -print -quit |
  grep -q .
then
  echo "*.pyc found outside ignored directories"
  fail=1
fi

if [[ "$fail" -ne 0 ]]; then
  exit 1
fi

echo "Repository hygiene checks passed."
