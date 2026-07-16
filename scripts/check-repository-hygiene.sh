#!/usr/bin/env bash
set -euo pipefail

fail=0
for p in DELIVERY.md DELIVERY-M2-FIX.md README-patch.md VERIFICATION.md; do
  if [ -e "$p" ]; then
    echo "Unexpected file: $p"
    fail=1
  fi
done

if find . -name "__pycache__" | grep -q .; then
  echo "__pycache__ found"
  fail=1
fi

if find . -name "*.pyc" | grep -q .; then
  echo "*.pyc found"
  fail=1
fi

if [ $fail -eq 0 ]; then
  echo "Repository hygiene checks passed."
else
  exit 1
fi
