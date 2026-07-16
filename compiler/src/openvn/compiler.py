from __future__ import annotations

import json
from pathlib import Path

from .errors import OpenVNError
from .ink_parser import parse_ink_file
from .model.validation import validate_story
from .project import load_project


def compile_project(project_dir: str | Path) -> Path:
    project = load_project(project_dir)
    story = parse_ink_file(project.entry_path)
    errors = validate_story(story)

    if errors:
        raise OpenVNError("\n".join(errors))

    project.output_path.write_text(
        json.dumps(story.to_dict(), indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    return project.output_path
