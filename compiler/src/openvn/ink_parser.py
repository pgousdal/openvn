from __future__ import annotations

from dataclasses import dataclass, replace
from pathlib import Path

from .diagnostics import Diagnostic, diagnostic_file
from .errors import SourceError
from .model import ChoiceNode, ChoiceOption, EndNode, JumpNode, Story, TextNode


@dataclass(frozen=True)
class ParsedLine:
    number: int
    text: str


@dataclass
class Section:
    name: str
    nodes: list[TextNode | ChoiceNode | JumpNode | EndNode]


def _source_error(
    message: str,
    *,
    source_path: str | Path | None,
    line: int | None = None,
    column: int | None = 1,
    code: str = "OVN001",
) -> SourceError:
    diagnostic = Diagnostic(
        severity="error",
        code=code,
        message=message,
        file=diagnostic_file(source_path),
        line=line,
        column=column if line is not None else None,
    )
    return SourceError(message, diagnostic=diagnostic)


def _node_id(section: str, index: int) -> str:
    return f"{section}-{index:04d}"


def _choice(
    line: ParsedLine,
    *,
    source_path: str | Path | None,
) -> tuple[str, str]:
    value = line.text.strip().removeprefix("*").strip()
    if not value.startswith("[") or "]" not in value:
        raise _source_error(
            "choice must use '* [Text] -> target'",
            source_path=source_path,
            line=line.number,
        )

    close = value.index("]")
    text = value[1:close].strip()
    remainder = value[close + 1 :].strip()

    if not text:
        raise _source_error(
            "empty choice text",
            source_path=source_path,
            line=line.number,
        )
    if not remainder.startswith("->") or not remainder.removeprefix("->").strip():
        raise _source_error(
            "choice must use '* [Text] -> target'",
            source_path=source_path,
            line=line.number,
        )

    return text, remainder.removeprefix("->").strip()


def _resolve_target(reference: str, symbols: dict[str, str], node_ids: set[str]) -> str:
    if reference in symbols:
        return symbols[reference]
    if reference in node_ids:
        return reference
    return reference


def _link_text_nodes(
    sections: list[Section],
) -> list[TextNode | ChoiceNode | JumpNode | EndNode]:
    linked: list[TextNode | ChoiceNode | JumpNode | EndNode] = []

    for section in sections:
        for index, node in enumerate(section.nodes):
            if isinstance(node, TextNode):
                next_id = section.nodes[index + 1].id if index + 1 < len(section.nodes) else None
                linked.append(replace(node, next=next_id))
            else:
                linked.append(node)

    return linked


def parse_ink_text(
    text: str,
    *,
    source_path: str | Path | None = None,
) -> Story:
    lines = [ParsedLine(i, line.rstrip()) for i, line in enumerate(text.splitlines(), start=1)]
    sections = [Section(name="start", nodes=[])]
    current = sections[0]
    node_index = 1
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
                raise _source_error(
                    "empty knot name",
                    source_path=source_path,
                    line=line.number,
                )

            duplicate = any(
                section.name == name for section in sections if section.nodes or name != "start"
            )
            if duplicate:
                raise _source_error(
                    f"duplicate knot: {name}",
                    source_path=source_path,
                    line=line.number,
                    code="OVN003",
                )

            if current.name == "start" and not current.nodes and name == "start":
                current.name = name
            else:
                current = Section(name=name, nodes=[])
                sections.append(current)
            node_index = 1
            i += 1
            continue

        if stripped == "-> END":
            current.nodes.append(EndNode(id=_node_id(current.name, node_index), type="end"))
            node_index += 1
            i += 1
            continue

        if stripped.startswith("->"):
            target = stripped.removeprefix("->").strip()
            if not target:
                raise _source_error(
                    "missing jump target",
                    source_path=source_path,
                    line=line.number,
                )
            current.nodes.append(
                JumpNode(id=_node_id(current.name, node_index), type="jump", target=target)
            )
            node_index += 1
            i += 1
            continue

        if stripped.startswith("*"):
            options: list[ChoiceOption] = []
            while i < len(lines) and lines[i].text.strip().startswith("*"):
                choice_text, target = _choice(lines[i], source_path=source_path)
                options.append(ChoiceOption(text=choice_text, target=target))
                i += 1

            current.nodes.append(
                ChoiceNode(
                    id=_node_id(current.name, node_index),
                    type="choice",
                    options=options,
                )
            )
            node_index += 1
            continue

        current.nodes.append(
            TextNode(
                id=_node_id(current.name, node_index),
                type="text",
                text=stripped,
            )
        )
        node_index += 1
        i += 1

    non_empty_sections = [section for section in sections if section.nodes]
    if not non_empty_sections:
        raise _source_error(
            "Ink source produced no story nodes",
            source_path=source_path,
        )

    symbols = {section.name: section.nodes[0].id for section in non_empty_sections}
    if len(symbols) != len(non_empty_sections):
        raise _source_error(
            "duplicate knot name",
            source_path=source_path,
            code="OVN003",
        )

    nodes = _link_text_nodes(non_empty_sections)
    node_ids = {node.id for node in nodes}
    resolved_nodes: list[TextNode | ChoiceNode | JumpNode | EndNode] = []

    for node in nodes:
        if isinstance(node, JumpNode):
            resolved_nodes.append(
                replace(node, target=_resolve_target(node.target, symbols, node_ids))
            )
        elif isinstance(node, ChoiceNode):
            resolved_nodes.append(
                replace(
                    node,
                    options=[
                        replace(
                            option,
                            target=_resolve_target(option.target, symbols, node_ids),
                        )
                        for option in node.options
                    ],
                )
            )
        else:
            resolved_nodes.append(node)

    first_section = non_empty_sections[0]
    return Story(
        version="0.3",
        entry=first_section.nodes[0].id,
        symbols=symbols,
        nodes=resolved_nodes,
    )


def parse_ink_file(path: str | Path) -> Story:
    source = Path(path)
    if not source.is_file():
        raise _source_error(
            f"missing Ink source file: {source}",
            source_path=source,
        )
    return parse_ink_text(
        source.read_text(encoding="utf-8"),
        source_path=source,
    )
