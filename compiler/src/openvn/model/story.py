from __future__ import annotations

from dataclasses import asdict, dataclass, field

from .nodes import Node


@dataclass
class Story:
    version: str = "0.3"
    entry: str = ""
    symbols: dict[str, str] = field(default_factory=dict)
    nodes: list[Node] = field(default_factory=list)

    def to_dict(self) -> dict[str, object]:
        return {
            "format": "openvn-story",
            "version": self.version,
            "entry": self.entry,
            "symbols": dict(sorted(self.symbols.items())),
            "nodes": [asdict(node) for node in self.nodes],
        }
