#!/usr/bin/env bash
set -euo pipefail

repo="${1:-$HOME/Projects/openvn}"
out="${2:-$repo/openvn-amiga-render-debug}"
archive="${out}.tar.gz"

cd "$repo"
rm -rf "$out" "$archive"
mkdir -p "$out"

copy_if_present() {
  local path="$1"
  if [[ -f "$path" ]]; then
    mkdir -p "$out/$(dirname "$path")"
    cp -a "$path" "$out/$path"
  fi
}

for path in \
  runtimes/amiga-native/src/main_amiga.c \
  runtimes/amiga-native/src/player_state.c \
  runtimes/amiga-native/src/player.c \
  runtimes/amiga-native/src/story.c \
  runtimes/amiga-native/src/graphics_amiga.c \
  runtimes/amiga-native/src/graphics.c \
  runtimes/amiga-native/src/image_ilbm.c \
  runtimes/amiga-native/src/assets_runtime.c \
  runtimes/amiga-native/include/openvn/player_state.h \
  runtimes/amiga-native/include/openvn/graphics.h \
  runtimes/amiga-native/include/openvn/image.h \
  runtimes/amiga-native/include/openvn/assets_runtime.h \
  runtimes/amiga-native/CMakeLists.txt \
  scripts/build-m68k-demo-player.sh \
  compiler/src/openvn/fsuae.py \
  examples/demo/dist/fs-uae/harddrive/S/Startup-Sequence
do
  copy_if_present "$path"
done

{
  echo "=== Git status ==="
  git status --short || true
  echo
  echo "=== Relevant source files ==="
  find runtimes/amiga-native -maxdepth 3 -type f | sort
  echo
  echo "=== Graphics/story call sites ==="
  grep -R -n -E \
    'graphics|background|image|ILBM|iff|present|blit|story_start|story_run|show_background|scene' \
    runtimes/amiga-native/src \
    runtimes/amiga-native/include \
    2>/dev/null || true
  echo
  echo "=== Generated asset paths ==="
  grep -R -n -E \
    'room\.iff|alice_neutral\.iff|assets/backgrounds|assets/characters' \
    examples/demo/dist \
    2>/dev/null || true
  echo
  echo "=== Packaged files ==="
  find examples/demo/dist/fs-uae/harddrive/OpenVN -type f -print | sort || true
} > "$out/diagnostics.txt"

if [[ -f examples/demo/dist/fs-uae/OpenVNDemo.fs-uae ]]; then
  cp -a examples/demo/dist/fs-uae/OpenVNDemo.fs-uae "$out/"
fi

tar -czf "$archive" -C "$(dirname "$out")" "$(basename "$out")"

printf 'Created:\n%s\n' "$archive"
