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
    original = text

    host_audio = '#include "openvn_audio_host.h"'
    host_graphics = '#include "openvn_graphics_host.h"'

    conditional = '''#if defined(__amigaos__) || defined(__AMIGA__) || defined(AMIGA)
#include "openvn_audio_amiga.h"
#include "openvn_graphics_amiga.h"
#define openvn_platform_audio_init openvn_audio_amiga_init
#define openvn_platform_graphics_init openvn_graphics_amiga_init
#else
#include "openvn_audio_host.h"
#include "openvn_graphics_host.h"
#define openvn_platform_audio_init openvn_audio_host_init
#define openvn_platform_graphics_init openvn_graphics_host_init
#endif'''

    if "openvn_platform_audio_init" not in text:
        if host_audio not in text or host_graphics not in text:
            fail(
                "expected host backend includes were not found in player_state.c; "
                "refusing to patch an unknown layout"
            )

        paired = host_audio + "\n" + host_graphics
        if paired in text:
            text = text.replace(paired, conditional, 1)
        else:
            text = text.replace(host_audio, "", 1)
            text = text.replace(host_graphics, conditional, 1)

    text = text.replace("openvn_audio_host_init(", "openvn_platform_audio_init(")
    text = text.replace("openvn_graphics_host_init(", "openvn_platform_graphics_init(")

    if "openvn_audio_host_init(" in text or "openvn_graphics_host_init(" in text:
        fail("not all host initializer calls were replaced")
    if "openvn_platform_audio_init(" not in text:
        fail("no audio initializer call found after patch")
    if "openvn_platform_graphics_init(" not in text:
        fail("no graphics initializer call found after patch")

    PLAYER_STATE.write_text(text, encoding="utf-8")
    print(f"patched: {PLAYER_STATE.relative_to(REPO)}")


if __name__ == "__main__":
    main()
