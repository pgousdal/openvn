#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

REPO = Path(__file__).resolve().parents[1]
SOURCE = REPO / "runtimes/amiga-native/src/graphics_amiga.c"

CANDIDATE_NAMES = (
    "inline/datatypes.h",
    "inline/datatypes_protos.h",
    "inline4/datatypes.h",
)


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def compiler_path() -> Path:
    candidates = [
        os.environ.get("M68K_CC"),
        "/opt/amiga/bin/m68k-amigaos-gcc",
        "/opt/m68k-amigaos/bin/m68k-amigaos-gcc",
        shutil.which("m68k-amigaos-gcc"),
    ]
    for candidate in candidates:
        if candidate and Path(candidate).is_file():
            return Path(candidate)
    fail("could not locate m68k-amigaos-gcc")
    raise AssertionError


def compiler_search_dirs(cc: Path) -> list[Path]:
    roots: list[Path] = []

    for args in (
        ["-print-sysroot"],
        ["-print-file-name=include"],
        ["-print-file-name=include-fixed"],
    ):
        result = subprocess.run(
            [str(cc), *args],
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        value = result.stdout.strip()
        if value and value not in {".", "include", "include-fixed"}:
            roots.append(Path(value))

    prefix = cc.parent.parent
    roots.extend(
        [
            prefix,
            prefix / "m68k-amigaos",
            prefix / "m68k-amigaos/include",
            prefix / "include",
            Path("/opt/amiga"),
            Path("/opt/amiga/m68k-amigaos/include"),
        ]
    )

    unique: list[Path] = []
    seen: set[str] = set()
    for root in roots:
        key = str(root)
        if key not in seen and root.exists():
            seen.add(key)
            unique.append(root)
    return unique


def find_inline_header(cc: Path) -> tuple[str, Path]:
    # First ask the compiler whether a normal include name resolves.
    for name in CANDIDATE_NAMES:
        source = f"#include <{name}>\n"
        result = subprocess.run(
            [str(cc), "-E", "-x", "c", "-"],
            input=source,
            text=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode == 0:
            # Find a physical copy for diagnostics.
            for root in compiler_search_dirs(cc):
                direct = root / name
                if direct.is_file():
                    return name, direct
                matches = list(root.glob(f"**/{name}"))
                if matches:
                    return name, matches[0]
            return name, Path(f"<compiler search path>/{name}")

    # Fall back to searching the SDK tree and derive an includable suffix.
    for root in compiler_search_dirs(cc):
        for basename in ("datatypes.h", "datatypes_protos.h"):
            for match in root.glob(f"**/inline*/{basename}"):
                parts = match.parts
                for marker in ("inline", "inline4"):
                    if marker in parts:
                        idx = parts.index(marker)
                        name = "/".join(parts[idx:])
                        test = subprocess.run(
                            [str(cc), "-E", "-x", "c", "-"],
                            input=f"#include <{name}>\n",
                            text=True,
                            stdout=subprocess.DEVNULL,
                            stderr=subprocess.DEVNULL,
                        )
                        if test.returncode == 0:
                            return name, match

    fail(
        "no GCC datatypes inline header found in the active Amiga SDK; "
        "searched compiler include paths and /opt/amiga"
    )
    raise AssertionError


def patch_source(header_name: str) -> None:
    if not SOURCE.is_file():
        fail(f"missing {SOURCE}")

    text = SOURCE.read_text(encoding="utf-8")

    marker = "/* OpenVN: explicit classic GCC datatypes inline binding. */"
    block = f"{marker}\n#include <{header_name}>"

    # Replace an earlier PR1n block when re-running with another SDK.
    text = re.sub(
        r"/\* OpenVN: explicit classic GCC datatypes inline binding\. \*/\s*"
        r"#include\s*<[^>]+>",
        block,
        text,
        flags=re.MULTILINE,
    )

    if marker not in text:
        proto = "#include <proto/datatypes.h>"
        if proto in text:
            text = text.replace(proto, proto + "\n" + block, 1)
        else:
            includes = list(
                re.finditer(r"^#include\s+[<\"].+[>\"]\s*$", text, re.MULTILINE)
            )
            if not includes:
                fail("could not locate include block in graphics_amiga.c")
            pos = includes[-1].end()
            text = text[:pos] + "\n" + block + text[pos:]

    if "DrawDTObjectA(" not in text:
        fail("DrawDTObjectA calls are missing; apply PR1k first")

    SOURCE.write_text(text, encoding="utf-8")
    print(f"patched: {SOURCE.relative_to(REPO)}")


def main() -> None:
    cc = compiler_path()
    header_name, header_path = find_inline_header(cc)

    print(f"compiler: {cc}")
    print(f"inline header: <{header_name}>")
    print(f"header path: {header_path}")

    patch_source(header_name)


if __name__ == "__main__":
    main()
