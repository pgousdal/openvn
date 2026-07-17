#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import sys

REPO = Path(__file__).resolve().parents[1]
PLAYER_STATE = REPO / "runtimes/amiga-native/src/player_state.c"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not PLAYER_STATE.is_file():
        fail(f"missing {PLAYER_STATE}")

    text = PLAYER_STATE.read_text(encoding="utf-8")

    amiga_marker = '#define openvn_platform_graphics_init openvn_graphics_amiga_init'
    host_marker = '#define openvn_platform_graphics_init openvn_graphics_host_init'

    if amiga_marker not in text or host_marker not in text:
        fail(
            "M6 PR1f backend-selection block was not found; "
            "apply PR1f before this recovery"
        )

    if "OpenVNPlatformGraphicsContext" not in text:
        text = text.replace(
            amiga_marker,
            amiga_marker
            + "\n"
            + "typedef OpenVNAmigaGraphicsContext OpenVNPlatformGraphicsContext;\n"
            + "typedef OpenVNAmigaAudioContext OpenVNPlatformAudioContext;",
            1,
        )
        text = text.replace(
            host_marker,
            host_marker
            + "\n"
            + "typedef OpenVNHostGraphicsContext OpenVNPlatformGraphicsContext;\n"
            + "typedef OpenVNHostAudioContext OpenVNPlatformAudioContext;",
            1,
        )

    replacements = {
        "static OpenVNHostGraphicsContext g_graphics_context;":
            "static OpenVNPlatformGraphicsContext g_graphics_context;",
        "static OpenVNHostAudioContext g_audio_context;":
            "static OpenVNPlatformAudioContext g_audio_context;",
    }

    for old, new in replacements.items():
        if old in text:
            text = text.replace(old, new, 1)
        elif new not in text:
            fail(f"expected declaration not found: {old}")

    if "static OpenVNHostGraphicsContext" in text:
        fail("host graphics context declaration remains")
    if "static OpenVNHostAudioContext" in text:
        fail("host audio context declaration remains")

    PLAYER_STATE.write_text(text, encoding="utf-8")
    print(f"patched: {PLAYER_STATE.relative_to(REPO)}")


if __name__ == "__main__":
    main()
