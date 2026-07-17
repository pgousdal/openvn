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
    struct RastPort *rastport,
    Object *object,
    LONG left,
    LONG top,
    LONG width,
    LONG height,
    LONG top_horiz,
    LONG top_vert,
    ULONG flags,
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
    draw_message.dtd_Flags = flags;
    draw_message.dtd_AttrList = attributes;

    return DoDTMethodA(
        object,
        NULL,
        NULL,
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

    pattern = re.escape(BEGIN) + r".*?" + re.escape(END)
    if not re.search(pattern, text, flags=re.DOTALL):
        fail("PR1P compatibility helper not found")

    text = re.sub(pattern, HELPER, text, count=1, flags=re.DOTALL)

    calls = len(re.findall(r"\bopenvn_draw_dt_object_a\s*\(", text))
    if calls != 3:
        fail(f"expected helper plus two calls, found {calls}")

    SOURCE.write_text(text, encoding="utf-8")
    print(f"patched: {SOURCE.relative_to(REPO)}")
    print("signature: RastPort, Object, geometry, scroll, flags, tags")
    print("dispatch: DoDTMethodA(object, NULL, NULL, dtDraw)")


if __name__ == "__main__":
    main()
