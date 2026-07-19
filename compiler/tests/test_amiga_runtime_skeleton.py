from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

import pytest

REPOSITORY = Path(__file__).parents[2]
RUNTIME = REPOSITORY / "runtimes" / "amiga-native"


def _working_c_compiler() -> str | None:
    candidates = [
        os.environ.get("CC"),
        shutil.which("cc"),
        shutil.which("gcc"),
        shutil.which("clang"),
    ]

    for candidate in candidates:
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

    return None


def _configure(build: Path) -> None:
    compiler = _working_c_compiler()
    if compiler is None:
        pytest.skip("no working native C compiler is available")

    subprocess.run(
        [
            "cmake",
            "-S",
            str(RUNTIME),
            "-B",
            str(build),
            f"-DCMAKE_C_COMPILER={compiler}",
        ],
        check=True,
    )


def test_runtime_contract_files_exist() -> None:
    expected = [
        RUNTIME / "include/openvn_dispatch.h",
        RUNTIME / "src/dispatch.c",
        RUNTIME / "src/player_state.c",
        RUNTIME / "src/main_amiga.c",
        RUNTIME / "src/main_host.c",
        RUNTIME / "tests/test_dispatch.c",
        RUNTIME / "CMakeLists.txt",
    ]
    assert all(path.is_file() for path in expected)


def test_host_runtime_build_and_contract(tmp_path: Path) -> None:
    build = tmp_path / "build"

    _configure(build)
    subprocess.run(["cmake", "--build", str(build)], check=True)
    subprocess.run(
        ["ctest", "--test-dir", str(build), "--output-on-failure"],
        check=True,
    )


def test_host_player_protocol(tmp_path: Path) -> None:
    build = tmp_path / "build"

    _configure(build)
    subprocess.run(["cmake", "--build", str(build)], check=True)

    process = subprocess.run(
        [str(build / "openvn-player-host")],
        input=(
            "LOAD story/story.openvn.json\n"
            "SCENE lighthouse_storm\n"
            "SHOW erik neutral\n"
            "MUSIC storm\n"
            "SOUND thunder\n"
            "RUN\n"
            "STEP\n"
            "QUIT\n"
        ),
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
        ("GRAPHICS LOAD ILBM runtimes/amiga-native/tests/fixtures/lighthouse_storm.iff 16 16 1"),
        "GRAPHICS SCENE lighthouse_storm",
        "GRAPHICS PRESENT 1",
        "OK SCENE",
        ("GRAPHICS LOAD ILBM runtimes/amiga-native/tests/fixtures/erik_neutral.iff 8 16 1"),
        "GRAPHICS SHOW erik neutral",
        "GRAPHICS PRESENT 2",
        "OK SHOW",
        ("AUDIO MUSIC storm runtimes/amiga-native/tests/fixtures/storm.mod 1"),
        "OK MUSIC",
        ("AUDIO SOUND thunder runtimes/amiga-native/tests/fixtures/thunder.8svx 8000 32"),
        "OK SOUND",
        "GRAPHICS TEXT Hello from OpenVN.",
        "GRAPHICS PRESENT 3",
        "OK RUN",
        "STATUS TEXT",
        "TEXT Hello from OpenVN.",
        "GRAPHICS CHOICES 2 SELECTED 0",
        "GRAPHICS PRESENT 4",
        "OK STEP",
        "STATUS CHOICE",
        "CHOICE 0 Continue",
        "CHOICE 1 Finish",
        "AUDIO CLOSE",
        "GRAPHICS CLOSE",
        "OK QUIT",
    ]
    assert "GRAPHICS SCENE lighthouse_storm" in lines
