from __future__ import annotations

from dataclasses import asdict, dataclass
from pathlib import Path


@dataclass(frozen=True)
class Diagnostic:
    severity: str
    code: str
    message: str
    file: str | None = None
    line: int | None = None
    column: int | None = None

    def to_dict(self) -> dict[str, object]:
        return asdict(self)

    def format_text(self) -> str:
        location = ""
        if self.file:
            location = self.file
            if self.line is not None:
                location += f":{self.line}"
                if self.column is not None:
                    location += f":{self.column}"
            location += ": "

        return f"{location}{self.severity} {self.code}: {self.message}"


def diagnostic_file(path: str | Path | None) -> str | None:
    if path is None:
        return None
    return str(Path(path))
