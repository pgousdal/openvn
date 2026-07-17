#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import sys

REPO = Path(__file__).resolve().parents[1]
CMAKE = REPO / "runtimes/amiga-native/CMakeLists.txt"
MAIN = REPO / "runtimes/amiga-native/src/main_amiga.c"

MARKER_BEGIN = "# BEGIN OPENVN M6 PR1I AMIGA SDK LINK"
MARKER_END = "# END OPENVN M6 PR1I AMIGA SDK LINK"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def repair_cmake() -> None:
    if not CMAKE.is_file():
        fail(f"missing {CMAKE}")

    text = CMAKE.read_text(encoding="utf-8")

    # Repair commands accidentally concatenated by the previous recovery.
    commands = (
        "add_executable",
        "add_library",
        "target_link_libraries",
        "target_include_directories",
        "target_compile_definitions",
        "target_compile_options",
        "set",
        "if",
        "elseif",
        "else",
        "endif",
        "add_test",
        "enable_testing",
    )
    for command in commands:
        text = re.sub(
            rf"\)([ \t]*){command}\s*\(",
            lambda m, c=command: ")\n\n" + c + "(",
            text,
        )

    # Remove the block if this recovery is being reapplied.
    text = re.sub(
        re.escape(MARKER_BEGIN) + r".*?" + re.escape(MARKER_END) + r"\s*",
        "",
        text,
        flags=re.DOTALL,
    )

    if not re.search(r"add_executable\s*\(\s*openvn-player\b", text, re.DOTALL):
        fail("could not find add_executable(openvn-player ...)")

    block = f"""
{MARKER_BEGIN}
if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    # SDK stubs used by DrawDTObject(), CreatePort(), and related Amiga calls.
    target_link_libraries(openvn-player PRIVATE amiga)
endif()
{MARKER_END}
"""

    text = text.rstrip() + "\n\n" + block.lstrip()
    CMAKE.write_text(text, encoding="utf-8")
    print(f"repaired: {CMAKE.relative_to(REPO)}")


def repair_main_header() -> None:
    if not MAIN.is_file():
        fail(f"missing {MAIN}")

    text = MAIN.read_text(encoding="utf-8")
    include = "#include <clib/alib_protos.h>"

    if include not in text:
        includes = list(
            re.finditer(r"^#include\s+[<\"].+[>\"]\s*$", text, re.MULTILINE)
        )
        if not includes:
            fail("could not locate include block in main_amiga.c")
        pos = includes[-1].end()
        text = text[:pos] + "\n" + include + text[pos:]
        MAIN.write_text(text, encoding="utf-8")
        print(f"patched: {MAIN.relative_to(REPO)}")
    else:
        print(f"already patched: {MAIN.relative_to(REPO)}")


def main() -> None:
    repair_cmake()
    repair_main_header()


if __name__ == "__main__":
    main()
