#!/usr/bin/env bash
set -euo pipefail
fail=0
find . -name "__pycache__" | grep -q . && { echo "__pycache__ found"; fail=1; } || true
find . -name "*.pyc" | grep -q . && { echo "*.pyc found"; fail=1; } || true
for f in DELIVERY.md DELIVERY-M2-FIX.md README-patch.md CHANGELOG-patch.md VERIFICATION.md README-replacement.md ci-snippet.yml REMOVE-FILES.txt; do
 [ -e "$f" ] && { echo "Unexpected file: $f"; fail=1; }
done
[ $fail -eq 0 ] && echo "Repository hygiene checks passed." || exit 1
