#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import sys

REPO = Path(__file__).resolve().parents[1]
CMAKE = REPO / "runtimes/amiga-native/CMakeLists.txt"

BEGIN = "# BEGIN OPENVN M6 PR1M GCC INLINE BINDINGS"
END = "# END OPENVN M6 PR1M GCC INLINE BINDINGS"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not CMAKE.is_file():
        fail(f"missing {CMAKE}")

    text = CMAKE.read_text(encoding="utf-8")

    # Idempotent replacement.
    text = re.sub(
        re.escape(BEGIN) + r".*?" + re.escape(END) + r"\s*",
        "",
        text,
        flags=re.DOTALL,
    )

    if not re.search(r"add_library\s*\(\s*openvn_dispatch\b", text, re.DOTALL):
        fail("could not find add_library(openvn_dispatch ...)")

    block = f"""
{BEGIN}
if(CMAKE_SYSTEM_NAME STREQUAL "AmigaOS")
    # Select GCC inline library-vector bindings from <proto/*.h>.
    # Without this, OS calls are emitted as unresolved external C symbols.
    target_compile_definitions(openvn_dispatch PRIVATE __USE_INLINE__)
    target_compile_definitions(openvn-player PRIVATE __USE_INLINE__)
endif()
{END}
"""

    CMAKE.write_text(text.rstrip() + "\n\n" + block.lstrip(), encoding="utf-8")
    print(f"patched: {CMAKE.relative_to(REPO)}")
    print("enabled: __USE_INLINE__ for openvn_dispatch and openvn-player")


if __name__ == "__main__":
    main()
