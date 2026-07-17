#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import re
import sys

REPO = Path(__file__).resolve().parents[1]
SOURCE = REPO / "runtimes/amiga-native/src/graphics_amiga.c"

BEGIN = "/* BEGIN OPENVN M6 PR1P DTM_DRAW COMPAT */"
END = "/* END OPENVN M6 PR1P DTM_DRAW COMPAT */"

HELPER = r'''
/* BEGIN OPENVN M6 PR1P DTM_DRAW COMPAT */
static ULONG openvn_draw_dt_object_a(
    Object *object,
    struct Window *window,
    struct Requester *requester,
    struct RastPort *rastport,
    LONG left,
    LONG top,
    LONG width,
    LONG height,
    LONG top_horiz,
    LONG top_vert,
    struct TagItem *attributes
) {
    struct dtDraw draw_message;

    draw_message.MethodID = DTM_DRAW;
    draw_message.dtd_RPort = rastport;
    draw_message.dtd_Left = left;
    draw_message.dtd_Top = top;
    draw_message.dtd_Width = width;
    draw_message.dtd_Height = height;
    draw_message.dtd_TopHoriz = top_horiz;
    draw_message.dtd_TopVert = top_vert;
    draw_message.dtd_AttrList = attributes;

    return DoDTMethodA(
        object,
        window,
        requester,
        (Msg)&draw_message
    );
}
/* END OPENVN M6 PR1P DTM_DRAW COMPAT */
'''.strip()


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    if not SOURCE.is_file():
        fail(f"missing {SOURCE}")

    text = SOURCE.read_text(encoding="utf-8")

    include = "#include <datatypes/datatypesclass.h>"
    if include not in text:
        includes = list(
            re.finditer(r'^#include\s+[<\"].+[>\"]\s*$', text, re.MULTILINE)
        )
        if not includes:
            fail("could not locate include block")
        pos = includes[-1].end()
        text = text[:pos] + "\n" + include + text[pos:]

    text = re.sub(
        re.escape(BEGIN) + r".*?" + re.escape(END) + r"\s*",
        "",
        text,
        flags=re.DOTALL,
    )

    includes = list(
        re.finditer(r'^#include\s+[<\"].+[>\"]\s*$', text, re.MULTILINE)
    )
    if not includes:
        fail("could not locate include block after patching")
    insert_at = includes[-1].end()
    text = text[:insert_at] + "\n\n" + HELPER + "\n" + text[insert_at:]

    text, replaced = re.subn(
        r"\bDrawDTObjectA\s*\(",
        "openvn_draw_dt_object_a(",
        text,
    )
    if replaced not in (0, 2):
        fail(f"expected 0 or 2 DrawDTObjectA calls, found {replaced}")

    calls = len(re.findall(r"\bopenvn_draw_dt_object_a\s*\(", text))
    if calls < 3:
        fail(
            "expected helper definition plus two compatibility calls; "
            f"found {calls}"
        )

    if re.search(r"\bDrawDTObjectA\s*\(", text):
        fail("DrawDTObjectA calls remain")
    if "DoDTMethodA(" not in text or "DTM_DRAW" not in text:
        fail("compatibility implementation is incomplete")

    SOURCE.write_text(text, encoding="utf-8")
    print(f"patched: {SOURCE.relative_to(REPO)}")
    print(f"replaced DrawDTObjectA calls: {replaced}")
    print("binding: DTM_DRAW via DoDTMethodA")


if __name__ == "__main__":
    main()
