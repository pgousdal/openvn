#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import sys

REPO = Path(__file__).resolve().parents[1]
SOURCE = REPO / "runtimes/amiga-native/src/graphics_amiga.c"


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def find_matching_paren(text: str, open_index: int) -> int:
    depth = 0
    in_string = False
    in_char = False
    escaped = False
    i = open_index

    while i < len(text):
        ch = text[i]

        if escaped:
            escaped = False
        elif ch == "\\" and (in_string or in_char):
            escaped = True
        elif ch == '"' and not in_char:
            in_string = not in_string
        elif ch == "'" and not in_string:
            in_char = not in_char
        elif not in_string and not in_char:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    return i

        i += 1

    fail("unbalanced parentheses while scanning DrawDTObject call")
    return -1


def patch_calls(text: str) -> tuple[str, int]:
    needle = "DrawDTObject"
    cursor = 0
    output: list[str] = []
    count = 0

    while True:
        start = text.find(needle, cursor)
        if start < 0:
            output.append(text[cursor:])
            break

        # Do not match DrawDTObjectA or identifiers containing this name.
        after_name = start + len(needle)
        if after_name < len(text) and (
            text[after_name].isalnum() or text[after_name] == "_"
        ):
            output.append(text[cursor:after_name])
            cursor = after_name
            continue

        before = text[start - 1] if start > 0 else ""
        if before.isalnum() or before == "_":
            output.append(text[cursor:after_name])
            cursor = after_name
            continue

        open_paren = after_name
        while open_paren < len(text) and text[open_paren].isspace():
            open_paren += 1
        if open_paren >= len(text) or text[open_paren] != "(":
            output.append(text[cursor:after_name])
            cursor = after_name
            continue

        close_paren = find_matching_paren(text, open_paren)
        args = text[open_paren + 1 : close_paren].rstrip()

        # Preserve existing args and supply the TagItem* required by the A form.
        if args.endswith("NULL"):
            new_args = args
        else:
            new_args = args + ", NULL"

        output.append(text[cursor:start])
        output.append("DrawDTObjectA(" + new_args + ")")
        cursor = close_paren + 1
        count += 1

    return "".join(output), count


def main() -> None:
    if not SOURCE.is_file():
        fail(f"missing {SOURCE}")

    text = SOURCE.read_text(encoding="utf-8")
    patched, count = patch_calls(text)

    if count == 0:
        if "DrawDTObjectA(" in text and "DrawDTObject(" not in text:
            print(f"already patched: {SOURCE.relative_to(REPO)}")
            return
        fail("no DrawDTObject calls found")

    if "DrawDTObject(" in patched:
        fail("DrawDTObject calls remain after patch")

    if "#include <stddef.h>" not in patched:
        first_include_end = patched.find("\n", patched.find("#include"))
        if first_include_end < 0:
            fail("could not locate include block")
        patched = (
            patched[: first_include_end + 1]
            + "#include <stddef.h>\n"
            + patched[first_include_end + 1 :]
        )

    SOURCE.write_text(patched, encoding="utf-8")
    print(f"patched {count} call(s): {SOURCE.relative_to(REPO)}")


if __name__ == "__main__":
    main()
