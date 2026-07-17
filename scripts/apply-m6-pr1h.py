#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import sys

REPO = Path(__file__).resolve().parents[1]
CMAKE = REPO / "runtimes/amiga-native/CMakeLists.txt"
MAIN = REPO / "runtimes/amiga-native/src/main_amiga.c"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def patch_main() -> None:
    if not MAIN.is_file():
        fail(f"missing {MAIN}")

    text = MAIN.read_text(encoding="utf-8")
    include = "#include <clib/alib_protos.h>"

    if include not in text:
        matches = list(re.finditer(r"^#include\s+[<\"].+[>\"]\s*$", text, re.MULTILINE))
        if not matches:
            fail("could not locate include block in main_amiga.c")
        pos = matches[-1].end()
        text = text[:pos] + "\n" + include + text[pos:]

    MAIN.write_text(text, encoding="utf-8")
    print(f"patched: {MAIN.relative_to(REPO)}")


def patch_cmake() -> None:
    if not CMAKE.is_file():
        fail(f"missing {CMAKE}")

    text = CMAKE.read_text(encoding="utf-8")

    if re.search(
        r"target_link_libraries\s*\(\s*openvn-player\b[^)]*\bamiga\b",
        text,
        re.IGNORECASE | re.DOTALL,
    ):
        print(f"already linked: {CMAKE.relative_to(REPO)}")
        return

    # Prefer adding the SDK support library directly after the executable target.
    pattern = re.compile(
        r"(add_executable\s*\(\s*openvn-player\b.*?\)\s*)",
        re.IGNORECASE | re.DOTALL,
    )
    match = pattern.search(text)
    if not match:
        fail("could not locate add_executable(openvn-player ...) in CMakeLists.txt")

    insertion = (
        match.group(1)
        + "\n"
        + "# AmigaOS SDK call stubs and utility routines such as DrawDTObject/CreatePort.\n"
        + "target_link_libraries(openvn-player PRIVATE openvn_dispatch amiga)\n"
    )

    # Avoid duplicating an existing openvn_dispatch link statement.
    tail = text[match.end():]
    existing = re.match(
        r"\s*target_link_libraries\s*\(\s*openvn-player\b.*?\)\s*",
        tail,
        re.IGNORECASE | re.DOTALL,
    )
    if existing:
        old_stmt = existing.group(0)
        if "openvn_dispatch" in old_stmt:
            replacement = re.sub(
                r"\)\s*$",
                " amiga)",
                old_stmt.rstrip(),
                count=1,
            )
            text = text[:match.end()] + replacement + tail[existing.end():]
        else:
            text = text[:match.end()] + insertion[len(match.group(1)):] + tail
    else:
        text = text[:match.start()] + insertion + text[match.end():]

    CMAKE.write_text(text, encoding="utf-8")
    print(f"patched: {CMAKE.relative_to(REPO)}")


def main() -> None:
    patch_main()
    patch_cmake()


if __name__ == "__main__":
    main()
