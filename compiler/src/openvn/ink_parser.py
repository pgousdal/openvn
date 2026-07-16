from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .errors import SourceError
from .model import ChoiceNode, ChoiceOption, EndNode, JumpNode, Story, TextNode


@dataclass(frozen=True)
class ParsedLine:
    number: int
    text: str


def _node_id(prefix: str, index: int) -> str:
    return f"{prefix}-{index:04d}"


def parse_ink_text(text: str) -> Story:
    """Parse the supported OpenVN Ink subset into a semantic Story."""
    lines = [ParsedLine(i, line.rstrip()) for i, line in enumerate(text.splitlines(), start=1)]
    nodes = []
    current_knot = "start"
    index = 1
    i = 0

    while i < len(lines):
        line = lines[i]
        stripped = line.text.strip()

        if not stripped or stripped.startswith("//"):
            i += 1
            continue

        if stripped.startswith("===") and stripped.endswith("==="):
            name = stripped.removeprefix("===").removesuffix("===").strip()
            if not name:
                raise SourceError(f"line {line.number}: empty knot name")
            current_knot = name
            i += 1
            continue

        if stripped == "-> END":
            nodes.append(EndNode(id=_node_id(current_knot, index), type="end"))
            index += 1
            i += 1
            continue

        if stripped.startswith("->"):
            target = stripped.removeprefix("->").strip()
            if not target:
                raise SourceError(f"line {line.number}: missing jump target")
            nodes.append(
                JumpNode(
                    id=_node_id(current_knot, index),
                    type="jump",
                    target=target,
                )
            )
            index += 1
            i += 1
            continue

        if stripped.startswith("*"):
            options: list[ChoiceOption] = []
            while i < len(lines):
                option_line = lines[i]
                option_text = option_line.text.strip()
                if not option_text.startswith("*"):
                    break

                label = option_text.removeprefix("*").strip()
                if label.startswith("[") and "]" in label:
                    close = label.index("]")
                    visible = label[1:close].strip()
                    remainder = label[close + 1 :].strip()
                else:
                    visible = label
                    remainder = ""

                target = remainder.removeprefix("->").strip() if remainder.startswith("->") else ""
                if not visible:
                    raise SourceError(f"line {option_line.number}: empty choice text")
                if not target:
                    raise SourceError(
                        f"line {option_line.number}: choice must use '* [Text] -> target'"
                    )

                options.append(ChoiceOption(text=visible, target=target))
                i += 1

            nodes.append(
                ChoiceNode(
                    id=_node_id(current_knot, index),
                    type="choice",
                    options=options,
                )
            )
            index += 1
            continue

        nodes.append(
            TextNode(
                id=_node_id(current_knot, index),
                type="text",
                text=stripped,
            )
        )
        index += 1
        i += 1

    if not nodes:
        raise SourceError("Ink source produced no story nodes")

    return Story(version="0.2", nodes=nodes)


def parse_ink_file(path: str | Path) -> Story:
    source = Path(path)
    if not source.is_file():
        raise SourceError(f"missing Ink source file: {source}")
    return parse_ink_text(source.read_text(encoding="utf-8"))
