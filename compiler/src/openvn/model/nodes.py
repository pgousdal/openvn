from __future__ import annotations

from dataclasses import dataclass, field


@dataclass(frozen=True)
class Node:
    id: str
    type: str


@dataclass(frozen=True)
class TextNode(Node):
    text: str
    next: str | None = None


@dataclass(frozen=True)
class ChoiceOption:
    text: str
    target: str


@dataclass(frozen=True)
class ChoiceNode(Node):
    options: list[ChoiceOption] = field(default_factory=list)


@dataclass(frozen=True)
class JumpNode(Node):
    target: str


@dataclass(frozen=True)
class EndNode(Node):
    pass
