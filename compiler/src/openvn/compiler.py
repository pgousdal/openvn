from __future__ import annotations

import json
from pathlib import Path

from .errors import OpenVNError
from .ink import load_ink, validate_minimal_ink
from .project import load_project
from .story import build_story


def compile_project(project_dir: str | Path) -> Path:
    project = load_project(project_dir)
    source = load_ink(project.entry_path)
    errors = validate_minimal_ink(source)

    if errors:
        raise OpenVNError("\n".join(errors))

    story = build_story(project, source)
    project.output_path.write_text(
        json.dumps(story.to_dict(), indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    return project.output_path
