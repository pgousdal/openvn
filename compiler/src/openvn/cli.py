from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .compiler import compile_project
from .doctor import run_doctor
from .errors import OpenVNError
from .project import load_project
from .validator import validate_project
from .version import __version__


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="openvn")
    parser.add_argument("--version", action="version", version=__version__)

    subparsers = parser.add_subparsers(dest="command", required=True)

    validate = subparsers.add_parser("validate")
    validate.add_argument("project", type=Path)
    validate.add_argument("--json", action="store_true", dest="json_output")

    compile_command = subparsers.add_parser("compile")
    compile_command.add_argument("project", type=Path)
    compile_command.add_argument("--strict", action="store_true")

    dump = subparsers.add_parser("dump")
    dump.add_argument("project", type=Path)

    doctor = subparsers.add_parser("doctor")
    doctor.add_argument(
        "repository",
        type=Path,
        nargs="?",
        default=Path.cwd(),
    )
    doctor.add_argument("--json", action="store_true", dest="json_output")

    return parser


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)

    try:
        if args.command == "validate":
            result = validate_project(args.project)

            if args.json_output:
                print(
                    json.dumps(
                        {
                            "ok": result.ok,
                            "diagnostics": [
                                diagnostic.to_dict() for diagnostic in result.diagnostics
                            ],
                        },
                        indent=2,
                    )
                )
            else:
                for diagnostic in result.diagnostics:
                    stream = sys.stderr if diagnostic.severity == "error" else sys.stdout
                    print(diagnostic.format_text(), file=stream)
                if result.ok:
                    print("OpenVN project is valid.")

            return 0 if result.ok else 1

        if args.command == "compile":
            output = compile_project(args.project, strict=args.strict)
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

        if args.command == "doctor":
            checks = run_doctor(args.repository)
            if args.json_output:
                print(
                    json.dumps(
                        {
                            "ok": all(check.ok for check in checks[:4]),
                            "checks": [
                                {
                                    "name": check.name,
                                    "ok": check.ok,
                                    "detail": check.detail,
                                }
                                for check in checks
                            ],
                        },
                        indent=2,
                    )
                )
            else:
                print(f"OpenVN {__version__}")
                print()
                for check in checks:
                    print(check.format_text())

            return 0 if all(check.ok for check in checks[:4]) else 1

    except OpenVNError as exc:
        print(exc.user_message(), file=sys.stderr)
        return 1

    return 2


if __name__ == "__main__":
    raise SystemExit(main())
