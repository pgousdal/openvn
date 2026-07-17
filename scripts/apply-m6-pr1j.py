#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
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
    before = text

    # DrawDTObject() is the stdarg convenience wrapper. Classic GCC
    # toolchains may not provide its linker stub. DrawDTObjectA() is the
    # actual datatypes.library entry point and accepts a TagItem pointer.
    pattern = re.compile(
        r"\bDrawDTObject\s*\("
        r"(?P<args>.*?)"
        r",\s*TAG_DONE\s*\)",
        re.DOTALL,
    )

    def replace(match: re.Match[str]) -> str:
        return f"DrawDTObjectA({match.group('args')}, NULL)"

    text, count = pattern.subn(replace, text)

    if count == 0:
        if "DrawDTObjectA(" in text and "DrawDTObject(" not in text:
            print(f"already patched: {SOURCE.relative_to(REPO)}")
            return
        fail("no DrawDTObject(..., TAG_DONE) calls found")

    if "DrawDTObject(" in text:
        fail("one or more DrawDTObject calls remain after patch")

    # Ensure NULL is available.
    if "#include <stddef.h>" not in text:
        include_pos = text.find("\n", text.find("#include"))
        if include_pos == -1:
            fail("could not locate include section")
        text = text[: include_pos + 1] + "#include <stddef.h>\n" + text[include_pos + 1 :]

    SOURCE.write_text(text, encoding="utf-8")
    print(f"patched {count} call(s): {SOURCE.relative_to(REPO)}")


if __name__ == "__main__":
    main()
