from __future__ import annotations

from collections import Counter

from openvn.diagnostics import Diagnostic

from .nodes import (
    ChoiceNode,
    ConditionNode,
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
    TextNode,
)
from .story import Story

_LINEAR_NODES = (
    TextNode,
    SceneNode,
    ShowNode,
    HideNode,
    MusicNode,
    SoundNode,
    SetBoolNode,
    SetIntNode,
    SetStringNode,
)


def _next_target(node: object) -> str | None:
    if isinstance(node, _LINEAR_NODES):
        return node.next
    return None


def validate_story(story: Story) -> list[Diagnostic]:
    diagnostics: list[Diagnostic] = []
    node_ids = [node.id for node in story.nodes]
    known_ids = set(node_ids)

    duplicates = sorted(node_id for node_id, count in Counter(node_ids).items() if count > 1)
    diagnostics.extend(
        Diagnostic("error", "OVN005", f"duplicate node id: {node_id}") for node_id in duplicates
    )

    if not story.entry:
        diagnostics.append(Diagnostic("error", "OVN005", "story entry is missing"))
    elif story.entry not in known_ids:
        diagnostics.append(Diagnostic("error", "OVN005", f"unknown story entry: {story.entry}"))

    for name, target in sorted(story.symbols.items()):
        if target not in known_ids:
            diagnostics.append(
                Diagnostic(
                    "error",
                    "OVN005",
                    f"symbol '{name}' has unknown target: {target}",
                )
            )

    for node in story.nodes:
        target = _next_target(node)
        if target is not None and target not in known_ids:
            diagnostics.append(
                Diagnostic(
                    "error",
                    "OVN002",
                    f"node '{node.id}' has unknown next target: {target}",
                )
            )
        elif isinstance(node, JumpNode) and node.target and node.target not in known_ids:
            diagnostics.append(Diagnostic("error", "OVN002", f"unknown jump target: {node.target}"))
        elif isinstance(node, ConditionNode):
            for branch_name, branch_target in (
                ("true", node.true_target),
                ("false", node.false_target),
            ):
                if branch_target and branch_target not in known_ids:
                    diagnostics.append(
                        Diagnostic(
                            "error",
                            "OVN002",
                            f"unknown {branch_name} condition target: {branch_target}",
                        )
                    )
        elif isinstance(node, ChoiceNode):
            if not node.options:
                diagnostics.append(
                    Diagnostic(
                        "error",
                        "OVN005",
                        f"choice node '{node.id}' has no options",
                    )
                )
            for option in node.options:
                if option.target not in known_ids:
                    diagnostics.append(
                        Diagnostic(
                            "error",
                            "OVN002",
                            f"unknown choice target: {option.target}",
                        )
                    )

    return diagnostics


def reachable_node_ids(story: Story) -> set[str]:
    nodes = {node.id: node for node in story.nodes}
    if story.entry not in nodes:
        return set()

    reachable: set[str] = set()
    pending = [story.entry]

    while pending:
        node_id = pending.pop()
        if node_id in reachable or node_id not in nodes:
            continue
        reachable.add(node_id)
        node = nodes[node_id]

        target = _next_target(node)
        if target is not None:
            pending.append(target)
        elif isinstance(node, JumpNode):
            if node.target:
                pending.append(node.target)
        elif isinstance(node, ConditionNode):
            pending.extend(target for target in (node.true_target, node.false_target) if target)
        elif isinstance(node, ChoiceNode):
            pending.extend(option.target for option in node.options)
        elif isinstance(node, EndNode):
            continue

    return reachable


def unreachable_node_ids(story: Story) -> set[str]:
    return {node.id for node in story.nodes} - reachable_node_ids(story)


def unreachable_diagnostics(story: Story) -> list[Diagnostic]:
    return [
        Diagnostic("warning", "OVN004", f"unreachable node: {node_id}")
        for node_id in sorted(unreachable_node_ids(story))
    ]
