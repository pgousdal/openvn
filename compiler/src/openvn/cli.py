from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .compiler import compile_project
from .errors import OpenVNError
from .project import load_project
from .validator import validate_project
from .version import __version__


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="openvn")
    parser.add_argument("--version", action="version", version=__version__)

    subparsers = parser.add_subparsers(dest="command", required=True)

    for command in ("validate", "compile", "dump"):
        sub = subparsers.add_parser(command)
        sub.add_argument("project", type=Path)

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)

    try:
        if args.command == "validate":
            result = validate_project(args.project)
            if result.ok:
                print("OpenVN project is valid.")
                return 0
            for error in result.errors:
                print(f"error: {error}", file=sys.stderr)
            return 1

        if args.command == "compile":
            output = compile_project(args.project)
            print(output)
            return 0

        if args.command == "dump":
            project = load_project(args.project)
            print(
                json.dumps(
                    {
                        "name": project.name,
                        "entry": project.entry,
                        "format_version": project.format_version,
                        "root": str(project.root),
                    },
                    indent=2,
                )
            )
            return 0

    except OpenVNError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    return 2


if __name__ == "__main__":
    raise SystemExit(main())
