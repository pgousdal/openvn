#!/usr/bin/env bash
set -Eeuo pipefail

REPO="${OPENVN_REPO:-$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)}"
CONFIG="${OPENVN_FS_UAE_CONFIG:-$REPO/examples/demo/dist/fs-uae/OpenVNDemo.fs-uae}"
KICKSTART="${OPENVN_FS_UAE_KICKSTART:-}"
SYSTEM_DIR="${OPENVN_FS_UAE_SYSTEM_DIR:-}"
FS_UAE="${OPENVN_FS_UAE:-fs-uae}"
PACKAGE_DIR="$(dirname "$CONFIG")/harddrive"

fail() { printf 'error: %s\n' "$*" >&2; exit 1; }

[[ -f "$CONFIG" ]] || fail "missing generated FS-UAE config: $CONFIG (run scripts/build-m68k-demo-player.sh)"
[[ -n "$KICKSTART" ]] || fail "set OPENVN_FS_UAE_KICKSTART to a legally obtained AmigaOS 3.x ROM"
[[ -f "$KICKSTART" ]] || fail "Kickstart ROM does not exist: $KICKSTART"
[[ -n "$SYSTEM_DIR" ]] || fail "set OPENVN_FS_UAE_SYSTEM_DIR to a legal AmigaOS 3.x system directory"
[[ -f "$SYSTEM_DIR/C/Assign" ]] || fail "AmigaOS system directory has no C/Assign: $SYSTEM_DIR"
[[ -f "$SYSTEM_DIR/Libs/datatypes.library" ]] || fail "AmigaOS system directory has no Libs/datatypes.library: $SYSTEM_DIR"
[[ -f "$PACKAGE_DIR/runtime/openvn-player" ]] || fail "generated package has no OpenVN player: $PACKAGE_DIR"
[[ -f "$PACKAGE_DIR/S/Startup-Sequence" ]] || fail "generated package has no Startup-Sequence: $PACKAGE_DIR"
command -v "$FS_UAE" >/dev/null 2>&1 || fail "FS-UAE executable was not found: $FS_UAE"

WORK_DIR="$(mktemp -d /tmp/openvn-fsuae-system.XXXXXX)"
trap 'rm -rf "$WORK_DIR"' EXIT
SYSTEM_COPY="$WORK_DIR/system"
cp -a "$SYSTEM_DIR" "$SYSTEM_COPY"
chmod -R u+rwX "$SYSTEM_COPY"

STARTUP="$SYSTEM_COPY/S/Startup-Sequence"
cp "$PACKAGE_DIR/S/Startup-Sequence" "$STARTUP"
sed -i \
    '/^runtime\/openvn-player$/i C:MakeDir RAM:T RAM:ENV\
C:Assign T: RAM:T\
C:Assign ENV: RAM:ENV\
C:SetPatch QUIET\
CD DH1:' \
    "$STARTUP"
grep -q '^runtime/openvn-player$' "$STARTUP" || \
    fail "generated Startup-Sequence does not launch runtime/openvn-player"

"$FS_UAE" "$CONFIG" \
    "--kickstart-file=$KICKSTART" \
    "--hard-drive-0=$SYSTEM_COPY" \
    "--hard-drive-0-label=SYSTEM" \
    "--hard-drive-1=$PACKAGE_DIR" \
    "--hard-drive-1-label=OPENVN"
