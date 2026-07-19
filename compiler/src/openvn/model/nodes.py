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


@dataclass(frozen=True)
class SceneNode(Node):
    background: str
    next: str | None = None


@dataclass(frozen=True)
class ShowNode(Node):
    character: str
    pose: str
    next: str | None = None


@dataclass(frozen=True)
class HideNode(Node):
    character: str
    next: str | None = None


@dataclass(frozen=True)
class MusicNode(Node):
    track: str | None
    next: str | None = None


@dataclass(frozen=True)
class SoundNode(Node):
    sound: str
    next: str | None = None


@dataclass(frozen=True)
class SetBoolNode(Node):
    name: str
    value: bool
    next: str | None = None


@dataclass(frozen=True)
class SetIntNode(Node):
    name: str
    value: int
    next: str | None = None


@dataclass(frozen=True)
class SetStringNode(Node):
    name: str
    value: str
    next: str | None = None
