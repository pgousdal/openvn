from __future__ import annotations

from dataclasses import dataclass, replace
from pathlib import Path

from .diagnostics import Diagnostic, diagnostic_file
from .errors import SourceError
from .model import (
    ChoiceNode,
    ChoiceOption,
    EndNode,
    HideNode,
    JumpNode,
    MusicNode,
    SceneNode,
    SetBoolNode,
    SetIntNode,
    SetStringNode,
    ShowNode,
    SoundNode,
    Story,
    TextNode,
)

StoryNode = (
    TextNode
    | ChoiceNode
    | JumpNode
    | EndNode
    | SceneNode
    | ShowNode
    | HideNode
    | MusicNode
    | SoundNode
    | SetBoolNode
    | SetIntNode
    | SetStringNode
)


@dataclass(frozen=True)
class ParsedLine:
    number: int
    text: str


@dataclass
class Section:
    name: str
    nodes: list[StoryNode]


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


def _choice(line: ParsedLine, *, source_path: str | Path | None) -> tuple[str, str]:
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
        raise _source_error("empty choice text", source_path=source_path, line=line.number)
    if not remainder.startswith("->") or not remainder.removeprefix("->").strip():
        raise _source_error(
            "choice must use '* [Text] -> target'",
            source_path=source_path,
            line=line.number,
        )

    return text, remainder.removeprefix("->").strip()


def _presentation_node(
    command: str,
    *,
    section: str,
    index: int,
    source_path: str | Path | None,
    line: int,
) -> StoryNode:
    parts = command.split()
    if not parts:
        raise _source_error("empty OpenVN command", source_path=source_path, line=line)

    name = parts[0]
    node_id = _node_id(section, index)

    if name == "scene" and len(parts) == 2:
        return SceneNode(id=node_id, type="scene", background=parts[1])
    if name == "show" and len(parts) == 3:
        return ShowNode(
            id=node_id,
            type="show",
            character=parts[1],
            pose=parts[2],
        )
    if name == "hide" and len(parts) == 2:
        return HideNode(id=node_id, type="hide", character=parts[1])
    if name == "music" and len(parts) == 2:
        track = None if parts[1] == "stop" else parts[1]
        return MusicNode(id=node_id, type="music", track=track)
    if name == "sound" and len(parts) == 2:
        return SoundNode(id=node_id, type="sound", sound=parts[1])
    if name == "set_bool" and len(parts) == 3 and parts[2] in {"true", "false"}:
        return SetBoolNode(
            id=node_id,
            type="set_bool",
            name=parts[1],
            value=parts[2] == "true",
        )
    if name == "set_int" and len(parts) == 3:
        try:
            value = int(parts[2], 10)
        except ValueError:
            pass
        else:
            if -(2**31) <= value < 2**31:
                return SetIntNode(
                    id=node_id,
                    type="set_int",
                    name=parts[1],
                    value=value,
                )
    if name == "set_string" and len(parts) >= 3:
        return SetStringNode(
            id=node_id,
            type="set_string",
            name=parts[1],
            value=command.split(maxsplit=2)[2],
        )

    raise _source_error(
        f"invalid OpenVN command: {command}",
        source_path=source_path,
        line=line,
    )


def _resolve_target(reference: str, symbols: dict[str, str], node_ids: set[str]) -> str:
    if reference in symbols:
        return symbols[reference]
    if reference in node_ids:
        return reference
    return reference


def _link_linear_nodes(sections: list[Section]) -> list[StoryNode]:
    linked: list[StoryNode] = []

    for section in sections:
        for index, node in enumerate(section.nodes):
            next_id = section.nodes[index + 1].id if index + 1 < len(section.nodes) else None
            if isinstance(
                node,
                (
                    TextNode,
                    SceneNode,
                    ShowNode,
                    HideNode,
                    MusicNode,
                    SoundNode,
                    SetBoolNode,
                    SetIntNode,
                    SetStringNode,
                ),
            ):
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
    uses_presentation = False

    while i < len(lines):
        line = lines[i]
        stripped = line.text.strip()

        if not stripped or stripped.startswith("//"):
            i += 1
            continue

        if stripped.startswith("===") and stripped.endswith("==="):
            name = stripped.removeprefix("===").removesuffix("===").strip()
            if not name:
                raise _source_error("empty knot name", source_path=source_path, line=line.number)

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

        if stripped.startswith("#openvn "):
            uses_presentation = True
            current.nodes.append(
                _presentation_node(
                    stripped.removeprefix("#openvn ").strip(),
                    section=current.name,
                    index=node_index,
                    source_path=source_path,
                    line=line.number,
                )
            )
            node_index += 1
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
        raise _source_error("Ink source produced no story nodes", source_path=source_path)

    symbols = {section.name: section.nodes[0].id for section in non_empty_sections}
    nodes = _link_linear_nodes(non_empty_sections)
    node_ids = {node.id for node in nodes}
    resolved_nodes: list[StoryNode] = []

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
        version="0.4" if uses_presentation else "0.3",
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
