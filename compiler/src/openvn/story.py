from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import UTC, datetime
from pathlib import Path

from .ink import InkSource
from .project import Project


@dataclass(frozen=True)
class SourceInfo:
    type: str
    entry: str


@dataclass(frozen=True)
class StoryDocument:
    format: str
    format_version: str
    project_name: str
    generated_at: str
    source: SourceInfo
    source_text: str

    def to_dict(self) -> dict[str, object]:
        return asdict(self)


def build_story(project: Project, source: InkSource) -> StoryDocument:
    return StoryDocument(
        format="openvn-story",
        format_version=project.format_version,
        project_name=project.name,
        generated_at=datetime.now(UTC).replace(microsecond=0).isoformat(),
        source=SourceInfo(type="ink", entry=str(Path(project.entry))),
        source_text=source.text,
    )
