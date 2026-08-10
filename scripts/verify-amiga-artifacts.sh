#!/usr/bin/env bash
set -Eeuo pipefail

REPO="${OPENVN_REPO:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)}"
PLAYER="${OPENVN_PLAYER:-$REPO/dist/openvn-player-m68k-amigaos}"
PACKAGE="${OPENVN_AMIGA_PACKAGE:-$REPO/examples/demo/dist/amiga-ocs}"

fail() { printf 'error: %s\n' "$*" >&2; exit 1; }

[[ -f "$PLAYER" ]] || fail "missing Amiga executable: $PLAYER"
[[ -s "$PLAYER" ]] || fail "Amiga executable is empty: $PLAYER"

HUNK_MAGIC="$(od -An -tx1 -N4 "$PLAYER" | tr -d ' \n')"
[[ "$HUNK_MAGIC" == "000003f3" ]] || fail "expected Amiga Hunk header 000003f3, got $HUNK_MAGIC"

FILE_DESCRIPTION="$(file -b "$PLAYER")"
printf 'Executable: %s\n' "$PLAYER"
printf 'Format: %s\n' "$FILE_DESCRIPTION"
printf 'Size: %s bytes\n' "$(wc -c <"$PLAYER")"

[[ "$FILE_DESCRIPTION" =~ AmigaOS|loadseg|m68k|Hunk|hunk ]] || fail "file(1) did not recognize an Amiga/m68k executable"

REQUIRED_FILES=(
    manifest.json
    story/story.generated.c
    story/story.generated.h
    story/assets.generated.c
    story/assets.generated.h
    story/story.openvn.json
    assets/backgrounds/room.iff
    assets/characters/alice_neutral.iff
    assets/music/intro.mod
    assets/sfx/click.8svx
)
for relative in "${REQUIRED_FILES[@]}"; do
    [[ -s "$PACKAGE/$relative" ]] || fail "missing or empty packaged artifact: $PACKAGE/$relative"
done

grep -q 'OPENVN_GENERATED_STORY' "$PACKAGE/story/story.generated.c" || fail "generated story table is missing"
grep -q 'assets/backgrounds/room\.iff' "$PACKAGE/story/assets.generated.c" || fail "generated background asset entry is missing"
grep -q 'assets/characters/alice_neutral\.iff' "$PACKAGE/story/assets.generated.c" || fail "generated character asset entry is missing"
grep -q 'assets/music/intro\.mod' "$PACKAGE/story/assets.generated.c" || fail "generated MOD asset entry is missing"

printf 'Package: %s\n' "$PACKAGE"
printf 'Artifact verification passed.\n'
