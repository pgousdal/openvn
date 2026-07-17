#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

cc="${M68K_CC:-/opt/amiga/bin/m68k-amigaos-gcc}"
if [[ ! -x "$cc" ]]; then
  echo "error: compiler not found: $cc" >&2
  exit 1
fi

work="${TMPDIR:-/tmp}/openvn-pr1o-datatypes"
rm -rf "$work"
mkdir -p "$work"

header="/opt/amiga/m68k-amigaos/ndk-include/inline/datatypes.h"
proto="/opt/amiga/m68k-amigaos/ndk-include/proto/datatypes.h"
graphics="runtimes/amiga-native/src/graphics_amiga.c"

{
  echo "=== compiler ==="
  "$cc" --version | head -n 2
  echo

  echo "=== relevant source includes and calls ==="
  grep -nE 'proto/datatypes|inline.*/datatypes|DrawDTObject' "$graphics" || true
  echo

  echo "=== proto/datatypes.h ==="
  if [[ -f "$proto" ]]; then
    sed -n '1,220p' "$proto"
  else
    echo "missing: $proto"
  fi
  echo

  echo "=== definitions mentioning DrawDTObject in inline header ==="
  if [[ -f "$header" ]]; then
    grep -n -B 8 -A 20 'DrawDTObject' "$header" || true
  else
    echo "missing: $header"
  fi
  echo

  echo "=== preprocessor macros after includes ==="
  printf '%s\n' \
    '#define __USE_INLINE__ 1' \
    '#include <proto/datatypes.h>' \
    '#include <inline/datatypes.h>' |
    "$cc" -dM -E -x c - |
    grep -E 'DrawDTObject|DataTypesBase|INLINE|NO_INLINE|USE_INLINE' |
    sort || true
  echo
} > "$work/header-report.txt" 2>&1

cat > "$work/probe.c" <<'EOF'
#define __USE_INLINE__ 1
#include <exec/types.h>
#include <intuition/intuition.h>
#include <datatypes/datatypes.h>
#include <proto/datatypes.h>
#include <inline/datatypes.h>

extern struct Library *DataTypesBase;

LONG openvn_probe(
    Object *object,
    struct Window *window,
    struct RastPort *rastport
) {
    return DrawDTObjectA(
        object,
        window,
        NULL,
        rastport,
        0,
        0,
        320,
        200,
        0,
        NULL
    );
}
EOF

{
  echo "=== probe compile command ==="
  echo "$cc -D__USE_INLINE__ -O0 -S $work/probe.c -o $work/probe.s"
  echo

  set +e
  "$cc" -D__USE_INLINE__ -O0 -S "$work/probe.c" -o "$work/probe.s"
  asm_status=$?
  "$cc" -D__USE_INLINE__ -O0 -c "$work/probe.c" -o "$work/probe.o"
  obj_status=$?
  set -e

  echo "assembly status: $asm_status"
  echo "object status: $obj_status"
  echo

  if [[ -f "$work/probe.s" ]]; then
    echo "=== probe assembly ==="
    sed -n '1,240p' "$work/probe.s"
    echo
  fi

  if [[ -f "$work/probe.o" ]]; then
    echo "=== undefined symbols in probe object ==="
    "${cc%gcc}nm" -u "$work/probe.o" 2>/dev/null ||
      m68k-amigaos-nm -u "$work/probe.o" 2>/dev/null ||
      nm -u "$work/probe.o" 2>/dev/null ||
      true
    echo
  fi

  echo "=== project object undefined symbols ==="
  object="build/amiga-demo-player/CMakeFiles/openvn_dispatch.dir/src/graphics_amiga.c.obj"
  if [[ -f "$object" ]]; then
    "${cc%gcc}nm" -u "$object" 2>/dev/null ||
      m68k-amigaos-nm -u "$object" 2>/dev/null ||
      nm -u "$object" 2>/dev/null ||
      true
  else
    echo "missing: $object"
  fi
} > "$work/probe-report.txt" 2>&1

report="$repo_root/openvn-pr1o-datatypes-diagnostic.txt"
{
  cat "$work/header-report.txt"
  echo
  cat "$work/probe-report.txt"
} > "$report"

echo "Diagnostic report written to:"
echo "  $report"
echo
echo "Relevant summary:"
grep -E 'DrawDTObject|DataTypesBase|assembly status|object status| U ' "$report" | tail -n 80 || true
