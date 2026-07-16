from dataclasses import asdict, dataclass

from .nodes import Node


@dataclass
class Story:
    version: str = "0.2"
    nodes: list[Node] = None

    def __post_init__(self):
        if self.nodes is None:
            self.nodes = []

    def to_dict(self):
        return {
            "format": "openvn-story",
            "version": self.version,
            "nodes": [asdict(n) for n in self.nodes],
        }
