from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from .version import __version__


@dataclass(frozen=True)
class DoctorCheck:
    name: str
    ok: bool
    detail: str = ""

    def format_text(self) -> str:
        status = "OK" if self.ok else "MISSING"
        suffix = f" ({self.detail})" if self.detail else ""
        return f"{self.name:<22}{status}{suffix}"


def run_doctor(repository_root: str | Path) -> list[DoctorCheck]:
    root = Path(repository_root).resolve()
    schema = root / "schemas" / "openvn-story-0.3.schema.json"
    compiler = root / "compiler" / "src" / "openvn"
    examples = root / "examples" / "minimal"
    renpy_backend = root / "backends" / "renpy"
    amiga_runtime = root / "runtimes" / "amiga-native"

    return [
        DoctorCheck("OpenVN version", True, __version__),
        DoctorCheck("Compiler package", compiler.is_dir(), str(compiler)),
        DoctorCheck("Story schema", schema.is_file(), str(schema)),
        DoctorCheck("Minimal example", examples.is_dir(), str(examples)),
        DoctorCheck("Ren'Py backend", renpy_backend.is_dir(), str(renpy_backend)),
        DoctorCheck("Amiga native runtime", amiga_runtime.is_dir(), str(amiga_runtime)),
    ]
