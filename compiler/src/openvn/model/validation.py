from __future__ import annotations

from collections import Counter

from .nodes import ChoiceNode, EndNode, JumpNode, TextNode
from .story import Story


def validate_story(story: Story) -> list[str]:
    errors: list[str] = []
    node_ids = [node.id for node in story.nodes]
    known_ids = set(node_ids)

    duplicates = sorted(node_id for node_id, count in Counter(node_ids).items() if count > 1)
    errors.extend(f"duplicate node id: {node_id}" for node_id in duplicates)

    if not story.entry:
        errors.append("story entry is missing")
    elif story.entry not in known_ids:
        errors.append(f"unknown story entry: {story.entry}")

    for name, target in sorted(story.symbols.items()):
        if target not in known_ids:
            errors.append(f"symbol '{name}' has unknown target: {target}")

    for node in story.nodes:
        if isinstance(node, TextNode) and node.next is not None and node.next not in known_ids:
            errors.append(f"node '{node.id}' has unknown next target: {node.next}")
        elif isinstance(node, JumpNode) and node.target not in known_ids:
            errors.append(f"unknown jump target: {node.target}")
        elif isinstance(node, ChoiceNode):
            if not node.options:
                errors.append(f"choice node '{node.id}' has no options")
            for option in node.options:
                if option.target not in known_ids:
                    errors.append(f"unknown choice target: {option.target}")

    return errors


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

        if isinstance(node, TextNode) and node.next is not None:
            pending.append(node.next)
        elif isinstance(node, JumpNode):
            pending.append(node.target)
        elif isinstance(node, ChoiceNode):
            pending.extend(option.target for option in node.options)
        elif isinstance(node, EndNode):
            continue

    return reachable


def unreachable_node_ids(story: Story) -> set[str]:
    return {node.id for node in story.nodes} - reachable_node_ids(story)
