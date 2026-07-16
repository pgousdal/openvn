from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .errors import SourceError


@dataclass(frozen=True)
class InkSource:
    path: Path
    text: str

    @property
    def lines(self) -> list[str]:
        return self.text.splitlines()


def load_ink(path: Path) -> InkSource:
    if not path.is_file():
        raise SourceError(f"missing Ink entry file: {path}")

    text = path.read_text(encoding="utf-8")
    if not text.strip():
        raise SourceError(f"Ink entry file is empty: {path}")

    return InkSource(path=path, text=text)


def validate_minimal_ink(source: InkSource) -> list[str]:
    errors: list[str] = []

    if "-> END" not in source.text:
        errors.append("Ink source must contain an explicit '-> END' terminator")

    return errors
