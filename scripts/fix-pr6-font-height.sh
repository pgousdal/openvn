#!/usr/bin/env bash
set -euo pipefail

FILE="runtimes/amiga-native/src/graphics_amiga.c"

if [[ ! -f "$FILE" ]]; then
    echo "ERROR: $FILE not found"
    exit 1
fi

COUNT=$(grep -c "OPENVN_BITMAP_FONT_HEIGHT" "$FILE" || true)

if [[ "$COUNT" -eq 0 ]]; then
    echo "Nothing to fix."
    exit 0
fi

sed -i \
    's/OPENVN_BITMAP_FONT_HEIGHT/OPENVN_BITMAP_FONT_LINE_HEIGHT/g' \
    "$FILE"

echo "Fixed $COUNT occurrence(s)."

grep -n "OPENVN_BITMAP_FONT_LINE_HEIGHT" "$FILE"
