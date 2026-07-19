from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def _compiler() -> str:
    for candidate in (
        os.environ.get("CC"),
        shutil.which("cc"),
        shutil.which("gcc"),
        shutil.which("clang"),
    ):
        if not candidate:
            continue
        probe = subprocess.run(
            [candidate, "-x", "c", "-", "-o", os.devnull],
            input="int main(void) { return 0; }\n",
            text=True,
            capture_output=True,
        )
        if probe.returncode == 0:
            return candidate
    pytest.skip("no working native C compiler is available")


def _build(tmp_path: Path) -> Path:
    build = tmp_path / "build"
    subprocess.run(
        [
            "cmake",
            "-S",
            str(RUNTIME),
            "-B",
            str(build),
            f"-DCMAKE_C_COMPILER={_compiler()}",
        ],
        check=True,
    )
    subprocess.run(["cmake", "--build", str(build)], check=True)
    return build


def test_generated_story_runtime_files_exist() -> None:
    assert (RUNTIME / "include/openvn_story.h").is_file()
    assert (RUNTIME / "src/story.c").is_file()
    assert (RUNTIME / "tests/story.generated.c").is_file()
    assert (RUNTIME / "tests/story.generated.h").is_file()


def test_story_core_ctest(tmp_path: Path) -> None:
    build = _build(tmp_path)
    subprocess.run(
        ["ctest", "--test-dir", str(build), "--output-on-failure"],
        check=True,
    )


def test_host_story_protocol(tmp_path: Path) -> None:
    build = _build(tmp_path)
    process = subprocess.run(
        [str(build / "openvn-player-host")],
        input=("LOAD static\nRUN\nSTEP\nCHOOSE 0\nSTEP\nSTEP\nSTATUS\nQUIT\n"),
        text=True,
        capture_output=True,
        check=True,
        cwd=REPOSITORY,
    )

    lines = process.stdout.splitlines()
    assert process.returncode == 0
    assert "ERROR FAILED" not in process.stdout
    assert lines[:2] == [
        "GRAPHICS OPEN 320 256 5 NATIVE",
        "AUDIO OPEN 8000 4",
    ]
    assert lines[2:] == [
        "OK LOAD",
        "GRAPHICS TEXT Hello from OpenVN.",
        "GRAPHICS PRESENT 1",
        "OK RUN",
        "STATUS TEXT",
        "TEXT Hello from OpenVN.",
        "GRAPHICS CHOICES 2 SELECTED 0",
        "GRAPHICS PRESENT 2",
        "OK STEP",
        "STATUS CHOICE",
        "CHOICE 0 Continue",
        "CHOICE 1 Finish",
        ("GRAPHICS LOAD ILBM runtimes/amiga-native/tests/fixtures/lighthouse_storm.iff 16 16 1"),
        "GRAPHICS SCENE lighthouse_storm",
        "GRAPHICS PRESENT 3",
        "OK CHOOSE",
        "STATUS SCENE",
        "SCENE lighthouse_storm",
        "OK STEP",
        "STATUS END",
        "OK STEP",
        "STATUS ENDED",
        "OK STATUS",
        "STATUS ENDED",
        "AUDIO CLOSE",
        "GRAPHICS CLOSE",
        "OK QUIT",
    ]
    assert "GRAPHICS SCENE lighthouse_storm" in lines
    assert "STATUS ENDED" in lines
