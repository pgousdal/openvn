from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml

from ...errors import OpenVNError


@dataclass(frozen=True)
class AmigaProfile:
    id: str
    display_name: str
    graphics: dict[str, Any]
    audio: dict[str, Any]
    runtime: dict[str, Any]


def load_amiga_profile(path: str | Path) -> AmigaProfile:
    profile_path = Path(path)
    if not profile_path.is_file():
        raise OpenVNError(f"missing Amiga profile: {profile_path}")

    raw = yaml.safe_load(profile_path.read_text(encoding="utf-8"))
    if not isinstance(raw, dict):
        raise OpenVNError("Amiga profile must contain a mapping")

    for key in ("id", "display_name", "graphics", "audio", "runtime"):
        if key not in raw:
            raise OpenVNError(f"Amiga profile is missing field: {key}")

    runtime = raw["runtime"]
    if not isinstance(runtime, dict):
        raise OpenVNError("Amiga profile runtime must be a mapping")
    if runtime.get("ace") != "forbidden":
        raise OpenVNError("Amiga profiles must forbid ACE")
    if runtime.get("system_friendly") is not True:
        raise OpenVNError("Amiga native C runtime must be system-friendly")

    return AmigaProfile(
        id=str(raw["id"]),
        display_name=str(raw["display_name"]),
        graphics=dict(raw["graphics"]),
        audio=dict(raw["audio"]),
        runtime=dict(runtime),
    )
