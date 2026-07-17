#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import sys

REPO = Path(__file__).resolve().parents[1]
SOURCE = REPO / "runtimes/amiga-native/src/graphics_amiga.c"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not SOURCE.is_file():
        fail(f"missing {SOURCE}")

    text = SOURCE.read_text(encoding="utf-8")

    assignment = "    draw_message.dtd_Flags = flags;\n"
    replacement = (
        "    (void)flags; "
        "/* This NDK's struct dtDraw has no flags field. */\n"
    )

    if assignment in text:
        text = text.replace(assignment, replacement, 1)
        changed = True
    elif "(void)flags;" in text:
        changed = False
    else:
        fail("expected dtd_Flags assignment not found")

    if "draw_message.dtd_Flags" in text:
        fail("dtd_Flags reference remains")

    SOURCE.write_text(text, encoding="utf-8")
    state = "patched" if changed else "already patched"
    print(f"{state}: {SOURCE.relative_to(REPO)}")
    print("compatibility flags argument retained but ignored for this NDK")


if __name__ == "__main__":
    main()
