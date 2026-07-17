from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"
STORY = RUNTIME / "tests" / "minimal-story.json"


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


def test_story_core_contract_files_exist() -> None:
    assert (RUNTIME / "include/openvn_story.h").is_file()
    assert (RUNTIME / "src/story.c").is_file()
    assert STORY.is_file()


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
        input=(f"LOAD {STORY}\nRUN\nSTEP\nCHOOSE 0\nSTEP\nSTEP\nSTATUS\nQUIT\n"),
        text=True,
        capture_output=True,
        check=True,
    )

    output = process.stdout
    assert "TEXT Hello from OpenVN." in output
    assert "CHOICE 0 Continue" in output
    assert "CHOICE 1 Finish" in output
    assert "SCENE lighthouse_storm" in output
    assert "STATUS ENDED" in output
