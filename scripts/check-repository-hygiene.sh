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

# Native runtime build output must never be committed.
if find runtimes \
  -type d \( -name build-host -o -name 'build-*' -o -name CMakeFiles \) \
  -print -quit 2>/dev/null | grep -q .; then
  echo "Native CMake build output found"
  exit 1
fi

if find . \
  -path './.git' -prune -o \
  -path './compiler/.venv' -prune -o \
  -type f \( \
    -name CMakeCache.txt -o \
    -name '*.o' -o \
    -name '*.obj' -o \
    -name '*.a' \
  \) \
  -print -quit | grep -q .; then
  echo "Native build artefacts found"
  exit 1
fi

# Catch accidental machine-specific home paths in tracked text files.
if git grep -nE '/home/[[:alnum:]_.-]+/' -- \
  ':!*.png' ':!*.jpg' ':!*.jpeg' ':!*.gif' ':!*.wav' ':!*.ogg' \
  ':!*.iff' ':!*.8svx' ':!*.mod' ':!*.zip' ':!*.pyc' \
  2>/dev/null | grep -vE \
  '(^|:)scripts/check-repository-hygiene\.sh:|/home/user/|/home/example/'; then
  echo "Machine-specific /home path found in tracked text"
  exit 1
fi
