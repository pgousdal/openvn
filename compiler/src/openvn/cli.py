from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

from .backends.amiga import export_amiga_package, load_amiga_profile
from .backends.renpy import export_renpy_project
from .compiler import compile_project
from .doctor import run_doctor
from .errors import OpenVNError
from .fsuae import package_fsuae
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

    build = subparsers.add_parser("build")
    build.add_argument("project", type=Path)
    build.add_argument("--output", type=Path)
    build.add_argument("--clean", action="store_true")

    export = subparsers.add_parser("export")
    export.add_argument("project", type=Path)
    export.add_argument("--backend", choices=["renpy", "amiga"], required=True)
    export.add_argument("--profile")
    export.add_argument("--output", type=Path, required=True)
    export.add_argument("--clean", action="store_true")

    fsuae = subparsers.add_parser("package-fsuae")
    fsuae.add_argument("project", type=Path)
    fsuae.add_argument("--output", type=Path)
    fsuae.add_argument("--player", type=Path)
    fsuae.add_argument("--toolchain-file", type=Path)
    fsuae.add_argument("--clean", action="store_true")

    dump = subparsers.add_parser("dump")
    dump.add_argument("project", type=Path)

    doctor = subparsers.add_parser("doctor")
    doctor.add_argument("repository", type=Path, nargs="?", default=Path.cwd())
    doctor.add_argument("--json", action="store_true", dest="json_output")

    return parser


def _repository_root(project: Path) -> Path:
    resolved = project.resolve()
    for parent in (resolved, *resolved.parents):
        if (parent / "profiles").is_dir() and (parent / "compiler").is_dir():
            return parent
    raise OpenVNError("could not locate OpenVN repository root")


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

        if args.command == "build":
            repository = _repository_root(args.project)
            project = load_project(args.project)
            output = (args.output or (project.root / "dist")).resolve()
            if args.clean and output.exists():
                import shutil

                shutil.rmtree(output)

            story_path = compile_project(project.root, strict=True)
            output.mkdir(parents=True, exist_ok=True)
            export_renpy_project(story_path, output / "renpy", clean=True)
            profile = load_amiga_profile(repository / "profiles" / "amiga-ocs.yaml")
            export_amiga_package(
                project_root=project.root,
                story_path=story_path,
                output_dir=output / "amiga-ocs",
                profile=profile,
                clean=True,
            )
            summary = {
                "format": "openvn-build",
                "version": "0.1",
                "project": project.name,
                "story": str(story_path.relative_to(project.root)),
                "targets": {
                    "renpy": "renpy",
                    "amiga-ocs": "amiga-ocs",
                },
            }
            (output / "build.json").write_text(
                json.dumps(summary, indent=2) + "\n", encoding="utf-8"
            )
            print(output)
            return 0

        if args.command == "package-fsuae":
            repository = _repository_root(args.project)
            project = load_project(args.project)
            build_output = project.root / "dist"
            amiga_output = build_output / "amiga-ocs"
            if not (amiga_output / "manifest.json").is_file():
                story_path = compile_project(project.root, strict=True)
                profile = load_amiga_profile(repository / "profiles" / "amiga-ocs.yaml")
                export_amiga_package(
                    project_root=project.root,
                    story_path=story_path,
                    output_dir=amiga_output,
                    profile=profile,
                    clean=True,
                )
            output = (args.output or (build_output / "fs-uae")).resolve()
            packaged = package_fsuae(
                repository=repository,
                amiga_package=amiga_output,
                output_dir=output,
                player=args.player,
                toolchain_file=args.toolchain_file,
                clean=args.clean,
            )
            print(packaged)
            return 0

        if args.command == "export":
            story_path = compile_project(args.project, strict=True)
            if args.backend == "renpy":
                output = export_renpy_project(
                    story_path,
                    args.output,
                    clean=args.clean,
                )
                print(output)
                return 0

            if args.backend == "amiga":
                if not args.profile:
                    raise OpenVNError("--profile is required for Amiga export")
                repository = _repository_root(args.project)
                profile_path = repository / "profiles" / f"{args.profile}.yaml"
                profile = load_amiga_profile(profile_path)
                output = export_amiga_package(
                    project_root=args.project,
                    story_path=story_path,
                    output_dir=args.output,
                    profile=profile,
                    clean=args.clean,
                )
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
