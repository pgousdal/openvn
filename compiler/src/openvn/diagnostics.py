from dataclasses import asdict, dataclass


@dataclass(frozen=True)
class Diagnostic:
    severity: str
    code: str
    message: str
    file: str | None = None
    line: int | None = None
    column: int | None = None

    def to_dict(self):
        return asdict(self)
