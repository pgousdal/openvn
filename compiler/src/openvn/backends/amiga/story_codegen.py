from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from ...errors import OpenVNError

_NODE_TYPES = {
    "text": "OPENVN_NODE_TEXT",
    "choice": "OPENVN_NODE_CHOICE",
    "jump": "OPENVN_NODE_JUMP",
    "end": "OPENVN_NODE_END",
    "scene": "OPENVN_NODE_SCENE",
    "show": "OPENVN_NODE_SHOW",
    "hide": "OPENVN_NODE_HIDE",
    "music": "OPENVN_NODE_MUSIC",
    "sound": "OPENVN_NODE_SOUND",
}


def _c_string(value: str | None) -> str:
    if value is None:
        value = ""
    return json.dumps(value, ensure_ascii=True)


def _choice_initializer(option: dict[str, Any]) -> str:
    return f"{{{_c_string(str(option['text']))}, {_c_string(str(option['target']))}}}"


def render_story_header() -> str:
    return """\
#ifndef OPENVN_STORY_GENERATED_H
#define OPENVN_STORY_GENERATED_H

#include "openvn_story.h"

extern const OpenVNGeneratedStory OPENVN_GENERATED_STORY;

#endif
"""


def render_story_source(document: dict[str, Any]) -> str:
    if document.get("format") != "openvn-story":
        raise OpenVNError("unsupported document format for Amiga code generation")
    if document.get("version") not in {"0.3", "0.4"}:
        raise OpenVNError(f"unsupported OpenVN format version: {document.get('version')}")

    nodes = document.get("nodes")
    entry = document.get("entry")
    if not isinstance(nodes, list) or not nodes:
        raise OpenVNError("OpenVN story has no nodes")
    if not isinstance(entry, str) or not entry:
        raise OpenVNError("OpenVN story entry is missing")

    node_lines: list[str] = []
    option_arrays: list[str] = []

    for index, node in enumerate(nodes):
        node_type = node.get("type")
        if node_type not in _NODE_TYPES:
            raise OpenVNError(f"unsupported Amiga node type: {node_type}")

        options_name = "0"
        option_count = 0
        if node_type == "choice":
            options = node.get("options")
            if not isinstance(options, list) or not options:
                raise OpenVNError(f"choice node {node.get('id')} has no options")
            option_count = len(options)
            options_name = f"openvn_options_{index}"
            rendered = ",\n    ".join(_choice_initializer(option) for option in options)
            option_arrays.append(
                f"static const OpenVNGeneratedChoice {options_name}[] = {{\n    {rendered}\n}};\n"
            )

        text = str(node.get("text", ""))
        next_target = node.get("next") or ""
        target = node.get("target") or ""
        argument1 = ""
        argument2 = ""

        if node_type == "scene":
            argument1 = str(node.get("background", ""))
        elif node_type == "show":
            argument1 = str(node.get("character", ""))
            argument2 = str(node.get("pose", ""))
        elif node_type == "hide":
            argument1 = str(node.get("character", ""))
        elif node_type == "music":
            argument1 = str(node.get("track") or "")
        elif node_type == "sound":
            argument1 = str(node.get("sound", ""))

        node_lines.append(
            "    {"
            f"{_c_string(str(node['id']))}, "
            f"{_NODE_TYPES[node_type]}, "
            f"{_c_string(text)}, "
            f"{_c_string(str(next_target))}, "
            f"{_c_string(str(target))}, "
            f"{_c_string(argument1)}, "
            f"{_c_string(argument2)}, "
            f"{options_name}, "
            f"{option_count}U"
            "}"
        )

    options_block = "\n".join(option_arrays)
    nodes_block = ",\n".join(node_lines)

    return (
        '#include "story.generated.h"\n\n'
        + options_block
        + ("\n" if options_block else "")
        + "static const OpenVNGeneratedNode openvn_nodes[] = {\n"
        + nodes_block
        + "\n};\n\n"
        + "const OpenVNGeneratedStory OPENVN_GENERATED_STORY = {\n"
        + f"    {_c_string(str(document['version']))},\n"
        + f"    {_c_string(entry)},\n"
        + "    openvn_nodes,\n"
        + f"    {len(nodes)}U\n"
        + "};\n"
    )


def write_generated_story(
    story_path: str | Path,
    output_dir: str | Path,
) -> tuple[Path, Path]:
    source = Path(story_path)
    if not source.is_file():
        raise OpenVNError(f"missing OpenVN story file: {source}")

    document = json.loads(source.read_text(encoding="utf-8"))
    output = Path(output_dir)
    output.mkdir(parents=True, exist_ok=True)

    header = output / "story.generated.h"
    implementation = output / "story.generated.c"

    header.write_text(render_story_header(), encoding="utf-8")
    implementation.write_text(render_story_source(document), encoding="utf-8")
    return header, implementation
