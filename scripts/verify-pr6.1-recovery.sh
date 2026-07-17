#!/usr/bin/env bash
set -euo pipefail

./scripts/cleanup-generated-artifacts.sh

uv sync --project compiler --extra dev
uv run --project compiler ruff check compiler/src compiler/tests
uv run --project compiler ruff format --check compiler/src compiler/tests
uv run --project compiler pytest

make -C runtimes/amiga-native clean
make -C runtimes/amiga-native test CC=/usr/bin/gcc

archive="runtimes/amiga-native/build-host/libopenvn_dispatch.a"
if ! nm "$archive" | grep -Eq '[[:space:]]T[[:space:]]+openvn_audio_update$'; then
  echo "openvn_audio_update was not exported by libopenvn_dispatch.a" >&2
  exit 1
fi

./scripts/cleanup-generated-artifacts.sh
./scripts/check-repository-hygiene.sh
